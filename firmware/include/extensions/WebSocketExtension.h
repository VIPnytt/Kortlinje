#pragma once

#include "modules/ExtensionModule.h"

#include <AsyncWebSocket.h>
#include <vector>

class WebSocketExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"WebSocket"};

    static inline std::vector<uint8_t> buffer{};

    static inline AsyncWebSocket socket{"/ws"};

    static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg,
                        const uint8_t *data, size_t len);

public:
    explicit WebSocketExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
    void onTransmit(JsonObjectConst payload, std::string_view source);

    [[nodiscard]] static AsyncWebSocket &server();
};
