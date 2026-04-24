#ifndef TCP_MQTT_H
#define	TCP_MQTT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        TCP_MQTT_IDLE = 0,
        TCP_MQTT_RESOLVE_DNS,
        TCP_MQTT_WAIT_DNS,
        TCP_MQTT_OPEN_SOCKET,
        TCP_MQTT_WAIT_TCP_CONN,
        TCP_MQTT_SEND_CONNECT,
        TCP_MQTT_WAIT_CONNACK,
        TCP_MQTT_SEND_SUB,
        TCP_MQTT_WAIT_SUBACK,
        TCP_MQTT_READY,
        TCP_MQTT_PUBLISHING,
        TCP_MQTT_ERROR
    } TCP_MQTT_STATE;

    bool TCPMqtt_Start(void);
    void TCPMqtt_Process(void);
    void TCPMqtt_Abort(void);
    bool TCPMqtt_IsReady(void);
    bool TCPMqtt_HasError(void);
    bool TCPMqtt_Publish(const char* topic, const char* payload);

#ifdef	__cplusplus
}
#endif

#endif