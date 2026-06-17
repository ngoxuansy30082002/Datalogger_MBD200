#include "led_indicate.h"

static const char * __TAG__ = "LED_IND";
static const DRV_LED_PLIB _ledPlib = {
    .power = GPIO_PIN_RE4,
    .status = GPIO_PIN_RE3,
    .rtu = GPIO_PIN_RE2,
    .gsm = GPIO_PIN_RG13,
};
static LED_CONTEXT _leds[LED_NUM_MAX];

static void LedIndicate_Process(LED_CONTEXT *led) {
    switch (led->mode) {
        case LED_MODE_OFF:
            GPIO_PinWrite(led->pin, 1);
            led->state = LED_STATE_IDLE;
            break;

        case LED_MODE_ON:
            GPIO_PinWrite(led->pin, 0);
            led->state = LED_STATE_IDLE;
            break;

        case LED_MODE_BLINK_INF:
        case LED_MODE_BLINK_N_REPEAT:
        case LED_MODE_BLINK_N_STOP:
            switch (led->state) {
                case LED_STATE_IDLE:
                    GPIO_PinWrite(led->pin, 0);
                    led->lastTick = TICK_NOW();
                    led->currentCount = 0;
                    led->state = LED_STATE_ON;
                    break;

                case LED_STATE_ON:
                    if (TIME_IS_EXPIRED(led->lastTick, LED_BLINK_ON_TIME_MS)) {
                        GPIO_PinWrite(led->pin, 1);
                        led->lastTick = TICK_NOW();
                        led->state = LED_STATE_OFF;
                    }
                    break;

                case LED_STATE_OFF:
                    if (TIME_IS_EXPIRED(led->lastTick, LED_BLINK_OFF_TIME_MS)) {
                        led->currentCount++;

                        if (led->mode != LED_MODE_BLINK_INF && led->currentCount >= led->targetCount) {
                            if (led->mode == LED_MODE_BLINK_N_STOP) {
                                led->mode = LED_MODE_OFF;
                            } else {
                                led->state = LED_STATE_PAUSE;
                                led->lastTick = TICK_NOW();
                            }
                        } else {
                            GPIO_PinWrite(led->pin, 0);
                            led->lastTick = TICK_NOW();
                            led->state = LED_STATE_ON;
                        }
                    }
                    break;

                case LED_STATE_PAUSE:
                    if (TIME_IS_EXPIRED(led->lastTick, LED_PAUSE_TIME_MS)) {
                        GPIO_PinWrite(led->pin, 0);
                        led->currentCount = 0;
                        led->lastTick = TICK_NOW();
                        led->state = LED_STATE_ON;
                    }
                    break;
            }
            break;
    }
}

void LedIndicate_Initialize(void) {
    _leds[LED_ID_POWER].pin = _ledPlib.power;
    _leds[LED_ID_STATUS].pin = _ledPlib.status;
    _leds[LED_ID_RTU].pin = _ledPlib.rtu;
    _leds[LED_ID_GSM].pin = _ledPlib.gsm;

    for (int i = 0; i < LED_NUM_MAX; i++) {
        _leds[i].mode = LED_MODE_OFF;
        _leds[i].state = LED_STATE_IDLE;
        GPIO_PinWrite(_leds[i].pin, 1);
    }

    LedIndicate_SetMode(LED_ID_POWER, LED_MODE_ON, 0);
}

void LedIndicate_Task(void) {
    for (int i = 0; i < LED_NUM_MAX; i++) {
        LedIndicate_Process(&_leds[i]);
    }
}

void LedIndicate_SetMode(LED_ID id, LED_MODE mode, uint16_t count) {
    if (id >= LED_NUM_MAX) return;

    _leds[id].mode = mode;
    _leds[id].state = LED_STATE_IDLE;
    _leds[id].targetCount = count;
}
