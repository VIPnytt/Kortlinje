import collections
import http.client
import ipaddress
import logging
import pathlib
import socket
import ssl
import typing
import urllib.parse
import urllib.request

import certifi
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
        self.project = project
        self._has_failures = False
        self._hosts = set()
        self._root_store = collections.defaultdict(list)
        self._selected_roots = set()
        self._ssl_contexts = ()
        self._url_opener = urllib.request.build_opener(_SafeRedirectHandler())
        self._visited_certificates = {}

    def initialize(self) -> None:
        if COMMAND_LINE_TARGETS in [
            ["compiledb"],
        ]:
            self.project.certificate = None
            return
        for cert in cryptography.x509.load_pem_x509_certificates(certifi.contents().encode()):
            self._root_store[cert.subject].append(cert)
        self._ssl_contexts = tuple(self._create_ssl_context(cipher) for cipher in ("ECDSA", "RSA"))

    def finalize(self) -> None:
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
        if hostname in self._hosts:
            return
        self._hosts.add(hostname)
        self._selected_roots.update(self._probe_host_branches(hostname))

    def _get_server_chain(
        self, ip: str, hostname: str, ssl_context: ssl.SSLContext
    ) -> list[cryptography.x509.Certificate]:
        with socket.create_connection((ip, 443), timeout=self.timeout) as sock:
            try:
                with ssl_context.wrap_socket(sock, server_hostname=hostname) as ssock:
                    return [cryptography.x509.load_der_x509_certificate(der) for der in ssock.get_unverified_chain()]
            except (ssl.SSLError, OSError):
                return []

    def _resolve_intermediate_aia(self, cert: cryptography.x509.Certificate) -> cryptography.x509.Certificate | None:
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
                if b"-----BEGIN CERTIFICATE-----" in raw:
                    return cryptography.x509.load_pem_x509_certificate(raw)
                return cryptography.x509.load_der_x509_certificate(raw)
            except (OSError, ValueError, http.client.HTTPException) as e:
                logging.getLogger(__name__).debug("Failed to load AIA certificate from %s: %s", url, e)
        return None

    def _find_candidate_roots(self, cert: cryptography.x509.Certificate) -> list[cryptography.x509.Certificate]:
        if not (candidates := self._root_store.get(cert.issuer)):
            return []
        try:
            if not (
                akid := cert.extensions.get_extension_for_class(
                    cryptography.x509.AuthorityKeyIdentifier
                ).value.key_identifier
            ):
                return candidates
        except cryptography.x509.ExtensionNotFound:
            return candidates
        matches: list[cryptography.x509.Certificate] = []
        for root in candidates:
            try:
                skid = root.extensions.get_extension_for_class(cryptography.x509.SubjectKeyIdentifier).value.digest
                if skid and akid != skid:
                    continue
            except cryptography.x509.ExtensionNotFound:
                pass
            matches.append(root)
        return matches

    def _trace_to_roots(
        self,
        chain: list[cryptography.x509.Certificate],
    ) -> list[cryptography.x509.Certificate]:
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
        ctx = ssl.create_default_context()
        ctx.check_hostname = False
        ctx.verify_mode = ssl.CERT_NONE
        ctx.set_ciphers(f"{cipher_pref}:!aNULL:!eNULL")
        return ctx

    @staticmethod
    def _is_safe_url(url: str) -> bool:
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
        if Certificate.path.is_file():
            Certificate.path.unlink()
            print(f"Removing {Certificate.path}")
