/* 
 * File:   eth_ftp.h
 * Author: LENOVO
 *
 * Created on June 6, 2026, 8:52 AM
 */

#ifndef ETH_FTP_H
#define	ETH_FTP_H

#include <stdlib.h>
#include "definitions.h"

#define MAX_FTP_RETRIES         2
#define FTP_CTRL_TIMEOUT_SEC    10

#ifdef	__cplusplus
extern "C" {
#endif

        /* * Enumeration for individual server status 
     */
    typedef enum {
        ETH_FTP_SERVER_IDLE = 0,
        ETH_FTP_SERVER_SUCCESS,
        ETH_FTP_SERVER_FAILED
    } ETH_FTP_SERVER_STATUS;

    /* * Structure to hold overall and individual upload status
     */
    typedef struct {
        bool isUploading; /* Common uploading status (true if FSM is busy) */
        ETH_FTP_SERVER_STATUS server1; /* Result for FTP Server 1 */
        ETH_FTP_SERVER_STATUS server2; /* Result for FTP Server 2 */
    } ETH_FTP_RESULT;

    typedef enum {
        FTP_STATE_IDLE = 0,
        FTP_STATE_INIT_SERVER,
        FTP_STATE_CONNECT,
        FTP_STATE_LOGIN,
        FTP_STATE_PREPARE_PATH,
        FTP_STATE_CWD_ROOT,
        FTP_STATE_MKD_START,
        FTP_STATE_MKD_DO,
        FTP_STATE_MKD_CWD,
        FTP_STATE_PUT_FILE,
        FTP_STATE_DISCONNECT,
        FTP_STATE_RETRY_DELAY,
        FTP_STATE_EVALUATE_NEXT
    } ETH_FTP_INTERNAL_STATE;

    /* * Initialize the FTP module 
     */
    void EthFtp_Initialize(void);

    /* * Non-blocking Task to be called in main loop 
     */
    void EthFtp_Task(void);

    /* * Trigger the upload process for selected servers
     * ftp1: set to true to upload to FTP server index 0
     * ftp2: set to true to upload to FTP server index 1
     */
    void EthFtp_TriggerUpload(bool ftp1, bool ftp2);

    /* * Get the detailed status of the FTP upload process
     */
    ETH_FTP_RESULT EthFtp_GetStatus(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ETH_FTP_H */

