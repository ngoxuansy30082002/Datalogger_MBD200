#include "fota.h"
#include "bootloader/bootloader_nvm_interface.h"

/* ================================================================ */
/* Internal context                                                  */

/* ================================================================ */

typedef struct {
    /* ---- Config / identity ---- */
    char deviceId [FOTA_DEVICE_ID_MAX_LEN];
    char fwVersion [FOTA_FW_VERSION_MAX_LEN];
    char hwVersion [FOTA_HW_VERSION_MAX_LEN];
    char deviceModel [FOTA_FW_VERSION_MAX_LEN];

    /* ---- Topics ---- */
    char topicHeartbeat [FOTA_TOPIC_MAX_LEN];
    char topicQuery [FOTA_TOPIC_MAX_LEN];
    char topicNotify [FOTA_TOPIC_MAX_LEN];
    char topicResponse [FOTA_TOPIC_MAX_LEN];

    /* ---- FSM ---- */
    FOTA_STATE state;
    uint32_t tStateEntry; /* tick at last state change       */
    uint32_t tHeartbeat; /* last successful heartbeat tick  */
    bool subscribed; /* MQTT subscriptions installed    */

    /* ---- Update target derived from MQTT response ---- */
    char latestVersion [FOTA_FW_VERSION_MAX_LEN];
    char dlHost [FOTA_HOST_MAX_LEN];
    uint16_t dlPort;
    char dlPath [FOTA_PATH_MAX_LEN];
    bool updatePending;

    /* ---- HTTP / auth ---- */
    char jwt [FOTA_TOKEN_MAX_LEN];
    uint8_t httpRxBuf [512]; /* small buffer for JSON */
    uint16_t httpRxLen;

    /* ---- Firmware streaming / hex parser ---- */
    bool flashErased;
    bool programmingFailed;
    CRYPT_MD5_CTX md5Ctx;
    uint8_t md5Digest [16];
    char hexLine [FOTA_HEX_LINE_MAX_LEN];
    uint16_t hexLineLen;
    uint32_t bytesProgrammed;

    /* ---- External trigger ---- */
    bool forceCheck;
} _FOTA_CTX;

static const char * __TAG__ = "FOTA";
static _FOTA_CTX _fota;

/* Reusable JSON output buffer (kept off the stack to spare it).     */
static char _fotaJsonBuf[256];

/* ================================================================ */
/* Helpers                                                           */

/* ================================================================ */

static void _Fota_GotoState(FOTA_STATE st) {
    _fota.state = st;
    _fota.tStateEntry = TICK_NOW();
}

/* Locate "key":"value" or "key":value inside a flat JSON object.
   Output is NUL-terminated and limited to outSize. Returns true on
   success. Numbers and booleans are returned as plain text.            */
static bool _Fota_JsonExtract(const char *json, const char *key,
        char *out, uint16_t outSize) {
    char pattern[48];
    (void) snprintf(pattern, sizeof (pattern), "\"%s\"", key);

    const char *p = strstr(json, pattern);
    if (p == NULL) {
        return false;
    }

    p = strchr(p + strlen(pattern), ':');
    if (p == NULL) {
        return false;
    }
    p++;
    while (*p == ' ' || *p == '\t') {
        p++;
    }

    const char *end;
    if (*p == '"') {
        p++;
        end = strchr(p, '"');
        if (end == NULL) {
            return false;
        }
    } else {
        end = p;
        while (*end != '\0' && *end != ',' && *end != '}' &&
                *end != ' ' && *end != '\r' && *end != '\n') {
            end++;
        }
    }
    uint16_t len = (uint16_t) (end - p);
    if (len >= outSize) {
        len = outSize - 1U;
    }
    (void) memcpy(out, p, len);
    out[len] = '\0';
    return true;
}

/* Split "host[:port]/path" (no scheme) into host, port and path.       */
static bool _Fota_ParseUrl(const char *url,
        char *host, uint16_t hostSize,
        uint16_t *port,
        char *path, uint16_t pathSize) {
    const char *p = url;
    if (strncmp(p, "http://", 7) == 0) {
        p += 7;
    }

    const char *slash = strchr(p, '/');
    const char *colon = strchr(p, ':');
    const char *hostEnd;

    if (colon != NULL && (slash == NULL || colon < slash)) {
        hostEnd = colon;
        *port = (uint16_t) atoi(colon + 1);
    } else {
        hostEnd = (slash != NULL) ? slash : (p + strlen(p));
        *port = ETH_HTTP_DEFAULT_PORT;
    }

    uint16_t hl = (uint16_t) (hostEnd - p);
    if (hl == 0U || hl >= hostSize) {
        return false;
    }
    (void) memcpy(host, p, hl);
    host[hl] = '\0';

    if (slash != NULL) {
        (void) strncpy(path, slash, pathSize - 1U);
        path[pathSize - 1U] = '\0';
    } else {
        (void) strncpy(path, "/", pathSize - 1U);
        path[pathSize - 1U] = '\0';
    }
    return true;
}

/* Convert two ASCII hex chars to a byte (no validation).               */
static uint8_t _Fota_HexFromChars(char hi, char lo) {
    return (uint8_t) (Helpers_HexFromChars((uint8_t) hi, (uint8_t) lo));
}

/* ================================================================ */
/* Heartbeat & query payload builders                                */
/* ================================================================ */

/* Caller passes a fake timestamp string; the device clock layer can
   replace this with a real ISO-8601 timestamp later.                   */
static void _Fota_NowIso8601(char *out, uint16_t outSize) {
    /* Plug your RTC here. For now a placeholder. */
    (void) snprintf(out, outSize, "1970-01-01T00:00:00Z");
}

static uint16_t _Fota_BuildHeartbeatJson(char *buf, uint16_t bufSize) {
    char ts[32];
    _Fota_NowIso8601(ts, sizeof (ts));
    int n = snprintf(buf, bufSize,
            "{"
            "\"deviceId\":\"%s\","
            "\"firmwareVersion\":\"%s\","
            "\"hardwareVersion\":\"%s\","
            "\"timestamp\":\"%s\","
            "\"deviceModel\":\"%s\""
            "}",
            _fota.deviceId, _fota.fwVersion, _fota.hwVersion,
            ts, _fota.deviceModel);
    return (n < 0) ? 0U : (uint16_t) n;
}

static uint16_t _Fota_BuildQueryJson(char *buf, uint16_t bufSize) {
    char ts[32];
    _Fota_NowIso8601(ts, sizeof (ts));
    int n = snprintf(buf, bufSize,
            "{"
            "\"deviceId\":\"%s\","
            "\"currentFirmware\":\"%s\","
            "\"hardwareVersion\":\"%s\","
            "\"timestamp\":\"%s\","
            "\"deviceModel\":\"%s\""
            "}",
            _fota.deviceId, _fota.fwVersion, _fota.hwVersion,
            ts, _fota.deviceModel);
    return (n < 0) ? 0U : (uint16_t) n;
}

/* ================================================================ */
/* MQTT callback                                                     */

/* ================================================================ */

static void _Fota_OnMqttMessage(const char *topic,
        const uint8_t *payload,
        uint16_t len) {
    /* Make a NUL-terminated stack copy so we can use string helpers. */
    char buf[512];
    if (len >= sizeof (buf)) {
        return;
    } /* drop oversize message */
    (void) memcpy(buf, payload, len);
    buf[len] = '\0';

    /* ---- Topic A: datalogger/firmware/notify ---- */
    if (strcmp(topic, _fota.topicNotify) == 0) {
        /* Accept any "action":"notify" with a firmwareVersion field.   */
        char action[16] = {0};
        (void) _Fota_JsonExtract(buf, "action", action, sizeof (action));
        if (action[0] != '\0' && strcmp(action, "notify") != 0) {
            return;
        }
        /* Trigger query as soon as the main task runs.                 */
        _fota.forceCheck = true;
        LOG_DEBUG("%s - %s\t  %s %s", __TAG__, __func__, _fota.topicNotify, buf);
        return;
    }

    /* ---- Topic B: datalogger/<id>/response ---- */
    if (strcmp(topic, _fota.topicResponse) == 0) {
        char hasUpdate[8] = {0};
        char downloadUrl[FOTA_URL_MAX_LEN] = {0};
        char latest[FOTA_FW_VERSION_MAX_LEN] = {0};

        (void) _Fota_JsonExtract(buf, "hasUpdate", hasUpdate, sizeof (hasUpdate));
        (void) _Fota_JsonExtract(buf, "latestVersion", latest, sizeof (latest));
        (void) _Fota_JsonExtract(buf, "downloadUrl", downloadUrl, sizeof (downloadUrl));

        if (strcmp(hasUpdate, "true") != 0) {
            /* No update -> ignore this notification. */
            _fota.updatePending = false;
            _Fota_GotoState(FOTA_STATE_IDLE);
            return;
        }
        if (downloadUrl[0] == '\0') {
            _Fota_GotoState(FOTA_STATE_IDLE);
            return;
        }

        if (!_Fota_ParseUrl(downloadUrl,
                _fota.dlHost, sizeof (_fota.dlHost),
                &_fota.dlPort,
                _fota.dlPath, sizeof (_fota.dlPath))) {
            _Fota_GotoState(FOTA_STATE_IDLE);
            return;
        }
        (void) strncpy(_fota.latestVersion, latest,
                sizeof (_fota.latestVersion) - 1U);
        _fota.updatePending = true;
        _Fota_GotoState(FOTA_STATE_HTTP_AUTH_CONNECT);
    }
}

/* ================================================================ */
/* HTTP callbacks - authentication                                   */

/* ================================================================ */

static bool _Fota_AuthOnHeaders(uint16_t status, uint32_t contentLen, void *ctx) {
    (void) contentLen;
    (void) ctx;
    _fota.httpRxLen = 0;
    return (status == 200U);
}

static bool _Fota_AuthOnBody(const uint8_t *chunk, uint16_t len,
        uint32_t offset, uint32_t totalLen, void *ctx) {
    (void) offset;
    (void) totalLen;
    (void) ctx;
    /* Stash the (small) JSON response. Truncate if it does not fit.  */
    uint16_t space = (uint16_t) (sizeof (_fota.httpRxBuf) - 1U - _fota.httpRxLen);
    uint16_t cpy = (len < space) ? len : space;
    (void) memcpy(&_fota.httpRxBuf[_fota.httpRxLen], chunk, cpy);
    _fota.httpRxLen += cpy;
    return true;
}

static void _Fota_AuthOnDone(ETH_HTTP_RESULT res, uint16_t status,
        uint32_t got, void *ctx) {
    (void) got;
    (void) ctx;
    _fota.httpRxBuf[_fota.httpRxLen] = '\0';
    (void) EthHttp_Disconnect();

    if (res != ETH_HTTP_RES_OK || status != 200U) {
        _Fota_GotoState(FOTA_STATE_ERROR);
        return;
    }

    char tok[FOTA_TOKEN_MAX_LEN] = {0};
    if (!_Fota_JsonExtract((const char *) _fota.httpRxBuf,
            "accessToken", tok, sizeof (tok)) ||
            tok[0] == '\0') {
        _Fota_GotoState(FOTA_STATE_ERROR);
        return;
    }
    (void) strncpy(_fota.jwt, tok, sizeof (_fota.jwt) - 1U);
    _fota.jwt[sizeof (_fota.jwt) - 1U] = '\0';
    (void) EthHttp_SetAuthToken(_fota.jwt);

    _Fota_GotoState(FOTA_STATE_HTTP_DL_CONNECT);
}

/* ================================================================ */
/* HTTP callbacks - firmware download (hex stream into NVM)          */
/* ================================================================ */

/* Pass one CRLF-terminated hex record (still containing ':' and the
   final CRLF in _fota.hexLine) to the bootloader programmer.         */
static bool _Fota_FlushHexLine(void) {
    uint16_t lineLen = _fota.hexLineLen;
    if (lineLen < 11U) {
        return false;
    } /* ':' + min hex header */

    CRYPT_MD5_DataAdd(&_fota.md5Ctx,
            (uint8_t *) _fota.hexLine, lineLen);

    uint8_t val[64];
    uint16_t i, j;
    for (i = 1U, j = 0U; i <= (lineLen - 3U) / 2U; i++, j++) {
        val[j] = _Fota_HexFromChars(_fota.hexLine[2U * i - 1U],
                _fota.hexLine[2U * i]);
    }
    if (bootloader_NvmProgramHexRecord(val, j) != HEX_REC_NORMAL) {
        _fota.programmingFailed = true;
        return false;
    }
    return true;
}

static bool _Fota_DlOnHeaders(uint16_t status, uint32_t contentLen, void *ctx) {
    (void) ctx;
    if (status != 200U) {
        return false;
    }
    if (contentLen == 0U) {
        return false;
    } /* refuse unknown size */
    if (contentLen > 2u * 1024u * 1024u) {
        return false;
    }

    /* Initialize MD5 and erase application flash, mirroring the
       original web-uploader behaviour: keep last 49152 B (data store)
       and the bootloader/bank metadata block.                          */
    CRYPT_MD5_Initialize(&_fota.md5Ctx);
    bootloader_NvmAppErase(APP_START_ADDRESS,
            FLASH_END_ADDRESS - 49152U - 16384U);
    bootloader_NvmAppErase(FLASH_END_ADDRESS - 16384U, FLASH_END_ADDRESS);

    _fota.flashErased = true;
    _fota.programmingFailed = false;
    _fota.hexLineLen = 0U;
    _fota.bytesProgrammed = 0U;
    return true;
}

static bool _Fota_DlOnBody(const uint8_t *chunk, uint16_t len,
        uint32_t offset, uint32_t totalLen, void *ctx) {
    (void) offset;
    (void) totalLen;
    (void) ctx;

    for (uint16_t i = 0; i < len; i++) {
        char c = (char) chunk[i];

        /* Filter out spaces the server might insert; keep CR/LF/':'/hex. */
        if (_fota.hexLineLen >= (FOTA_HEX_LINE_MAX_LEN - 1U)) {
            _fota.programmingFailed = true;
            return false;
        }
        _fota.hexLine[_fota.hexLineLen++] = c;

        if (_fota.hexLineLen >= 2U &&
                _fota.hexLine[_fota.hexLineLen - 2U] == '\r' &&
                _fota.hexLine[_fota.hexLineLen - 1U] == '\n') {
            if (!_Fota_FlushHexLine()) {
                return false;
            }
            _fota.bytesProgrammed += _fota.hexLineLen;
            _fota.hexLineLen = 0U;
        }
    }
    return true;
}

static void _Fota_DlOnDone(ETH_HTTP_RESULT res, uint16_t status,
        uint32_t got, void *ctx) {
    (void) got;
    (void) ctx;
    (void) EthHttp_Disconnect();

    if (res != ETH_HTTP_RES_OK || status != 200U) {
        _Fota_GotoState(FOTA_STATE_ERROR);
        return;
    }
    if (_fota.programmingFailed) {
        _Fota_GotoState(FOTA_STATE_ERROR);
        return;
    }
    _Fota_GotoState(FOTA_STATE_VERIFY_AND_COMMIT);
}

/* ================================================================ */
/* Per-state handlers                                                */

/* ================================================================ */

static void _Fota_TryPublishHeartbeat(void) {
    if (!EthMqtt_IsConnected()) {
        return;
    }
    if (!TIME_IS_EXPIRED(_fota.tHeartbeat, FOTA_HEARTBEAT_PERIOD_MS)) {
        return;
    }
    uint16_t n = _Fota_BuildHeartbeatJson(_fotaJsonBuf, sizeof (_fotaJsonBuf));
    if (n == 0U) {
        return;
    }
    if (EthMqtt_Publish(_fota.topicHeartbeat, (const uint8_t *) _fotaJsonBuf)) {
        _fota.tHeartbeat = TICK_NOW();
    }
}

static void _Fota_TrySubscribe(void) {
    if (_fota.subscribed || !EthMqtt_IsConnected()) {
        return;
    }
    bool a = EthMqtt_Subscribe(_fota.topicNotify, _Fota_OnMqttMessage);
    bool b = EthMqtt_Subscribe(_fota.topicResponse, _Fota_OnMqttMessage);
    _fota.subscribed = (a && b);
}

static void _Fota_DoIdle(void) {
    _Fota_TrySubscribe();
    _Fota_TryPublishHeartbeat();

    if (_fota.forceCheck) {
        _fota.forceCheck = false;
        /* Publish query and wait for the response on topic B. */
        uint16_t n = _Fota_BuildQueryJson(_fotaJsonBuf, sizeof (_fotaJsonBuf));
        if (n != 0U &&
                EthMqtt_Publish(_fota.topicQuery, (const uint8_t *) _fotaJsonBuf)) {
            _Fota_GotoState(FOTA_STATE_AWAIT_RESPONSE);
        }
    }
}

static void _Fota_DoAwaitResponse(void) {
    /* Heartbeat still runs even while waiting for /response.            */
    _Fota_TryPublishHeartbeat();

    /* Give the backend at most FOTA_HTTP_OP_TMO_MS to answer. The MQTT
       message handler moves us forward when the reply arrives.          */
    if (TIME_IS_EXPIRED(_fota.tStateEntry, FOTA_HTTP_OP_TMO_MS)) {
        _Fota_GotoState(FOTA_STATE_IDLE);
    }
}

static void _Fota_DoHttpAuthConnect(void) {
    if (EthHttp_IsBusy()) {
        return;
    }

    /* Reuse the host & port already parsed from downloadUrl. */
    if (EthHttp_ConnectByHost(_fota.dlHost, _fota.dlPort) != ETH_HTTP_RES_OK) {
        _Fota_GotoState(FOTA_STATE_ERROR);
        return;
    }
    _Fota_GotoState(FOTA_STATE_HTTP_AUTH_REQUEST);
}

static void _Fota_DoHttpAuthRequest(void) {
    if (!EthHttp_IsConnected()) {
        if (TIME_IS_EXPIRED(_fota.tStateEntry, FOTA_HTTP_OP_TMO_MS)) {
            _Fota_GotoState(FOTA_STATE_ERROR);
        }
        return;
    }

    /* Clear any previous JWT so the login request itself is anonymous. */
    (void) EthHttp_SetAuthToken(NULL);

    char body[96];
    int n = snprintf(body, sizeof (body),
            "{\"username\":\"%s\",\"password\":\"%s\"}",
            FOTA_AUTH_USERNAME, FOTA_AUTH_PASSWORD);
    if (n <= 0) {
        _Fota_GotoState(FOTA_STATE_ERROR);
        return;
    }

    ETH_HTTP_RESULT r = EthHttp_Post(FOTA_AUTH_PATH,
            "application/json",
            (const uint8_t *) body, (uint32_t) n,
            NULL,
            _Fota_AuthOnHeaders,
            _Fota_AuthOnBody,
            _Fota_AuthOnDone,
            NULL);
    if (r != ETH_HTTP_RES_OK) {
        _Fota_GotoState(FOTA_STATE_ERROR);
        return;
    }
    _Fota_GotoState(FOTA_STATE_HTTP_AUTH_WAIT);
}

static void _Fota_DoHttpAuthWait(void) {
    /* The done-callback advances the state. Just guard against a hang. */
    if (TIME_IS_EXPIRED(_fota.tStateEntry, FOTA_HTTP_OP_TMO_MS)) {
        (void) EthHttp_Disconnect();
        _Fota_GotoState(FOTA_STATE_ERROR);
    }
}

static void _Fota_DoHttpDlConnect(void) {
    if (EthHttp_IsBusy()) {
        return;
    }
    if (EthHttp_ConnectByHost(_fota.dlHost, _fota.dlPort) != ETH_HTTP_RES_OK) {
        _Fota_GotoState(FOTA_STATE_ERROR);
        return;
    }
    _Fota_GotoState(FOTA_STATE_HTTP_DL_REQUEST);
}

static void _Fota_DoHttpDlRequest(void) {
    if (!EthHttp_IsConnected()) {
        if (TIME_IS_EXPIRED(_fota.tStateEntry, FOTA_HTTP_OP_TMO_MS)) {
            _Fota_GotoState(FOTA_STATE_ERROR);
        }
        return;
    }

    /* Bearer token already loaded by _Fota_AuthOnDone. */
    ETH_HTTP_RESULT r = EthHttp_Get(_fota.dlPath,
            NULL,
            _Fota_DlOnHeaders,
            _Fota_DlOnBody,
            _Fota_DlOnDone,
            NULL);
    if (r != ETH_HTTP_RES_OK) {
        _Fota_GotoState(FOTA_STATE_ERROR);
        return;
    }
    _Fota_GotoState(FOTA_STATE_HTTP_DL_STREAMING);
}

static void _Fota_DoHttpDlStreaming(void) {
    /* All progress is driven by HTTP body callbacks. We only watchdog
       a stalled download here.                                          */
    if (TIME_IS_EXPIRED(_fota.tStateEntry, 5U * FOTA_HTTP_OP_TMO_MS)) {
        (void) EthHttp_Disconnect();
        _Fota_GotoState(FOTA_STATE_ERROR);
    }
}

static void _Fota_DoVerifyAndCommit(void) {
    CRYPT_MD5_Finalize(&_fota.md5Ctx, _fota.md5Digest);

    /* Persist the MD5 of the newly-programmed image and request the
       bootloader to jump to the application on next reset.              */
    for (uint8_t i = 0U; i < 16U; i++) {
        gDeviceInfo.fwHashCode[i] = _fota.md5Digest[i];
    }
    InFlash_SaveDeviceInfo((uint8_t *) & gDeviceInfo, sizeof (gDeviceInfo));
    //    showJumpFirmware = 1;

    _fota.updatePending = false;
    _Fota_GotoState(FOTA_STATE_IDLE);
}

static void _Fota_DoError(void) {
    /* Always release any resource and fall back to IDLE so the heartbeat
       loop keeps the device visible to the operator.                    */
    (void) EthHttp_Disconnect();
    _fota.updatePending = false;
    _Fota_GotoState(FOTA_STATE_IDLE);
}

/* ================================================================ */
/* Public API                                                        */

/* ================================================================ */

void Fota_Initialize(const FOTA_CONFIG *cfg) {
    (void) memset(&_fota, 0, sizeof (_fota));

    const char *id = (cfg && cfg->deviceId) ? cfg->deviceId : "MBD200";
    const char *fw = (cfg && cfg->firmwareVersion) ? cfg->firmwareVersion : "3.0.0";
    const char *hw = (cfg && cfg->hardwareVersion) ? cfg->hardwareVersion : "2.0";
    const char *mod = (cfg && cfg->deviceModel) ? cfg->deviceModel : id;

    (void) strncpy(_fota.deviceId, id, sizeof (_fota.deviceId) - 1U);
    (void) strncpy(_fota.fwVersion, fw, sizeof (_fota.fwVersion) - 1U);
    (void) strncpy(_fota.hwVersion, hw, sizeof (_fota.hwVersion) - 1U);
    (void) strncpy(_fota.deviceModel, mod, sizeof (_fota.deviceModel) - 1U);

    (void) snprintf(_fota.topicHeartbeat, sizeof (_fota.topicHeartbeat),
            "datalogger/%s/heartbeat", _fota.deviceId);
    (void) snprintf(_fota.topicQuery, sizeof (_fota.topicQuery),
            "datalogger/%s/firmware/query", _fota.deviceId);
    (void) snprintf(_fota.topicNotify, sizeof (_fota.topicNotify),
            "datalogger/firmware/notify");
    (void) snprintf(_fota.topicResponse, sizeof (_fota.topicResponse),
            "datalogger/%s/response", _fota.deviceId);

    /* Force the first heartbeat to fire almost immediately. */
    _fota.tHeartbeat = TICK_NOW() - MS_TO_TICK(FOTA_HEARTBEAT_PERIOD_MS);
    _Fota_GotoState(FOTA_STATE_IDLE);
}

FOTA_STATE Fota_GetState(void) {
    return _fota.state;
}

void Fota_TriggerCheck(void) {
    _fota.forceCheck = true;
}

void Fota_Task(void) {
    switch (_fota.state) {
        case FOTA_STATE_UNINIT: break;
        case FOTA_STATE_IDLE: _Fota_DoIdle();
            break;
        case FOTA_STATE_QUERY_PENDING: /* unused, kept for symmetry */ break;
        case FOTA_STATE_AWAIT_RESPONSE: _Fota_DoAwaitResponse();
            break;
        case FOTA_STATE_HTTP_AUTH_CONNECT: _Fota_DoHttpAuthConnect();
            break;
        case FOTA_STATE_HTTP_AUTH_REQUEST: _Fota_DoHttpAuthRequest();
            break;
        case FOTA_STATE_HTTP_AUTH_WAIT: _Fota_DoHttpAuthWait();
            break;
        case FOTA_STATE_HTTP_DL_CONNECT: _Fota_DoHttpDlConnect();
            break;
        case FOTA_STATE_HTTP_DL_REQUEST: _Fota_DoHttpDlRequest();
            break;
        case FOTA_STATE_HTTP_DL_STREAMING: _Fota_DoHttpDlStreaming();
            break;
        case FOTA_STATE_VERIFY_AND_COMMIT: _Fota_DoVerifyAndCommit();
            break;
        case FOTA_STATE_ERROR: _Fota_DoError();
            break;
        default:
            _Fota_GotoState(FOTA_STATE_ERROR);
            break;
    }
}