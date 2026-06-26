/* 
 * File:   eth_mqtt.h
 * Author: LENOVO
 *
 * Created on June 20, 2026, 3:04 PM
 */

#ifndef ETH_MQTT_H
#define	ETH_MQTT_H

#include <stdlib.h>
#include "definitions.h"

#define ETH_MQTT_MAX_SUBS           8

#ifdef	__cplusplus
extern "C" {
#endif

    /* ==========================================================================
     *  Public types (ALL CAPS for typedef enum/struct)
     * ========================================================================== */

    /* Message callback prototype for subscribers */
    typedef void (*ETH_MQTT_MSG_CALLBACK)(const char *topic,
            const uint8_t *payload,
            uint16_t payloadLen);

    /* Top-level state machine states */
    typedef enum {
        ETH_MQTT_STATE_INIT = 0,
        ETH_MQTT_STATE_IDLE,
        ETH_MQTT_STATE_DNS_RESOLVE,
        ETH_MQTT_STATE_NET_CONNECT,
        ETH_MQTT_STATE_NET_WAIT,
        ETH_MQTT_STATE_MQTT_CONNECT,
        ETH_MQTT_STATE_CONNECTED,
        ETH_MQTT_STATE_SUBSCRIBE,
        ETH_MQTT_STATE_UNSUBSCRIBE,
        ETH_MQTT_STATE_PUBLISH,
        ETH_MQTT_STATE_WAIT_MESSAGE,
        ETH_MQTT_STATE_PING,
        ETH_MQTT_STATE_DISCONNECT,
        ETH_MQTT_STATE_ERROR,
        ETH_MQTT_STATE_RECONNECT_WAIT
    } ETH_MQTT_STATES;

    /* ==========================================================================
     *  Public API (prefix EthMqtt_)
     * ========================================================================== */

    /* Module initialization. Must be called once at system start-up. */
    void EthMqtt_Initialize(void);

    /* Module state machine tick. Must be called periodically from SYS_Tasks(). */
    void EthMqtt_Task(void);

    /* Request opening the connection to the broker (non-blocking).
     * Returns true if the request was accepted. */
    bool EthMqtt_Open(void);

    /* Request closing of broker connection (non-blocking). */
    void EthMqtt_Close(void);

    /* Publish a NUL-terminated message to a topic.
     * Returns true if the request was queued. */
    bool EthMqtt_Publish(const char *topic, const char *message);

    /* Subscribe to a topic and register a callback that will be invoked
     * each time a PUBLISH matching the topic arrives.
     * Returns true if the registration was queued. */
    bool EthMqtt_Subscribe(const char *topic, ETH_MQTT_MSG_CALLBACK callback);

    /* Returns true when the MQTT session is fully established. */
    bool EthMqtt_IsConnected(void);

    /* Returns the current state of the internal state machine. */
    ETH_MQTT_STATES EthMqtt_GetState(void);

    /*
     * Unsubscribe from a topic.
     *   topic == NULL  -> unsubscribe from ALL active topics
     *   topic != NULL  -> unsubscribe from the matching topic only
     * Returns true if the request was queued.
     */
    bool EthMqtt_Unsubscribe(const char *topic);


#ifdef	__cplusplus
}
#endif

#endif	/* ETH_MQTT_H */

