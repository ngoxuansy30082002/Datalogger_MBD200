/* 
 * File:   eth_http.h
 * Author: LENOVO
 *
 * Created on June 26, 2026, 3:51 PM
 */

#ifndef ETH_HTTP_H
#define	ETH_HTTP_H

#include <stdlib.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif


    /* ============================================================== */
    /* Compile-time configuration                                      */
    /* ============================================================== */
#define ETH_HTTP_HOST_MAX_LEN         64U
#define ETH_HTTP_PATH_MAX_LEN         128U
#define ETH_HTTP_TOKEN_MAX_LEN        512U     /* JWT can be long       */
#define ETH_HTTP_EXTRA_HDR_MAX_LEN    256U
#define ETH_HTTP_CT_MAX_LEN           64U
#define ETH_HTTP_RX_LINE_BUF          256U     /* one HTTP header line  */
#define ETH_HTTP_CONNECT_TMO_MS       10000U
#define ETH_HTTP_DNS_TMO_MS           10000U
#define ETH_HTTP_RESPONSE_TMO_MS      15000U
#define ETH_HTTP_DEFAULT_PORT         80U
#define ETH_HTTP_USER_AGENT           "MBD-200/1.0"

    /* ============================================================== */
    /* Public enumerations                                             */
    /* ============================================================== */

    /* Top-level FSM state (queryable by upper layer for debug).      */
    typedef enum {
        ETH_HTTP_STATE_UNINIT = 0,
        ETH_HTTP_STATE_IDLE, /* socket closed, ready for connect */
        ETH_HTTP_STATE_DNS_RESOLVING, /* waiting DNS answer               */
        ETH_HTTP_STATE_TCP_OPENING, /* TCP_ClientOpen issued            */
        ETH_HTTP_STATE_CONNECTING, /* waiting TCPIP_TCP_IsConnected()  */
        ETH_HTTP_STATE_CONNECTED, /* socket up, no transaction        */
        ETH_HTTP_STATE_REQ_SEND_LINE, /* sending request line             */
        ETH_HTTP_STATE_REQ_SEND_HEADERS, /* sending header block             */
        ETH_HTTP_STATE_REQ_SEND_BODY, /* sending POST body                */
        ETH_HTTP_STATE_RSP_WAIT, /* waiting first response bytes     */
        ETH_HTTP_STATE_RSP_STATUS_LINE, /* parsing "HTTP/1.1 200 OK"        */
        ETH_HTTP_STATE_RSP_HEADERS, /* parsing response headers         */
        ETH_HTTP_STATE_RSP_BODY, /* streaming body to user callback  */
        ETH_HTTP_STATE_DISCONNECTING,
        ETH_HTTP_STATE_ERROR
    } ETH_HTTP_STATE;

    /* Supported HTTP methods.                                         */
    typedef enum {
        ETH_HTTP_METHOD_GET = 0,
        ETH_HTTP_METHOD_POST
    } ETH_HTTP_METHOD;

    /* Final result code passed to ETH_HTTP_DONE_CB.                   */
    typedef enum {
        ETH_HTTP_RES_OK = 0, /* full response delivered          */
        ETH_HTTP_RES_ERR_DNS,
        ETH_HTTP_RES_ERR_TCP_OPEN,
        ETH_HTTP_RES_ERR_TCP_CONNECT,
        ETH_HTTP_RES_ERR_TCP_RESET,
        ETH_HTTP_RES_ERR_TIMEOUT,
        ETH_HTTP_RES_ERR_BUSY, /* request rejected, FSM busy       */
        ETH_HTTP_RES_ERR_NOT_CONNECTED,
        ETH_HTTP_RES_ERR_PARAM,
        ETH_HTTP_RES_ERR_PARSE, /* malformed response               */
        ETH_HTTP_RES_ERR_USER_ABORT, /* body callback returned false     */
        ETH_HTTP_RES_ERR_INTERNAL
    } ETH_HTTP_RESULT;

    /* ============================================================== */
    /* Upper-layer callbacks                                           */
    /* ============================================================== */

    /*
       Headers callback.
       Called exactly once per transaction after the status line and all
       response headers have been parsed, before the first body chunk.

       Parameters:
           status     - HTTP status code (200, 404, ...)
           contentLen - value of Content-Length, or 0 if not advertised
           ctx        - user context passed at request time

       Return value:
           true  to continue receiving the body
           false to abort the transaction (callback will be invoked once
                 more with ETH_HTTP_RES_ERR_USER_ABORT)
     */
    typedef bool (*ETH_HTTP_HDR_CB)(uint16_t status,
            uint32_t contentLen,
            void *ctx);

    /*
       Body chunk callback.
       Invoked zero or more times during the body phase, each time with the
       newly received slice of bytes.

       Parameters:
           chunk      - pointer to bytes (valid only during the call)
           len        - size of this chunk in bytes
           offset     - byte offset of this chunk inside the full body
           totalLen   - announced total size (Content-Length), 0 if unknown
           ctx        - user context passed at request time

       Return value:
           true  to keep receiving
           false to abort the transaction
     */
    typedef bool (*ETH_HTTP_BODY_CB)(const uint8_t *chunk,
            uint16_t len,
            uint32_t offset,
            uint32_t totalLen,
            void *ctx);

    /*
       Done callback.
       Invoked exactly once per transaction, after the last body chunk has
       been delivered or when the transaction is aborted because of an error.
     */
    typedef void (*ETH_HTTP_DONE_CB)(ETH_HTTP_RESULT result,
            uint16_t status,
            uint32_t bodyReceived,
            void *ctx);

    /* ============================================================== */
    /* Request descriptor                                              */

    /* ============================================================== */
    typedef struct {
        ETH_HTTP_METHOD method;
        const char *path; /* "/api/v1/data"                  */
        const char *contentType; /* POST only, may be NULL          */
        const uint8_t *body; /* POST only, may be NULL          */
        uint32_t bodyLen;
        const char *extraHeaders; /* raw "Key: Value\r\n" lines, opt.*/
        ETH_HTTP_HDR_CB onHeaders; /* optional                        */
        ETH_HTTP_BODY_CB onBodyChunk; /* optional                        */
        ETH_HTTP_DONE_CB onDone; /* mandatory                       */
        void *ctx; /* opaque pointer for callbacks    */
    } ETH_HTTP_REQUEST;

    /* ============================================================== */
    /* Public API                                                      */
    /* ============================================================== */

    /* Initialize internal state, call once at boot. */
    void EthHttp_Initialize(void);

    /* Periodic task. Call from super-loop / RTOS thread. */
    void EthHttp_Task(void);

    /* Open a TCP connection to a server given a hostname (DNS resolved
       internally). Non-blocking.                                              */
    ETH_HTTP_RESULT EthHttp_ConnectByHost(const char *hostname, uint16_t port);

    /* Open a TCP connection to a server given a numeric IPv4 address.
       The "host" string parameter is used only for the HTTP Host: header. */
    ETH_HTTP_RESULT EthHttp_ConnectByIp(const char *host,
            const IP_MULTI_ADDRESS *addr,
            uint16_t port);

    /* Close current connection (graceful). Safe to call when idle. */
    ETH_HTTP_RESULT EthHttp_Disconnect(void);

    /* True if TCP socket is currently ESTABLISHED. */
    bool EthHttp_IsConnected(void);

    /* True if FSM is busy with an in-flight transaction. */
    bool EthHttp_IsBusy(void);

    /* Current FSM state (debug). */
    ETH_HTTP_STATE EthHttp_GetState(void);

    /* Set or clear the JWT bearer token. The token is copied internally and
       automatically emitted as "Authorization: Bearer <token>\r\n" for every
       subsequent request. Pass NULL or "" to clear.                           */
    ETH_HTTP_RESULT EthHttp_SetAuthToken(const char *token);

    /* High-level request helper. Both onDone and ctx are mandatory; the
       other callbacks may be NULL.                                            */
    ETH_HTTP_RESULT EthHttp_SendRequest(const ETH_HTTP_REQUEST *req);

    /* Convenience wrappers. */
    ETH_HTTP_RESULT EthHttp_Get(const char *path,
            const char *extraHeaders,
            ETH_HTTP_HDR_CB onHeaders,
            ETH_HTTP_BODY_CB onBodyChunk,
            ETH_HTTP_DONE_CB onDone,
            void *ctx);

    ETH_HTTP_RESULT EthHttp_Post(const char *path,
            const char *contentType,
            const uint8_t *body,
            uint32_t bodyLen,
            const char *extraHeaders,
            ETH_HTTP_HDR_CB onHeaders,
            ETH_HTTP_BODY_CB onBodyChunk,
            ETH_HTTP_DONE_CB onDone,
            void *ctx);



#ifdef	__cplusplus
}
#endif

#endif	/* ETH_HTTP_H */

