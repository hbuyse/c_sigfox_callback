/**
 * @file fcgi_sigfox.h
 * @author hbuyse
 * @date 17/06/2016
 *
 * @brief  Thread functions to handle the FCGI server
 */


#ifndef __FCGI_THREADS_H__
#define __FCGI_THREADS_H__

#include <fcgiapp.h>          // FCGX_GetParam, FCGX_Request

#ifdef __cplusplus
extern "C" {
#endif


typedef struct Thread_arg_s Thread_arg_t;

struct Thread_arg_s {
    int socket_id;
    unsigned short thread_id;
};


/**
 * @brief Function for fastcgi pthread_create
 * @details This is the function that is treated by the threads.
 *          It only prints on the navigator the number of the requests and
 *
 * @param arg The pointer to the arguments given for the function
 * @return Pointer to the result
 */
void* fastcgi_thread_function(void *arg);


#ifdef     __cplusplus
}
#endif

#endif          // __FCGI_THREADS_H__