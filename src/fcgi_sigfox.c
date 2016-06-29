/**
 * @file fcgi_sigfox.c
 * @author hbuyse
 * @date 17/06/2016
 *
 * @brief  Thread functions to handle the FCGI server
 */


#include <fcgiapp.h>          // FCGX_Request, FCGX_Accept_r, FCGX_FPrintF, FCGX_Finish_r
#include <pthread.h>          // pthread_mutex_lock, pthread_mutex_unlock, pthread_t, pthread_mutex_t, pthread_exit
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>          // NULL
#include <string.h>          // memset, strstr
#include <time.h>          // time_t, time
#include <inttypes.h>

#include <fcgi_sigfox.h>          // Thread_arg_t



typedef struct fcgi_request_s fcgi_request_t;


/**
 * @struct fcgi_request_s
 * @brief Another request structure to stock the FCGX request
 *
 */
struct fcgi_request_s {
    char *content_length;
    char *content_string;
    char *content_type;
    char *method;
    char *path_info;
    char *query_string;
    char *remote_addr;
    char *remote_port;
    char *script_filename;
    char *script_name;
    char *server_name;
    char *uri;
    unsigned char is_multipart;
};


/**
 * @brief Parse a FCGX request and store its elements in an request structure
 *
 * @param[in] request The FCGX request to parse
 * @param[out] req The request structure to fill
 *
 * @return Error code
 */
static short request_fill(FCGX_Request request, fcgi_request_t *req);


/**
 * @brief Display all the elements of an request structure
 *
 * @param[in] req The request structure to display
 * @param[in] request_nb The number of the FCGX request
 */
static void request_display(fcgi_request_t req, unsigned int request_nb);


/**
 * @brief Get the content_string from the FCGX request
 *
 * @param[in] request The FCGX request from which we get the CONTENT_STRING
 * @param[in,out] req The request structure to fill with the CONTENT_STRING
 *
 * @return Error code
 */
static short get_content_string(FCGX_Request request, fcgi_request_t *req);


/**
 * @brief Check if the FCGX request is a multipart
 *
 * @param[in,out] req The request structure to fill
 */
static void is_request_multipart(fcgi_request_t *req);


void* fastcgi_thread_function(void *arg)
{
    int                 rc;
    Thread_arg_t        *thread_arg = (Thread_arg_t *) arg;
    FCGX_Request        request;
    time_t              rawtime;


    memset(&request, 0, sizeof(FCGX_Request) );

    if ( FCGX_InitRequest(&request, thread_arg->socket_id, 0) != 0 )
    {
        fprintf(stderr, "[THREAD_ID %02d] Can not initiate request\n", thread_arg->thread_id);

        return (NULL);
    }

#ifdef __DEBUG__
    printf("[THREAD_ID %02d] Request is initiated\n", thread_arg->thread_id);
#endif

    while ( 1 )
    {
        static pthread_mutex_t      accept_mutex    = PTHREAD_MUTEX_INITIALIZER;
        static uint64_t             request_nb      = 0;
        fcgi_request_t              req;

        memset(&req, 0, sizeof(fcgi_request_t) );

#ifdef __DEBUG__
        printf("[THREAD_ID %02d] Waiting for a new request\n", thread_arg->thread_id);
#endif


        pthread_mutex_lock(&accept_mutex);          // Lock the mutex
        rc = FCGX_Accept_r(&request);          // Accept a new request (multi-thread safe).
        request_fill(request, &req);          // Fill the request structure with the infos contained in the FCGX_Request
        int     r = ++request_nb; // Get the number of the request
        time(&rawtime);          // Get the time of the computer
        pthread_mutex_unlock(&accept_mutex);          // Unlock the mutex

        request_display(req, r);


        // If there is a problem when accepting the new request, we display a message in the terminal
        if ( rc < 0 )
        {
            fprintf(stderr, "[THREAD_ID %02d] Can not accept this new request\n", thread_arg->thread_id);
            break;
        }

        printf("[THREAD_ID %02d] Request %06d is accepted\n", thread_arg->thread_id, r);

        if ( strstr(req.method, "POST") != NULL )
        {
            FCGX_FPrintF(request.out,
                         "Content-type: text/html\r\n"
                         "\r\n"
                         "<html>\r\n"
                             "<head>\r\n"
                                 "<title>FastCGI Hello! (multi-threaded C, fcgiapp library)</title>\r\n"
                                 "</head>\r\n"
                                 "<body style=\"font-family: monospace;\">\r\n"
                                     "<h1>FastCGI Hello! (multi-threaded C, fcgiapp library)</h1>\r\n"
                                         "<p><b>Host</b> : <i>%s</i> (%s)<br/>\r\n"
                                             "<b>Time</b> : %s (<i>Unix timestamp</i> : %d)<br/>\r\n"
                                                 "<b>Request</b> : %06d<br/>\r\n"
                                                     "<b>Thread</b> : %02d<br/>\r\n"
                                                         "<b>Method</b> : %s<br/>\r\n"
                                                             "<b>Is Multipart ?</b> : %s<br/>\r\n"
                                                                 "<b>Content length</b> : %s<br/>\r\n"
                                                                     "<b>Content string</b> : %s</p>\r\n"
                                                                     "</body>\r\n"
                                                                 "</html>\r\n",
                         req.server_name ? req.server_name : "?",
                         req.remote_addr,
                         ctime(&rawtime),
                         rawtime,
                         r,
                         thread_arg->thread_id,
                         req.method,
                         req.is_multipart ? "true" : "false",
                         req.content_length,
                         req.content_string);
        }
        else
        {
            FCGX_FPrintF(request.out,
                         "Content-type: text/html\r\n"
                         "\r\n"
                         "<html>\r\n"
                             "<head>\r\n"
                                 "<title>FastCGI Hello! (multi-threaded C, fcgiapp library)</title>\r\n"
                                 "</head>\r\n"
                                 "<body style=\"font-family: monospace;\">\r\n"
                                     "<h1>FastCGI Hello! (multi-threaded C, fcgiapp library)</h1>\r\n"
                                         "<p><b>Host</b> : <i>%s</i> (%s)<br/>\r\n"
                                             "<b>Time</b> : %s (<i>Unix timestamp</i> : %d)<br/>\r\n"
                                                 "<b>Request</b> : %06d<br/>\r\n"
                                                     "<b>Thread</b> : %02d<br/>\r\n"
                                                         "<b>Method</b> : %s</p>\r\n"
                                                         "</body>\r\n"
                                                     "</html>\r\n",
                         req.server_name ? req.server_name : "?",
                         req.remote_addr,
                         ctime(&rawtime),
                         rawtime,
                         r,
                         thread_arg->thread_id,
                         req.method);
        }

        FCGX_Finish_r(&request);
    }

    pthread_exit(NULL);          // equivalent to pthread_exit(0)

    return (NULL);
}



static short request_fill(FCGX_Request      request,
                          fcgi_request_t    *req
                          )
{
    req->content_length     = FCGX_GetParam("CONTENT_LENGTH", request.envp);
    req->content_type       = FCGX_GetParam("CONTENT_TYPE", request.envp);
    req->method             = FCGX_GetParam("REQUEST_METHOD", request.envp);
    req->path_info          = FCGX_GetParam("PATH_INFO", request.envp);
    req->query_string       = FCGX_GetParam("QUERY_STRING", request.envp);
    req->remote_addr        = FCGX_GetParam("REMOTE_ADDR", request.envp);
    req->remote_port        = FCGX_GetParam("REMOTE_PORT", request.envp);
    req->script_filename    = FCGX_GetParam("SCRIPT_FILENAME", request.envp);
    req->script_name        = FCGX_GetParam("SCRIPT_NAME", request.envp);
    req->server_name        = FCGX_GetParam("SERVER_NAME", request.envp);
    req->uri = FCGX_GetParam("REQUEST_URI", request.envp);


    // Getting the content_string if any
    if ( req->content_length != NULL )
    {
        get_content_string(request, req);
    }


    // Check if the request is a multipart one
    is_request_multipart(req);

    return (0);
}



static void request_display(fcgi_request_t  req,
                            unsigned int    request_nb
                            )
{
    printf("--------------------------------\n");
    printf("\033[1mREQUEST NUMBER      >\033[0m %u\n", request_nb);
    printf("\033[1mreq.content_length  >\033[0m %s\n", req.content_length);
    printf("\033[1mreq.content_string  >\033[0m %s\n", req.content_string);
    printf("\033[1mreq.content_type    >\033[0m %s\n", req.content_type);
    printf("\033[1mreq.is_multipart    >\033[0m %u\n", req.is_multipart);
    printf("\033[1mreq.method          >\033[0m %s\n", req.method);
    printf("\033[1mreq.path_info       >\033[0m %s\n", req.path_info);
    printf("\033[1mreq.query_string    >\033[0m %s\n", req.query_string);
    printf("\033[1mreq.remote_addr     >\033[0m %s\n", req.remote_addr);
    printf("\033[1mreq.remote_port     >\033[0m %s\n", req.remote_port);
    printf("\033[1mreq.script_filename >\033[0m %s\n", req.script_filename);
    printf("\033[1mreq.script_name     >\033[0m %s\n", req.script_name);
    printf("\033[1mreq.server_name     >\033[0m %s\n", req.server_name);
    printf("\033[1mreq.uri             >\033[0m %s\n", req.uri);
}



static short get_content_string(FCGX_Request    request,
                                fcgi_request_t  *req
                                )
{
    int     msg_length = 0;


    msg_length          = atoi(req->content_length) + 1;


    // Allocate and initiate to zero the string
    req->content_string = (char *) calloc(msg_length, sizeof(char) );


    // Fill the string with the content_string
    FCGX_GetStr(req->content_string, msg_length, request.in);

    return (0);
}



static void is_request_multipart(fcgi_request_t *req)
{
    if ( req->content_type )
    {
        if ( strstr(req->content_type, "multipart/") != 0 )
        {
            req->is_multipart = 1;
        }
        else
        {
            req->is_multipart = 0;
        }
    }
}