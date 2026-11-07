#pragma once

#include "modules/HandlerModule.h"

#include <BearSSLHelpers.h>

class CertificateHandler final : public HandlerModule
{
public:
    [[nodiscard]] static const BearSSL::X509List &list();
};
