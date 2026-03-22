#include "rule_engine.h"

// thay b?ng ADC/GPIO th?t
float AI1_value = 0;
float AI2_value = 0;
uint8_t DI1_value = 0;

//  load t? flash/SD
static Rule rules[] = {
    {CH_AI1, OP_GT, 20, ACT_ALARM},
    {CH_AI2, OP_LT, 5,  ACT_RELAY1_ON}
};

#define NUM_RULES (sizeof(rules)/sizeof(Rule))

// ===== ACTION =====
static void executeAction(Action act)
{
    switch(act)
    {
        case ACT_ALARM:
            // b?t alarm
            break;

        case ACT_RELAY1_ON:
            // b?t relay
            break;

        case ACT_RELAY1_OFF:
            // t?t relay
            break;

        default:
            break;
    }
}

// ===== GET VALUE =====
static float getValue(Channel ch)
{
    switch(ch)
    {
        case CH_AI1: return AI1_value;
        case CH_AI2: return AI2_value;
        case CH_DI1: return DI1_value;
        default: return 0;
    }
}

// ===== CHECK CONDITION =====
static uint8_t checkCondition(float val, Operator op, float threshold)
{
    switch(op)
    {
        case OP_GT: return (val > threshold);
        case OP_LT: return (val < threshold);
        case OP_EQ: return (val == threshold);
        default: return 0;
    }
}

// ===== MAIN ENGINE =====
void RuleEngine_Run(void)
{
    for(int i = 0; i < NUM_RULES; i++)
    {
        float val = getValue(rules[i].channel);

        if(checkCondition(val, rules[i].op, rules[i].value))
        {
            executeAction(rules[i].action);
        }
    }
}

void RuleEngine_Init(void)
{
    // load rule ? ?ây
    
}