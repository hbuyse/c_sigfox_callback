/**
 * @file: main.c
 * @author: hbuyse
 */

#include <sqlite3.h>          // sqlite3
#include <pthread.h>


#include <db_plugin_sqlite.h>          // db_open, db_close, db_op
#include <logging.h>            // gprintf, iprintf, eprintf
#include <frames.h>             // sigfox_device_t
#include <mongoose.h>           // struct mg_str, struct mg_connection, struct mg_serve_http_opts, mg_printf, mg_serve_http, mg_mgr_init, mg_bind,
                                // mg_set_protocol_http_websocket, mg_enable_multithreading, mg_mgr_poll, mg_mgr_free


/**
 * @brief  Port for the HTTP websocket
 */
#define HTTP_PORT       "8000"


/**
 * @brief  Path to the database
 */
#define DATABASE_PATH   "api_server.db"


/**
 * Macro to ans wer 501 Not Implemented
 */
#define MG_PRINTF_501   mg_printf(nc, "%s", "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n")


/**
 * @brief List of devices that the program follows
 */
const sigfox_device_t     devices[] =
{
    {.id_modem = "12FED", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FEE", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FEF", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FF0", .attribution = 0, .timestamp_attribution = 0},
};


/**
 * @brief      HTTP server options
 */
static struct mg_serve_http_opts     s_http_server_opts;


/**
 * @brief Pointer to the SQLite3 database
 */
static void     *s_db_handle = NULL;


/**
 * @brief Mongoose string for the GET method
 */
static const struct mg_str     s_get_method     = MG_MK_STR("GET");


/**
 * @brief Mongoose string for the POST method
 */
static const struct mg_str     s_post_method    = MG_MK_STR("POST");


/**
 * @brief Mongoose string for the PUT method
 */
static const struct mg_str     s_put_method     = MG_MK_STR("PUT");


/**
 * @brief Mongoose string for the DELETE method
 */
static const struct mg_str     s_delele_method  = MG_MK_STR("DELETE");


/**
 * @brief Signal number caught (if 0, the program continues)
 */
static int     s_sig_num = 0;


/**
 * @brief      Signal handler
 *
 * @param[in]  sig_num  The signal number
 */
static void signal_handler(int sig_num)
{
    eprintf("Signal %s caught\n", strsignal(sig_num) );

    signal(sig_num, signal_handler);
    s_sig_num = sig_num;
}



/**
 * @brief      Check if two Mongoose structure have the same prefix
 *
 * @param[in]  uri     The uri
 * @param[in]  prefix  The prefix
 *
 * @return     True if they have the same prefix, False otherwise.
 */
static int has_prefix(const struct mg_str   *uri,
                      const struct mg_str   *prefix
                      )
{
    return ( (uri->len >= prefix->len) && (memcmp(uri->p, prefix->p, prefix->len) == 0) );
}



/**
 * @brief      Determines if two Mongoose structure are equal
 *
 * @param[in]  s1    The first structure
 * @param[in]  s2    The second structure
 *
 * @return     True if equal, False otherwise.
 */
static int is_equal(const struct mg_str *s1,
                    const struct mg_str *s2
                    )
{
    return ( (s1->len == s2->len) && (memcmp(s1->p, s2->p, s2->len) == 0) );
}



/**
 * @brief      Event hadler
 *
 * @param[in,out]  nc       The non-client
 * @param[in]      ev       The event
 * @param[in]      ev_data  The event data
 */
static void ev_handler(struct mg_connection *nc,
                       int                  ev,
                       void                 *ev_data
                       )
{
    static const struct mg_str      api_prefix  = MG_MK_STR("/api");
    struct http_message             *hm         = (struct http_message *) ev_data;
    struct mg_str     key;


    switch ( ev )
    {
        case MG_EV_HTTP_REQUEST:
        {
#ifdef __DEBUG__
            char                body[4096];
            char                uri[1024];
            char                method[10];

            snprintf(body, hm->body.len + sizeof(char), "%s", hm->body.p);
            snprintf(uri, hm->uri.len + sizeof(char), "%s", hm->uri.p);
            snprintf(method, hm->method.len + sizeof(char), "%s", hm->method.p);

            iprintf("%s %s %zu %s\n", method, uri, hm->body.len, body);
#endif

            if ( has_prefix(&hm->uri, &api_prefix) )
            {
                API_Operation       op = API_OP_NULL;

                key.p   = hm->uri.p + api_prefix.len;       // Memory chunk pointer
                key.len = hm->uri.len - api_prefix.len;          // Memory chunk length

                if ( is_equal(&hm->method, &s_get_method) )
                {
                    op = API_OP_GET;
                }
                else if ( is_equal(&hm->method, &s_put_method) )
                {
                    op = API_OP_SET;
                }
                else if ( is_equal(&hm->method, &s_post_method) )
                {
                    op = API_OP_SET;
                }
                else if ( is_equal(&hm->method, &s_delele_method) )
                {
                    op = API_OP_DEL;
                }

                if ( op == API_OP_NULL )
                {
                    MG_PRINTF_501;
                }
                else
                {
                    db_op(nc, hm, &key, s_db_handle, op);
                }
            }
            else
            {
                mg_serve_http(nc, hm, s_http_server_opts);          /* Serve static content */
                gprintf("404 Not Found\n");
            }

            break;
        }

        default:
        {
            break;
        }
    }
}



/**
 * @brief      Main program
 *
 * @return     0 if everything went well
 */
int main(void)
{
    struct mg_mgr               mgr;
    struct mg_connection        *nc;


    // Initiate the manager
    mg_mgr_init(&mgr, NULL);


    // Open listening socket
    nc = mg_bind(&mgr, HTTP_PORT, ev_handler);

    if ( ! nc )
    {
        eprintf("Binding failed!\n");

        return (1);
    }


    // Use websocket
    mg_set_protocol_http_websocket(nc);


    // For each new connection, execute ev_handler in a separate thread
    mg_enable_multithreading(nc);


    // Set the docuement root
    s_http_server_opts.document_root = "web_root";


    // Handle the system signals
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);


    // Open the database
    if ( (s_db_handle = db_open(DATABASE_PATH) ) == NULL )
    {
        eprintf("Cannot open DB [%s]\n", DATABASE_PATH);
        exit(EXIT_FAILURE);
    }


    // Run event loop until signal is received
    gprintf("Starting RESTful server on port %s\n", HTTP_PORT);

    while ( s_sig_num == 0 )
    {
        /*
         * This function performs the actual IO, and must be called in a loop
         * (an event loop). Returns the current timestamp.
         * `milli` is the maximum number of milliseconds to sleep.
         * `mg_mgr_poll()` checks all connection for IO readiness. If at least one
         * of the connections is IO-ready, `mg_mgr_poll()` triggers respective
         * event handlers and returns.
         */
        mg_mgr_poll(&mgr, 1000);
    }


    // Clean up the manager and the database connection
    mg_mgr_free(&mgr);
    db_close(&s_db_handle);

    iprintf("Exiting on signal %d\n", s_sig_num);

    return (0);
}