#include "eth_mqtt.h"

#include "wolfmqtt/mqtt_client.h"
#include "wolfmqtt/mqtt_packet.h"
#include "wolfmqtt/mqtt_socket.h"
#include "wolfmqtt/mqtt_types.h"

/* ============================================================================
 * 1. CONFIGURATION
 * ========================================================================= */

typedef struct {
    uint16_t keepAliveSec;
    bool cleanSession;
    bool useTls;
    uint32_t cmdTimeoutMs;
    uint32_t pingPeriodMs;
    uint32_t reconnectDelayMs;
} ETH_MQTT_CONFIG;

static const ETH_MQTT_CONFIG _config = {
    .keepAliveSec = 60,
    .cleanSession = true,
    .useTls = false,
    .cmdTimeoutMs = 5000,
    .pingPeriodMs = 30000,
    .reconnectDelayMs = 5000
};

/* ============================================================================
 * 2. PRIVATE TYPES
 * ========================================================================= */

typedef struct {
    char topic[MQTT_TOPIC_LEN];
    ETH_MQTT_MSG_CALLBACK callback;
    bool active;
    bool registered;
    bool unsubRequested;
} ETH_MQTT_SUB_ENTRY;

typedef struct {
    char topic[MQTT_TOPIC_LEN];
    uint8_t message[MQTT_PAYLOAD_LEN];
    uint16_t messageLen;
    bool pending;
} ETH_MQTT_PUB_REQ;

typedef struct {
    NET_PRES_SKT_HANDLE_T socket;
    bool connected;
    uint32_t connectStartMs;
} ETH_MQTT_NET_CTX;

/* ============================================================================
 * 3. STATIC BUFFERS AND GLOBALS
 * ========================================================================= */

#define _TX_BUF_SIZE  4096
#define _RX_BUF_SIZE  1024

static const char *__TAG__ = "ETHMQTT";

static uint8_t _txBuf[_TX_BUF_SIZE];
static uint8_t _rxBuf[_RX_BUF_SIZE];

static MqttClient _mqttClient;
static MqttNet _mqttNet;
static ETH_MQTT_NET_CTX _netCtx;

static MqttConnect _mqttConnect;
static MqttPublish _mqttPublish;
static MqttSubscribe _mqttSubscribe;
static MqttUnsubscribe _mqttUnsubscribe;
static MqttTopic _mqttTopics[ETH_MQTT_MAX_SUBS];
static MqttMessage _mqttMsgRx;
static MqttPing _mqttPing;

static ETH_MQTT_SUB_ENTRY _subs[ETH_MQTT_MAX_SUBS];
static ETH_MQTT_PUB_REQ _pubReq;

static IPV4_ADDR _resolvedIp;

static ETH_MQTT_STATES _state;
static uint16_t _packetId;
static uint32_t _stateEnterMs;
static uint32_t _lastPingMs;

static bool _openRequested;
static bool _closeRequested;
static bool _isConnected;
static int _pendingSubIdx;
static int _pendingUnsubIdx;

/* ============================================================================
 * 4. STATIC HELPERS
 * ========================================================================= */

static uint32_t _GetTickMs(void) {
    return (uint32_t) (SYS_TIME_CounterGet() /
            (SYS_TIME_FrequencyGet() / 1000U));
}

static uint16_t _GetPacketId(void) {
    if (++_packetId == 0U) {
        _packetId = 1U;
    }
    return _packetId;
}

static void _SetState(ETH_MQTT_STATES next) {
    _state = next;
    _stateEnterMs = _GetTickMs();
}

static int _FindFreeSubSlot(void) {
    for (int i = 0; i < ETH_MQTT_MAX_SUBS; ++i) {
        if (!_subs[i].active) return i;
    }
    return -1;
}

static int _FindPendingSubIdx(void) {
    for (int i = 0; i < ETH_MQTT_MAX_SUBS; ++i) {
        if (_subs[i].active && !_subs[i].registered) return i;
    }
    return -1;
}

static int _FindPendingUnsubIdx(void) {
    for (int i = 0; i < ETH_MQTT_MAX_SUBS; ++i) {
        if (_subs[i].active && _subs[i].unsubRequested) {
            return i;
        }
    }
    return -1;
}

/* ============================================================================
 * 5. NETWORK CALLBACKS (wired into MqttNet)
 *    Adapt Harmony NET_PRES sockets to wolfMQTT in non-blocking mode.
 * ========================================================================= */


static int _NetConnect(void *context, const char *host, word16 port,
        int timeoutMs) {
    ETH_MQTT_NET_CTX *ctx = (ETH_MQTT_NET_CTX *) context;
    IP_MULTI_ADDRESS addr;
    (void) host;

    addr.v4Add = _resolvedIp;

    if (ctx->socket == INVALID_SOCKET) {
        ctx->socket = NET_PRES_SocketOpen(
                0, NET_PRES_SKT_UNENCRYPTED_STREAM_CLIENT,
                IP_ADDRESS_TYPE_IPV4, port,
                (NET_PRES_ADDRESS *) & addr, NULL);
        if (ctx->socket == INVALID_SOCKET) {
            return MQTT_CODE_ERROR_NETWORK;
        }
        uint16_t tx = _TX_BUF_SIZE, rx = _RX_BUF_SIZE;
        NET_PRES_SocketOptionsSet(ctx->socket, TCP_OPTION_TX_BUFF, (void*) (uintptr_t) tx);
        NET_PRES_SocketOptionsSet(ctx->socket, TCP_OPTION_RX_BUFF, (void*) (uintptr_t) rx);
        ctx->connectStartMs = _GetTickMs();
    }

    if (!NET_PRES_SocketIsConnected(ctx->socket)) {
        if (timeoutMs > 0 &&
                (_GetTickMs() - ctx->connectStartMs) > (uint32_t) timeoutMs) {
            NET_PRES_SocketClose(ctx->socket);
            ctx->socket = INVALID_SOCKET;
            return MQTT_CODE_ERROR_NETWORK;
        }
        return MQTT_CODE_CONTINUE;
    }

    ctx->connected = true;
    return MQTT_CODE_SUCCESS;
}

static int _NetRead(void *context, byte *buf, int bufLen, int timeoutMs) {
    ETH_MQTT_NET_CTX *ctx = (ETH_MQTT_NET_CTX *) context;
    (void) timeoutMs;

    if (!ctx->connected) return MQTT_CODE_ERROR_NETWORK;

    uint16_t avail = NET_PRES_SocketReadIsReady(ctx->socket);
    if (avail == 0U) return MQTT_CODE_CONTINUE;

    uint16_t want = (avail < (uint16_t) bufLen) ? avail : (uint16_t) bufLen;
    int n = NET_PRES_SocketRead(ctx->socket, buf, want);
    if (n <= 0) return MQTT_CODE_ERROR_NETWORK;
    return n;
}

static int _NetWrite(void *context, const byte *buf, int bufLen, int timeoutMs) {
    ETH_MQTT_NET_CTX *ctx = (ETH_MQTT_NET_CTX *) context;
    (void) timeoutMs;

    if (!ctx->connected) return MQTT_CODE_ERROR_NETWORK;

    uint16_t room = NET_PRES_SocketWriteIsReady(ctx->socket,
            (uint16_t) bufLen, 0);
    if (room == 0U) return MQTT_CODE_CONTINUE;

    uint16_t chunk = (room < (uint16_t) bufLen) ? room : (uint16_t) bufLen;
    int n = NET_PRES_SocketWrite(ctx->socket, (uint8_t *) buf, chunk);
    if (n <= 0) return MQTT_CODE_ERROR_NETWORK;
    return n;
}

static int _NetDisconnect(void *context) {
    ETH_MQTT_NET_CTX *ctx = (ETH_MQTT_NET_CTX *) context;
    if (ctx->socket != INVALID_SOCKET) {
        NET_PRES_SocketClose(ctx->socket);
        ctx->socket = INVALID_SOCKET;
    }
    ctx->connected = false;
    return MQTT_CODE_SUCCESS;
}

/* ============================================================================
 * 6. MQTT MESSAGE CALLBACK (dispatcher to per-topic callbacks)
 * ========================================================================= */

static int _MqttMsgCb(MqttClient *client, MqttMessage *msg,
        byte msgNew, byte msgDone) {
    static char topicBuf[MQTT_TOPIC_LEN];
    static uint8_t payloadBuf[MQTT_PAYLOAD_LEN];
    static uint16_t payloadPos;

    (void) client;

    if (msgNew) {
        uint16_t tlen = (msg->topic_name_len < (MQTT_TOPIC_LEN - 1))
                ? msg->topic_name_len
                : (MQTT_TOPIC_LEN - 1);
        memcpy(topicBuf, msg->topic_name, tlen);
        topicBuf[tlen] = '\0';
        payloadPos = 0U;
    }

    /* Accumulate payload chunks */
    uint16_t copy = (uint16_t) msg->buffer_len;
    if ((payloadPos + copy) > MQTT_PAYLOAD_LEN) {
        copy = (uint16_t) (MQTT_PAYLOAD_LEN - payloadPos);
    }
    memcpy(&payloadBuf[payloadPos], msg->buffer, copy);
    payloadPos += copy;

    if (msgDone) {
        for (int i = 0; i < ETH_MQTT_MAX_SUBS; ++i) {
            if (_subs[i].active && _subs[i].callback != NULL &&
                    strcmp(_subs[i].topic, topicBuf) == 0) {
                _subs[i].callback(topicBuf, payloadBuf, payloadPos);
            }
        }
    }
    return MQTT_CODE_SUCCESS;
}

/* ============================================================================
 * 7. PUBLIC API
 * ========================================================================= */

void EthMqtt_Initialize(void) {
    memset(&_mqttClient, 0, sizeof (_mqttClient));
    memset(&_mqttNet, 0, sizeof (_mqttNet));
    memset(&_netCtx, 0, sizeof (_netCtx));
    memset(_subs, 0, sizeof (_subs));
    memset(&_pubReq, 0, sizeof (_pubReq));

    _netCtx.socket = INVALID_SOCKET;
    _netCtx.connected = false;

    _mqttNet.connect = _NetConnect;
    _mqttNet.read = _NetRead;
    _mqttNet.write = _NetWrite;
    _mqttNet.disconnect = _NetDisconnect;
    _mqttNet.context = &_netCtx;

    MqttClient_Init(&_mqttClient, &_mqttNet, _MqttMsgCb,
            _txBuf, _TX_BUF_SIZE,
            _rxBuf, _RX_BUF_SIZE,
            (int) _config.cmdTimeoutMs);

    _packetId = 0U;
    _isConnected = false;
    _openRequested = false;
    _closeRequested = false;
    _pendingSubIdx = -1;
    _pendingUnsubIdx = -1;

    _SetState(ETH_MQTT_STATE_INIT);
}

bool EthMqtt_Open(void) {
    if (_isConnected) return true;
    _openRequested = true;
    return true;
}

void EthMqtt_Close(void) {
    _closeRequested = true;
}

bool EthMqtt_Publish(const char *topic, const char *message) {
    if (topic == NULL || message == NULL) return false;
    if (_pubReq.pending) return false;
    if (strlen(topic) >= MQTT_TOPIC_LEN) return false;
    if (strlen(message) >= MQTT_PAYLOAD_LEN) return false;

    strncpy(_pubReq.topic, topic, MQTT_TOPIC_LEN - 1);
    _pubReq.topic[MQTT_TOPIC_LEN - 1] = '\0';

    _pubReq.messageLen = (uint16_t) strlen(message);
    memcpy(_pubReq.message, message, _pubReq.messageLen);
    _pubReq.pending = true;
    return true;
}

bool EthMqtt_Subscribe(const char *topic, ETH_MQTT_MSG_CALLBACK callback) {
    if (topic == NULL || callback == NULL) return false;
    if (strlen(topic) >= MQTT_TOPIC_LEN) return false;

    int idx = _FindFreeSubSlot();
    if (idx < 0) return false;

    strncpy(_subs[idx].topic, topic, MQTT_TOPIC_LEN - 1);
    _subs[idx].topic[MQTT_TOPIC_LEN - 1] = '\0';
    _subs[idx].callback = callback;
    _subs[idx].active = true;
    _subs[idx].registered = false;
    return true;
}

bool EthMqtt_Unsubscribe(const char *topic) {
    bool queued = false;

    if (topic == NULL) {
        /* Unsubscribe ALL active topics */
        for (int i = 0; i < ETH_MQTT_MAX_SUBS; ++i) {
            if (_subs[i].active) {
                _subs[i].unsubRequested = true;
                queued = true;
            }
        }
        return queued;
    }

    /* Unsubscribe a specific topic */
    if (strlen(topic) >= MQTT_TOPIC_LEN) return false;

    for (int i = 0; i < ETH_MQTT_MAX_SUBS; ++i) {
        if (_subs[i].active && strcmp(_subs[i].topic, topic) == 0) {
            _subs[i].unsubRequested = true;
            return true;
        }
    }
    return false;
}

bool EthMqtt_IsConnected(void) {
    return _isConnected;
}

ETH_MQTT_STATES EthMqtt_GetState(void) {
    return _state;
}

/* ============================================================================
 * 8. STATE MACHINE (non-blocking)
 * ========================================================================= */

void EthMqtt_Task(void) {
    int rc;

    switch (_state) {
        case ETH_MQTT_STATE_INIT:
        {
            _SetState(ETH_MQTT_STATE_IDLE);
            break;
        }

        case ETH_MQTT_STATE_IDLE:
        {
            if (_openRequested) {
                if (!TCPIP_STACK_NetIsReady(TCPIP_STACK_IndexToNet(0))) {
                    break;
                }
                _openRequested = false;
                _SetState(ETH_MQTT_STATE_DNS_RESOLVE);
            }
            break;
        }

        case ETH_MQTT_STATE_DNS_RESOLVE:
        {
            /* Skip DNS if host is already an IP literal */
            if (TCPIP_Helper_StringToIPAddress(gAppCfg.mqtt.host, &_resolvedIp)) {
                LOG_DEBUG("%s - host is IP literal", __TAG__);
                _SetState(ETH_MQTT_STATE_NET_CONNECT);
                break;
            }

            /* Trigger DNS resolution on state entry */
            if ((_GetTickMs() - _stateEnterMs) < 50U) {
                TCPIP_DNS_RESULT dnsRc =
                        TCPIP_DNS_Resolve(gAppCfg.mqtt.host, TCPIP_DNS_TYPE_A);

                if (dnsRc != TCPIP_DNS_RES_OK &&
                        dnsRc != TCPIP_DNS_RES_PENDING &&
                        dnsRc != TCPIP_DNS_RES_NAME_IS_IPADDRESS) {
                    LOG_DEBUG("%s - DNS_Resolve fail rc=%d", __TAG__, dnsRc);
                    _SetState(ETH_MQTT_STATE_ERROR);
                    break;
                }
            }

            IP_MULTI_ADDRESS ipResult;
            TCPIP_DNS_RESULT dnsRc = TCPIP_DNS_IsResolved(
                    gAppCfg.mqtt.host, &ipResult,
                    IP_ADDRESS_TYPE_IPV4);

            if (dnsRc == TCPIP_DNS_RES_PENDING) {
                if ((_GetTickMs() - _stateEnterMs) > 10000U) {
                    LOG_DEBUG("%s - DNS timeout", __TAG__);
                    _SetState(ETH_MQTT_STATE_ERROR);
                }
                break;
            }
            if (dnsRc != TCPIP_DNS_RES_OK) {
                LOG_DEBUG("%s - DNS fail rc=%d", __TAG__, dnsRc);
                _SetState(ETH_MQTT_STATE_ERROR);
                break;
            }

            _resolvedIp = ipResult.v4Add;
            LOG_DEBUG("%s - DNS ok: %d.%d.%d.%d", __TAG__,
                    _resolvedIp.v[0], _resolvedIp.v[1],
                    _resolvedIp.v[2], _resolvedIp.v[3]);
            _SetState(ETH_MQTT_STATE_NET_CONNECT);
            break;
        }

        case ETH_MQTT_STATE_NET_CONNECT:
        {
            rc = MqttClient_NetConnect(&_mqttClient,
                    gAppCfg.mqtt.host,
                    gAppCfg.mqtt.port,
                    (int) _config.cmdTimeoutMs,
                    _config.useTls ? 1 : 0,
                    NULL);


            if (rc == MQTT_CODE_CONTINUE) {
                if ((_GetTickMs() - _stateEnterMs) > 15000U) {
                    LOG_DEBUG("%s - TCP connect timeout", __TAG__);
                    _SetState(ETH_MQTT_STATE_DISCONNECT);
                }
                break;
            }

            if (rc != MQTT_CODE_SUCCESS) {
                LOG_DEBUG("%s - NetConnect ERR rc=%d", __TAG__, rc);
                _SetState(ETH_MQTT_STATE_DISCONNECT);
                break;
            }
            LOG_DEBUG("%s - NetConnect OK", __TAG__);
            _SetState(ETH_MQTT_STATE_MQTT_CONNECT);
            break;
        }

        case ETH_MQTT_STATE_MQTT_CONNECT:
        {
            /* Initialize struct only on the first entry of this state */
            if (_mqttConnect.client_id == NULL) {
                memset(&_mqttConnect, 0, sizeof (_mqttConnect));
                _mqttConnect.keep_alive_sec = _config.keepAliveSec;
                _mqttConnect.clean_session = _config.cleanSession ? 1 : 0;
                _mqttConnect.client_id = gAppCfg.mqtt.clientId;
                _mqttConnect.enable_lwt = 0;

                if (gAppCfg.mqtt.username[0] != '\0') {
                    _mqttConnect.username = gAppCfg.mqtt.username;
                    _mqttConnect.password = gAppCfg.mqtt.password;
                }
            }

            rc = MqttClient_Connect(&_mqttClient, &_mqttConnect);
            if (rc == MQTT_CODE_CONTINUE) break;

            /* Reset marker so the struct will be re-initialized next time */
            _mqttConnect.client_id = NULL;

            if (rc != MQTT_CODE_SUCCESS) {
                LOG_DEBUG("%s - Mqtt connect Error rc=%d (%s)", __TAG__, rc,
                        MqttClient_ReturnCodeToString(rc));
                _SetState(ETH_MQTT_STATE_DISCONNECT);
                break;
            }
            _isConnected = true;
            _lastPingMs = _GetTickMs();
            LOG_DEBUG("%s - Mqtt connect OK", __TAG__);
            _SetState(ETH_MQTT_STATE_CONNECTED);
            break;
        }

        case ETH_MQTT_STATE_CONNECTED:
        {
            if (_closeRequested) {
                _closeRequested = false;
                _SetState(ETH_MQTT_STATE_DISCONNECT);
                break;
            }

            /* Priority: unsubscribe > subscribe > publish > read > ping */
            _pendingUnsubIdx = _FindPendingUnsubIdx();
            if (_pendingUnsubIdx >= 0) {
                _SetState(ETH_MQTT_STATE_UNSUBSCRIBE);
                break;
            }

            _pendingSubIdx = _FindPendingSubIdx();
            if (_pendingSubIdx >= 0) {
                _SetState(ETH_MQTT_STATE_SUBSCRIBE);
                break;
            }

            if (_pubReq.pending) {
                _SetState(ETH_MQTT_STATE_PUBLISH);
                break;
            }

            if ((_GetTickMs() - _lastPingMs) >= _config.pingPeriodMs) {
                _SetState(ETH_MQTT_STATE_PING);
                break;
            }

            _SetState(ETH_MQTT_STATE_WAIT_MESSAGE);
            break;
        }

        case ETH_MQTT_STATE_SUBSCRIBE:
        {
            static bool subStarted = false;
            int idx = _pendingSubIdx;

            if (!subStarted) {
                memset(&_mqttSubscribe, 0, sizeof (_mqttSubscribe));
                memset(&_mqttTopics[0], 0, sizeof (_mqttTopics[0]));
                _mqttTopics[0].topic_filter = _subs[idx].topic;
                if (gAppCfg.mqtt.qos == 2) _mqttTopics[0].qos = MQTT_QOS_2;
                else if (gAppCfg.mqtt.qos == 1) _mqttTopics[0].qos = MQTT_QOS_1;
                else _mqttTopics[0].qos = MQTT_QOS_0;
                _mqttSubscribe.packet_id = _GetPacketId();
                _mqttSubscribe.topic_count = 1;
                _mqttSubscribe.topics = &_mqttTopics[0];
                subStarted = true;
            }

            rc = MqttClient_Subscribe(&_mqttClient, &_mqttSubscribe);
            if (rc == MQTT_CODE_CONTINUE) break;

            subStarted = false;
            if (rc == MQTT_CODE_SUCCESS) {
                _subs[idx].registered = true;
            } else {
                /* Mark inactive to avoid endless retry on this slot */
                _subs[idx].active = false;
            }
            _SetState(ETH_MQTT_STATE_CONNECTED);
            break;
        }

        case ETH_MQTT_STATE_UNSUBSCRIBE:
        {
            static bool unsubStarted = false;
            int idx = _pendingUnsubIdx;

            if (!unsubStarted) {
                memset(&_mqttUnsubscribe, 0, sizeof (_mqttUnsubscribe));
                memset(&_mqttTopics[0], 0, sizeof (_mqttTopics[0]));
                _mqttTopics[0].topic_filter = _subs[idx].topic;
                _mqttUnsubscribe.packet_id = _GetPacketId();
                _mqttUnsubscribe.topic_count = 1;
                _mqttUnsubscribe.topics = &_mqttTopics[0];
                unsubStarted = true;
            }

            rc = MqttClient_Unsubscribe(&_mqttClient, &_mqttUnsubscribe);
            if (rc == MQTT_CODE_CONTINUE) break;

            unsubStarted = false;

            if (rc == MQTT_CODE_SUCCESS) {
                LOG_DEBUG("%s - Unsubscribe OK: %s", __TAG__, _subs[idx].topic);
                /* Free the slot completely */
                _subs[idx].active = false;
                _subs[idx].registered = false;
                _subs[idx].unsubRequested = false;
                _subs[idx].callback = NULL;
                _subs[idx].topic[0] = '\0';
            } else {
                LOG_DEBUG("%s - Unsubscribe ERR rc=%d", __TAG__, rc);
                /* Keep the slot active; the request will be retried next tick */
                _SetState(ETH_MQTT_STATE_DISCONNECT);
                break;
            }
            _SetState(ETH_MQTT_STATE_CONNECTED);
            break;
        }

        case ETH_MQTT_STATE_PUBLISH:
        {
            static bool pubStarted = false;
            static uint32_t pubStartedMs = 0;

            if (!pubStarted) {
                memset(&_mqttPublish, 0, sizeof (_mqttPublish));
                _mqttPublish.retain = 0;
                if (gAppCfg.mqtt.qos == 2) _mqttTopics[0].qos = MQTT_QOS_2;
                else if (gAppCfg.mqtt.qos == 1) _mqttTopics[0].qos = MQTT_QOS_1;
                else _mqttTopics[0].qos = MQTT_QOS_0;
                _mqttPublish.duplicate = 0;
                _mqttPublish.topic_name = _pubReq.topic;
                _mqttPublish.packet_id = _GetPacketId();
                _mqttPublish.buffer = _pubReq.message;
                _mqttPublish.total_len = _pubReq.messageLen;
                pubStarted = true;
                pubStartedMs = _GetTickMs();
            }

            rc = MqttClient_Publish(&_mqttClient, &_mqttPublish);

            if (rc == MQTT_CODE_CONTINUE) {
                /* Safety net: never hang here forever */
                if ((_GetTickMs() - pubStartedMs) > _config.cmdTimeoutMs * 2U) {
                    LOG_DEBUG("%s - Publish stuck > %ums, force abort",
                            __TAG__, _config.cmdTimeoutMs * 2U);
                    pubStarted = false;
                    _pubReq.pending = false; /* CRITICAL: clear flag */
                    _SetState(ETH_MQTT_STATE_DISCONNECT);
                }
                break;
            }

            /* Whatever the outcome, clear the request and the marker */
            pubStarted = false;
            _pubReq.pending = false;
            _lastPingMs = _GetTickMs();

            if (rc != MQTT_CODE_SUCCESS) {
                LOG_DEBUG("%s - Publish ERR rc=%d (%s)", __TAG__, rc,
                        MqttClient_ReturnCodeToString(rc));
                _SetState(ETH_MQTT_STATE_DISCONNECT);
            } else {
                _SetState(ETH_MQTT_STATE_CONNECTED);
            }
            break;
        }

        case ETH_MQTT_STATE_WAIT_MESSAGE:
        {
            rc = MqttClient_WaitMessage_ex(&_mqttClient,
                    (MqttObject *) & _mqttMsgRx, 50);

            if (rc == MQTT_CODE_CONTINUE || rc == MQTT_CODE_ERROR_TIMEOUT) {
                _SetState(ETH_MQTT_STATE_CONNECTED);
                break;
            }
            if (rc != MQTT_CODE_SUCCESS) {
                _SetState(ETH_MQTT_STATE_DISCONNECT);
                break;
            }
            _SetState(ETH_MQTT_STATE_CONNECTED);
            break;
        }

        case ETH_MQTT_STATE_PING:
        {
            static bool pingStarted = false;

            if (!pingStarted) {
                memset(&_mqttPing, 0, sizeof (_mqttPing));
                pingStarted = true;
            }

            rc = MqttClient_Ping_ex(&_mqttClient, &_mqttPing);
            if (rc == MQTT_CODE_CONTINUE) break;

            pingStarted = false;
            _lastPingMs = _GetTickMs();

            if (rc != MQTT_CODE_SUCCESS) {
                LOG_DEBUG("%s - Ping ERR rc=%d", __TAG__, rc);
                _SetState(ETH_MQTT_STATE_DISCONNECT);
            } else {
                _SetState(ETH_MQTT_STATE_CONNECTED);
            }
            break;
        }

        case ETH_MQTT_STATE_DISCONNECT:
        {
            (void) MqttClient_Disconnect(&_mqttClient);
            (void) MqttClient_NetDisconnect(&_mqttClient);
            _isConnected = false;

            for (int i = 0; i < ETH_MQTT_MAX_SUBS; ++i) {
                if (_subs[i].active) {
                    _subs[i].registered = false;

                    /* If user requested unsubscribe before disconnect,
                     * free the slot now since broker will drop session anyway. */
                    if (_subs[i].unsubRequested && _config.cleanSession) {
                        _subs[i].active = false;
                        _subs[i].unsubRequested = false;
                        _subs[i].callback = NULL;
                        _subs[i].topic[0] = '\0';
                    }
                }
            }
            _SetState(ETH_MQTT_STATE_RECONNECT_WAIT);
            break;
        }

        case ETH_MQTT_STATE_RECONNECT_WAIT:
        {
            if ((_GetTickMs() - _stateEnterMs) >= _config.reconnectDelayMs) {
                _SetState(ETH_MQTT_STATE_DNS_RESOLVE); // <-- ??i t? NET_CONNECT
            }
            break;
        }

        case ETH_MQTT_STATE_ERROR:
        default:
        {
            _SetState(ETH_MQTT_STATE_DISCONNECT);
            break;
        }
    }
}