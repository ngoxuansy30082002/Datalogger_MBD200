#include "consoleLogs.h"

CONSOLE_DATA csLogsDt;

static void _consoleLogsQueueInit(CONSOLE_QUEUE_T *q);
static bool _consoleLogsQueueIsFull(CONSOLE_QUEUE_T *q);
static bool _consoleLogsQueueEmpty(CONSOLE_QUEUE_T *q);
static bool _consoleLogsEnQueue(CONSOLE_QUEUE_T *q, const CONSOLE_ENTRY *entry);
static bool _consoleLogsDeQueue(CONSOLE_QUEUE_T *q);
static bool _consoleLogsPeekQueue(CONSOLE_QUEUE_T *q, CONSOLE_ENTRY *entry);

void CONSOLE_LOGS_Initialize(void) {
    _consoleLogsQueueInit(&csLogsDt.queue);
}

bool ConsoleLos_Push(char* content, int len, CONSOLE_TYPE type) {
    bool res = false;
    CONSOLE_ENTRY entry;

    if (len <= 0) return false;

    if (len > sizeof (entry.content) - 1) {
        len = sizeof (entry.content) - 1;
    }
    entry.type = type;
    memcpy(entry.content, content, len);

    entry.content[len] = '\0';
    snprintf(entry.timestamp, sizeof (entry.timestamp), "%04u/%02u/%02u %02u:%02u:%02u",
            RTC_Dt.sysTime.year, RTC_Dt.sysTime.month, RTC_Dt.sysTime.day,
            RTC_Dt.sysTime.hour, RTC_Dt.sysTime.minute, RTC_Dt.sysTime.second);

    entry.len = len;
    //    SYS_CONSOLE_PRINT("content: %s \n\n", entry.content);
    //    SYS_CONSOLE_PRINT("len: %u \n\n", entry.len);
    //    SYS_CONSOLE_PRINT("type: %u \n\n", entry.type);

    res = _consoleLogsEnQueue(&csLogsDt.queue, &entry);

    return res;
}

bool ConsoleLos_Get(char* content, size_t content_size, char* type, size_t type_size, char* timestamp) {
    bool res = false;
    CONSOLE_ENTRY entry;

    res = _consoleLogsPeekQueue(&csLogsDt.queue, &entry);
    if (!res) {
        if (content) content[0] = '\0';
        if (type) type[0] = '\0';
        return res;
    }

    if (content && content_size > 0) {
        size_t copy_len = (entry.len < content_size - 1) ? entry.len : content_size - 1;
        memcpy(content, entry.content, copy_len);
        content[copy_len] = '\0';
    }

    if (type && type_size > 0) {
        switch (entry.type) {
            case CONSOLE_INFO:
                strncpy(type, "info", type_size - 1);
                break;
            case CONSOLE_SUCCESS:
                strncpy(type, "success", type_size - 1);
                break;
            case CONSOLE_WARNING:
                strncpy(type, "warning", type_size - 1);
                break;
            case CONSOLE_ERROR:
                strncpy(type, "error", type_size - 1);
                break;
            default:
                strncpy(type, "unknown", type_size - 1);
                break;
        }
        type[type_size - 1] = '\0';
    }

    memcpy(timestamp, entry.timestamp, strlen(entry.timestamp));
    timestamp[strlen(entry.timestamp)] = '\0';

    res = _consoleLogsDeQueue(&csLogsDt.queue);

    return res;
}

/* ---------------------------
 Queue implementation 
 ---------------------------*/
static void _consoleLogsQueueInit(CONSOLE_QUEUE_T * q) {

    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

static bool _consoleLogsQueueIsFull(CONSOLE_QUEUE_T * q) {

    return q->size == CONSOLE_MAX_ENTRY;
}

static bool _consoleLogsQueueEmpty(CONSOLE_QUEUE_T * q) {

    return q->size == 0;
}

static bool _consoleLogsEnQueue(CONSOLE_QUEUE_T *q, const CONSOLE_ENTRY * entry) {
    if (!q || _consoleLogsQueueIsFull(q)) {
        q->front = (q->front + 1) % CONSOLE_MAX_ENTRY;
        q->size--;
    }
    q->rear = (q->rear + 1) % CONSOLE_MAX_ENTRY;
    memcpy(&q->entry[q->rear], entry, sizeof (CONSOLE_ENTRY));
    q->size++;

    return true;
}

static bool _consoleLogsDeQueue(CONSOLE_QUEUE_T * q) {
    if (!q || _consoleLogsQueueEmpty(q)) {
        return false;
    }
    q->front = (q->front + 1) % CONSOLE_MAX_ENTRY;
    q->size--;

    return true;
}

static bool _consoleLogsPeekQueue(CONSOLE_QUEUE_T *q, CONSOLE_ENTRY * entry) {
    if (!q || _consoleLogsQueueEmpty(q) || !entry) {
        return false;
    }
    memcpy(entry, &q->entry[q->front], sizeof (CONSOLE_ENTRY));
    return true;
}
