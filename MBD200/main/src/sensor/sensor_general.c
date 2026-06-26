#include "sensor_general.h"

SENSOR_GENERAL_DATA ssGeneralDt = {0};

static const char * __TAG__ = "SENSOR";
static LOG_FILE_STATE _logState = LOG_FILE_IDLE;
static LOG_FILE_QUEUE _fileQueue;
static LOG_FILE _fileBuffer;
static LOG_FILE _currentFile;
static TIME _logNextTime = {0};

static UPLINK_PHASE _uplinkPhase = UPLINK_PHASE_PRIMARY;
static bool _fallbackServer[MAX_FTP_SERVER] = {0};

static char _mqttJsonBuf[MQTT_PAYLOAD_LEN];
static uint32_t _mqttLastPublishMs = 0;


static RULE_RUNTIME_STATE _ruleState[MAX_RULE] = {0};
static uint32_t _ruleScanLastMs = 0;
static char _ruleMqttBuf[MQTT_PAYLOAD_LEN];

static void _logFileInitQueue(LOG_FILE_QUEUE *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

static int _logFileGetSize(LOG_FILE_QUEUE *q) {
    return q->size;
}

static bool _logFileIsQueueFull(LOG_FILE_QUEUE *q) {
    return q->size == FILE_QUEUE_SIZE;
}

static bool _logFileIsQueueEmpty(LOG_FILE_QUEUE *q) {
    return q->size == 0;
}

static bool _logFileEnqueue(LOG_FILE_QUEUE *q, LOG_FILE *f) {
    if (_logFileIsQueueFull(q)) {
        return false; // Queue is full
    }
    q->rear = (q->rear + 1) % FILE_QUEUE_SIZE;
    q->items[q->rear] = *f;
    q->size++;
    return true;
}

static bool _logFileDequeue(LOG_FILE_QUEUE *q, LOG_FILE *f) {
    if (_logFileIsQueueEmpty(q)) {
        return false; // Queue is empty
    }
    *f = q->items[q->front];
    q->front = (q->front + 1) % FILE_QUEUE_SIZE;
    q->size--;
    return true;
}

static void _replaceSubString(char *target, size_t maxLen, const char *find, const char *replace) {
    char buffer[128]; // Ensure this is large enough for your filename
    char *insertPoint = &buffer[0];
    const char *tmp = target;
    size_t findLen = strlen(find);
    size_t replaceLen = strlen(replace);

    while (1) {
        const char *p = strstr(tmp, find);
        if (p == NULL) {
            strcpy(insertPoint, tmp);
            break;
        }

        // Copy the part before the found string
        memcpy(insertPoint, tmp, p - tmp);
        insertPoint += p - tmp;

        // Copy the replacement string
        memcpy(insertPoint, replace, replaceLen);
        insertPoint += replaceLen;

        // Move to the rest of the string
        tmp = p + findLen;
    }

    // Copy back to the target, ensuring we don't overflow
    strncpy(target, buffer, maxLen - 1);
    target[maxLen - 1] = '\0';
}

// Generate actual filename by replacing tags in the template with current RTC time

static void _generateFilenameFromTemplate(char *outName, size_t maxLen, TIME curTime) {
    char buf[10];

    // Start with the user's template
    strncpy(outName, gSensorCfg.filenameTemplate, maxLen - 1);
    outName[maxLen - 1] = '\0';

    snprintf(buf, sizeof (buf), "%04u", curTime.year);
    _replaceSubString(outName, maxLen, "{YYYY}", buf);

    snprintf(buf, sizeof (buf), "%02u", curTime.month);
    _replaceSubString(outName, maxLen, "{MM}", buf);

    snprintf(buf, sizeof (buf), "%02u", curTime.day);
    _replaceSubString(outName, maxLen, "{DD}", buf);

    snprintf(buf, sizeof (buf), "%02u", curTime.hour);
    _replaceSubString(outName, maxLen, "{hh}", buf);

    snprintf(buf, sizeof (buf), "%02u", curTime.minute);
    _replaceSubString(outName, maxLen, "{mm}", buf);

    snprintf(buf, sizeof (buf), "%02u", curTime.second);
    _replaceSubString(outName, maxLen, "{ss}", buf);

    // Add extension based on type
    if (gSensorCfg.typefile == FILE_TYPE_TXT) {
        strncat(outName, ".txt", maxLen - strlen(outName) - 1);
    } else {
        strncat(outName, ".csv", maxLen - strlen(outName) - 1);
    }
}

static bool _createFile(void) {
    memset(&_fileBuffer.file, 0, sizeof (_fileBuffer.file));
    _fileBuffer.numFile = 0;
    _fileBuffer.isRetry = false;

    char delimiter[4];
    uint16_t fileSize = 0;
    char newLine[128];
    char unitStr[24];
    char nameStr[32];
    char timeStr[24];
    char valStr[24];
    char sttStr[8];

    // Determine delimiter based on file type
    if (gSensorCfg.typefile == FILE_TYPE_TXT) {
        strcpy(delimiter, "\t");
    } else {
        strcpy(delimiter, ",");
    }

    // 1. Generate the actual file name
    _generateFilenameFromTemplate(_fileBuffer.file.name, sizeof (_fileBuffer.file.name), rtcDt.sysTime);
    _fileBuffer.file.time = rtcDt.sysTime;

    // Time string for data lines (YYYYMMDDhhmmss format is common for both DNA and TT24)
    snprintf(timeStr, sizeof (timeStr), "%04u%02u%02u%02u%02u%02u",
            rtcDt.sysTime.year, rtcDt.sysTime.month, rtcDt.sysTime.day,
            rtcDt.sysTime.hour, rtcDt.sysTime.minute, rtcDt.sysTime.second);

    // 2. Iterate through all configured sensors
    for (uint8_t idx = 0; idx < gSensorCfg.numSensor; idx++) {
        SENSOR_ENTRY_CONFIG *sEntry = &gSensorCfg.entry[idx];

        if (!sEntry->enable)
            continue;

        memset(newLine, 0, sizeof (newLine));
        memset(unitStr, 0, sizeof (unitStr));
        memset(nameStr, 0, sizeof (nameStr));
        memset(valStr, 0, sizeof (valStr));
        memset(sttStr, 0, sizeof (sttStr));

        uint8_t tagIdx = sEntry->indexOfType;
        bool sensorValid = false;

        // --- Fetch Data based on Sensor Type ---
        switch (sEntry->type) {
            case SENSOR_MBRTU:
                if (gMbrtuCfg.entry[tagIdx].enable) {
                    sensorValid = true;
                    // Format Name
                    if (gSensorCfg.formatFile == FORMAT_FILE_TT24) {
                        snprintf(nameStr, sizeof (nameStr), "%s%s", gMbrtuCfg.entry[tagIdx].name, delimiter);
                        snprintf(unitStr, sizeof (unitStr), "%s%s", gMbrtuCfg.entry[tagIdx].unit, delimiter);
                    } else {
                        snprintf(nameStr, sizeof (nameStr), "%s", gMbrtuCfg.entry[tagIdx].name);
                        snprintf(unitStr, sizeof (unitStr), "%s", gMbrtuCfg.entry[tagIdx].unit);
                    }

                    if (mbrtuMasterDt.entry[tagIdx].dataType == DATA_FLOAT)
                        snprintf(valStr, sizeof (valStr), "%.02f%s", mbrtuMasterDt.entry[tagIdx].value.floatVal, delimiter);
                    if (mbrtuMasterDt.entry[tagIdx].dataType == DATA_INT)
                        snprintf(valStr, sizeof (valStr), "%lld%s", mbrtuMasterDt.entry[tagIdx].value.intVal, delimiter);
                    if (mbrtuMasterDt.entry[tagIdx].dataType == DATA_UINT)
                        snprintf(valStr, sizeof (valStr), "%llu%s", mbrtuMasterDt.entry[tagIdx].value.uintVal, delimiter);
                }
                break;

            case SENSOR_ANALOG:
                if (gAnalogCfg.entry[tagIdx].enable) {
                    sensorValid = true;
                    if (gSensorCfg.formatFile == FORMAT_FILE_TT24) {
                        snprintf(nameStr, sizeof (nameStr), "%s%s", gAnalogCfg.entry[tagIdx].name, delimiter);
                        snprintf(unitStr, sizeof (unitStr), "%s%s", gAnalogCfg.entry[tagIdx].unit, delimiter);
                    } else {
                        snprintf(nameStr, sizeof (nameStr), "%s", gAnalogCfg.entry[tagIdx].name);
                        snprintf(unitStr, sizeof (unitStr), "%s", gAnalogCfg.entry[tagIdx].unit);
                    }

                    snprintf(valStr, sizeof (valStr), "%.02f%s", adcDt.entry[tagIdx].value, delimiter);
                }
                break;

            case SENSOR_INPUT_CAPTURE:
                if (gInCaptureCfg.entry[tagIdx].enable) {
                    sensorValid = true;
                    if (gSensorCfg.formatFile == FORMAT_FILE_TT24) {
                        snprintf(nameStr, sizeof (nameStr), "%s%s", gInCaptureCfg.entry[tagIdx].name, delimiter);
                        snprintf(unitStr, sizeof (unitStr), "%s%s", gInCaptureCfg.entry[tagIdx].unit, delimiter);
                    } else {
                        snprintf(nameStr, sizeof (nameStr), "%s", gInCaptureCfg.entry[tagIdx].name);
                        snprintf(unitStr, sizeof (unitStr), "%s", gInCaptureCfg.entry[tagIdx].unit);
                    }

                    snprintf(valStr, sizeof (valStr), "%.02f%s", inputCaptureDt.entry[tagIdx].value, delimiter);
                }
                break;

            default:
                break;
        }

        if (!sensorValid)
            continue;

        // --- Determine Status Code ---
        // TODO: You need to implement APP_CalculateStatusSensorNew(idx) based on the new conditions 
        int8_t stt = SensorGeneral_calculateSensorStatusInput(idx);
        if (stt == 0) snprintf(sttStr, sizeof (sttStr), "00"); // GOOD
        else if (stt == 1) snprintf(sttStr, sizeof (sttStr), "01"); // CALIBRATION
        else if (stt == 2) snprintf(sttStr, sizeof (sttStr), "02"); // BAD/ERROR
        else {
            if (sEntry->calibrate)
                snprintf(sttStr, sizeof (sttStr), "%s", "01");
            else {
                stt = SensorGeneral_calculateSensorStatusAuto(sEntry->type, tagIdx);
                if (stt == -1)
                    continue;

                snprintf(sttStr, sizeof (sttStr), "%02u", stt);
            }
        }

        // --- Assemble Line ---
        if (gSensorCfg.formatFile == FORMAT_FILE_TT24) {
            // TT24 Format: Name[delim]Value[delim]Unit[delim]Time[delim]Status
            fileSize += snprintf(newLine, sizeof (newLine), "%s%s%s%s%s%s",
                    nameStr, valStr, unitStr, timeStr, delimiter, sttStr);
        } else {
            // DNA Format: Time[delim]Name[delim]Value[delim]Unit
            fileSize += snprintf(newLine, sizeof (newLine), "%s%s%s%s%s",
                    timeStr, delimiter, nameStr, delimiter, valStr, unitStr);
        }

        strcat(_fileBuffer.file.content, newLine);

        // Add newline if it's not the last valid sensor
        if (idx != gSensorCfg.numSensor - 1) {
            strcat(_fileBuffer.file.content, "\n");
            fileSize++;
        }
    }

    _fileBuffer.file.size = fileSize;

    if (fileSize > 0) {
        _fileBuffer.numFile = 1;
        LOG_DEBUG("%s - %s\t Created %s (Size: %u bytes)", __TAG__, __func__, _fileBuffer.file.name, fileSize);
        //        LOG_DEBUG("%s - %s\t Content %s", __TAG__, __func__, _fileBuffer.file.content);
        return true;
    }

    LOG_DEBUG("%s - %s\t Failed to create file. Size is 0.", __TAG__, __func__);
    return false;
}

static void _createLogFileTask(void) {
    if (!rtcDt.f.bits.isValidTime || gSensorCfg.logInterval <= 0)
        return;
    if (_logNextTime.day == 0 && _logNextTime.month == 0) {
        _logNextTime = Rtc_getNextTime(rtcDt.sysTime, gSensorCfg.logInterval);
        LOG_DEBUG("%s - %s\t %04u-%02u-%02uT%02u:%02u:%02u",
                __TAG__, __func__,
                _logNextTime.year, _logNextTime.month, _logNextTime.day,
                _logNextTime.hour, _logNextTime.minute, _logNextTime.second);
    }

    if (Rtc_isTimeEqual(rtcDt.sysTime, _logNextTime)) {
        if (_createFile()) {
            if (_logFileEnqueue(&_fileQueue, &_fileBuffer)) {
                LOG_DEBUG("%s - %s\t Pushed to queue.", __TAG__, __func__);
            } else {
                LOG_DEBUG("%s - %s\t Queue full. Dropped.", __TAG__, __func__);
            }
        }
        _logNextTime = Rtc_getNextTime(rtcDt.sysTime, gSensorCfg.logInterval);
        LOG_DEBUG("%s - %s\t %04u-%02u-%02uT%02u:%02u:%02u",
                __TAG__, __func__,
                _logNextTime.year, _logNextTime.month, _logNextTime.day,
                _logNextTime.hour, _logNextTime.minute, _logNextTime.second);
    }
}

static bool _IsFtpDone(void) {
    switch (gAppCfg.network.uplink) {
        case UPLINK_GSM:
            return (SIMMain_FTPIsSuccess() ||
                    SIMMain_FTPHasError());

        case UPLINK_ETH:
        {
            ETH_FTP_RESULT ethFtpRes = EthFtp_GetStatus();
            if (!ethFtpRes.isUploading)
                return true;

            break;
        }


        case UPLINK_ALL:
        {
            if (_uplinkPhase == UPLINK_PHASE_PRIMARY) {
                ETH_FTP_RESULT ethFtpRes = EthFtp_GetStatus();
                if (!ethFtpRes.isUploading)
                    return true;
            } else {
                return SIMMain_FTPIsSuccess();
            }
            break;
        }


        default: return true;
    }
    return false;
}

static bool _hasFtpError(void) {
    bool isError = false;

    switch (gAppCfg.network.uplink) {
        case UPLINK_GSM:
        {
            bool ftp1Success = false, ftp2Success = false;
            SIMMain_FTPGetResult(&ftp1Success, &ftp2Success);
            if (gAppCfg.ftpServer[0].enable && !ftp1Success) {
                isError = true;
                _currentFile.server[0].isErr = true;
            }
            if (gAppCfg.ftpServer[1].enable && !ftp2Success) {
                _currentFile.server[1].isErr = true;
                isError = true;
            }

            break;
        }

        case UPLINK_ETH:
        {
            ETH_FTP_RESULT ethFtpRes = EthFtp_GetStatus();
            if (gAppCfg.ftpServer[0].enable &&
                    ethFtpRes.server1 == ETH_FTP_SERVER_FAILED) {
                isError = true;
                _currentFile.server[0].isErr = true;
            }
            if (gAppCfg.ftpServer[1].enable &&
                    ethFtpRes.server2 == ETH_FTP_SERVER_FAILED) {
                _currentFile.server[1].isErr = true;
                isError = true;
            }

            break;
        }

        case UPLINK_ALL:
        {
            if (_uplinkPhase == UPLINK_PHASE_PRIMARY) {
                ETH_FTP_RESULT ethFtpRes = EthFtp_GetStatus();
                if (gAppCfg.ftpServer[0].enable &&
                        ethFtpRes.server1 == ETH_FTP_SERVER_FAILED) {
                    isError = true;
                    _currentFile.server[0].isErr = true;
                }
                if (gAppCfg.ftpServer[1].enable &&
                        ethFtpRes.server2 == ETH_FTP_SERVER_FAILED) {
                    isError = true;
                    _currentFile.server[1].isErr = true;
                }
            } else {
                bool ftp1Success = false, ftp2Success = false;
                SIMMain_FTPGetResult(&ftp1Success, &ftp2Success);
                if (_fallbackServer[0] && !ftp1Success) {
                    isError = true;
                    _currentFile.server[0].isErr = true;
                }
                if (_fallbackServer[1] && !ftp2Success) {
                    isError = true;
                    _currentFile.server[1].isErr = true;
                }
            }
            break;
        }

        default: return false;
    }
    return isError;
}

/* JSON-escape a string into dst. Returns bytes written (excl. NUL). */
static size_t _jsonEscapeString(char *dst, size_t dstSize, const char *src) {
    size_t i = 0;
    if (dst == NULL || dstSize == 0) return 0;
    if (src == NULL) {
        dst[0] = '\0';
        return 0;
    }

    while (*src != '\0' && i < dstSize - 1) {
        char c = *src++;
        if (c == '"' || c == '\\') {
            if (i + 2 >= dstSize - 1) break;
            dst[i++] = '\\';
            dst[i++] = c;
        } else if (c == '\n') {
            if (i + 2 >= dstSize - 1) break;
            dst[i++] = '\\';
            dst[i++] = 'n';
        } else if (c == '\r') {
            if (i + 2 >= dstSize - 1) break;
            dst[i++] = '\\';
            dst[i++] = 'r';
        } else if (c == '\t') {
            if (i + 2 >= dstSize - 1) break;
            dst[i++] = '\\';
            dst[i++] = 't';
        } else {
            dst[i++] = c;
        }
    }
    dst[i] = '\0';
    return i;
}

/* Build sensor data JSON array. Returns true if at least one sensor emitted. */
static bool _buildSensorJson(char *out, size_t outSize) {
    if (out == NULL || outSize < 4) return false;

    size_t pos = 0;
    int written;
    bool firstItem = true;

    written = snprintf(out + pos, outSize - pos, "[");
    if (written < 0) return false;
    pos += (size_t) written;

    for (uint8_t idx = 0; idx < gSensorCfg.numSensor; idx++) {
        SENSOR_ENTRY_CONFIG *sEntry = &gSensorCfg.entry[idx];
        if (!sEntry->enable) continue;

        uint8_t tagIdx = sEntry->indexOfType;
        const char *name = NULL;
        const char *unit = NULL;
        char valStr[32] = {0};
        bool sensorValid = false;

        switch (sEntry->type) {
            case SENSOR_MBRTU:
                if (gMbrtuCfg.entry[tagIdx].enable) {
                    sensorValid = true;
                    name = gMbrtuCfg.entry[tagIdx].name;
                    unit = gMbrtuCfg.entry[tagIdx].unit;

                    if (mbrtuMasterDt.entry[tagIdx].dataType == DATA_FLOAT)
                        snprintf(valStr, sizeof (valStr), "%.2f",
                            mbrtuMasterDt.entry[tagIdx].value.floatVal);
                    else if (mbrtuMasterDt.entry[tagIdx].dataType == DATA_INT)
                        snprintf(valStr, sizeof (valStr), "%lld",
                            mbrtuMasterDt.entry[tagIdx].value.intVal);
                    else if (mbrtuMasterDt.entry[tagIdx].dataType == DATA_UINT)
                        snprintf(valStr, sizeof (valStr), "%llu",
                            mbrtuMasterDt.entry[tagIdx].value.uintVal);
                    else
                        snprintf(valStr, sizeof (valStr), "0");
                }
                break;

            case SENSOR_ANALOG:
                if (gAnalogCfg.entry[tagIdx].enable) {
                    sensorValid = true;
                    name = gAnalogCfg.entry[tagIdx].name;
                    unit = gAnalogCfg.entry[tagIdx].unit;
                    snprintf(valStr, sizeof (valStr), "%.2f",
                            adcDt.entry[tagIdx].value);
                }
                break;

            case SENSOR_INPUT_CAPTURE:
                if (gInCaptureCfg.entry[tagIdx].enable) {
                    sensorValid = true;
                    name = gInCaptureCfg.entry[tagIdx].name;
                    unit = gInCaptureCfg.entry[tagIdx].unit;
                    snprintf(valStr, sizeof (valStr), "%.2f",
                            inputCaptureDt.entry[tagIdx].value);
                }
                break;

            default:
                break;
        }

        if (!sensorValid) continue;

        /* Determine status code, same priority as _createFile() */
        int8_t stt = SensorGeneral_calculateSensorStatusInput(idx);
        if (stt < 0) {
            if (sEntry->calibrate) {
                stt = 1;
            } else {
                stt = SensorGeneral_calculateSensorStatusAuto(sEntry->type, tagIdx);
                if (stt < 0) continue;
            }
        }

        char nameEsc[64];
        char unitEsc[24];
        _jsonEscapeString(nameEsc, sizeof (nameEsc), name);
        _jsonEscapeString(unitEsc, sizeof (unitEsc), unit);

        written = snprintf(out + pos, outSize - pos,
                "%s{\"name\":\"%s\",\"value\":%s,\"unit\":\"%s\",\"statusText\":%d,\"type\":%d}",
                firstItem ? "" : ",",
                nameEsc, valStr, unitEsc, (int) stt, (int) sEntry->type);

        if (written < 0 || (size_t) written >= outSize - pos) {
            LOG_WARN("%s - %s\t JSON buffer overflow at idx=%u",
                    __TAG__, __func__, idx);
            break;
        }
        pos += (size_t) written;
        firstItem = false;
    }

    if (pos + 2 > outSize) return false;
    out[pos++] = ']';
    out[pos] = '\0';

    return !firstItem;
}

/* Periodic MQTT publish task - executes every publishInterval seconds */
static void _mqttPublishTask(void) {
    /* Skip if MQTT not configured or not connected */
    if (gAppCfg.mqtt.publishInterval == 0) return;
    if (gAppCfg.mqtt.valueTopic[0] == '\0') return;
    if (!EthMqtt_IsConnected()) return;

    if (!TIME_IS_EXPIRED(_mqttLastPublishMs, gAppCfg.mqtt.publishInterval * 1000U)) return;
    _mqttLastPublishMs = TICK_NOW();

    if (!_buildSensorJson(_mqttJsonBuf, sizeof (_mqttJsonBuf))) {
        LOG_DEBUG("%s - %s\t No sensor data to publish", __TAG__, __func__);
        return;
    }

    if (EthMqtt_Publish(gAppCfg.mqtt.valueTopic, _mqttJsonBuf)) {
        LOG_INFO("%s - %s\t MQTT publish to %s (%u bytes)",
                __TAG__, __func__, gAppCfg.mqtt.valueTopic,
                (unsigned) strlen(_mqttJsonBuf));
    } else {
        LOG_WARN("%s - %s\t MQTT publish queue busy", __TAG__, __func__);
    }
}

static bool _ruleGetSensorValue(uint8_t sensorIdx, float *outVal) {
    if (sensorIdx >= gSensorCfg.numSensor || outVal == NULL) return false;

    SENSOR_ENTRY_CONFIG *sEntry = &gSensorCfg.entry[sensorIdx];
    if (!sEntry->enable) return false;

    uint8_t tagIdx = sEntry->indexOfType;

    switch (sEntry->type) {
        case SENSOR_MBRTU:
            if (!gMbrtuCfg.entry[tagIdx].enable) return false;
            switch (mbrtuMasterDt.entry[tagIdx].dataType) {
                case DATA_FLOAT: *outVal = (float) mbrtuMasterDt.entry[tagIdx].value.floatVal;
                    break;
                case DATA_INT: *outVal = (float) mbrtuMasterDt.entry[tagIdx].value.intVal;
                    break;
                case DATA_UINT: *outVal = (float) mbrtuMasterDt.entry[tagIdx].value.uintVal;
                    break;
                default: return false;
            }
            return true;

        case SENSOR_ANALOG:
            if (!gAnalogCfg.entry[tagIdx].enable) return false;
            *outVal = adcDt.entry[tagIdx].value;
            return true;

        case SENSOR_INPUT_CAPTURE:
            if (!gInCaptureCfg.entry[tagIdx].enable) return false;
            *outVal = inputCaptureDt.entry[tagIdx].value;
            return true;

        default: return false;
    }
}

static inline bool _ruleCompare(RULE_OPERATOR op, float v, float th) {
    switch (op) {
        case RULE_GREATER_THAN: return v > th;
        case RULE_LESS_THAN: return v < th;
        case RULE_GREATER_OR_EQUAL: return v >= th;
        case RULE_LESS_OR_EQUAL: return v <= th;
        case RULE_EQUAL: return v == th;
        case RULE_NOT_EQUAL: return v != th;
        default: return false;
    }
}

static uint32_t _ruleDebounceToMs(float value, RULE_DEBOUNCE_UNIT unit) {
    switch (unit) {
        case RULE_DEBOUNCE_MILISECOND: return (uint32_t) (value);
        case RULE_DEBOUNCE_SECOND: return (uint32_t) (value * 1000.0f);
        case RULE_DEBOUNCE_MINUTE: return (uint32_t) (value * 60000.0f);
        case RULE_DEBOUNCE_HOUR: return (uint32_t) (value * 3600000.0f);
        default: return 0U;
    }
}

static void _rulePushHistory(RULE_RUNTIME_STATE *st, float v, uint32_t nowMs) {
    st->histVal [st->histHead] = v;
    st->histTick[st->histHead] = nowMs;
    st->histHead = (st->histHead + 1U) % RULE_DELTA_HISTORY_SIZE;
    if (st->histCount < RULE_DELTA_HISTORY_SIZE) st->histCount++;
}

/* Use the oldest sample that is still in the window [now-window, now].
 * Returns true if |vNow - vRef| >= deltaThreshold.       */
static bool _ruleEvalDelta(RULE_RUNTIME_STATE *st,
        float vNow, float deltaTh,
        uint32_t windowMs, uint32_t nowMs) {
    float vRef = vNow;
    bool found = false;

    for (uint8_t i = 0; i < st->histCount; i++) {
        uint8_t idx = (st->histHead + RULE_DELTA_HISTORY_SIZE - 1U - i)
                % RULE_DELTA_HISTORY_SIZE;
        uint32_t age = nowMs - st->histTick[idx];
        if (age <= windowMs) {
            vRef = st->histVal[idx];
            found = true;
        } else {
            break;
        }
    }
    if (!found) return false;
    float diff = vNow - vRef;
    if (diff < 0) diff = -diff;
    return diff >= deltaTh;
}

static bool _ruleEvalThreshold(const RULE_ENTRY_CONFIG *r) {
    float v1 = 0.0f;
    if (!_ruleGetSensorValue(r->sensorId1, &v1)) return false;
    bool c1 = _ruleCompare(r->op1, v1, r->value1);

    if (!r->enableCondition1) return c1;

    float v2 = 0.0f;
    if (!_ruleGetSensorValue(r->sensorId2, &v2)) return false;
    bool c2 = _ruleCompare(r->op2, v2, r->value2);

    return (r->logic == RULE_AND) ? (c1 && c2) : (c1 || c2);
}

static const char* _ruleOpToStr(RULE_OPERATOR op) {
    switch (op) {
        case RULE_GREATER_THAN: return ">";
        case RULE_LESS_THAN: return "<";
        case RULE_GREATER_OR_EQUAL: return ">=";
        case RULE_LESS_OR_EQUAL: return "<=";
        case RULE_EQUAL: return "==";
        case RULE_NOT_EQUAL: return "!=";
        default: return "?";
    }
}

static void _ruleBuildDetail(const RULE_ENTRY_CONFIG *r,
        char *out, size_t outSize) {
    if (r->type == RULE_THRESHOLD) {
        if (!r->enableCondition1) {
            snprintf(out, outSize,
                    "Value exceeded threshold (%s %.2f)",
                    _ruleOpToStr(r->op1), r->value1);
        } else {
            snprintf(out, outSize,
                    "Value exceeded threshold (%s %.2f %s %s %.2f)",
                    _ruleOpToStr(r->op1), r->value1,
                    (r->logic == RULE_AND) ? "AND" : "OR",
                    _ruleOpToStr(r->op2), r->value2);
        }
    } else { /* RULE_DELTA */
        uint32_t winMs = r->enableDebounce
                ? _ruleDebounceToMs(r->debounceValue, r->debounceUnit)
                : RULE_DELTA_DEFAULT_MS;
        snprintf(out, outSize,
                "Value changed too rapidly within %lu ms (delta >= %.2f)",
                (unsigned long) winMs, r->value1);
    }
}

static bool _ruleNotifyMqtt(const RULE_ENTRY_CONFIG *r) {
    if (gAppCfg.mqtt.notifyTopic[0] == '\0') return false;
    if (!EthMqtt_IsConnected()) return false;

    char nameEsc [SENSOR_NAME_LEN * 2];
    char detailRaw[160];
    char detailEsc[256];

    _jsonEscapeString(nameEsc, sizeof (nameEsc), r->name);
    _ruleBuildDetail(r, detailRaw, sizeof (detailRaw));
    _jsonEscapeString(detailEsc, sizeof (detailEsc), detailRaw);

    int written = snprintf(_ruleMqttBuf, sizeof (_ruleMqttBuf),
            "{\"target\":\"%s\","
            "\"detail\":\"%s\","
            "\"severity\":\"WARNING\"}",
            nameEsc, detailEsc);
    if (written < 0 || (size_t) written >= sizeof (_ruleMqttBuf)) {
        LOG_WARN("%s - %s\t Notify payload overflow", __TAG__, __func__);
        return false;
    }

    if (EthMqtt_Publish(gAppCfg.mqtt.notifyTopic, _ruleMqttBuf)) {
        LOG_INFO("%s - %s\t Rule '%s' fired -> %s",
                __TAG__, __func__, r->name, _ruleMqttBuf);
        return true;
    }
    LOG_WARN("%s - %s\t MQTT busy, retry next scan", __TAG__, __func__);
    return false;
}

static void _ruleEngineTask(void) {
    if (gSensorCfg.numRule == 0) return;
    if (!TIME_IS_EXPIRED(_ruleScanLastMs, RULE_SCAN_INTERVAL_MS)) return;
    _ruleScanLastMs = TICK_NOW();

    uint32_t nowMs = _ruleScanLastMs;

    for (uint8_t i = 0; i < gSensorCfg.numRule && i < MAX_RULE; i++) {
        RULE_ENTRY_CONFIG *r = &gSensorCfg.ruleEntry[i];
        RULE_RUNTIME_STATE *st = &_ruleState[i];

        if (!r->enable) {
            st->conditionActive = false;
            st->conditionStartMs = 0;
            continue;
        }

        bool match = false;
        /* ========== Evaluate condition ========== */
        if (r->type == RULE_THRESHOLD) {
            match = _ruleEvalThreshold(r);

            /* Optional debounce: condition must hold steadily */
            if (r->enableDebounce) {
                uint32_t dbMs = _ruleDebounceToMs(r->debounceValue,
                        r->debounceUnit);
                if (match) {
                    if (!st->conditionActive) {
                        st->conditionActive = true;
                        st->conditionStartMs = nowMs;
                    }
                    if ((nowMs - st->conditionStartMs) < dbMs) {
                        match = false; 
                    }
                } else {
                    st->conditionActive = false;
                    st->conditionStartMs = 0;
                }
            }

        } else if (r->type == RULE_DELTA) {
            float v = 0.0f;
            if (!_ruleGetSensorValue(r->sensorId1, &v)) continue;

            _rulePushHistory(st, v, nowMs);

            uint32_t winMs = r->enableDebounce
                    ? _ruleDebounceToMs(r->debounceValue,
                    r->debounceUnit)
                    : RULE_DELTA_DEFAULT_MS;
            match = _ruleEvalDelta(st, v, r->value1, winMs, nowMs);
        }

        if (!match) continue;

        /* ========== Anti-spam ========== */
        if (st->lastNotifyMs != 0 &&
                (nowMs - st->lastNotifyMs) < RULE_NOTIFY_REARM_MS) {
            continue;
        }

        /* ========== Dispatch notification ========== */
        bool sent = false;
        if (r->notifyAction == RULE_NOTIFY_MQTT ||
                r->notifyAction == RULE_NOTIFY_ALL) {
            sent |= _ruleNotifyMqtt(r);
        }
        /* RULE_NOTIFY_SMS: (SIMMain_SendSms?) */

        if (sent) st->lastNotifyMs = nowMs;
    }
}

void SensorGeneral_Initialize(void) {
    _logFileInitQueue(&_fileQueue);
}

void SensorGeneral_Task(void) {
    /* 1. Run the periodic check task and generate log files (enqueue when scheduled) */
    _createLogFileTask();

    /* 2. State machine: dequeue files and process them (Save to SD card, Upload via FTP, etc.) */
    switch (_logState) {
        case LOG_FILE_IDLE:
        {
            /* If there are files in the queue and the SD card is idle, start processing */
            if (!_logFileIsQueueEmpty(&_fileQueue) && !SDcard_isBusy())
                _logState = LOG_FILE_PROCESS_QUEUE;
            break;
        }

        case LOG_FILE_PROCESS_QUEUE:
        {
            /* Dequeue a file for processing */
            if (_logFileDequeue(&_fileQueue, &_currentFile)) {
                LOG_INFO("%s - %s\t Processing file: %s",
                        __TAG__, __func__, _currentFile.file.name);
            } else {
                _logState = LOG_FILE_IDLE;
                break;
            }

            /* A. Save to SD card (if enabled in configuration) */
            if (gSensorCfg.saveSdcard && !_currentFile.isRetry) {
                char fullPath[100];

                /* Directory structure example: YYYYMM/YYYYMMDD/filename.csv */
                snprintf(fullPath, sizeof (fullPath), "%04u%02u/%04u%02u%02d/%s",
                        _currentFile.file.time.year, _currentFile.file.time.month,
                        _currentFile.file.time.year, _currentFile.file.time.month, _currentFile.file.time.day,
                        _currentFile.file.name);

                LOG_DEBUG("%s - %s\t Saving file to SD: %s",
                        __TAG__, __func__, fullPath);

                // SDcard_WriteLog(fullPath, _currentFile.file.content);
            }

            /* B. Upload via FTP (if enabled in configuration) */
            bool triggerFtp = false;
            if (gSensorCfg.uploadFtp) {
                for (uint8_t i = 0; i < MAX_FTP_SERVER; i++) {
                    if (!_currentFile.isRetry) {
                        /* Clear previous error status before uploading */
                        if (gAppCfg.ftpServer[i].enable) {
                            triggerFtp = true;
                        }
                    } else {
                        triggerFtp = true;
                    }
                }
            }

            if (triggerFtp) {
                bool ftp1Enable = gAppCfg.ftpServer[0].enable;
                bool ftp2Enable = gAppCfg.ftpServer[1].enable;
                /* Trigger FTP upload according to the configured uplink priority (Ethernet or GSM) */
                if (gAppCfg.network.uplink == UPLINK_ALL || gAppCfg.network.uplink == UPLINK_ETH) {
                    if (_currentFile.isRetry) {
                        if (_currentFile.retryFtpId == 0)
                            EthFtp_TriggerUpload(ftp1Enable, false);
                        else
                            EthFtp_TriggerUpload(false, ftp2Enable);
                    } else EthFtp_TriggerUpload(ftp1Enable, ftp2Enable);


                    if (gAppCfg.network.uplink == UPLINK_ALL)
                        _uplinkPhase = UPLINK_PHASE_PRIMARY;

                    LOG_INFO("%s - %s\t Trigger ETH FTP",
                            __TAG__, __func__);
                }

                if (gAppCfg.network.uplink == UPLINK_GSM) {
                    if (_currentFile.isRetry) {
                        if (_currentFile.retryFtpId == 0)
                            SIMMain_FTPTrigger(ftp1Enable, false);
                        else
                            SIMMain_FTPTrigger(false, ftp2Enable);
                    } else SIMMain_FTPTrigger(ftp1Enable, ftp2Enable);

                    LOG_INFO("%s - %s\t Trigger GSM FTP",
                            __TAG__, __func__);
                }

                _logState = LOG_FILE_WAIT_UPLINK;
            } else {
                LOG_DEBUG("%s - %s\t No FTP upload required",
                        __TAG__, __func__);

                /* No network upload required, continue with the next file */
                _logState = LOG_FILE_IDLE;
            }
            break;
        }

        case LOG_FILE_WAIT_UPLINK:
        {
            /* Check the FTP upload progress */
            /* TODO: Implement the appropriate status check function, e.g. IsFtpDone() */
            if (!_IsFtpDone()) break; /* Upload still in progress */

            if (_hasFtpError()) {
                if (gAppCfg.network.uplink == UPLINK_ALL &&
                        _uplinkPhase == UPLINK_PHASE_PRIMARY) {

                    LOG_WARN("%s - %s\t ETH FTP failed, falling back to GSM",
                            __TAG__, __func__);

                    _fallbackServer[0] = _currentFile.server[0].isErr;
                    _fallbackServer[1] = _currentFile.server[1].isErr;

                    _currentFile.server[0].isErr = false;
                    _currentFile.server[1].isErr = false;

                    SIMMain_FTPTrigger(_fallbackServer[0], _fallbackServer[1]);
                    _uplinkPhase = UPLINK_PHASE_FALLBACK;

                    LOG_INFO("%s - %s\t Trigger GSM FTP fallback (S1:%d, S2:%d)",
                            __TAG__, __func__,
                            _fallbackServer[0], _fallbackServer[1]);
                    break;
                }

                ssGeneralDt.ftpStatus = FTP_STS_ERROR;
                LOG_WARN("%s - %s\t FTP upload failed",
                        __TAG__, __func__);

                /* Mark the failed server based on the upload result */
                _logState = LOG_FILE_HANDLE_ERROR;
            } else {
                ssGeneralDt.ftpStatus = FTP_STS_GOOD;
                LOG_INFO("%s - %s\t FTP upload successful",
                        __TAG__, __func__);

                _logState = LOG_FILE_IDLE;
            }
            break;
        }

        case LOG_FILE_HANDLE_ERROR:
        {
            if (SDcard_isBusy()) break;

            LOG_WARN("%s - %s\t Processing failed FTP upload",
                    __TAG__, __func__);

            /* Store failed files in error directories (SFTP1, SFTP2) for retry when the connection is restored */
            char errPath[100];

            if (_currentFile.server[0].isErr) {
                //                snprintf(errPath, sizeof (errPath), "SFTP1/%s_%s",
                //                        gAppCfg.ftpServer[0].namePrefix,
                //                        _currentFile.file.name);

                LOG_INFO("%s - %s\t Save retry file: %s",
                        __TAG__, __func__, errPath);

                //                SDcard_WriteLog(errPath, _currentFile.file.content);
            }

            if (_currentFile.server[1].isErr) {
                //                snprintf(errPath, sizeof (errPath), "SFTP2/%s_%s",
                //                        gAppCfg.ftpServer[1].namePrefix,
                //                        _currentFile.file.name);

                LOG_INFO("%s - %s\t Save retry file: %s",
                        __TAG__, __func__, errPath);

                //                SDcard_WriteLog(errPath, _currentFile.file.content);
            }

            _logState = LOG_FILE_IDLE;
            break;
        }
    }


    /* 3. MQTT periodic publish task */
    _mqttPublishTask();

    /* 4. Rule Engine task */
    _ruleEngineTask();

}

int8_t SensorGeneral_calculateSensorStatusInput(uint8_t i) {
    switch (gSensorCfg.entry[i].typeStatus) {
        case FROM_MBRTU:
        {
            uint8_t idxRun = gSensorCfg.entry[i].indexOfTypeGood;
            uint8_t idxCalib = gSensorCfg.entry[i].indexOfTypeCalib;
            uint8_t idxErr = gSensorCfg.entry[i].indexOfTypeErr;

            if ((MbRtu_getValueFromIndex(idxCalib) & gSensorCfg.entry[i].calibValueAND) == gSensorCfg.entry[i].calibValueCompare)
                return (int8_t) 01;
            if ((MbRtu_getValueFromIndex(idxRun) & gSensorCfg.entry[i].goodValueAND) == gSensorCfg.entry[i].goodValueCompare)
                return (int8_t) 00;
            if ((MbRtu_getValueFromIndex(idxErr) & gSensorCfg.entry[i].errorValueAND) == gSensorCfg.entry[i].errorValueCompare)
                return (int8_t) 02;

            return (int8_t) - 1;
        }
        case FROM_DIGITAL_INPUT:
        {
            return (int8_t) - 1;
        }

        default:
            break;
    }

    return -1;
}

int8_t SensorGeneral_calculateSensorStatusAuto(SENSOR_TYPE type, uint8_t index) {
    SENSOR_STATUS status = STATUS_DISABLE;

    switch (type) {
        case SENSOR_MBRTU:
            status = mbrtuMasterDt.entry[index].status;
            break;

        case SENSOR_ANALOG:
            status = adcDt.entry[index].status;
            break;

        case SENSOR_INPUT_CAPTURE:
            status = inputCaptureDt.entry[index].status;
            break;

        default: break;
    }

    if (status == STATUS_GOOD) return 0;
    else if (status == STATUS_BAD) return 2;

    return -1;
}

const char* FileMgr_GetUploadFileName(void) {
    return _currentFile.file.name;
}

const char* FileMgr_GetUploadFileData(void) {
    return _currentFile.file.content;
}

uint32_t FileMgr_GetUploadFileSize(void) {
    return _currentFile.file.size;
}

TIME FileMgr_GetUploadFileTime(void) {
    return _currentFile.file.time;
}