#include "handlers/CertificateHandler.h"

#include "handlers/EmbedHandler.h"

const BearSSL::X509List &CertificateHandler::list()
{
    static constexpr auto pem{EmbedHandler::embed<uint8_t>({
#embed "../../../x509/cacert.pem"
    })};
    static const BearSSL::X509List bundle{pem.data(), pem.size()};
    return bundle;
}
