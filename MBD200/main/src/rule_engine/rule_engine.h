#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include <stdint.h>

typedef enum {
    CH_AI1,
    CH_AI2,
    CH_DI1
} Channel;

typedef enum {
    OP_GT,   // >
    OP_LT,   // <
    OP_EQ    // =
} Operator;

typedef enum {
    ACT_NONE,
    ACT_ALARM,
    ACT_RELAY1_ON,
    ACT_RELAY1_OFF
} Action;

typedef struct {
    Channel channel;
    Operator op;
    float value;
    Action action;
} Rule;

// API
void RuleEngine_Init(void);
void RuleEngine_Run(void);

#endif