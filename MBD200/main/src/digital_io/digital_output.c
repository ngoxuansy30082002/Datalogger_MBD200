#include "digital_output.h"

DO_DATA doDt;

static const char * __TAG__ = "DIGITAL_OUT";
static const DRV_DO_PLIB _doPlib = {
    .relay1 = GPIO_PIN_RA10,
    .relay2 = GPIO_PIN_RA9,
};
static DO_CONTEXT _doCtx[MAX_DIGITAL_OUTPUT];

static void DigitalOutput_Process(uint8_t id) {
    DO_CONTEXT *ctx = &_doCtx[id];

    uint16_t onTime = gAppCfg.io.out[id].ontime;
    uint16_t offTime = gAppCfg.io.out[id].offtime;
    uint16_t targetPulse = gAppCfg.io.out[id].pulseCount;

    if (gAppCfg.io.out[id].mode == OUT_PULSE) {
        switch (ctx->state) {
            case DO_STATE_IDLE:
                break;

            case DO_STATE_ON:
                if (TIME_IS_EXPIRED(ctx->lastTick, onTime)) {
                    doDt.out[id].level = false;
                    GPIO_PinWrite(ctx->pin, 0);
                    ctx->lastTick = TICK_NOW();
                    ctx->state = DO_STATE_OFF;
                }
                break;

            case DO_STATE_OFF:
                if (TIME_IS_EXPIRED(ctx->lastTick, offTime)) {
                    ctx->currentCount++;
                    if (ctx->currentCount >= targetPulse && targetPulse != 0) {
                        ctx->state = DO_STATE_IDLE;
                    } else {
                        doDt.out[id].level = true;
                        GPIO_PinWrite(ctx->pin, 1);
                        ctx->lastTick = TICK_NOW();
                        ctx->state = DO_STATE_ON;
                    }
                }
                break;
        }
    }
}

void DigitalOutput_Initialize(void) {
    _doCtx[0].pin = _doPlib.relay1;
    _doCtx[1].pin = _doPlib.relay2;

    for (int i = 0; i < MAX_DIGITAL_OUTPUT; i++) {
        _doCtx[i].state = DO_STATE_IDLE;
        GPIO_PinWrite(_doCtx[i].pin, 0);
    }
}

void DigitalOutput_Task(void) {
    for (int i = 0; i < MAX_DIGITAL_OUTPUT; i++) {
        DigitalOutput_Process(i);
    }
}

void DigitalOutput_Set(uint8_t id, bool enable) {
    if (id >= MAX_DIGITAL_OUTPUT) return;

    DO_CONTEXT *ctx = &_doCtx[id];
    if (enable) {
        doDt.out[id].level = true;

        if (gAppCfg.io.out[id].mode == OUT_HOLD) {
            GPIO_PinWrite(ctx->pin, 1);
            ctx->state = DO_STATE_IDLE;
        } else if (gAppCfg.io.out[id].mode == OUT_PULSE) {
            GPIO_PinWrite(ctx->pin, 1);
            ctx->lastTick = TICK_NOW();
            ctx->currentCount = 0;
            ctx->state = DO_STATE_ON;
        }
    } else {
        doDt.out[id].level = false;
        GPIO_PinWrite(ctx->pin, 0);
        ctx->state = DO_STATE_IDLE;
    }

    doDt.out[id].state = enable;
}