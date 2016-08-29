/**
 * @file: main.c
 * @author: hbuyse
 */

#include <sqlite3.h>          // sqlite3
#include <unistd.h>          // getopt, opterr, optarg, optopt, optind
#include <getopt.h>          // struct option


#include <db_plugin_sqlite.h>          // db_open, db_close, db_op
#include <logging.h>            // gprintf, iprintf, eprintf, cprintf
#include <frames.h>             // sigfox_device_t
#include <mongoose.h>           // struct mg_str, struct mg_connection, struct mg_serve_http_opts, mg_printf, mg_serve_http, mg_mgr_init, mg_bind,
                                // mg_set_protocol_http_websocket, mg_enable_multithreading, mg_mgr_poll, mg_mgr_free

#ifdef __DEBUG__


/**
 * \brief HTTP Request Body max length
 */
    #define BODY_MAX_LENGTH     4096


/**
 * \brief HTTP Request Uri max length
 */
    #define URI_MAX_LENGTH      1024


/**
 * \brief HTTP Request method max length
 */
    #define METHOD_MAX_LENGTH   10
#endif


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
 * @brief Mongoose string for the DELETE method
 */
static const struct mg_str     s_delele_method  = MG_MK_STR("DELETE");


/**
 * @brief Signal number caught (if 0, the program continues)
 */
static int     s_sig_num = 0;


/**
 * @brief Shows how to use the program
 *
 * @param program_name The program's name
 */
static void usage(char *program_name);


/**
 * @brief      Signal handler
 *
 * @param[in]  sig_num  The signal number
 */
static void signal_handler(int sig_num);


/**
 * @brief      Check if two Mongoose structure have the same prefix
 *
 * @param[in]  uri     The uri
 * @param[in]  prefix  The prefix
 *
 * @return     True if they have the same prefix, False otherwise.
 */
static int has_prefix(const struct mg_str *uri, const struct mg_str *prefix);


/**
 * @brief      Determines if two Mongoose structure are equal
 *
 * @param[in]  s1    The first structure
 * @param[in]  s2    The second structure
 *
 * @return     True if equal, False otherwise.
 */
static int is_equal(const struct mg_str *s1, const struct mg_str *s2);


/**
 * @brief      Event hadler
 *
 * @param[in,out]  nc       The non-client
 * @param[in]      ev       The event
 * @param[in]      ev_data  The event data
 */
static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);



/**
 * @brief The program
 *
 * @param argc Number of arguments
 * @param argv Lists of pointers that points to the arguments
 *
 * @return Exit code
 */
int main(int    argc,
         char   **argv
         )
{
    // getopt vars
    int         opt         = 0;
    int         long_index  = 0;
    char        *port       = NULL;
    static struct option        long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {"port", optional_argument, 0, 'p'},
        {0, 0, 0, 0}
    };


    // Mongoose vars
    struct mg_mgr               mgr;
    struct mg_connection        *nc;


    /* CAREFUL:
     * An option character in this string can be followed by a colon (‘:’) to indicate that it takes a required
     * argument. If an option character is followed by two colons (‘::’), its argument is optional; this is a GNU
     * extension.
     */
    while ( (opt = getopt_long(argc, argv, "hp", long_options, &long_index) ) != -1 )
    {
        switch ( opt )
        {
            case 'p':
                {
                    port = optarg;
                    break;
                }


            case 'h':
                {
                    usage(argv[0]);
                    exit(EXIT_SUCCESS);
                }

            case '?':
                {
                    if ( optopt == 'p' )
                    {
                        eprintf("Option -%c requires an argument.\n", optopt);
                    }
                    else
                    {
                        eprintf("Unknown option character `\\x%x'.\n", optopt);
                    }

                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }

            default:
                break;
        }
    }

    if ( strtol((port) ? port : HTTP_PORT, NULL, 10) == 0L )
    {
        eprintf("%s is not a good port for the RESTful server...\n", port);
        eprintf("Please select an other one.\n");
        exit(EXIT_FAILURE);
    }


    // Initiate the manager
    mg_mgr_init(&mgr, NULL);


    // Open listening socket
    nc = mg_bind(&mgr, (port) ? port : HTTP_PORT, ev_handler);

    if ( ! nc )
    {
        eprintf("Binding failed on port %s!\n", (port) ? port : HTTP_PORT);

        return (1);
    }


    // Use websocket
    mg_set_protocol_http_websocket(nc);


    // For each new connection, execute ev_handler in a separate thread
    // mg_enable_multithreading(nc);


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
    gprintf("Starting RESTful server on port %s\n", port);

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



static void usage(char *program_name)
{
    fprintf(stdout, "Usage: %s [-h] -p port\n", program_name);
    fprintf(stdout, "\t-h | --help              Display this help.\n");
    fprintf(stdout, "\t-p | --port=PORT         RESTful server port.\n");
}



static void signal_handler(int sig_num)
{
    eprintf("Signal %s caught\n", strsignal(sig_num) );

    signal(sig_num, signal_handler);
    s_sig_num = sig_num;
}



static int has_prefix(const struct mg_str   *uri,
                      const struct mg_str   *prefix
                      )
{
    return ( (uri->len >= prefix->len) && (memcmp(uri->p, prefix->p, prefix->len) == 0) );
}



static int is_equal(const struct mg_str *s1,
                    const struct mg_str *s2
                    )
{
    return ( (s1->len == s2->len) && (memcmp(s1->p, s2->p, s2->len) == 0) );
}



static void ev_handler(struct mg_connection *nc,
                       int                  ev,
                       void                 *ev_data
                       )
{
    static const struct mg_str      api_prefix  = MG_MK_STR("/api");
    static const struct mg_str      root_prefix = MG_MK_STR("/");
    struct http_message             *hm         = (struct http_message *) ev_data;
    struct mg_str     key;


    switch ( ev )
    {
        case MG_EV_HTTP_REQUEST:
            {
#ifdef __DEBUG__
                char        body[BODY_MAX_LENGTH];
                char        uri[URI_MAX_LENGTH];
                char        method[METHOD_MAX_LENGTH];

                snprintf(body, hm->body.len + sizeof(char), "%s", hm->body.p);
                snprintf(uri, hm->uri.len + sizeof(char), "%s", hm->uri.p);
                snprintf(method, hm->method.len + sizeof(char), "%s", hm->method.p);

                iprintf("%s %s %zu %s\n", method, uri, hm->body.len, body);
#endif

                if ( has_prefix(&hm->uri, &api_prefix) )
                {
                    API_Operation     op = API_OP_NULL;

                    key.p   = hm->uri.p + api_prefix.len;   // Memory chunk pointer
                    key.len = hm->uri.len - api_prefix.len;      // Memory chunk length

                    if ( is_equal(&hm->method, &s_get_method) )
                    {
                        op = API_OP_GET;
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
                    mg_serve_http(nc, hm, s_http_server_opts);      /* Serve static content */

                    if ( is_equal(&hm->uri, &root_prefix) )
                    {
                        gprintf("200 OK\n");
                    }
                    else
                    {
                        cprintf("404 Not Found\n");
                    }
                }

                break;
            }

        default:
            {
                break;
            }
    }
}
