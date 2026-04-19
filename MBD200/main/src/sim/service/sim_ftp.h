/* 
 * File:   sim_ftp.h
 * Author: LENOVO
 *
 * Created on March 31, 2026, 8:21 PM
 */

#ifndef SIM_FTP_H
#define	SIM_FTP_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef enum {
        SIM_FTP_IDLE = 0,

        SIM_FTP_CFG_ACCOUNT,
        SIM_FTP_CFG_FILETYPE,
        SIM_FTP_CFG_TRANSMODE,
        SIM_FTP_CFG_CONTEXTID,
        SIM_FTP_CFG_RSPTIMEOUT,

        SIM_FTP_OPEN,
        SIM_FTP_WAIT_LOGIN,
        SIM_FTP_TRY_CLOSE,
        SIM_FTP_TRY_WAIT_LOGOUT,

        SIM_FTP_UPLOAD_ROOT,
        SIM_FTP_UPLOAD_FULLPATH,
        SIM_FTP_UPLOAD_CWD,
        SIM_FTP_UPLOAD_MKD,
        SIM_FTP_UPLOAD_NEXT_DIR,
        SIM_FTP_UPLOAD_PUT_CMD,
        SIM_FTP_UPLOAD_PUT_DATA,

        SIM_FTP_CLOSE,
        SIM_FTP_WAIT_LOGOUT,

        SIM_FTP_READY,
        SIM_FTP_ERROR,
        SIM_FTP_COUNT
    } SIM_FTP_STATE;

    bool SIMFtp_Start(uint8_t idxServer, const char * path, uint16_t pathLen, const char * fileData, uint16_t fileSize);
    void SIMFtp_Process(void);
    void SIMFtp_Abort(void);
    bool SIMFtp_IsReady(void);
    bool SIMFtp_HasError(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SIM_FTP_H */

