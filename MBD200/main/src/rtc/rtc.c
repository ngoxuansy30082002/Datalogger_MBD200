#include "rtc.h"

RTC_DATA rtcDt;
RTC_DATA *ptrRtcDt = &rtcDt;

static const char * __TAG__ = "RTC";
static uint8_t _rxBuffer[ISL1208_RTC_SECTION_LEN] = {0};
static uint8_t _txBuffer[ISL1208_RTC_SECTION_LEN + 1] = {0};
static TIME _bufferTime = {0};

const RTC_I2C_PLIB _i2cPlib = {
    .read_t = (RTC_I2C_READ) I2C2_Read,
    .write_t = (RTC_I2C_WRITE) I2C2_Write,
    .transAbort = (RTC_I2C_TRANSFER_ABORT) I2C2_TransferAbort,
    .writeRead = (RTC_I2C_WRITE_READ) I2C2_WriteRead,
    .isBusy = (RTC_I2C_IS_BUSY) I2C2_IsBusy,
    .CallbackRegister = (RTC_I2C_CALLBACK_REGISTER) I2C2_CallbackRegister
};

/* Private function */

static void _i2cCallbackHandler(uintptr_t context) {
    RTC_DATA *ptr = (RTC_DATA *) context;
    rtcDt.state = ptr->nextState;
}

static void i2cSetTime(const TIME * tmp) {
    memset(_txBuffer, 0, sizeof (_txBuffer));

    _txBuffer[0] = ISL1208_REG_SC;
    _txBuffer[ISL1208_REG_YR + 1] = _bin2bcd(tmp->year % 100);
    _txBuffer[ISL1208_REG_MO + 1] = _bin2bcd(tmp->month);
    _txBuffer[ISL1208_REG_DT + 1] = _bin2bcd(tmp->day);
    _txBuffer[ISL1208_REG_DW + 1] = _bin2bcd(tmp->dayOfWeek);
    _txBuffer[ISL1208_REG_HR + 1] = _bin2bcd(tmp->hour) | 0x80; /* 24h clock */
    _txBuffer[ISL1208_REG_MN + 1] = _bin2bcd(tmp->minute);
    _txBuffer[ISL1208_REG_SC + 1] = _bin2bcd(tmp->second);
    _i2cPlib.write_t(ISL1208_ADDRESS, _txBuffer, ISL1208_RTC_SECTION_LEN + 1);
}

static void _i2cGetTime() {
    memset(_rxBuffer, 0, sizeof (_rxBuffer));
    memset(_txBuffer, 0, sizeof (_txBuffer));

    _txBuffer[0] = ISL1208_REG_SC;
    _i2cPlib.writeRead(ISL1208_ADDRESS, _txBuffer, 1, _rxBuffer, ISL1208_RTC_SECTION_LEN);
}

static uint8_t _calculateDayOfWeek(uint16_t year, uint8_t month, uint8_t day) {
    if (month < 3) {
        month += 12;
        year -= 1;
    }
    int k = year % 100;
    int j = year / 100;
    int dayOfWeek = (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    return ((dayOfWeek + 5) % 7) + 1; /* 1 = Monday, 2 = Tuesday, ..., 7 = Sunday */
}

static uint8_t _isLeapYear(int year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

static uint8_t _daysInMonth(int month, int year) {
    static const uint8_t days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && _isLeapYear(year)) {
        return 29;
    }
    return days[month - 1];
}

/* Public function */

void Rtc_Initialize() {
    _i2cPlib.CallbackRegister(_i2cCallbackHandler, (uintptr_t) ptrRtcDt);
    memset(&rtcDt, 0, sizeof (RTC_DATA));
}

void Rtc_Task() {
    static uint32_t pollTick = 0;

    switch (rtcDt.state) {
        case RTC_IDLE:
            if (rtcDt.f.bits.forceSet) {
                rtcDt.f.bits.forceSet = 0;
                rtcDt.state = RTC_SET;
                break;
            }

            if (TIME_IS_EXPIRED(pollTick, 200)
                    && ptrRtcDt->nextState == RTC_IDLE) {
                rtcDt.state = RTC_GET;
                pollTick = TICK_NOW();
            }
            break;
        case RTC_INIT:
            _txBuffer[0] = (uint8_t) (ISL1208_REG_SR);
            _txBuffer[1] = (uint8_t) (0x00 | ISL1208_REG_SR_WRTC);
            _i2cPlib.write_t(ISL1208_ADDRESS, _txBuffer, 2);
            rtcDt.f.bits.setNextTime = 1;
            ptrRtcDt->nextState = RTC_IDLE;
            rtcDt.state = RTC_IDLE;
            break;
        case RTC_SET:
            i2cSetTime(&_bufferTime);
            rtcDt.f.bits.isValidTime = 0;
            ptrRtcDt->nextState = RTC_GET;
            rtcDt.state = RTC_IDLE;
            break;
        case RTC_GET:
            _i2cGetTime();
            ptrRtcDt->nextState = RTC_PARSE_TIME;
            rtcDt.state = RTC_IDLE;
            break;
        case RTC_PARSE_TIME:
            rtcDt.sysTime.second = _bcd2bin(_rxBuffer[ISL1208_REG_SC] & 0x7F);
            rtcDt.sysTime.minute = _bcd2bin(_rxBuffer[ISL1208_REG_MN] & 0x7F);
            rtcDt.sysTime.hour = _bcd2bin(_rxBuffer[ISL1208_REG_HR] & 0x3F);
            rtcDt.sysTime.day = _bcd2bin(_rxBuffer[ISL1208_REG_DT] & 0x3F);
            rtcDt.sysTime.month = _bcd2bin(_rxBuffer[ISL1208_REG_MO] & 0x1F);
            rtcDt.sysTime.year = _bcd2bin(_rxBuffer[ISL1208_REG_YR]) + ((uint16_t) gAppCfg.time.yearNumber * 100);
            rtcDt.sysTime.dayOfWeek = _bcd2bin(_rxBuffer[ISL1208_REG_DW] & 0x07);
            //            SYS_CONSOLE_PRINT("%s - %s\t %04u-%02u-%02uT%02u:%02u:%02u\r\n",
            //                    __TAG__, __func__,
            //                    rtcDt.sysTime.year, rtcDt.sysTime.month, rtcDt.sysTime.day, rtcDt.sysTime.hour, rtcDt.sysTime.minute, rtcDt.sysTime.second);

            if (rtcDt.sysTime.day >= 1 && rtcDt.sysTime.day <= 31 &&
                    rtcDt.sysTime.month >= 1 && rtcDt.sysTime.month <= 12 &&
                    rtcDt.sysTime.year >= 2000 && rtcDt.sysTime.year <= 3000)
                rtcDt.f.bits.isValidTime = 1;


            ptrRtcDt->nextState = RTC_IDLE;
            rtcDt.state = RTC_IDLE;
            break;
    }
}

void Rtc_updateFromManual(const TIME *newTime) {
    if (newTime == NULL) return;

    _bufferTime.year = newTime->year;
    _bufferTime.month = newTime->month;
    _bufferTime.day = newTime->day;
    _bufferTime.hour = newTime->hour;
    _bufferTime.minute = newTime->minute;
    _bufferTime.second = newTime->second;
    _bufferTime.dayOfWeek = _calculateDayOfWeek(_bufferTime.year, _bufferTime.month, _bufferTime.day);

    rtcDt.f.bits.forceSet = 1;
}

static time_t _adjustTimeZone(time_t tUnixSeconds, uint8_t timeZone) {
    int offset_seconds = (int) (timeZone * 3600);
    tUnixSeconds += offset_seconds;

    return tUnixSeconds;
}

void Rtc_parseUnixTime(time_t tUnixSeconds, TIME *rtc) {
    tUnixSeconds = _adjustTimeZone(tUnixSeconds, gAppCfg.time.timeZone);

    struct tm *time_info;
    time_info = localtime(&tUnixSeconds);

    rtc->hour = time_info->tm_hour;
    rtc->minute = time_info->tm_min;
    rtc->second = time_info->tm_sec;
    rtc->day = time_info->tm_mday;
    rtc->month = time_info->tm_mon + 1;
    rtc->year = time_info->tm_year + 1900;
    rtc->dayOfWeek = time_info->tm_wday;
}

void Rtc_updateFromGsmNtp(const char * timeString) {
    struct tm t;
    int tz_units;

    memset(&t, 0, sizeof (struct tm));
    if (sscanf(timeString, "%d/%d/%d,%d:%d:%d%d",
            &t.tm_year, &t.tm_mon, &t.tm_mday,
            &t.tm_hour, &t.tm_min, &t.tm_sec,
            &tz_units) < 7) {
        return;
    }

    t.tm_year += 2000;
    //    LOG_DEBUG("%s - %s\t %04u-%02u-%02uT%02u:%02u:%02u+%02u\r\n",
    //            __TAG__, __func__,
    //            t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, tz_units);

    t.tm_year -= 1900;
    t.tm_mon -= 1;

    time_t unix_time = mktime(&t);
    unix_time -= (time_t) tz_units * 900;

    Rtc_parseUnixTime(unix_time, &_bufferTime);
    rtcDt.f.bits.forceSet = 1;
}

void Rtc_updateFromEthNtp(time_t tUnixSeconds) {
    Rtc_parseUnixTime(tUnixSeconds, &_bufferTime);
    rtcDt.f.bits.forceSet = 1;
}

TIME Rtc_getNextTime(TIME currentTime, int interval) {
    TIME nextTime = currentTime;

    uint8_t totalMinutes = nextTime.minute + interval;
    nextTime.minute = totalMinutes % 60;
    uint8_t totalHours = nextTime.hour + totalMinutes / 60;
    nextTime.hour = totalHours % 24;
    uint8_t totalDays = totalHours / 24;

    while (totalDays > 0) {
        totalDays--;

        uint8_t daysInCurrentMonth = _daysInMonth(nextTime.month, nextTime.year);
        if (nextTime.day < daysInCurrentMonth) {
            nextTime.day++;
        } else {
            nextTime.day = 1;
            if (nextTime.month == 12) {
                nextTime.month = 1;
                nextTime.year++;
            } else {
                nextTime.month++;
            }
        }
    }

    nextTime.dayOfWeek = (nextTime.dayOfWeek + totalDays) % 7;

    return nextTime;
}

bool Rtc_isTimeEqual(TIME time1, TIME time2) {
    return (time1.hour == time2.hour &&
            time1.minute == time2.minute &&
            time1.day == time2.day &&
            time1.month == time2.month &&
            time1.year == time2.year);
}