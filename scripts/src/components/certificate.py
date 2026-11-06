import collections
import logging
import pathlib
import socket
import ssl
import typing
import urllib.request
import warnings

import cryptography.hazmat.primitives.serialization
import cryptography.x509
import cryptography.x509.oid

from ..config.version import VERSION

if typing.TYPE_CHECKING:
    from ..Kortlinje import Kortlinje
    from .Types import COMMAND_LINE_TARGETS
else:
    from SCons.Script import COMMAND_LINE_TARGETS


class Certificate:
    path: typing.Final[pathlib.Path] = pathlib.Path("x509") / "cabundle.pem"
    project: "Kortlinje"
    timeout: typing.Final[float] = 10
    _hosts: set[str]
    _root_store: dict[cryptography.x509.Name, list[cryptography.x509.Certificate]]
    _selected_roots: dict[bytes, cryptography.x509.Certificate]
    _ssl_contexts: dict[str, ssl.SSLContext]

    def __init__(self, project: "Kortlinje") -> None:
        self.project = project
        self._hosts: set[str] = set()
        self._root_store: dict[cryptography.x509.Name, list[cryptography.x509.Certificate]] = {}
        self._selected_roots: dict[bytes, cryptography.x509.Certificate] = {}
        self._ssl_contexts: dict[str, ssl.SSLContext] = {}

    def initialize(self) -> None:
        if COMMAND_LINE_TARGETS in [
            ["compiledb"],
        ]:
            self.project.certificate = None
            return
        self._root_store: dict[cryptography.x509.Name, list[cryptography.x509.Certificate]] = (
            self._fetch_mozilla_roots()
        )
        self._ssl_contexts: dict[str, ssl.SSLContext] = {
            cipher: self._create_ssl_context(cipher) for cipher in ("ECDSA", "RSA")
        }

    def finalize(self) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        with open(self.path, "wb") as file:
            file.writelines(
                cert.public_bytes(cryptography.hazmat.primitives.serialization.Encoding.PEM)
                for cert in sorted(
                    self._selected_roots.values(),
                    key=lambda cert: cert.subject.rfc4514_string(),
                )
            )

    def add_host(self, hostname: str) -> None:
        if hostname in self._hosts:
            return
        self._hosts.add(hostname)
        self._selected_roots.update(self._probe_host_branches(hostname, self._root_store))

    def _fetch_mozilla_roots(self) -> dict[cryptography.x509.Name, list[cryptography.x509.Certificate]]:
        with urllib.request.urlopen(
            urllib.request.Request(
                "https://curl.se/ca/cacert.pem",
                headers={"User-Agent": f"Kortlinje/{VERSION} (+https://github.com/VIPnytt/Kortlinje)"},
            ),
            timeout=self.timeout,
        ) as response:
            pem_data: bytes = response.read()
        roots_by_subject: dict[cryptography.x509.Name, list[cryptography.x509.Certificate]] = collections.defaultdict(
            list
        )
        with warnings.catch_warnings(record=True) as captured:
            for raw_block in pem_data.split(b"-----END CERTIFICATE-----"):
                if b"-----BEGIN CERTIFICATE-----" in raw_block:
                    pem_cert = raw_block + b"-----END CERTIFICATE-----\n"
                    cert = cryptography.x509.load_pem_x509_certificate(pem_cert)
                    roots_by_subject[cert.subject].append(cert)
            for record in captured:
                logging.getLogger(__name__).debug("Certificate parser warning: %s", record.message)
        return roots_by_subject

    def _get_server_chain(self, ip: str, hostname: str, cipher_pref: str) -> list[cryptography.x509.Certificate] | None:
        ctx = self._ssl_contexts[cipher_pref]
        try:
            with (
                socket.create_connection((ip, 443), timeout=self.timeout) as sock,
                ctx.wrap_socket(sock, server_hostname=hostname) as ssock,
            ):
                if hasattr(ssock, "get_unverified_chain"):
                    chain = ssock.get_unverified_chain()
                    if chain:
                        return [cryptography.x509.load_der_x509_certificate(der) for der in chain]
                der_cert = ssock.getpeercert(binary_form=True)
                return [cryptography.x509.load_der_x509_certificate(der_cert)] if der_cert is not None else None
        except (OSError, ssl.SSLError):
            return None

    def _resolve_intermediate_aia(self, cert: cryptography.x509.Certificate) -> cryptography.x509.Certificate | None:
        try:
            for desc in cert.extensions.get_extension_for_class(cryptography.x509.AuthorityInformationAccess).value:
                if desc.access_method == cryptography.x509.oid.AuthorityInformationAccessOID.CA_ISSUERS and isinstance(
                    desc.access_location.value, str
                ):
                    with urllib.request.urlopen(desc.access_location.value, timeout=self.timeout) as response:
                        raw = response.read()
                        try:
                            return cryptography.x509.load_der_x509_certificate(raw)
                        except ValueError:
                            return cryptography.x509.load_pem_x509_certificate(raw)
        except (cryptography.x509.ExtensionNotFound, OSError):
            pass
        return None

    def _find_candidate_roots(
        self,
        cert: cryptography.x509.Certificate,
        root_store: dict[cryptography.x509.Name, list[cryptography.x509.Certificate]],
    ) -> list[cryptography.x509.Certificate]:
        candidates = root_store.get(cert.issuer, [])
        if not candidates:
            return []
        try:
            akid = cert.extensions.get_extension_for_class(
                cryptography.x509.AuthorityKeyIdentifier
            ).value.key_identifier
        except cryptography.x509.ExtensionNotFound:
            akid = None
        if not akid:
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
        root_store: dict[cryptography.x509.Name, list[cryptography.x509.Certificate]],
    ) -> list[cryptography.x509.Certificate]:
        top = chain[-1]
        candidates = self._find_candidate_roots(top, root_store)
        while not candidates:
            parent = self._resolve_intermediate_aia(top)
            if not parent:
                break
            candidates = self._find_candidate_roots(parent, root_store)
            top = parent
        return candidates

    def _probe_host_branches(
        self, hostname: str, root_store: dict[cryptography.x509.Name, list[cryptography.x509.Certificate]]
    ) -> dict[bytes, cryptography.x509.Certificate]:
        discovered_roots: dict[bytes, cryptography.x509.Certificate] = {}
        try:
            ips = sorted({str(entry[4][0]) for entry in socket.getaddrinfo(hostname, 443, proto=socket.IPPROTO_TCP)})
        except socket.gaierror:
            return discovered_roots
        for ip in ips:
            for cipher in ("ECDSA", "RSA"):
                chain = self._get_server_chain(ip, hostname, cipher)
                if not chain:
                    continue
                for root in self._trace_to_roots(chain, root_store):
                    discovered_roots[root.public_bytes(cryptography.hazmat.primitives.serialization.Encoding.DER)] = (
                        root
                    )
        return discovered_roots

    @staticmethod
    def _create_ssl_context(cipher_pref: str) -> ssl.SSLContext:
        ctx = ssl.create_default_context()
        ctx.check_hostname = False
        ctx.verify_mode = ssl.CERT_NONE
        ctx.set_ciphers(f"{cipher_pref}:!aNULL:!eNULL")
        return ctx

    @staticmethod
    def clean() -> None:
        if Certificate.path.is_file():
            Certificate.path.unlink()
            print(f"Removing {Certificate.path}")
