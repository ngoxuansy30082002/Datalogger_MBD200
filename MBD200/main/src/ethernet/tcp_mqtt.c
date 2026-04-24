#include "tcp_mqtt.h"
#include "definitions.h"
#include "tcpip/tcpip.h"
#include <string.h>

static const char * __TAG__ = "TCPMQTT";

typedef struct {
    uint8_t client_idx;
    const char* host;
    uint16_t port;
    const char* client_id;
    const char* username;
    const char* password;
} MQTT_CONFIG_T;

static MQTT_CONFIG_T _mqttCfg = {
    .client_idx = 0,
    .host = "44.232.241.40",  // ÉP C?NG IP VÀO ?ÂY
    .port = 1883,
    .client_id = "clientExample",
    .username = "user123",
    .password = "pass123"
};

static TCP_MQTT_STATE _currentState = TCP_MQTT_IDLE;
static TCP_SOCKET _socket = INVALID_SOCKET;
static IP_MULTI_ADDRESS _resolvedIp;
static uint32_t _stateTimer = 0;

static uint8_t _txBuf[512]; 
static char _pubTopic[64];
static char _pubPayload[256];

static int _putString(uint8_t* buf, int idx, const char* str) {
    int len = strlen(str);
    buf[idx++] = (len >> 8) & 0xFF;
    buf[idx++] = len & 0xFF;
    memcpy(&buf[idx], str, len);
    return idx + len;
}

static int _encodeLength(uint8_t* buf, int idx, uint32_t len) {
    do {
        uint8_t encodedByte = len % 128;
        len = len / 128;
        if (len > 0) encodedByte |= 128;
        buf[idx++] = encodedByte;
    } while (len > 0);
    return idx;
}

static void _changeState(TCP_MQTT_STATE newState) {
    _currentState = newState;
    _stateTimer = SYS_TMR_TickCountGet();
}

static bool _checkTimeout(uint32_t timeoutMs) {
    uint32_t curTick = SYS_TMR_TickCountGet();
    uint32_t passedMs = (curTick - _stateTimer) / (SYS_TMR_TickCounterFrequencyGet() / 1000);
    return (passedMs >= timeoutMs);
}

bool TCPMqtt_Start(void) {
    if (_currentState > TCP_MQTT_IDLE && _currentState != TCP_MQTT_ERROR) return false;
    
    // NH?Y CÓC B? QUA DNS: D?ch IP tr?c ti?p t? chu?i luôn!
    TCPIP_Helper_StringToIPAddress(_mqttCfg.host, &_resolvedIp.v4Add);
    
    // Nh?y th?ng t?i tr?m m? Socket
    _changeState(TCP_MQTT_OPEN_SOCKET); 
    return true;
}

void TCPMqtt_Abort(void) {
    if (_socket != INVALID_SOCKET) {
        TCPIP_TCP_Close(_socket);
        _socket = INVALID_SOCKET;
    }
    _currentState = TCP_MQTT_IDLE;
}

bool TCPMqtt_IsReady(void) {
    return (_currentState == TCP_MQTT_READY);
}

bool TCPMqtt_HasError(void) {
    return (_currentState == TCP_MQTT_ERROR);
}

bool TCPMqtt_Publish(const char* topic, const char* payload) {
    if (_currentState != TCP_MQTT_READY) return false;
    strncpy(_pubTopic, topic, sizeof(_pubTopic) - 1);
    strncpy(_pubPayload, payload, sizeof(_pubPayload) - 1);
    _changeState(TCP_MQTT_PUBLISHING);
    return true;
}

void TCPMqtt_Process(void) {
    switch (_currentState) {
        case TCP_MQTT_IDLE:
        case TCP_MQTT_ERROR:
            break;

        case TCP_MQTT_RESOLVE_DNS:
        case TCP_MQTT_WAIT_DNS:
            // Hai tr?m này gi? b? ph? võ công, m?ch không thèm ch?y vào ?ây n?a.
            break;

        case TCP_MQTT_OPEN_SOCKET:
        {
            IP_MULTI_ADDRESS brokerAddr;
            brokerAddr.v4Add.Val = _resolvedIp.v4Add.Val; 
            
            _socket = TCPIP_TCP_ClientOpen(IP_ADDRESS_TYPE_IPV4, _mqttCfg.port, &brokerAddr);
            
            if (_socket != INVALID_SOCKET) {
                SYS_CONSOLE_PRINT("%s: Mo Socket OK. Dang ket noi Server...\r\n", __TAG__);
                _changeState(TCP_MQTT_WAIT_TCP_CONN);
            } else {
                SYS_CONSOLE_PRINT("%s: Het bo nho RAM de mo Socket!\r\n", __TAG__);
                _currentState = TCP_MQTT_ERROR;
            }
            break;
        }

        case TCP_MQTT_WAIT_TCP_CONN:
            if (TCPIP_TCP_IsConnected(_socket)) {
                _changeState(TCP_MQTT_SEND_CONNECT);
            } 
            else if (TCPIP_TCP_WasDisconnected(_socket) || _checkTimeout(15000)) {
                SYS_CONSOLE_PRINT("%s: TCP Connect FAILED / TIMEOUT!\r\n", __TAG__);
                TCPIP_TCP_Close(_socket);
                _currentState = TCP_MQTT_ERROR;
            }
            break;

        case TCP_MQTT_SEND_CONNECT:
        {
            uint32_t remLen = 10 + (2 + strlen(_mqttCfg.client_id)) + (2 + strlen(_mqttCfg.username)) + (2 + strlen(_mqttCfg.password));
            int idx = 0;
            _txBuf[idx++] = 0x10; 
            idx = _encodeLength(_txBuf, idx, remLen);
            const uint8_t varHeader[] = {0x00, 0x04, 'M', 'Q', 'T', 'T', 0x04, 0xC2, 0x00, 0x3C};
            memcpy(&_txBuf[idx], varHeader, 10);
            idx += 10;
            idx = _putString(_txBuf, idx, _mqttCfg.client_id);
            idx = _putString(_txBuf, idx, _mqttCfg.username);
            idx = _putString(_txBuf, idx, _mqttCfg.password);

            TCPIP_TCP_ArrayPut(_socket, _txBuf, idx);
            TCPIP_TCP_Flush(_socket);
            
            SYS_CONSOLE_PRINT("%s: Gui MQTT CONNECT. Cho phan hoi...\r\n", __TAG__);
            _changeState(TCP_MQTT_WAIT_CONNACK);
            break;
        }

        case TCP_MQTT_WAIT_CONNACK:
        {
            uint16_t rxLen = TCPIP_TCP_GetIsReady(_socket);
            if (rxLen >= 4) {
                uint8_t rxBuf[4];
                TCPIP_TCP_ArrayGet(_socket, rxBuf, 4);
                if (rxBuf[0] == 0x20 && rxBuf[3] == 0x00) {
                    SYS_CONSOLE_PRINT("\r\n=================================\r\n");
                    SYS_CONSOLE_PRINT("%s: LOGIN SUCCESS! Dang Subscribe...\r\n", __TAG__);
                    SYS_CONSOLE_PRINT("=================================\r\n\r\n");
                    _changeState(TCP_MQTT_SEND_SUB);
                } else {
                    SYS_CONSOLE_PRINT("%s: LOGIN DENIED!\r\n", __TAG__);
                    _currentState = TCP_MQTT_ERROR;
                }
            } else if (_checkTimeout(15000)) {
                SYS_CONSOLE_PRINT("%s: Khong nhan duoc CONNACK!\r\n", __TAG__);
                _currentState = TCP_MQTT_ERROR;
            }
            break;
        }

        case TCP_MQTT_SEND_SUB:
        {
            const char* subTopic = "/test";
            int idx = 0;
            _txBuf[idx++] = 0x82;
            idx = _encodeLength(_txBuf, idx, 2 + (2 + strlen(subTopic)) + 1);
            
            _txBuf[idx++] = 0x00; _txBuf[idx++] = 0x01;
            idx = _putString(_txBuf, idx, subTopic);
            _txBuf[idx++] = 0x01;

            TCPIP_TCP_ArrayPut(_socket, _txBuf, idx);
            TCPIP_TCP_Flush(_socket);
            _changeState(TCP_MQTT_WAIT_SUBACK);
            break;
        }

        case TCP_MQTT_WAIT_SUBACK:
        {
            uint16_t rxLen = TCPIP_TCP_GetIsReady(_socket);
            if (rxLen >= 5) {
                uint8_t rxBuf[5];
                TCPIP_TCP_ArrayGet(_socket, rxBuf, 5);
                SYS_CONSOLE_PRINT("%s: VAO MANG & SUBSCRIBE OK!\r\n", __TAG__);
                _changeState(TCP_MQTT_READY);
            } else if (_checkTimeout(15000)) {
                _currentState = TCP_MQTT_ERROR;
            }
            break;
        }

        case TCP_MQTT_READY:
            if (!TCPIP_TCP_IsConnected(_socket)) {
                SYS_CONSOLE_PRINT("%s: TCP MAT KET NOI! Resetting...\r\n", __TAG__);
                _socket = INVALID_SOCKET;
                _currentState = TCP_MQTT_ERROR;
            }
            
            uint16_t rxLen = TCPIP_TCP_GetIsReady(_socket);
            if (rxLen > 0) {
                uint8_t rxBuf[256];
                uint16_t readLen = rxLen < 256 ? rxLen : 256;
                TCPIP_TCP_ArrayGet(_socket, rxBuf, readLen);
            }
            break;

        case TCP_MQTT_PUBLISHING:
        {
            uint32_t payloadLen = strlen(_pubPayload);
            uint32_t remLen = (2 + strlen(_pubTopic)) + payloadLen;

            int idx = 0;
            _txBuf[idx++] = 0x30;
            idx = _encodeLength(_txBuf, idx, remLen);
            idx = _putString(_txBuf, idx, _pubTopic);
            memcpy(&_txBuf[idx], _pubPayload, payloadLen);
            idx += payloadLen;

            TCPIP_TCP_ArrayPut(_socket, _txBuf, idx);
            TCPIP_TCP_Flush(_socket);

            SYS_CONSOLE_PRINT("%s: Published -> %s\r\n", __TAG__, _pubPayload);
            _changeState(TCP_MQTT_READY);
            break;
        }
    }
}