/* 
 * File:   debug_uart.h
 * Author: LENOVO
 *
 * Created on April 20, 2026, 9:18 PM
 */

#ifndef DEBUG_UART_H
#define	DEBUG_UART_H

#ifdef	__cplusplus
extern "C" {
#endif

#define CLR_RESET       "\033[0m" // default
#define CLR_FATAL       "\033[1;31m" // Bold Red
#define CLR_ERROR       "\033[0;91m" // Red
#define CLR_WARN        "\033[0;93m" // Yellow
#define CLR_SUCCESS     "\033[0;92m" // Green
#define CLR_INFO        "\033[0;94m" // blue

#define LOG_FATAL(fmt, ...) \
    SYS_DEBUG_PRINT(SYS_ERROR_FATAL, CLR_FATAL fmt CLR_RESET "\r\n", ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    SYS_DEBUG_PRINT(SYS_ERROR_ERROR, CLR_ERROR fmt CLR_RESET "\r\n", ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    SYS_DEBUG_PRINT(SYS_ERROR_WARNING, CLR_WARN fmt CLR_RESET "\r\n", ##__VA_ARGS__)

#define LOG_SUCCESS(fmt, ...) \
    SYS_DEBUG_PRINT(SYS_ERROR_INFO, CLR_SUCCESS fmt CLR_RESET "\r\n", ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, CLR_INFO fmt CLR_RESET "\r\n", ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) \
    SYS_DEBUG_PRINT(SYS_ERROR_DEBUG, fmt "\r\n", ##__VA_ARGS__)

#ifdef	__cplusplus
}
#endif

#endif	/* DEBUG_UART_H */

