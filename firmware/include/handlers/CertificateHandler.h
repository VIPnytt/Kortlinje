#pragma once

#include "handlers/EmbedHandler.h"
#include "modules/HandlerModule.h"

#include <cstdint>

class CertificateHandler final : public HandlerModule
{
public:
    static constexpr auto bundle{EmbedHandler::embed<uint8_t>({
#embed "../../../x509/cacert.pem"
    })};
};
