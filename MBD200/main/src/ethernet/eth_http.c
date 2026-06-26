#include "eth_http.h"

/* ================================================================ */
/* Internal context                                                  */

/* ================================================================ */

typedef struct {
    /* ---- FSM ---- */
    ETH_HTTP_STATE state;
    uint32_t tStamp; /* tick at state entry */

    /* ---- Connection ---- */
    TCP_SOCKET sock;
    char host[ETH_HTTP_HOST_MAX_LEN]; /* used for Host hdr   */
    bool hostIsName; /* needs DNS resolve   */
    IP_MULTI_ADDRESS remoteAddr;
    uint16_t port;

    /* ---- Authentication ---- */
    char jwt[ETH_HTTP_TOKEN_MAX_LEN];
    bool jwtSet;

    /* ---- Current transaction (request side) ---- */
    ETH_HTTP_METHOD method;
    char path[ETH_HTTP_PATH_MAX_LEN];
    char contentType[ETH_HTTP_CT_MAX_LEN];
    char extraHdr[ETH_HTTP_EXTRA_HDR_MAX_LEN];
    const uint8_t *txBody;
    uint32_t txBodyLen;
    uint32_t txBodySent;

    /* ---- Upper-layer callbacks ---- */
    ETH_HTTP_HDR_CB cbHdr;
    ETH_HTTP_BODY_CB cbBody;
    ETH_HTTP_DONE_CB cbDone;
    void *cbCtx;

    /* ---- Response parsing ---- */
    char lineBuf[ETH_HTTP_RX_LINE_BUF];
    uint16_t lineLen;
    uint16_t rspStatus;
    uint32_t rspContentLen; /* 0 = unknown        */
    bool rspContentLenValid;
    uint32_t rspBodyGot;
    bool rspChunked; /* not supported      */
    bool hdrCbDone;
    bool aborted;
    bool inUse;
} _ETH_HTTP_CTX;

static _ETH_HTTP_CTX _http;

/* ================================================================ */
/* Small helpers                                                     */

/* ================================================================ */

static void _Http_GotoState(ETH_HTTP_STATE st) {
    _http.state = st;
    _http.tStamp = TICK_NOW();
}

static void _Http_FinishTx(ETH_HTTP_RESULT res) {
    /* Snapshot then clear, so re-entrant requests in the user callback
       see a clean context. */
    ETH_HTTP_DONE_CB cb = _http.cbDone;
    void *ctx = _http.cbCtx;
    uint16_t status = _http.rspStatus;
    uint32_t got = _http.rspBodyGot;

    _http.inUse = false;
    _http.cbHdr = NULL;
    _http.cbBody = NULL;
    _http.cbDone = NULL;

    if (cb != NULL) {
        cb(res, status, got, ctx);
    }
}

static void _Http_AbortError(ETH_HTTP_RESULT res) {
    if (_http.sock != INVALID_SOCKET) {
        TCPIP_TCP_Abort(_http.sock, false);
        TCPIP_TCP_Close(_http.sock);
        _http.sock = INVALID_SOCKET;
    }
    _Http_FinishTx(res);
    _Http_GotoState(ETH_HTTP_STATE_IDLE);
}

/* Write a NUL-terminated string in one shot. Returns true only when
   all bytes fit in the TX FIFO; otherwise leaves it untouched so the
   caller can retry on the next tick. */
static bool _Http_PutAll(const char *s) {
    uint16_t len = (uint16_t) strlen(s);
    uint16_t free = TCPIP_TCP_PutIsReady(_http.sock);
    if (free < len) {
        return false;
    }
    (void) TCPIP_TCP_ArrayPut(_http.sock, (const uint8_t *) s, len);
    return true;
}

/* Drain one CRLF-terminated line into _http.lineBuf.
   Returns:  1 = full line ready, 0 = need more bytes, -1 = overflow. */
static int8_t _Http_ReadLine(void) {
    uint8_t b;
    while (TCPIP_TCP_GetIsReady(_http.sock) > 0U) {
        if (TCPIP_TCP_Get(_http.sock, &b) == 0U) {
            break;
        }
        if (_http.lineLen >= (ETH_HTTP_RX_LINE_BUF - 1U)) {
            return -1;
        }
        _http.lineBuf[_http.lineLen++] = (char) b;
        if (_http.lineLen >= 2U &&
                _http.lineBuf[_http.lineLen - 2U] == '\r' &&
                _http.lineBuf[_http.lineLen - 1U] == '\n') {
            _http.lineBuf[_http.lineLen - 2U] = '\0'; /* strip CRLF */
            return 1;
        }
    }
    return 0;
}

static void _Http_ResetLine(void) {
    _http.lineLen = 0;
    _http.lineBuf[0] = '\0';
}

/* Detect whether the string looks like a numeric IPv4 dotted address. */
static bool _Http_IsIpv4Literal(const char *s) {
    int dots = 0;
    while (*s != '\0') {
        if (*s == '.') {
            dots++;
        } else if (*s < '0' || *s > '9') {
            return false;
        }
        s++;
    }
    return (dots == 3);
}

/* ================================================================ */
/* Public API                                                        */

/* ================================================================ */

void EthHttp_Initialize(void) {
    (void) memset(&_http, 0, sizeof (_http));
    _http.sock = INVALID_SOCKET;
    _Http_GotoState(ETH_HTTP_STATE_IDLE);
}

bool EthHttp_IsConnected(void) {
    return (_http.sock != INVALID_SOCKET) &&
            TCPIP_TCP_IsConnected(_http.sock);
}

bool EthHttp_IsBusy(void) {
    return _http.inUse;
}

ETH_HTTP_STATE EthHttp_GetState(void) {
    return _http.state;
}

ETH_HTTP_RESULT EthHttp_SetAuthToken(const char *token) {
    if (token == NULL || token[0] == '\0') {
        _http.jwtSet = false;
        _http.jwt[0] = '\0';
        return ETH_HTTP_RES_OK;
    }
    if (strlen(token) >= ETH_HTTP_TOKEN_MAX_LEN) {
        return ETH_HTTP_RES_ERR_PARAM;
    }
    (void) strncpy(_http.jwt, token, ETH_HTTP_TOKEN_MAX_LEN - 1U);
    _http.jwt[ETH_HTTP_TOKEN_MAX_LEN - 1U] = '\0';
    _http.jwtSet = true;
    return ETH_HTTP_RES_OK;
}

/* Common state setup used by both connect entry points. */
static ETH_HTTP_RESULT _Http_PrepareConnect(const char *host, uint16_t port) {
    if (host == NULL || host[0] == '\0') {
        return ETH_HTTP_RES_ERR_PARAM;
    }
    if (strlen(host) >= ETH_HTTP_HOST_MAX_LEN) {
        return ETH_HTTP_RES_ERR_PARAM;
    }
    if (_http.state != ETH_HTTP_STATE_IDLE &&
            _http.state != ETH_HTTP_STATE_ERROR) {
        return ETH_HTTP_RES_ERR_BUSY;
    }

    (void) strncpy(_http.host, host, ETH_HTTP_HOST_MAX_LEN - 1U);
    _http.host[ETH_HTTP_HOST_MAX_LEN - 1U] = '\0';
    _http.port = (port == 0U) ? ETH_HTTP_DEFAULT_PORT : port;
    return ETH_HTTP_RES_OK;
}

ETH_HTTP_RESULT EthHttp_ConnectByHost(const char *hostname, uint16_t port) {
    ETH_HTTP_RESULT r = _Http_PrepareConnect(hostname, port);
    if (r != ETH_HTTP_RES_OK) {
        return r;
    }

    /* If the supplied "hostname" is actually a numeric IPv4 literal,
       parse it directly and skip DNS. */
    if (_Http_IsIpv4Literal(hostname)) {
        IPV4_ADDR ip;
        if (!TCPIP_Helper_StringToIPAddress(hostname, &ip)) {
            return ETH_HTTP_RES_ERR_PARAM;
        }
        _http.remoteAddr.v4Add = ip;
        _http.hostIsName = false;
        _Http_GotoState(ETH_HTTP_STATE_TCP_OPENING);
        return ETH_HTTP_RES_OK;
    }

    /* Kick off DNS resolution; result is polled in EthHttp_Task(). */
    TCPIP_DNS_RESULT dr = TCPIP_DNS_Resolve(_http.host, TCPIP_DNS_TYPE_A);
    if (dr < 0) {
        return ETH_HTTP_RES_ERR_DNS;
    }
    _http.hostIsName = true;
    _Http_GotoState(ETH_HTTP_STATE_DNS_RESOLVING);
    return ETH_HTTP_RES_OK;
}

ETH_HTTP_RESULT EthHttp_ConnectByIp(const char *host,
        const IP_MULTI_ADDRESS *addr,
        uint16_t port) {
    ETH_HTTP_RESULT r;
    if (addr == NULL) {
        return ETH_HTTP_RES_ERR_PARAM;
    }

    r = _Http_PrepareConnect(host, port);
    if (r != ETH_HTTP_RES_OK) {
        return r;
    }

    _http.remoteAddr = *addr;
    _http.hostIsName = false;
    _Http_GotoState(ETH_HTTP_STATE_TCP_OPENING);
    return ETH_HTTP_RES_OK;
}

ETH_HTTP_RESULT EthHttp_Disconnect(void) {
    if (_http.sock == INVALID_SOCKET) {
        _Http_GotoState(ETH_HTTP_STATE_IDLE);
        return ETH_HTTP_RES_OK;
    }
    (void) TCPIP_TCP_Disconnect(_http.sock);
    _Http_GotoState(ETH_HTTP_STATE_DISCONNECTING);
    return ETH_HTTP_RES_OK;
}

/* ---------------------------------------------------------------- */
/* Build the transaction context from a request descriptor.          */

/* ---------------------------------------------------------------- */
ETH_HTTP_RESULT EthHttp_SendRequest(const ETH_HTTP_REQUEST *req) {
    if (req == NULL || req->path == NULL || req->onDone == NULL) {
        return ETH_HTTP_RES_ERR_PARAM;
    }
    if (strlen(req->path) >= ETH_HTTP_PATH_MAX_LEN) {
        return ETH_HTTP_RES_ERR_PARAM;
    }
    if (req->extraHeaders != NULL &&
            strlen(req->extraHeaders) >= ETH_HTTP_EXTRA_HDR_MAX_LEN) {
        return ETH_HTTP_RES_ERR_PARAM;
    }
    if (!EthHttp_IsConnected()) {
        return ETH_HTTP_RES_ERR_NOT_CONNECTED;
    }
    if (_http.inUse || _http.state != ETH_HTTP_STATE_CONNECTED) {
        return ETH_HTTP_RES_ERR_BUSY;
    }

    _http.method = req->method;
    (void) strncpy(_http.path, req->path, ETH_HTTP_PATH_MAX_LEN - 1U);
    _http.path[ETH_HTTP_PATH_MAX_LEN - 1U] = '\0';

    _http.contentType[0] = '\0';
    if (req->contentType != NULL && req->contentType[0] != '\0') {
        (void) strncpy(_http.contentType, req->contentType,
                ETH_HTTP_CT_MAX_LEN - 1U);
        _http.contentType[ETH_HTTP_CT_MAX_LEN - 1U] = '\0';
    }

    _http.extraHdr[0] = '\0';
    if (req->extraHeaders != NULL) {
        (void) strncpy(_http.extraHdr, req->extraHeaders,
                ETH_HTTP_EXTRA_HDR_MAX_LEN - 1U);
        _http.extraHdr[ETH_HTTP_EXTRA_HDR_MAX_LEN - 1U] = '\0';
    }

    _http.txBody = req->body;
    _http.txBodyLen = (req->method == ETH_HTTP_METHOD_POST) ? req->bodyLen : 0U;
    _http.txBodySent = 0;

    _http.cbHdr = req->onHeaders;
    _http.cbBody = req->onBodyChunk;
    _http.cbDone = req->onDone;
    _http.cbCtx = req->ctx;

    _http.rspStatus = 0;
    _http.rspContentLen = 0;
    _http.rspContentLenValid = false;
    _http.rspBodyGot = 0;
    _http.rspChunked = false;
    _http.hdrCbDone = false;
    _http.aborted = false;
    _Http_ResetLine();

    _http.inUse = true;
    _Http_GotoState(ETH_HTTP_STATE_REQ_SEND_LINE);
    return ETH_HTTP_RES_OK;
}

ETH_HTTP_RESULT EthHttp_Get(const char *path,
        const char *extraHeaders,
        ETH_HTTP_HDR_CB onHeaders,
        ETH_HTTP_BODY_CB onBodyChunk,
        ETH_HTTP_DONE_CB onDone,
        void *ctx) {
    ETH_HTTP_REQUEST r = {0};
    r.method = ETH_HTTP_METHOD_GET;
    r.path = path;
    r.extraHeaders = extraHeaders;
    r.onHeaders = onHeaders;
    r.onBodyChunk = onBodyChunk;
    r.onDone = onDone;
    r.ctx = ctx;
    return EthHttp_SendRequest(&r);
}

ETH_HTTP_RESULT EthHttp_Post(const char *path,
        const char *contentType,
        const uint8_t *body,
        uint32_t bodyLen,
        const char *extraHeaders,
        ETH_HTTP_HDR_CB onHeaders,
        ETH_HTTP_BODY_CB onBodyChunk,
        ETH_HTTP_DONE_CB onDone,
        void *ctx) {
    ETH_HTTP_REQUEST r = {0};
    r.method = ETH_HTTP_METHOD_POST;
    r.path = path;
    r.contentType = contentType;
    r.body = body;
    r.bodyLen = bodyLen;
    r.extraHeaders = extraHeaders;
    r.onHeaders = onHeaders;
    r.onBodyChunk = onBodyChunk;
    r.onDone = onDone;
    r.ctx = ctx;
    return EthHttp_SendRequest(&r);
}

/* ================================================================ */
/* Per-state handlers                                                */

/* ================================================================ */

static void _Http_DoDnsResolving(void) {
    IP_MULTI_ADDRESS addr;
    TCPIP_DNS_RESULT r = TCPIP_DNS_IsResolved(_http.host, &addr,
            IP_ADDRESS_TYPE_IPV4);
    if (r == TCPIP_DNS_RES_OK) {
        _http.remoteAddr = addr;
        _Http_GotoState(ETH_HTTP_STATE_TCP_OPENING);
        return;
    }
    if (r < 0) {
        _Http_FinishTx(ETH_HTTP_RES_ERR_DNS);
        _Http_GotoState(ETH_HTTP_STATE_IDLE);
        return;
    }
    if (TIME_IS_EXPIRED(_http.tStamp, ETH_HTTP_DNS_TMO_MS)) {
        _Http_FinishTx(ETH_HTTP_RES_ERR_DNS);
        _Http_GotoState(ETH_HTTP_STATE_IDLE);
    }
}

static void _Http_DoTcpOpening(void) {
    _http.sock = TCPIP_TCP_ClientOpen(IP_ADDRESS_TYPE_IPV4,
            _http.port,
            &_http.remoteAddr);
    if (_http.sock == INVALID_SOCKET) {
        _Http_FinishTx(ETH_HTTP_RES_ERR_TCP_OPEN);
        _Http_GotoState(ETH_HTTP_STATE_IDLE);
        return;
    }
    _Http_GotoState(ETH_HTTP_STATE_CONNECTING);
}

static void _Http_DoConnecting(void) {
    if (TCPIP_TCP_IsConnected(_http.sock)) {
        _Http_GotoState(ETH_HTTP_STATE_CONNECTED);
        return;
    }
    if (TIME_IS_EXPIRED(_http.tStamp, ETH_HTTP_CONNECT_TMO_MS)) {
        TCPIP_TCP_Close(_http.sock);
        _http.sock = INVALID_SOCKET;
        _Http_FinishTx(ETH_HTTP_RES_ERR_TCP_CONNECT);
        _Http_GotoState(ETH_HTTP_STATE_IDLE);
    }
}

static void _Http_DoSendRequestLine(void) {
    char line[ETH_HTTP_PATH_MAX_LEN + 32U];
    const char *m = (_http.method == ETH_HTTP_METHOD_GET) ? "GET" : "POST";
    (void) snprintf(line, sizeof (line), "%s %s HTTP/1.1\r\n", m, _http.path);
    if (_Http_PutAll(line)) {
        _Http_GotoState(ETH_HTTP_STATE_REQ_SEND_HEADERS);
    }
}

static void _Http_DoSendHeaders(void) {
    /* Build the whole header block in one buffer and ship it atomically. */
    char hdr[ETH_HTTP_EXTRA_HDR_MAX_LEN + ETH_HTTP_TOKEN_MAX_LEN + 256U];
    int n = 0;

    n += snprintf(hdr + n, sizeof (hdr) - n,
            "Host: %s\r\n"
            "User-Agent: %s\r\n"
            "Accept: */*\r\n"
            "Connection: keep-alive\r\n",
            _http.host, ETH_HTTP_USER_AGENT);

    if (_http.jwtSet) {
        n += snprintf(hdr + n, sizeof (hdr) - n,
                "Authorization: Bearer %s\r\n", _http.jwt);
    }
    if (_http.method == ETH_HTTP_METHOD_POST) {
        const char *ct = (_http.contentType[0] != '\0')
                ? _http.contentType : "application/octet-stream";
        n += snprintf(hdr + n, sizeof (hdr) - n,
                "Content-Type: %s\r\n"
                "Content-Length: %lu\r\n",
                ct, (unsigned long) _http.txBodyLen);
    }
    if (_http.extraHdr[0] != '\0') {
        n += snprintf(hdr + n, sizeof (hdr) - n, "%s", _http.extraHdr);
    }
    n += snprintf(hdr + n, sizeof (hdr) - n, "\r\n");

    if (n <= 0 || n >= (int) sizeof (hdr)) {
        _Http_AbortError(ETH_HTTP_RES_ERR_INTERNAL);
        return;
    }
    if (!_Http_PutAll(hdr)) {
        return; /* TX FIFO too small right now, retry next tick */
    }

    if (_http.method == ETH_HTTP_METHOD_POST && _http.txBodyLen > 0U) {
        _Http_GotoState(ETH_HTTP_STATE_REQ_SEND_BODY);
    } else {
        (void) TCPIP_TCP_Flush(_http.sock);
        _Http_GotoState(ETH_HTTP_STATE_RSP_WAIT);
    }
}

static void _Http_DoSendBody(void) {
    uint16_t free = TCPIP_TCP_PutIsReady(_http.sock);
    uint32_t rem = _http.txBodyLen - _http.txBodySent;
    uint16_t wr;

    if (free == 0U) {
        return;
    }
    wr = (free < rem) ? free : (uint16_t) rem;
    wr = TCPIP_TCP_ArrayPut(_http.sock,
            _http.txBody + _http.txBodySent, wr);
    _http.txBodySent += wr;

    if (_http.txBodySent >= _http.txBodyLen) {
        (void) TCPIP_TCP_Flush(_http.sock);
        _Http_GotoState(ETH_HTTP_STATE_RSP_WAIT);
    }
}

static void _Http_DoRspWait(void) {
    if (TCPIP_TCP_GetIsReady(_http.sock) > 0U) {
        _Http_ResetLine();
        _Http_GotoState(ETH_HTTP_STATE_RSP_STATUS_LINE);
        return;
    }
    if (TIME_IS_EXPIRED(_http.tStamp, ETH_HTTP_RESPONSE_TMO_MS)) {
        _Http_AbortError(ETH_HTTP_RES_ERR_TIMEOUT);
    }
}

static void _Http_DoRspStatusLine(void) {
    int8_t r = _Http_ReadLine();
    if (r == 0) {
        if (TIME_IS_EXPIRED(_http.tStamp, ETH_HTTP_RESPONSE_TMO_MS)) {
            _Http_AbortError(ETH_HTTP_RES_ERR_TIMEOUT);
        }
        return;
    }
    if (r < 0) {
        _Http_AbortError(ETH_HTTP_RES_ERR_PARSE);
        return;
    }

    /* Expected: "HTTP/1.1 <code> <reason>" */
    unsigned int code = 0;
    if (sscanf(_http.lineBuf, "HTTP/%*d.%*d %u", &code) != 1) {
        _Http_AbortError(ETH_HTTP_RES_ERR_PARSE);
        return;
    }
    _http.rspStatus = (uint16_t) code;
    _Http_ResetLine();
    _Http_GotoState(ETH_HTTP_STATE_RSP_HEADERS);
}

static void _Http_DoRspHeaders(void) {
    int8_t r = _Http_ReadLine();
    if (r == 0) {
        if (TIME_IS_EXPIRED(_http.tStamp, ETH_HTTP_RESPONSE_TMO_MS)) {
            _Http_AbortError(ETH_HTTP_RES_ERR_TIMEOUT);
        }
        return;
    }
    if (r < 0) {
        _Http_AbortError(ETH_HTTP_RES_ERR_PARSE);
        return;
    }

    /* Empty line marks end of headers. */
    if (_http.lineBuf[0] == '\0') {
        _Http_ResetLine();

        if (_http.rspChunked) {
            _Http_AbortError(ETH_HTTP_RES_ERR_PARSE); /* not supported */
            return;
        }

        /* Notify upper layer once. It may abort large downloads here
           if the announced size is unacceptable.                          */
        if (_http.cbHdr != NULL) {
            bool keep = _http.cbHdr(_http.rspStatus,
                    _http.rspContentLen,
                    _http.cbCtx);
            if (!keep) {
                _http.aborted = true;
                _Http_AbortError(ETH_HTTP_RES_ERR_USER_ABORT);
                return;
            }
        }
        _http.hdrCbDone = true;
        _Http_GotoState(ETH_HTTP_STATE_RSP_BODY);
        return;
    }

    if (strncmp(_http.lineBuf, "Content-Length:", 15) == 0) {
        _http.rspContentLen = (uint32_t) strtoul(_http.lineBuf + 15,
                NULL, 10);
        _http.rspContentLenValid = true;
    } else if (strncmp(_http.lineBuf, "Transfer-Encoding:", 18) == 0) {
        if (strstr(_http.lineBuf + 18, "chunked") != NULL) {
            _http.rspChunked = true;
        }
    }
    _Http_ResetLine();
}

/* Streaming body delivery.
   We pull bytes straight from the TCP RX FIFO into a small stack buffer
   and forward them to the user callback. This is what makes a ~1 MB
   firmware download safe in RAM-constrained MCUs.                       */
static void _Http_DoRspBody(void) {
    uint8_t buf[256];
    uint16_t avail = TCPIP_TCP_GetIsReady(_http.sock);

    while (avail > 0U) {
        uint16_t want = (avail > sizeof (buf)) ? sizeof (buf) : avail;

        /* If Content-Length is known, do not over-read past it.          */
        if (_http.rspContentLenValid) {
            uint32_t left = _http.rspContentLen - _http.rspBodyGot;
            if ((uint32_t) want > left) {
                want = (uint16_t) left;
            }
            if (want == 0U) {
                break;
            }
        }

        uint16_t got = TCPIP_TCP_ArrayGet(_http.sock, buf, want);
        if (got == 0U) {
            break;
        }

        if (_http.cbBody != NULL) {
            bool keep = _http.cbBody(buf, got,
                    _http.rspBodyGot,
                    _http.rspContentLen, /* 0 if unknown */
                    _http.cbCtx);
            if (!keep) {
                _http.aborted = true;
                _Http_AbortError(ETH_HTTP_RES_ERR_USER_ABORT);
                return;
            }
        }
        _http.rspBodyGot += got;
        avail -= got;

        /* Refresh "expected" feedback each iteration so we don't spin
           on a deceptive avail value when the FIFO is being filled by
           an ISR.                                                        */
        if (_http.rspContentLenValid &&
                _http.rspBodyGot >= _http.rspContentLen) {
            break;
        }
    }

    /* Completion conditions. */
    if (_http.rspContentLenValid &&
            _http.rspBodyGot >= _http.rspContentLen) {
        _Http_FinishTx(ETH_HTTP_RES_OK);
        _Http_GotoState(ETH_HTTP_STATE_CONNECTED);
        /* Reset response timer for the next transaction. */
        _http.tStamp = TICK_NOW();
        return;
    }

    /* If the peer closed the stream and we had no Content-Length, the
       body is implicitly defined by EOF (RFC 7230 §3.3.3 #7).            */
    if (TCPIP_TCP_WasDisconnected(_http.sock) ||
            !TCPIP_TCP_IsConnected(_http.sock)) {
        ETH_HTTP_RESULT res = _http.rspContentLenValid
                ? ETH_HTTP_RES_ERR_TCP_RESET
                : ETH_HTTP_RES_OK;
        TCPIP_TCP_Close(_http.sock);
        _http.sock = INVALID_SOCKET;
        _Http_FinishTx(res);
        _Http_GotoState(ETH_HTTP_STATE_IDLE);
        return;
    }

    if (TIME_IS_EXPIRED(_http.tStamp, ETH_HTTP_RESPONSE_TMO_MS)) {
        _Http_AbortError(ETH_HTTP_RES_ERR_TIMEOUT);
    } else if (avail == 0U) {
        /* If we actually consumed data this tick, refresh the watchdog
           so a long but progressing download does not time out.          */
        if (_http.rspBodyGot != 0U) {
            _http.tStamp = TICK_NOW();
        }
    }
}

static void _Http_DoDisconnecting(void) {
    if (_http.sock == INVALID_SOCKET ||
            !TCPIP_TCP_IsConnected(_http.sock) ||
            TCPIP_TCP_WasDisconnected(_http.sock) ||
            TIME_IS_EXPIRED(_http.tStamp, 2000U)) {
        if (_http.sock != INVALID_SOCKET) {
            TCPIP_TCP_Close(_http.sock);
            _http.sock = INVALID_SOCKET;
        }
        _Http_GotoState(ETH_HTTP_STATE_IDLE);
    }
}

/* ================================================================ */
/* Top-level task                                                    */

/* ================================================================ */

void EthHttp_Task(void) {
    /* Generic socket health check: handle peer RST in any post-open
       state. Skip while still resolving DNS / opening a socket.        */
    if (_http.sock != INVALID_SOCKET &&
            _http.state >= ETH_HTTP_STATE_CONNECTING &&
            TCPIP_TCP_WasReset(_http.sock)) {
        _Http_AbortError(ETH_HTTP_RES_ERR_TCP_RESET);
        return;
    }

    switch (_http.state) {
        case ETH_HTTP_STATE_UNINIT:
        case ETH_HTTP_STATE_IDLE:
        case ETH_HTTP_STATE_CONNECTED:
        case ETH_HTTP_STATE_ERROR:
            break;

        case ETH_HTTP_STATE_DNS_RESOLVING: _Http_DoDnsResolving();
            break;
        case ETH_HTTP_STATE_TCP_OPENING: _Http_DoTcpOpening();
            break;
        case ETH_HTTP_STATE_CONNECTING: _Http_DoConnecting();
            break;
        case ETH_HTTP_STATE_REQ_SEND_LINE: _Http_DoSendRequestLine();
            break;
        case ETH_HTTP_STATE_REQ_SEND_HEADERS: _Http_DoSendHeaders();
            break;
        case ETH_HTTP_STATE_REQ_SEND_BODY: _Http_DoSendBody();
            break;
        case ETH_HTTP_STATE_RSP_WAIT: _Http_DoRspWait();
            break;
        case ETH_HTTP_STATE_RSP_STATUS_LINE: _Http_DoRspStatusLine();
            break;
        case ETH_HTTP_STATE_RSP_HEADERS: _Http_DoRspHeaders();
            break;
        case ETH_HTTP_STATE_RSP_BODY: _Http_DoRspBody();
            break;
        case ETH_HTTP_STATE_DISCONNECTING: _Http_DoDisconnecting();
            break;

        default:
            _Http_GotoState(ETH_HTTP_STATE_ERROR);
            break;
    }
}