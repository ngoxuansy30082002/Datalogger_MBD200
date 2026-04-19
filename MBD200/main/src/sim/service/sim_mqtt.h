/* 
 * File:   sim_mqtt.h
 * Author: LENOVO
 *
 * Created on March 31, 2026, 8:22 PM
 */

#ifndef SIM_MQTT_H
#define	SIM_MQTT_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        SIM_MQTT_IDLE = 0,
        SIM_MQTT_CFG_RECV,
        SIM_MQTT_OPEN,
        SIM_MQTT_CONN,
        SIM_MQTT_SUB,
        SIM_MQTT_PUB_CMD,
        SIM_MQTT_PUB_DATA,
        SIM_MQTT_READY,
        SIM_MQTT_ERROR,
        SIM_MQTT_COUNT
    } SIM_MQTT_STATE;

    bool SIMMqtt_Start(void);
    void SIMMqtt_Process(void);
    void SIMMqtt_Abort(void);
    bool SIMMqtt_IsReady(void);
    bool SIMMqtt_HasError(void);
    bool SIMMqtt_Publish(const char* topic, const char* payload);

#ifdef	__cplusplus
}
#endif

#endif	/* SIM_MQTT_H */

