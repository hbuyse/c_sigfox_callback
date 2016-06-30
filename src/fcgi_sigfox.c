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
#include <stdlib.h>          // NULL, strtol
#include <string.h>          // memset, strstr
#include <time.h>          // time_t, time
#include <inttypes.h>       // uint64_t
#include <json/json.h>      // json_object

#include <fcgi_sigfox.h>          // Thread_arg_t
#include <logging.h>            // eprintf
#include <database.h>       //sigfox_select_devices


#define URI_DEVICES "/fcgi/devices"
#define URI_RAWS    "/fcgi/raws"

#define OK(stream)          FCGX_FPrintF(stream, "Status: 200 OK\r\nContent-Type: application/json\r\n\r\n");
#define BAD_REQUEST(stream) FCGX_FPrintF(stream, "Status: 400 Bad Request\r\nContent-Type: application/json\r\n\r\n");
#define FORBIDDEN(stream)   FCGX_FPrintF(stream, "Status: 403 Forbidden\r\nContent-Type: application/json\r\n\r\n");
#define NOTFOUND(stream)    FCGX_FPrintF(stream, "Status: 404 Not Found\r\nContent-Type: application/json\r\n\r\n");


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
        eprintf("THREAD_ID %02d: Can not initiate request\n", thread_arg->thread_id);

        return (NULL);
    }

#ifdef __DEBUG__
    gprintf("THREAD_ID %02d: Request is initiated\n", thread_arg->thread_id);
#endif

    while ( 1 )
    {
        static pthread_mutex_t      accept_mtx    = PTHREAD_MUTEX_INITIALIZER;
        static pthread_mutex_t      sigfox_db_mtx    = PTHREAD_MUTEX_INITIALIZER;
        static uint64_t             request_nb      = 0;
        fcgi_request_t              req;

        memset(&req, 0, sizeof(fcgi_request_t) );

#ifdef __DEBUG__
        iprintf("THREAD_ID %02d: Waiting for a new request\n", thread_arg->thread_id);
#endif


        pthread_mutex_lock(&accept_mtx);          // Lock the mutex
        rc = FCGX_Accept_r(&request);          // Accept a new request (multi-thread safe).
        request_fill(request, &req);          // Fill the request structure with the infos contained in the FCGX_Request
        int     r = ++request_nb; // Get the number of the request
        time(&rawtime);          // Get the time of the computer
        pthread_mutex_unlock(&accept_mtx);          // Unlock the mutex

        // Print informations about the FCGX request
        iprintf("%s %s %s %s\n", req.method, req.script_name, req.content_length, req.content_string);

        // If there is a problem when accepting the new request, we display a message in the terminal
        if ( rc < 0 )
        {
            eprintf("THREAD_ID %02d: Can not accept this new request\n", thread_arg->thread_id);
            break;
        }

        iprintf("THREAD_ID %02d: Request %06d is accepted\n", thread_arg->thread_id, r);

        if ( strcmp(req.method, "HEAD") == 0 )
        {
            if ((strcmp(req.uri, URI_DEVICES) ==0) || (strcmp(req.uri, URI_RAWS) ==0))
            {
                OK(request.out);
            }
            else
            {
                NOTFOUND(request.out);
            }
        }
        else if ( strcmp(req.method, "POST") == 0 )
        {
            if (strcmp(req.content_type, "Content-Type: application/json") != 0)
            {
                BAD_REQUEST(request.out);
            }
            else
            {
                if (strcmp(req.uri, URI_DEVICES) == 0)
                {
                    #if 0
                    json_object* device = json_tokener_parse(req.content_string);
                    #endif

                    FORBIDDEN(request.out);
                }
                else if (strcmp(req.uri, URI_RAWS) == 0)
                {
                    sigfox_raws_t raws;
                    json_object* jraws = NULL;
                    
                    // Parse the incomming JSON
                    jraws = json_tokener_parse(req.content_string);


                    sigfox_raws_from_json(&raws, jraws);
                    if (sigfox_insert_raws(&(thread_arg->sigfox_db), raws) == 0)
                    {
                        OK(request.out);
                    }
                    else
                    {
                        BAD_REQUEST(request.out);
                    }
                }
                else
                {
                    NOTFOUND(request.out);
                }
            }
        }
        else if ( strcmp(req.method, "GET") == 0 )
        {
            if (strcmp(req.uri, URI_DEVICES) == 0)
            {
                json_object         *jarray     = NULL;
                
                // Get the list of devices
                pthread_mutex_lock(&sigfox_db_mtx);
                jarray = sigfox_select_devices(&(thread_arg->sigfox_db));
                pthread_mutex_unlock(&sigfox_db_mtx);

                // Send back the datas
                FCGX_FPrintF(request.out, "Content-type: application/json\r\n\r\n%s", json_object_to_json_string(jarray));
            }
            else if (strcmp(req.uri, URI_RAWS) == 0)
            {
                json_object         *jarray     = NULL;
                
                // Get the list of devices
                pthread_mutex_lock(&sigfox_db_mtx);
                jarray = sigfox_select_raws(&(thread_arg->sigfox_db));
                pthread_mutex_unlock(&sigfox_db_mtx);

                // Send back the datas
                FCGX_FPrintF(request.out, "Content-type: application/json\r\n\r\n%s", json_object_to_json_string(jarray));
            }
            else if (strcmp(req.uri, "/fcgi") == 0)
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
                                     "<ul>\r\n"
                                        "<li><a href=\"" URI_DEVICES "\">Devices list</a></li>\r\n"
                                        "<li><a href=\"" URI_RAWS "\">Raws list</a></li>\r\n"
                                     "</ul>\r\n"
                                 "</body>\r\n"
                             "</html>");
            }
            else
            {
                NOTFOUND(request.out);
            }
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
    iprintf("--------------------------------\n");
    iprintf("REQUEST NUMBER      > %u\n", request_nb);
    iprintf("req.content_length  > %s\n", req.content_length);
    iprintf("req.content_string  > %s\n", req.content_string);
    iprintf("req.content_type    > %s\n", req.content_type);
    iprintf("req.is_multipart    > %u\n", req.is_multipart);
    iprintf("req.method          > %s\n", req.method);
    iprintf("req.path_info       > %s\n", req.path_info);
    iprintf("req.query_string    > %s\n", req.query_string);
    iprintf("req.remote_addr     > %s\n", req.remote_addr);
    iprintf("req.remote_port     > %s\n", req.remote_port);
    iprintf("req.script_filename > %s\n", req.script_filename);
    iprintf("req.script_name     > %s\n", req.script_name);
    iprintf("req.server_name     > %s\n", req.server_name);
    iprintf("req.uri             > %s\n", req.uri);
}



static short get_content_string(FCGX_Request    request,
                                fcgi_request_t  *req
                                )
{
    int     msg_length = 0;


    msg_length          = strtol(req->content_length, NULL, 10) + 1;


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