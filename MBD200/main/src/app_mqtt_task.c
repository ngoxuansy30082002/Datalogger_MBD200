#include "app_mqtt_task.h"
#include "definitions.h"
#include "mqtt_net_glue.h"         
#include "wolfmqtt/mqtt_client.h"  

typedef struct {
    const char* host;
    uint16_t port;
    const char* client_id;
    const char* username;
    const char* password;
} ETH_MQTT_CONFIG_T;

static ETH_MQTT_CONFIG_T _ethMqttCfg = {
    .host = "35.172.255.228",
    .port = 1883,
    .client_id = "mbd200_eth_client",
    .username = "",
    .password = ""
};

#define MQTT_TX_BUF_SIZE 1024
#define MQTT_RX_BUF_SIZE 1024

static MqttClient    _mqttClient;
static MqttNet       _mqttNet;
static uint8_t       _txBuf[MQTT_TX_BUF_SIZE];
static uint8_t       _rxBuf[MQTT_RX_BUF_SIZE];

static MqttConnect   _mqttConnect;
static MqttSubscribe _mqttSubscribe;
static MqttTopic     _mqttTopics[1];
static MqttPing      _mqttPing;
static MqttPublish   _mqttPublish;

static ETH_MQTT_STATE _currentState = ETH_MQTT_IDLE;
static bool _isDisconnected = true;
static uint32_t _tmoTick = 0;

static char _pubTopic[64];
static char _pubPayload[256];

static const char * __TAG__ = "ETHMQTT";

static int _mqtt_rx_cb(MqttClient *client, MqttMessage *msg, byte msg_new, byte msg_done) {
    if (msg_new) {
        SYS_CONSOLE_PRINT("\r\n%s - %s:\t INCOMING TOPIC: %s\r\n", __TAG__, __func__, msg->topic_name);
    }
    SYS_CONSOLE_PRINT("%s - %s:\t PAYLOAD: %.*s\r\n", __TAG__, __func__, msg->buffer_len, msg->buffer);
    
    return MQTT_CODE_SUCCESS;
}

bool ETHMqtt_Start(void) {
    if (_currentState > ETH_MQTT_IDLE)
        return false;

    _currentState = ETH_MQTT_INIT;
    _isDisconnected = true;
    return true;
}

void ETHMqtt_Abort(void) {
    if (!_isDisconnected) {
        MqttClient_NetDisconnect(&_mqttClient);
        _isDisconnected = true;
    }
    _currentState = ETH_MQTT_IDLE;
}

bool ETHMqtt_IsReady(void) {
    return (_currentState == ETH_MQTT_READY);
}

bool ETHMqtt_HasError(void) {
    return (_currentState == ETH_MQTT_ERROR);
}

bool ETHMqtt_Publish(const char* topic, const char* payload) {
    if (_currentState != ETH_MQTT_READY) {
        SYS_CONSOLE_PRINT("%s - %s:\t Error: MQTT not ready for publish!\r\n", __TAG__, __func__);
        return false;
    }

    strncpy(_pubTopic, topic, sizeof(_pubTopic) - 1);
    strncpy(_pubPayload, payload, sizeof(_pubPayload) - 1);

    XMEMSET(&_mqttPublish, 0, sizeof(MqttPublish));
    _mqttPublish.retain = 0;          
    _mqttPublish.qos = MQTT_QOS_0;    
    _mqttPublish.duplicate = 0;
    _mqttPublish.topic_name = _pubTopic;
    _mqttPublish.buffer = (byte*)_pubPayload;
    _mqttPublish.total_len = (word32)strlen(_pubPayload);

    _currentState = ETH_MQTT_PUB;
    return true;
}

void ETHMqtt_Process(void) {
    if (_currentState == ETH_MQTT_IDLE || _currentState == ETH_MQTT_ERROR) {
        return;
    }

    int rc = 0;

    switch (_currentState) {
        
        case ETH_MQTT_INIT:
        {
            TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(0);
            if (netH != NULL && TCPIP_STACK_NetIsReady(netH) && TCPIP_STACK_NetAddress(netH) != 0) {
                SYS_CONSOLE_PRINT("%s - %s:\t LAN OK, Init wolfMQTT\r\n", __TAG__, __func__);
                
                WMQTT_NETGlue_Initialize(&_mqttNet);
                rc = MqttClient_Init(&_mqttClient, &_mqttNet, _mqtt_rx_cb, 
                                     _txBuf, MQTT_TX_BUF_SIZE, _rxBuf, MQTT_RX_BUF_SIZE, 5000);
                
                if (rc == MQTT_CODE_SUCCESS) {
                    _currentState = ETH_MQTT_NET_CONN;
                } else {
                    SYS_CONSOLE_PRINT("%s - %s:\t Init Error %d\r\n", __TAG__, __func__, rc);
                    _currentState = ETH_MQTT_ERROR;
                }
            }
            break;
        }

        case ETH_MQTT_NET_CONN:
            rc = MqttClient_NetConnect(&_mqttClient, _ethMqttCfg.host, _ethMqttCfg.port, 10000, 0, NULL);
            if (rc == MQTT_CODE_SUCCESS) {
                SYS_CONSOLE_PRINT("%s - %s:\t TCP Socket OK. Logging in...\r\n", __TAG__, __func__);
                
                XMEMSET(&_mqttConnect, 0, sizeof(MqttConnect));
                _mqttConnect.keep_alive_sec = 60;
                _mqttConnect.clean_session = 1;
                _mqttConnect.client_id = _ethMqttCfg.client_id; 
                
                if (strlen(_ethMqttCfg.username) > 0) {
                    _mqttConnect.username = _ethMqttCfg.username;
                    _mqttConnect.password = _ethMqttCfg.password;
                }
                
                _isDisconnected = false;
                _currentState = ETH_MQTT_CONN; 
            } 
            else if (rc != MQTT_CODE_CONTINUE) {
                SYS_CONSOLE_PRINT("%s - %s:\t NetConnect Error: %d\r\n", __TAG__, __func__, rc);
                _currentState = ETH_MQTT_ERROR; 
            }
            break;

        case ETH_MQTT_CONN:
            rc = MqttClient_Connect(&_mqttClient, &_mqttConnect);
            if (rc == MQTT_CODE_SUCCESS) {
                SYS_CONSOLE_PRINT("%s - %s:\t Login MQTT OK. Subscribing...\r\n", __TAG__, __func__);
                
                XMEMSET(&_mqttSubscribe, 0, sizeof(MqttSubscribe));
                _mqttTopics[0].topic_filter = "/test"; //Change topic
                _mqttTopics[0].qos = MQTT_QOS_0;
                _mqttSubscribe.packet_id = 1;
                _mqttSubscribe.topic_count = 1;
                _mqttSubscribe.topics = _mqttTopics;

                _currentState = ETH_MQTT_SUB;
            } 
            else if (rc != MQTT_CODE_CONTINUE) {
                SYS_CONSOLE_PRINT("%s - %s:\t Connect Error: %d\r\n", __TAG__, __func__, rc);
                _currentState = ETH_MQTT_ERROR;
            }
            break;

        case ETH_MQTT_SUB:
            rc = MqttClient_Subscribe(&_mqttClient, &_mqttSubscribe);
            if (rc == MQTT_CODE_SUCCESS) {
                SYS_CONSOLE_PRINT("%s - %s:\t Subscribe OK. Ready!\r\n", __TAG__, __func__);
                _tmoTick = SYS_TMR_TickCountGet(); 
                _currentState = ETH_MQTT_READY;
            } 
            else if (rc != MQTT_CODE_CONTINUE) {
                SYS_CONSOLE_PRINT("%s - %s:\t Subscribe Error: %d\r\n", __TAG__, __func__, rc);
                _currentState = ETH_MQTT_ERROR;
            }
            break;

        case ETH_MQTT_READY:
        {
            uint32_t curTick = SYS_TMR_TickCountGet();
            
            if (MqttClient_IsMessageActive(&_mqttClient, (MqttObject*)&_mqttPing)) {
                rc = MqttClient_Ping_ex(&_mqttClient, &_mqttPing);
                if (rc != MQTT_CODE_SUCCESS && rc != MQTT_CODE_CONTINUE) {
                    SYS_CONSOLE_PRINT("%s - %s:\t Ping Error/Disconnect!\r\n", __TAG__, __func__);
                    _currentState = ETH_MQTT_ERROR;
                } else if (rc == MQTT_CODE_SUCCESS) {
                    _tmoTick = curTick; 
                }
            } 
            else if (curTick - _tmoTick >= (SYS_TMR_TickCounterFrequencyGet() * 30)) {
                XMEMSET(&_mqttPing, 0, sizeof(MqttPing));
                MqttClient_Ping_ex(&_mqttClient, &_mqttPing); 
                _tmoTick = curTick;
            }

            rc = MqttClient_WaitMessage(&_mqttClient, 0); 
            if (rc == MQTT_CODE_ERROR_NETWORK || rc == MQTT_CODE_ERROR_TIMEOUT) {
                SYS_CONSOLE_PRINT("%s - %s:\t Connection Lost!\r\n", __TAG__, __func__);
                _currentState = ETH_MQTT_ERROR;
            }
            break;
        }

        case ETH_MQTT_PUB:
            rc = MqttClient_Publish(&_mqttClient, &_mqttPublish);
            if (rc == MQTT_CODE_SUCCESS) {
                SYS_CONSOLE_PRINT("%s - %s:\t Publish OK\r\n", __TAG__, __func__);
                _currentState = ETH_MQTT_READY; 
            } 
            else if (rc != MQTT_CODE_CONTINUE) {
                SYS_CONSOLE_PRINT("%s - %s:\t Publish Error: %d\r\n", __TAG__, __func__, rc);
                _currentState = ETH_MQTT_ERROR; 
            }
            break;

        default:
            break;
    }
}

//ETHMqtt_Start();
//
//    uint32_t lastPubTick = SYS_TMR_TickCountGet();
//    uint32_t errorTick = 0;
//    bool isErrorState = false;
//
//    while ( true )
//    {
//        // Nuôi lõi h? th?ng
//        SYS_Tasks ( );
//
//        // Nuôi máy tr?ng thái MQTT
//        ETHMqtt_Process();
//
//        // LOGIC 1: ?ang k?t n?i t?t -> 10 giây b?n data 1 l?n
//        if (ETHMqtt_IsReady()) {
//            uint32_t currentTick = SYS_TMR_TickCountGet();
//            
//            if (currentTick - lastPubTick >= (SYS_TMR_TickCounterFrequencyGet() * 10)) {
//                ETHMqtt_Publish("/test", "{\"device\":\"PIC32\", \"status\":\"ETH_MQTT_OK\"}");
//                lastPubTick = currentTick;
//            }
//            isErrorState = false;
//        } 
//        
//        // LOGIC 2: B? r?t m?ng -> Ch?n l?i, g? socket, b?t ??u ??m 5 giây
//        else if (ETHMqtt_HasError() && !isErrorState) {
//            ETHMqtt_Abort(); // L?nh này s? set State v? IDLE
//            errorTick = SYS_TMR_TickCountGet();
//            isErrorState = true;
//        }
//
//        // LOGIC 3: H?t 5 giây ch? -> Kh?i ??ng l?i vòng l?p m?ng
//        if (isErrorState) {
//            if (SYS_TMR_TickCountGet() - errorTick >= (SYS_TMR_TickCounterFrequencyGet() * 5)) {
//                ETHMqtt_Start();
//                isErrorState = false;
//                lastPubTick = SYS_TMR_TickCountGet(); // Reset ??ng h? ?? không b? pub g?p
//            }
//        }
//    }