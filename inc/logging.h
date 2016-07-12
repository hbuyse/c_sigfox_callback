/**
 * @file logging.h
 * @author hbuyse
 * @date 17/06/2016
 *
 * @brief  Logging functions
 */


#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <stdio.h>          // fprintf, stderr, stdout

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief      Print an information on the standard output
 *
 * @param      format  The format
 * @param      ...     The list of the arguments
 */
#ifdef __DEBUG__
    #define iprintf(format, ...)    fprintf(stdout, "\e[1m[%s]\e[0m " format, __func__, ## __VA_ARGS__)
#else
    #define iprintf(format, ...)    fprintf(stdout, "INFO:    " format, ## __VA_ARGS__)
#endif


/**
 * @brief      Print an error on the standard error output
 *
 * @param      format  The format
 * @param      ...     The list of the arguments
 */
#ifdef __DEBUG__
    #define eprintf(format, ...)    fprintf(stderr, "\e[1;31m[%s]\e[0m " format, __func__, ## __VA_ARGS__)
#else
    #define eprintf(format, ...)    fprintf(stdout, "\e[1;31mERROR:\e[0m   " format, ## __VA_ARGS__)
#endif


/**
 * @brief      Print a warning (caution) on the standard output
 *
 * @param      format  The format
 * @param      ...     The list of the arguments
 */
#ifdef __DEBUG__
    #define cprintf(format, ...)    fprintf(stdout, "\e[1;33m[%s]\e[0m " format, __func__, ## __VA_ARGS__)
#else
    #define cprintf(format, ...)    fprintf(stdout, "\e[1;33mCAUTION:\e[0m " format, ## __VA_ARGS__)
#endif


/**
 * @brief      Print on the standard output
 *
 * @param      format  The format
 * @param      ...     The list of the arguments
 */
#ifdef __DEBUG__
    #define gprintf(format, ...)    fprintf(stdout, "\e[1;32m[%s]\e[0m " format, __func__, ## __VA_ARGS__)
#else
    #define gprintf(format, ...)    fprintf(stdout, format, ## __VA_ARGS__)
#endif

#ifdef     __cplusplus
}
#endif

#endif          // __LOGGING_H__