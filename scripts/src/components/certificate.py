import collections
import http.client
import ipaddress
import itertools
import logging
import pathlib
import socket
import ssl
import typing
import urllib.parse
import urllib.request

import certifi
import cryptography.exceptions
import cryptography.hazmat.primitives.serialization
import cryptography.x509
import cryptography.x509.oid

if typing.TYPE_CHECKING:
    from ..Kortlinje import Kortlinje
    from .Types import COMMAND_LINE_TARGETS
else:
    from SCons.Script import COMMAND_LINE_TARGETS


class _SafeRedirectHandler(urllib.request.HTTPRedirectHandler):
    def redirect_request(
        self,
        req: urllib.request.Request,
        fp: typing.Any,
        code: int,
        msg: str,
        headers: typing.Any,
        newurl: str,
    ) -> urllib.request.Request | None:
        """Reject redirects whose destination fails the certificate URL safety check.

        Otherwise, use the standard redirect handling, including its HTTPError
        for unsupported redirects; return None when the destination is rejected."""
        if not Certificate._is_safe_url(newurl):
            logging.getLogger(__name__).debug("Blocked unsafe AIA redirect to: %s", newurl)
            return None
        return super().redirect_request(req, fp, code, msg, headers, newurl)


class Certificate:
    path: typing.Final[pathlib.Path] = pathlib.Path("x509") / "cacert.pem"
    project: "Kortlinje"
    timeout: typing.Final[float] = 10
    _has_failures: bool
    _hosts: set[str]
    _root_store: dict[cryptography.x509.Name, list[cryptography.x509.Certificate]]
    _selected_roots: set[cryptography.x509.Certificate]
    _ssl_contexts: tuple[ssl.SSLContext, ...]
    _url_opener: urllib.request.OpenerDirector
    _visited_certificates: dict[cryptography.x509.Certificate, list[cryptography.x509.Certificate]]

    def __init__(self, project: "Kortlinje") -> None:
        """Associate the build project with an empty certificate discovery state."""
        self.project = project
        self._has_failures = False
        self._hosts = set()
        self._root_store = collections.defaultdict(list)
        self._selected_roots = set()
        self._ssl_contexts = ()
        self._url_opener = urllib.request.build_opener(_SafeRedirectHandler())
        self._visited_certificates = {}

    def initialize(self) -> None:
        """Load certifi roots and prepare ECDSA and RSA TLS probes.

        Ensure the bundle file exists and disable the project certificate
        component when the sole target is ``compiledb``. Bundle read/parse
        errors, filesystem errors, and SSL context errors propagate."""
        if COMMAND_LINE_TARGETS in [
            ["compiledb"],
        ]:
            if not self.path.is_file():
                self.finalize()
            self.project.certificate = None
            return
        for cert in cryptography.x509.load_pem_x509_certificates(certifi.contents().encode()):
            self._root_store[cert.subject].append(cert)
        self._ssl_contexts = tuple(self._create_ssl_context(cipher) for cipher in ("ECDSA", "RSA"))

    def finalize(self) -> None:
        """Create or overwrite the PEM bundle at ``path``.

        Write selected roots sorted by subject when discovery found roots without
        failures. Otherwise, write all certifi roots with blank and comment lines
        removed. Filesystem and certifi read errors propagate."""
        self.path.parent.mkdir(parents=True, exist_ok=True)
        if self._selected_roots and not self._has_failures:
            self.path.write_bytes(
                b"".join(
                    cert.public_bytes(cryptography.hazmat.primitives.serialization.Encoding.PEM)
                    for cert in sorted(
                        self._selected_roots,
                        key=lambda cert: cert.subject.rfc4514_string(),
                    )
                )
            )
            return
        if self._has_failures:
            logging.getLogger(__name__).warning("Falling back to stripped certifi bundle.")
        self.path.write_bytes(
            (
                "\n".join(line for line in certifi.contents().splitlines() if line and not line.startswith("#")) + "\n"
            ).encode()
        )

    def add_host(self, hostname: str) -> None:
        """Probe a hostname once per instance and accumulate its candidate roots.

        Call initialize first to prepare TLS probes. DNS failures and discovery
        that finds no roots mark the final bundle for fallback to certifi.
        Repeated hostnames are skipped, including those whose discovery failed."""
        if hostname in self._hosts:
            return
        self._hosts.add(hostname)
        self._selected_roots.update(self._probe_host_branches(hostname))

    def _get_server_chain(
        self, ip: str, hostname: str, ssl_context: ssl.SSLContext
    ) -> list[cryptography.x509.Certificate]:
        """Fetch the presented TLS chain from an IP on port 443 using hostname as SNI.

        TCP connection errors propagate. TLS or socket errors after connection
        return an empty list; invalid DER raises ValueError. Each connection uses
        ``timeout`` seconds."""
        with socket.create_connection((ip, 443), timeout=self.timeout) as sock:
            try:
                with ssl_context.wrap_socket(sock, server_hostname=hostname) as ssock:
                    return [cryptography.x509.load_der_x509_certificate(der) for der in ssock.get_unverified_chain()]
            except (ssl.SSLError, OSError):
                return []

    def _resolve_intermediate_aia(self, cert: cryptography.x509.Certificate) -> cryptography.x509.Certificate | None:
        """Return the first verified issuer certificate from an eligible CA Issuers AIA URL.

        Only URLs passing the safety check are fetched, with ``timeout`` seconds
        per request and a 65,536-byte response limit. Missing AIA, rejected URLs,
        oversized responses, and caught network, parse, or signature failures
        yield None if no other URL succeeds."""
        max_cert_size: typing.Final[int] = 1 << 16
        try:
            aia = cert.extensions.get_extension_for_class(cryptography.x509.AuthorityInformationAccess).value
        except cryptography.x509.ExtensionNotFound:
            return None
        for desc in aia:
            if desc.access_method != cryptography.x509.oid.AuthorityInformationAccessOID.CA_ISSUERS or not isinstance(
                desc.access_location.value, str
            ):
                continue
            url = desc.access_location.value
            if not self._is_safe_url(url):
                logging.getLogger(__name__).debug("Skipping untrusted or non-global AIA URL: %s", url)
                continue
            try:
                with self._url_opener.open(url, timeout=self.timeout) as response:
                    raw = response.read(max_cert_size + 1)
                if len(raw) > max_cert_size:
                    logging.getLogger(__name__).debug(
                        "AIA certificate from %s exceeded %d bytes limit; skipping",
                        url,
                        max_cert_size,
                    )
                    continue
                issuer = (
                    cryptography.x509.load_pem_x509_certificate(raw)
                    if b"-----BEGIN CERTIFICATE-----" in raw
                    else cryptography.x509.load_der_x509_certificate(raw)
                )
                cert.verify_directly_issued_by(issuer)
                return issuer
            except (
                OSError,
                ValueError,
                TypeError,
                http.client.HTTPException,
                cryptography.exceptions.InvalidSignature,
            ) as e:
                logging.getLogger(__name__).debug("Failed to load AIA certificate from %s: %s", url, e)
        return None

    def _find_candidate_roots(self, cert: cryptography.x509.Certificate) -> list[cryptography.x509.Certificate]:
        """Return stored roots whose subject and signature match the certificate issuer."""
        matches: list[cryptography.x509.Certificate] = []
        for root in self._root_store.get(cert.issuer, ()):
            try:
                cert.verify_directly_issued_by(root)
                matches.append(root)
            except (ValueError, TypeError, cryptography.exceptions.InvalidSignature):
                continue
        return matches

    def _trace_to_roots(
        self,
        chain: list[cryptography.x509.Certificate],
    ) -> list[cryptography.x509.Certificate]:
        """Find candidate roots starting from the last certificate in a nonempty chain.

        Verify chain link signatures, then follow AIA issuers until candidates
        are found or traversal stops or cycles. Cache successful candidates for
        all supplied and traversed certificates. Return an empty list if chain
        verification fails or no roots are found; an empty chain raises IndexError."""
        try:
            for child, issuer in itertools.pairwise(chain):
                child.verify_directly_issued_by(issuer)
        except (ValueError, TypeError, cryptography.exceptions.InvalidSignature):
            return []
        traversed: set[cryptography.x509.Certificate] = set()
        current: cryptography.x509.Certificate | None = chain[-1]
        candidates: list[cryptography.x509.Certificate] = []
        while current and current not in traversed:
            traversed.add(current)
            if candidates := self._visited_certificates.get(current) or self._find_candidate_roots(current):
                break
            current = self._resolve_intermediate_aia(current)
        if candidates:
            for cert in traversed.union(chain):
                self._visited_certificates[cert] = candidates
        return candidates

    def _probe_host_branches(self, hostname: str) -> set[cryptography.x509.Certificate]:
        """Collect candidate roots from resolved IPs using each configured TLS context.

        Stop probing an IP version after three TCP connection failures. DNS
        failure or finding no roots marks the bundle for fallback; caught TCP
        and TLS failures do not escape. Certificate parsing errors outside the
        AIA fetch can propagate."""
        discovered_roots: set[cryptography.x509.Certificate] = set()
        max_failed_ips_per_host: typing.Final[int] = 3
        try:
            ips = sorted(
                {
                    str(entry[4][0])
                    for entry in socket.getaddrinfo(hostname, 443, proto=socket.IPPROTO_TCP, flags=socket.AI_ADDRCONFIG)
                }
            )
        except socket.gaierror as e:
            logging.getLogger(__name__).warning(
                "Certificate discovery failed: cannot resolve DNS for '%s': %s", hostname, e
            )
            self._has_failures = True
            return discovered_roots
        failed_ips: collections.Counter[int] = collections.Counter()
        for ip in ips:
            version = ipaddress.ip_address(ip).version
            if failed_ips[version] >= max_failed_ips_per_host:
                continue
            for ssl_context in self._ssl_contexts:
                try:
                    chain = self._get_server_chain(ip, hostname, ssl_context)
                except OSError as e:
                    logging.getLogger(__name__).debug("TCP connection to '%s' (%s) failed: %s", hostname, ip, e)
                    failed_ips[version] += 1
                    if failed_ips[version] == max_failed_ips_per_host:
                        logging.getLogger(__name__).warning(
                            "Aborting remaining IPv%d probes for '%s': reached failure threshold (%d/%d failed)",
                            version,
                            hostname,
                            failed_ips[version],
                            max_failed_ips_per_host,
                        )
                    break
                if chain:
                    discovered_roots.update(self._trace_to_roots(chain))
        if not discovered_roots:
            logging.getLogger(__name__).warning(
                "Certificate discovery incomplete: no root CAs found for '%s'", hostname
            )
            self._has_failures = True
        return discovered_roots

    @staticmethod
    def _create_ssl_context(cipher_pref: str) -> ssl.SSLContext:
        """Return a TLS probe context with certificate and hostname checks disabled.

        cipher_pref is an OpenSSL cipher selection expression; anonymous and null
        encryption ciphers are excluded. SSL context creation and cipher selection
        errors propagate."""
        ctx = ssl.create_default_context()
        ctx.check_hostname = False
        ctx.verify_mode = ssl.CERT_NONE
        ctx.set_ciphers(f"{cipher_pref}:!aNULL:!eNULL")
        return ctx

    @staticmethod
    def _is_safe_url(url: str) -> bool:
        """Accept HTTP(S) URLs on default ports, port 80, or port 443 with global IPs.

        Resolve the hostname and require every returned address to be global.
        Invalid URLs, DNS failures, and empty DNS results return False."""
        try:
            parsed = urllib.parse.urlparse(url)
            if parsed.scheme not in ("http", "https") or not parsed.hostname or parsed.port not in (None, 80, 443):
                return False
            addr_info = socket.getaddrinfo(parsed.hostname, parsed.port or 80, proto=socket.IPPROTO_TCP)
            return bool(addr_info) and all(ipaddress.ip_address(entry[4][0]).is_global for entry in addr_info)
        except (ValueError, socket.gaierror):
            return False

    @staticmethod
    def clean() -> None:
        """Remove the generated bundle if it is a file, leaving its directory intact.

        Print the removed path. Filesystem errors propagate."""
        if Certificate.path.is_file():
            Certificate.path.unlink()
            print(f"Removing {Certificate.path}")
