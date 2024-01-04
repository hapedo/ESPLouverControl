#include "louver.h"
#include <Arduino.h>
#include "time.h"
#include "log.h"
#include "config.h"

Louver::Louver() :
    m_lastKeyUpState(false),
    m_lastKeyDownState(false),
    m_lastKeyUpChangeTime(0),
    m_lastKeyDownChangeTime(0),
    m_state(ST_IDLE),
    m_closePercent(0),
    m_keyUpReleased(false),
    m_keyDownReleased(false)
{
    setDefaultsPrivate();
}

void Louver::setDefaults()
{
    getInstance().setDefaultsPrivate();
}

void Louver::loadConfig()
{
    getInstance().loadConfigPrivate();
}

void Louver::setDefaultsPrivate()
{
    m_pinKeyUp = DEFAULT_PIN_KEY_UP;
    m_pinKeyDown = DEFAULT_PIN_KEY_DOWN;
    m_pinRelayUp = DEFAULT_PIN_RELAY_UP;
    m_pinRelayDown = DEFAULT_PIN_RELAY_DOWN;
    m_keyUpActiveHigh = DEFAULT_KEY_ACTIVE_HIGH;
    m_keyDownActiveHigh = DEFAULT_KEY_ACTIVE_HIGH;
    m_keyUpPullEnabled = DEFAULT_KEY_PULL_ACTIVE;
    m_keyDownPullEnabled = DEFAULT_KEY_PULL_ACTIVE;
    m_relayUpActiveHigh = DEFAULT_RELAY_ACTIVE_HIGH;
    m_relayDownActiveHigh = DEFAULT_RELAY_ACTIVE_HIGH;
    m_timeUp = (uint32_t)(DEFAULT_TIME_UP_SECS * 1000);
    m_timeDown = (uint32_t)(DEFAULT_TIME_DOWN_SECS * 1000);
    m_timeOpenLamellas = (uint32_t)(DEFAULT_TIME_OPEN_LAMELLAS_SECS * 1000);
    m_timeShortMovement = (uint32_t)(DEFAULT_TIME_SHORT_MOVEMENT_SECS * 1000);
    initPins();
    Log::info("Louver", "Defaults set");
}

void Louver::loadConfigPrivate()
{
    m_pinKeyUp = Config::getInt("gpio/key_up", DEFAULT_PIN_KEY_UP);
    m_pinKeyDown = Config::getInt("gpio/key_down", DEFAULT_PIN_KEY_DOWN);
    m_pinRelayUp = Config::getInt("gpio/relay_up", DEFAULT_PIN_RELAY_UP);
    m_pinRelayDown = Config::getInt("gpio/relay_down", DEFAULT_PIN_RELAY_DOWN);
    m_keyUpActiveHigh = Config::getBool("gpio/key_up_active_high", DEFAULT_KEY_ACTIVE_HIGH);
    m_keyDownActiveHigh = Config::getBool("gpio/key_down_active_high", DEFAULT_KEY_ACTIVE_HIGH);
    m_keyUpPullEnabled = Config::getBool("gpio/key_up_pull_enabled", DEFAULT_KEY_PULL_ACTIVE);
    m_keyDownPullEnabled = Config::getBool("gpio/key_down_pull_enabled", DEFAULT_KEY_PULL_ACTIVE);
    m_relayUpActiveHigh = Config::getBool("gpio/relay_up_active_high", DEFAULT_RELAY_ACTIVE_HIGH);
    m_relayDownActiveHigh = Config::getBool("gpio/relay_down_active_high", DEFAULT_RELAY_ACTIVE_HIGH);
    m_timeUp = (uint32_t)(Config::getFloat("timing/up", DEFAULT_TIME_UP_SECS) * 1000);
    m_timeDown = (uint32_t)(Config::getFloat("timing/down", DEFAULT_TIME_DOWN_SECS) * 1000);
    m_timeOpenLamellas = (uint32_t)(Config::getFloat("timing/open_lamellas", DEFAULT_TIME_OPEN_LAMELLAS_SECS) * 1000);
    m_timeShortMovement = (uint32_t)(Config::getFloat("timing/short_movement", DEFAULT_TIME_SHORT_MOVEMENT_SECS) * 1000);
    initPins();
    Log::info("Louver", "Configuration loaded");
}

void Louver::configureGpio(Direction dir, uint8_t pinKey, uint8_t pinRelay, bool keyActiveHigh, bool relayActiveHigh, bool enablePull)
{
    Louver& inst = getInstance();
    if (dir == DIR_UP)
    {
        inst.m_pinKeyUp = pinKey;
        Config::setInt("gpio/key_up", inst.m_pinKeyUp);
        inst.m_pinRelayUp = pinRelay;
        Config::setInt("gpio/relay_up", inst.m_pinRelayUp);
        inst.m_keyUpActiveHigh = keyActiveHigh;
        Config::setBool("gpio/key_up_active_high", inst.m_keyUpActiveHigh);
        inst.m_relayUpActiveHigh = relayActiveHigh;
        Config::setBool("gpio/relay_up_active_high", inst.m_relayUpActiveHigh);
        inst.m_keyUpPullEnabled = enablePull;
        Config::setBool("gpio/key_up_pull_enabled", inst.m_keyUpPullEnabled);
        Config::flush();
        inst.initPins();
        Log::info("Louver", "Up key configuration, pin = %d, relay pin = %d", pinKey, pinRelay);
    }
    else
    {
        inst.m_pinKeyDown = pinKey;
        Config::setInt("gpio/key_down", inst.m_pinKeyDown);
        inst.m_pinRelayDown = pinRelay;
        Config::setInt("gpio/relay_down", inst.m_pinRelayDown);
        inst.m_keyDownActiveHigh = keyActiveHigh;
        Config::setBool("gpio/key_down_active_high", inst.m_keyDownActiveHigh);
        inst.m_relayDownActiveHigh = relayActiveHigh;
        Config::setBool("gpio/relay_down_active_high", inst.m_relayDownActiveHigh);
        inst.m_keyDownPullEnabled = enablePull;
        Config::setBool("gpio/key_down_pull_enabled", inst.m_keyDownPullEnabled);
        Config::flush();
        inst.initPins();
        Log::info("Louver", "Down key configuration, pin = %d, relay pin = %d", pinKey, pinRelay);
    }
}

void Louver::getGpioConfig(Direction dir, uint8_t& pinKey, uint8_t& pinRelay, bool& keyActiveHigh, bool& relayActiveHigh, bool& pullEnabled)
{
    Louver& inst = getInstance();
    if (dir == DIR_UP)
    {
        pinKey = inst.m_pinKeyUp;
        pinRelay = inst.m_pinRelayUp;
        keyActiveHigh = inst.m_keyUpActiveHigh;
        relayActiveHigh = inst.m_relayUpActiveHigh;
        pullEnabled = inst.m_keyUpPullEnabled;
    }
    else
    {
        pinKey = inst.m_pinKeyDown;
        pinRelay = inst.m_pinRelayDown;
        keyActiveHigh = inst.m_keyDownActiveHigh;
        relayActiveHigh = inst.m_relayDownActiveHigh;
        pullEnabled = inst.m_keyDownPullEnabled;
    }
}

void Louver::configureTimes(float timeFullOpenSecs, float timeFullCloseSecs, float timeOpenLamellasSecs, float shortMovementSecs)
{
    Louver& inst = getInstance();
    inst.m_timeUp = (uint32_t)(timeFullOpenSecs * 1000);
    Config::setFloat("timing/up", timeFullOpenSecs);
    inst.m_timeDown = (uint32_t)(timeFullCloseSecs * 1000);
    Config::setFloat("timing/down", timeFullCloseSecs);
    inst.m_timeOpenLamellas = (uint32_t)(timeOpenLamellasSecs * 1000);
    Config::setFloat("timing/open_lamellas", timeOpenLamellasSecs);
    inst.m_timeShortMovement = (uint32_t)(shortMovementSecs * 1000);
    Config::setFloat("timing/short_movement", shortMovementSecs);
    Config::flush();
    Log::info("Louver", "Timing set, full open=%f s, full close=%f s, short=%f s, open lamellas=%f s", timeFullOpenSecs, timeFullCloseSecs, shortMovementSecs, timeOpenLamellasSecs);
}

void Louver::getTimesConfig(float& timeFullOpenSecs, float& timeFullCloseSecs, float& timeOpenLamellasSecs, float& shortMovementSecs)
{
    Louver& inst = getInstance();
    timeFullOpenSecs = (float)inst.m_timeUp / 1000;
    timeFullCloseSecs = (float)inst.m_timeDown / 1000;
    timeOpenLamellasSecs = (float)inst.m_timeOpenLamellas / 1000;
    shortMovementSecs = (float)inst.m_timeShortMovement / 1000;
}

float Louver::getShortMovementSecs()
{
    return (float)getInstance().m_timeShortMovement / 1000;
}

void Louver::fullOpen()
{
    Louver& inst = getInstance();
    MovementStep step;
    step.direction = DIR_UP;
    step.timeMilli = Time::nowRelativeMilli() + inst.m_timeUp;
    inst.m_movement.clear();
    inst.m_movement.push_back(step);
    Log::info("Louver", "Full open movement");
    inst.startMovement();
}

void Louver::fullClose()
{
    Louver& inst = getInstance();
    MovementStep step;
    step.direction = DIR_DOWN;
    step.timeMilli = Time::nowRelativeMilli() + inst.m_timeDown;
    inst.m_movement.clear();
    inst.m_movement.push_back(step);
    Log::info("Louver", "Full close movement");
    inst.startMovement();
}

void Louver::shortOpen(float timeSecs)
{
    Louver& inst = getInstance();
    MovementStep step;
    if (timeSecs == -1)
        timeSecs = (float)inst.m_timeShortMovement / 1000;
    step.direction = DIR_UP;
    step.timeMilli = Time::nowRelativeMilli() + (uint32_t)(timeSecs * 1000);
    inst.m_movement.clear();
    inst.m_movement.push_back(step);
    Log::info("Louver", "Short open movement, time %f seconds", timeSecs);
    inst.startMovement();
}

void Louver::shortClose(float timeSecs)
{
    Louver& inst = getInstance();
    MovementStep step;
    if (timeSecs == -1)
        timeSecs = (float)inst.m_timeShortMovement / 1000;
    step.direction = DIR_DOWN;
    step.timeMilli = Time::nowRelativeMilli() + (uint32_t)(timeSecs * 1000);
    inst.m_movement.clear();
    inst.m_movement.push_back(step);
    Log::info("Louver", "Short close movement, time %f seconds", timeSecs);
    inst.startMovement();
}


void Louver::fullCloseAndOpenLamellas()
{
    Louver& inst = getInstance();
    MovementStep step;
    step.direction = DIR_DOWN;
    step.timeMilli = Time::nowRelativeMilli() + inst.m_timeDown;
    inst.m_movement.clear();
    inst.m_movement.push_back(step);
    step.direction = DIR_UP;
    step.timeMilli += inst.m_timeOpenLamellas;
    inst.m_movement.push_back(step);
    Log::info("Louver", "Full close and open lamellas movement");
    inst.startMovement();
}

void Louver::stop()
{
    getInstance().delay(ST_WAIT_RELEASE);
}

void Louver::initPins()
{
    uint8_t pull = INPUT_PULLUP;
#ifdef ESP32
    if (m_keyUpActiveHigh)
        pull = INPUT_PULLDOWN;
#else
    // No pulldown for ESP8266
    if (m_keyUpActiveHigh)
        pull = 0;
#endif
    if (!m_keyUpPullEnabled)
        pull = 0;
    pinMode(m_pinKeyUp, INPUT | pull);
    pull = INPUT_PULLUP;
#ifdef ESP32
    if (m_keyDownActiveHigh)
        pull = INPUT_PULLDOWN;
#else
    // No pulldown for ESP8266
    if (m_keyDownActiveHigh)
        pull = 0;
#endif
    if (!m_keyDownPullEnabled)
        pull = 0;
    pinMode(m_pinKeyDown, INPUT | pull);
    pinMode(m_pinRelayUp, OUTPUT);
    pinMode(m_pinRelayDown, OUTPUT);
    if (m_relayUpActiveHigh)
        digitalWrite(m_pinRelayUp, 0);
    else
        digitalWrite(m_pinRelayUp, 1);
    if (m_relayDownActiveHigh)
        digitalWrite(m_pinRelayDown, 0);
    else
        digitalWrite(m_pinRelayDown, 1);
}

void Louver::updateRelays()
{
    switch(m_state)
    {
        case ST_IDLE:
        case ST_WAIT_RELEASE:
        case ST_DELAY:
            relaysIdle();
            break;
        case ST_UP:
            relaysUp();
            break;
        case ST_DOWN:
            relaysDown();
            break;
        case ST_MOVEMENT:
            {
                Direction dir = DIR_DOWN;
                int index = m_stepIndex;
                if (index >= m_movement.size())
                    index = -1;
                else
                {
                    dir = m_movement[index].direction;
                }
                if (index == -1)
                {
                    relaysIdle();
                }
                else
                {
                    if (dir == DIR_UP)
                        relaysUp();
                    else
                        relaysDown();
                }
            }
            break;
    }
}

void Louver::relaysUp()
{
    if (m_relayDownActiveHigh)
        digitalWrite(m_pinRelayDown, 0);
    else
        digitalWrite(m_pinRelayDown, 1);
    if (m_relayUpActiveHigh)
        digitalWrite(m_pinRelayUp, 1);
    else
        digitalWrite(m_pinRelayUp, 0);
}

void Louver::relaysDown()
{
    if (m_relayUpActiveHigh)
        digitalWrite(m_pinRelayUp, 0);
    else
        digitalWrite(m_pinRelayUp, 1);
    if (m_relayDownActiveHigh)
        digitalWrite(m_pinRelayDown, 1);
    else
        digitalWrite(m_pinRelayDown, 0);
}

void Louver::relaysIdle()
{
    if (m_relayUpActiveHigh)
        digitalWrite(m_pinRelayUp, 0);
    else
        digitalWrite(m_pinRelayUp, 1);
    if (m_relayDownActiveHigh)
        digitalWrite(m_pinRelayDown, 0);
    else
        digitalWrite(m_pinRelayDown, 1);
}

void Louver::startMovement()
{
    m_keyUpReleased = false;
    m_keyDownReleased = false;
    m_state = ST_MOVEMENT;
    m_stepIndex = 0;
    Log::info("Louver", "Starting movement, number of steps = %d", m_movement.size());
}

void Louver::delay(State nextState)
{
    m_nextState = nextState;
    m_delayTimeout = Time::nowRelativeMilli() + MOVEMENT_DELAY_MILLI;
    m_state = ST_DELAY;
    Log::debug("Louver", "Entering delay state");
}

void Louver::process()
{
    Louver& inst = getInstance();
    bool isKeyUpActive = digitalRead(inst.m_pinKeyUp) == HIGH;
    if (!inst.m_keyUpActiveHigh)
        isKeyUpActive = !isKeyUpActive;
    bool isKeyDownActive = digitalRead(inst.m_pinKeyDown) == HIGH;
    if (!inst.m_keyDownActiveHigh)
        isKeyDownActive = !isKeyDownActive;
    uint64_t now = Time::nowRelativeMilli();
    if (isKeyUpActive != inst.m_lastKeyUpState)
    {
        inst.m_lastKeyUpState = isKeyUpActive;
        inst.m_lastKeyUpChangeTime = now;
    }
    if (isKeyDownActive != inst.m_lastKeyDownState)
    {
        inst.m_lastKeyDownState = isKeyDownActive;
        inst.m_lastKeyDownChangeTime = now;
    }

    bool isUpPressDebounced = (now - inst.m_lastKeyUpChangeTime) >= DEBOUNCE_PRESS_MILLI;
    bool isUpHoldDebounced = (now - inst.m_lastKeyUpChangeTime) >= DEBOUNCE_HOLD_MILLI;
    bool isDownPressDebounced = (now - inst.m_lastKeyDownChangeTime) >= DEBOUNCE_PRESS_MILLI;
    bool isDownHoldDebounced = (now - inst.m_lastKeyDownChangeTime) >= DEBOUNCE_HOLD_MILLI;

    switch(inst.m_state)
    {
        case ST_IDLE:
            if (isKeyUpActive && isUpPressDebounced)
            {
                inst.m_state = ST_UP;
                Log::info("Louver", "Up pressed");
            }
            else if (isKeyDownActive && isDownPressDebounced)
            {
                inst.m_state = ST_DOWN;
                Log::info("Louver", "Down pressed");
            }
            break;
        case ST_UP:
            if (!isKeyUpActive)
            {
                Log::info("Louver", "Up released");
                inst.delay(ST_IDLE);
                break;
            }
            if (isKeyUpActive && isUpHoldDebounced)
            {
                Log::info("Louver", "Up hold detected");
                inst.fullOpen();
            }
            break;
        case ST_DOWN:
            if (!isKeyDownActive)
            {
                Log::info("Louver", "Down released");
                inst.delay(ST_IDLE);
                break;
            }
            if (isKeyDownActive && isDownHoldDebounced)
            {
                Log::info("Louver", "Down hold detected");
                inst.fullCloseAndOpenLamellas();
            }
            break;
        case ST_MOVEMENT:
            {
                if (!isKeyUpActive)
                {
                    inst.m_keyUpReleased = true;
                }
                if (!isKeyDownActive)
                {
                    inst.m_keyDownReleased = true;
                }

                if ((inst.m_keyUpReleased) && (isKeyUpActive))
                {
                    Log::info("Louver", "Terminating movement due to up key press");
                    inst.delay(ST_WAIT_RELEASE);
                    break;
                }
                if ((inst.m_keyDownReleased) && (isKeyDownActive))
                {
                    Log::info("Louver", "Terminating movement due to down key press");
                    inst.delay(ST_WAIT_RELEASE);
                    break;
                }
                int index = inst.m_stepIndex;
                if (index >= inst.m_movement.size())
                {
                    Log::info("Louver", "Movement finished");
                    inst.delay(ST_WAIT_RELEASE);
                    break;
                }
                if (now >= inst.m_movement[index].timeMilli)
                {
                    inst.m_stepIndex++;
                    if (inst.m_stepIndex < inst.m_movement.size())
                    {
                        Log::info("Louver", "Next movement step, index = %d, direction %d", inst.m_stepIndex, inst.m_movement[inst.m_stepIndex].direction);
                    }
                    inst.delay(ST_MOVEMENT);
                }
            }
            break;
        case ST_WAIT_RELEASE:
            if ((!isKeyUpActive) && (!isKeyDownActive))
            {
                inst.m_state = ST_IDLE;
                Log::info("Louver", "All keys released");
            }
            break;
        case ST_DELAY:
            if (now >= inst.m_delayTimeout)
            {
                Log::debug("Louver", "Exiting delay state");
                inst.m_state = inst.m_nextState;
            }
            break;
    }
    inst.updateRelays();
}
