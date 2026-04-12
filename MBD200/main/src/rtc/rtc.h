/* 
 * File:   rtc.h
 * Author: Syxn
 *
 * Created on August 6, 2024, 8:17 AM
 */

#ifndef RTC_H
#define	RTC_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "definitions.h"

#define ISL1208_ADDRESS         0x6F


#define ISL1208_REG_SC          0x00
#define ISL1208_REG_MN          0x01
#define ISL1208_REG_HR          0x02
#define ISL1208_REG_HR_MIL      (1<<7)	/* 24h/12h mode */
#define ISL1208_REG_HR_PM       (1<<5)	/* PM/AM bit in 12h mode */
#define ISL1208_REG_DT          0x03
#define ISL1208_REG_MO          0x04
#define ISL1208_REG_YR          0x05
#define ISL1208_REG_DW          0x06
#define ISL1208_RTC_SECTION_LEN 7


#define ISL1208_REG_SR          0x07
#define ISL1208_REG_SR_ARST     (1<<7)	/* auto reset */
#define ISL1208_REG_SR_XTOSCB   (1<<6)	/* crystal oscillator */
#define ISL1208_REG_SR_WRTC     (1<<4)	/* write rtc */
#define ISL1208_REG_SR_EVT      (1<<3)	/* event */
#define ISL1208_REG_SR_ALM      (1<<2)	/* alarm */
#define ISL1208_REG_SR_BAT      (1<<1)	/* battery */
#define ISL1208_REG_SR_RTCF     (1<<0)	/* rtc fail */
#define ISL1208_REG_INT         0x08
#define ISL1208_REG_INT_ALME    (1<<6)   /* alarm enable */
#define ISL1208_REG_INT_IM      (1<<7)   /* interrupt/alarm mode */
#define ISL1219_REG_EV          0x09
#define ISL1219_REG_EV_EVEN     (1<<4)   /* event detection enable */
#define ISL1219_REG_EV_EVIENB   (1<<7)   /* event in pull-up disable */
#define ISL1208_REG_ATR         0x0a
#define ISL1208_REG_DTR         0x0b


#ifdef	__cplusplus
extern "C" {
#endif

    typedef void (* RTC_I2C_CALLBACK)(uintptr_t context);
    typedef bool (* RTC_I2C_READ)(uint16_t address, uint8_t* rdata, size_t rlength);
    typedef bool (* RTC_I2C_WRITE)(uint16_t address, uint8_t* wdata, size_t wlength);
    typedef bool (* RTC_I2C_WRITE_READ)(uint16_t address, uint8_t* wdata, size_t wlength, uint8_t* rdata, size_t rlength);
    typedef bool (* RTC_I2C_IS_BUSY)(void);
    typedef bool (* RTC_I2C_TRANSFER_ABORT)(void);
    typedef void (* RTC_I2C_CALLBACK_REGISTER)(RTC_I2C_CALLBACK callback, uintptr_t context);

    typedef struct {
        RTC_I2C_READ read_t;
        RTC_I2C_WRITE write_t;
        RTC_I2C_WRITE_READ writeRead;
        RTC_I2C_IS_BUSY isBusy;
        RTC_I2C_TRANSFER_ABORT transAbort;
        RTC_I2C_CALLBACK_REGISTER CallbackRegister;
    } RTC_I2C_PLIB;

    typedef union {

        struct {
            unsigned int forceSet : 1;
            unsigned int isValidTime : 1;
            unsigned int setNextTime : 1;
            unsigned int reserved : 5;
        } bits;
        uint8_t val;
    } RTC_FLAG;

    typedef enum {
        RTC_IDLE = 0,
        RTC_INIT,
        RTC_SET,
        RTC_GET,
        RTC_PARSE_TIME,
    } RTC_STATES;

    typedef struct {
        RTC_STATES state;
        RTC_STATES nextState;
        RTC_FLAG f;
        TIME sysTime;
    } RTC_DATA;

    void Rtc_Initialize(void);
    void Rtc_Task(void);

    extern RTC_DATA rtcDt;

    void Rtc_updateFromManual(const char *timeString);
    void Rtc_updateFromGsmNtp(const char *timeString);
    void Rtc_updateFromEthNtp(time_t tUnixSeconds);

    TIME Rtc_getNextTime(TIME currentTime, int interval);
    bool Rtc_isTimeEqual(TIME time1, TIME time2);

    static inline uint8_t _bcd2bin(uint8_t bcd) {
        return (bcd & 0x0F) + 10 * ((bcd & 0xF0) >> 4);
    }

    static inline uint8_t _bin2bcd(uint8_t bin) {
        return ((bin / 10) << 4) | (bin % 10);
    }

#ifdef	__cplusplus
}
#endif

#endif	/* RTC_H */

