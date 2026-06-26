/* 
 * File:   fota.h
 * Author: LENOVO
 *
 * Created on June 26, 2026, 5:08 PM
 */

#ifndef FOTA_H
#define	FOTA_H

#include <stdlib.h>
#include <string.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif


    /* ============================================================== */
    /* Compile-time configuration                                      */
    /* ============================================================== */
#define FOTA_DEVICE_ID_MAX_LEN          32U
#define FOTA_FW_VERSION_MAX_LEN         16U
#define FOTA_HW_VERSION_MAX_LEN         16U
#define FOTA_TOPIC_MAX_LEN              96U
#define FOTA_URL_MAX_LEN                160U
#define FOTA_HOST_MAX_LEN               64U
#define FOTA_PATH_MAX_LEN               128U
#define FOTA_TOKEN_MAX_LEN              512U
#define FOTA_HEX_LINE_MAX_LEN           600U

#define FOTA_HEARTBEAT_PERIOD_MS        30000U
#define FOTA_HTTP_OP_TMO_MS             30000U

#define FOTA_AUTH_USERNAME              "admin"
#define FOTA_AUTH_PASSWORD              "admin123"
#define FOTA_AUTH_PATH                  "/api/auth/login"

    /* ============================================================== */
    /* Configuration passed at init time                               */

    /* ============================================================== */
    typedef struct {
        const char *deviceId; /* used in heartbeat topic & payload    */
        const char *firmwareVersion; /* current image version, e.g. "3.2.0"  */
        const char *hardwareVersion; /* e.g. "2.0"                           */
        const char *deviceModel; /* e.g. "MBD200"                        */
    } FOTA_CONFIG;

    /* ============================================================== */
    /* High-level FSM state, queryable for diagnostics                 */

    /* ============================================================== */
    typedef enum {
        FOTA_STATE_UNINIT = 0,
        FOTA_STATE_IDLE, /* steady state, doing heartbeat only */
        FOTA_STATE_QUERY_PENDING, /* notify received, query published   */
        FOTA_STATE_AWAIT_RESPONSE, /* waiting for /response message      */
        FOTA_STATE_HTTP_AUTH_CONNECT,
        FOTA_STATE_HTTP_AUTH_REQUEST,
        FOTA_STATE_HTTP_AUTH_WAIT,
        FOTA_STATE_HTTP_DL_CONNECT,
        FOTA_STATE_HTTP_DL_REQUEST,
        FOTA_STATE_HTTP_DL_STREAMING,
        FOTA_STATE_VERIFY_AND_COMMIT,
        FOTA_STATE_ERROR
    } FOTA_STATE;

    /* ============================================================== */
    /* Public API                                                      */
    /* ============================================================== */

    void Fota_Initialize(const FOTA_CONFIG *cfg);
    void Fota_Task(void);
    FOTA_STATE Fota_GetState(void);

    /* Optional: external trigger (e.g. CLI command) to force a check. */
    void Fota_TriggerCheck(void);



#ifdef	__cplusplus
}
#endif

#endif	/* FOTA_H */

