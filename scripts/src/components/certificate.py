import collections
import ipaddress
import logging
import pathlib
import socket
import ssl
import typing

import certifi
import cryptography.hazmat.primitives.serialization
import cryptography.x509

if typing.TYPE_CHECKING:
    from ..Kortlinje import Kortlinje
    from .Types import COMMAND_LINE_TARGETS
else:
    from SCons.Script import COMMAND_LINE_TARGETS


class Certificate:
    path: typing.Final[pathlib.Path] = pathlib.Path("x509") / "cacert.pem"
    project: "Kortlinje"
    timeout: typing.Final[float] = 10
    _has_failures: bool
    _hosts: set[str]
    _selected_roots: set[cryptography.x509.Certificate]
    _ssl_contexts: tuple[ssl.SSLContext, ...]

    def __init__(self, project: "Kortlinje") -> None:
        """Associate the build project with an empty certificate discovery state."""
        self.project = project
        self._has_failures = False
        self._hosts = set()
        self._selected_roots = set()
        self._ssl_contexts = ()

    def initialize(self) -> None:
        """Prepare ECDSA and RSA TLS probes verified against certifi.

        Ensure the bundle file exists and disable the project certificate
        component when the sole target is ``compiledb``. Filesystem and SSL
        context errors propagate."""
        if COMMAND_LINE_TARGETS == ["compiledb"]:
            if not self.path.is_file():
                self.finalize()
            self.project.certificate = None
            return
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
        else:
            logging.getLogger(__name__).warning(
                "Failed to retrieve verified roots for one or more hosts; falling back to full certifi bundle."
                if self._has_failures
                else "No hosts were declared; falling back to full certifi bundle."
            )
            self.path.write_bytes(
                (
                    "\n".join(line for line in certifi.contents().splitlines() if line and not line.startswith("#"))
                    + "\n"
                ).encode()
            )
        self.project.env.Depends(
            self.project.env.File("${BUILD_DIR}/src/handlers/CertificateHandler.cpp.o"),
            self.project.env.File(f"#{self.path.as_posix()}"),
        )

    def add_host(self, hostname: str) -> None:
        """Probe a hostname once per instance and accumulate its verified roots.

        Call initialize first to prepare TLS probes. DNS failures and discovery
        that finds no roots mark the final bundle for fallback to certifi.
        Repeated hostnames are skipped, including those whose discovery failed."""
        if hostname in self._hosts:
            return
        self._hosts.add(hostname)
        self._selected_roots.update(self._probe_host_branches(hostname))

    def _get_server_root(
        self, ip: str, hostname: str, ssl_context: ssl.SSLContext
    ) -> cryptography.x509.Certificate | None:
        """Fetch the verified root CA from an IP on port 443 using hostname as SNI.

        TCP connection errors propagate. TLS or socket errors after connection
        return None; invalid DER raises ValueError. Each connection uses
        ``timeout`` seconds."""
        with socket.create_connection((ip, 443), timeout=self.timeout) as sock:
            try:
                with ssl_context.wrap_socket(sock, server_hostname=hostname) as ssock:
                    if chain := ssock.get_verified_chain():
                        return cryptography.x509.load_der_x509_certificate(chain[-1])
                    return None
            except (ssl.SSLError, OSError):
                return None

    def _probe_host_branches(self, hostname: str) -> set[cryptography.x509.Certificate]:
        """Collect verified roots from resolved IPs using each configured TLS context.

        Stop probing an IP version after three TCP connection failures. DNS
        failure or finding no roots marks the bundle for fallback; caught TCP
        and TLS failures do not escape."""
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
                    root = self._get_server_root(ip, hostname, ssl_context)
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
                if root:
                    discovered_roots.add(root)
        if not discovered_roots:
            logging.getLogger(__name__).warning(
                "Certificate discovery incomplete: no root CAs found for '%s'", hostname
            )
            self._has_failures = True
        return discovered_roots

    @staticmethod
    def _create_ssl_context(cipher_pref: str) -> ssl.SSLContext:
        """Return a TLS probe context verified against certifi roots.

        cipher_pref is an OpenSSL cipher selection expression; anonymous and null
        encryption ciphers are excluded. SSL context creation, certifi loading,
        and cipher selection errors propagate."""
        ctx = ssl.create_default_context(cafile=certifi.where())
        ctx.maximum_version = ssl.TLSVersion.TLSv1_2
        ctx.minimum_version = ssl.TLSVersion.TLSv1_2
        ctx.set_ciphers(f"{cipher_pref}:!aNULL:!eNULL")
        return ctx

    @staticmethod
    def clean() -> None:
        """Remove the generated bundle if it is a file, leaving its directory intact.

        Print the removed path. Filesystem errors propagate."""
        if Certificate.path.is_file():
            Certificate.path.unlink()
            print(f"Removing {Certificate.path}")
