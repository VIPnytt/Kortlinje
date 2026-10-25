#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <ArduinoOTA.h>
#include <array>
#include <bits/unique_ptr.h>
#include <optional>
#include <pico/time.h>

class SnakeMode final : public ModeModule
{
private:
    enum class Stage : uint8_t
    {
        READY,
        MOVE,
        DEATH,
        REMOVE,
    };

    bool timerRunning{false};
    static inline volatile bool tick{false};

    uint8_t blinkCount{0U};

    size_t head{0U};
    size_t length{0U};
    size_t target{0U};

    repeating_timer timer{};

    std::array<bool, GRID_COLUMNS * GRID_ROWS> occupied{};

    std::array<size_t, GRID_COLUMNS * GRID_ROWS> snake{};

    Stage stage{Stage::READY};

    void blink();
    void clean();
    void idle();
    void move();
    void setDead();
    void setTarget();
    void snakeReset(size_t start);
    void snakeClear();
    void startTimer();
    void stopTimer();

    static bool onTimer(repeating_timer *rt);

    [[nodiscard]] bool snakePushBack(size_t pixel);

    [[nodiscard]] size_t snakeAt(size_t index) const;
    [[nodiscard]] size_t snakePopFront();

    [[nodiscard]] std::optional<size_t> findStepAvailable() const;
    [[nodiscard]] std::optional<size_t> findStepPath() const;

public:
    static constexpr std::string_view name{"Snake"};

    explicit SnakeMode() : ModeModule(name) {};
    ~SnakeMode() override;

    void begin() override;
    void handle() override;
    void end() override;
};
