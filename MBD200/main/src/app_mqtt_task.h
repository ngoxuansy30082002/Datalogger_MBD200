

#ifndef APP_MQTT_TASK_H
#define	APP_MQTT_TASK_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

    typedef enum {
        ETH_MQTT_IDLE = 0,
        ETH_MQTT_INIT,
        ETH_MQTT_NET_CONN,
        ETH_MQTT_CONN,
        ETH_MQTT_SUB,
        ETH_MQTT_PUB,
        ETH_MQTT_READY,
        ETH_MQTT_ERROR,
        ETH_MQTT_COUNT
    } ETH_MQTT_STATE;

    bool ETHMqtt_Start(void);
    void ETHMqtt_Process(void);
    void ETHMqtt_Abort(void);
    bool ETHMqtt_IsReady(void);
    bool ETHMqtt_HasError(void);
    bool ETHMqtt_Publish(const char* topic, const char* payload);

#ifdef	__cplusplus
}
#endif

#endif	/* APP_MQTT_TASK_H */