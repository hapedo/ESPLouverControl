#pragma once
#include <stdint.h>
#include <vector>
#include "profiles.h"

class Louver
{
public:
    static constexpr uint8_t DEFAULT_PIN_KEY_UP = PROFILE_DEFAULT_PIN_KEY_UP;
    static constexpr uint8_t DEFAULT_PIN_KEY_DOWN = PROFILE_DEFAULT_PIN_KEY_DOWN;
    static constexpr uint8_t DEFAULT_PIN_RELAY_UP = PROFILE_DEFAULT_PIN_RELAY_UP;
    static constexpr uint8_t DEFAULT_PIN_RELAY_DOWN = PROFILE_DEFAULT_PIN_RELAY_DOWN;
    static constexpr bool DEFAULT_KEY_ACTIVE_HIGH = PROFILE_DEFAULT_KEY_ACTIVE_HIGH;
    static constexpr bool DEFAULT_KEY_PULL_ACTIVE = PROFILE_DEFAULT_KEY_PULL_ACTIVE;
    static constexpr bool DEFAULT_RELAY_ACTIVE_HIGH = PROFILE_DEFAULT_RELAY_ACTIVE_HIGH;
    static constexpr float DEFAULT_TIME_UP_SECS = 30;
    static constexpr float DEFAULT_TIME_DOWN_SECS = 30;
    static constexpr float DEFAULT_TIME_OPEN_LAMELLAS_SECS = 2;
    static constexpr float DEFAULT_TIME_SHORT_MOVEMENT_SECS = 0.25;

    enum Direction
    {
        DIR_UP = 0,
        DIR_DOWN
    };

    struct MovementStep
    {
        Direction direction;
        uint32_t timeMilli;
        bool checkConditions;
    };

    Louver();

    static void setDefaults();

    static void loadConfig();

    static void configureGpio(Direction dir, uint8_t pinKey, uint8_t pinRelay, bool keyActiveHigh = true, bool relayActiveHigh = true, bool enablePull = true);

    static void getGpioConfig(Direction dir, uint8_t& pinKey, uint8_t& pinRelay, bool& keyActiveHigh, bool& relayActiveHigh, bool& pullEnabled);

    static void configureTimes(float timeFullOpenSecs, float timeFullCloseSecs, float timeOpenLamellasSecs, float shortMovementSecs);

    static void getTimesConfig(float& timeFullOpenSecs, float& timeFullCloseSecs, float& timeOpenLamellasSecs, float& shortMovementSecs);

    static float getShortMovementSecs();

    static void configurePowerCondStop(bool upStopCond1, bool downStopCodn2);

    static void getPowerCondStop(bool& upStopCond1, bool& downStopCodn2);

    static void fullOpen();

    static void fullClose();

    static void fullCloseAndOpenLamellas();

    static void shortOpen(float timeSecs = -1);

    static void shortClose(float timeSecs = -1);

    static void stop();

    static void process();

private:

    static constexpr uint64_t DEBOUNCE_PRESS_MILLI = 50;
    static constexpr uint64_t DEBOUNCE_HOLD_MILLI = 2000; 
    static constexpr uint32_t MOVEMENT_DELAY_MILLI = 200;
    static constexpr uint32_t POSITION_REPORT_PERIOD_MILLI = 1000;
    static constexpr uint32_t POSITION_UPDATE_PERIOD_MILLI = 10;

    enum State
    {
        ST_IDLE = 0,
        ST_UP,
        ST_DOWN,
        ST_MOVEMENT,
        ST_WAIT_RELEASE,
        ST_DELAY
    };

    static inline Louver& getInstance()
    {
        static Louver louver;
        return louver;
    }

    void setDefaultsPrivate();

    void loadConfigPrivate();

    void initPins();

    void updateRelays();

    void recalculatePercents();

    void updatePosition(Direction direction);

    void relaysUp();

    void relaysDown();

    void relaysIdle();

    void startMovement();

    void delay(State nextState);

    float m_position;
    uint8_t m_pinKeyUp;
    uint8_t m_pinKeyDown;
    uint8_t m_pinRelayUp;
    uint8_t m_pinRelayDown;
    bool m_keyUpActiveHigh;
    bool m_keyDownActiveHigh;
    bool m_relayUpActiveHigh;
    bool m_relayDownActiveHigh;
    bool m_keyUpPullEnabled;
    bool m_keyDownPullEnabled;
    bool m_lastKeyUpState;
    bool m_lastKeyDownState;
    uint64_t m_lastKeyUpChangeTime;
    uint64_t m_lastKeyDownChangeTime;
    uint32_t m_timeUp;
    uint32_t m_timeDown;
    uint32_t m_timeOpenLamellas;
    uint32_t m_timeShortMovement;
    bool m_stopUpOnPowerCond1;
    bool m_stopDownOnPowerCond2;
    State m_state;
    State m_nextState;
    uint8_t m_closePercent;
    bool m_keyUpReleased;
    bool m_keyDownReleased;
    int m_stepIndex;
    uint64_t m_delayTimeout;
    std::vector<MovementStep> m_movement;
    uint64_t m_movementStartTime;
    bool m_mqttKeyUpReported;
    bool m_mqttKeyDownReported;
    bool m_mqttKeyUpHoldReported;
    bool m_mqttKeyDownHoldReported;
    float m_percentPerMilliUp;
    float m_percentPerMilliDown;
    uint64_t m_lastPositionReportTime;
    uint64_t m_lastPositionUpdateTime;
};