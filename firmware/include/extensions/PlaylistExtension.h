#pragma once

#include "modules/ExtensionModule.h"

#include <pico/time.h>
#include <string>
#include <string_view>
#include <vector>

class PlaylistExtension final : public ExtensionModule
{
public:
    struct Item
    {
        std::string_view mode{};
        uint16_t duration{0U};
    };

    explicit PlaylistExtension() : ExtensionModule(name) {};
    ~PlaylistExtension() override;

    void begin() override;
    void handle() override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;
    void onTransmit(JsonObjectConst payload, std::string_view source) override;

    static void setEnabled(bool enable);

    [[nodiscard]] static bool getEnabled();

private:
    static constexpr std::string_view name{"Playlist"};

    static constexpr const char *schedulePath{"/playlist.json"};
    static constexpr const char *scheduleTempPath{"/playlist.tmp"};

    static inline bool enabled{false};

    static inline size_t step{0U};

    static inline alarm_id_t alarmId{0};

    static inline std::string currentMode{};

    static inline std::vector<Item> schedule{};

    static inline volatile bool tick{false};

    void loadSchedule();
    void saveSchedule();
    void setSchedule(std::vector<Item> items);

    static void advance();
    static void armTimer(uint32_t milliseconds);
    static void cancelTimer();
    static void transmit();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);
};
