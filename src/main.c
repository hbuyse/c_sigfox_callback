/**
 * @file: main.c
 * @author: hbuyse
 */

#include <stdio.h>          // fprintf
#include <sqlite3.h>          // sqlite3
#include <pthread.h>


// #include <database.h>          // sigfox_open_db
#include <db_plugin_sqlite.h>          // db_open
#include <logging.h>            // gprintf, iprintf, eprintf
#include <frames.h>             // sigfox_device_t
#include <mongoose.h>


#define HTTP_PORT       "8000"
#define DATABASE_PATH   "api_server.db"
#define MG_PRINTF_501   mg_printf(nc, "%s", "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n")


/**
 * \brief List of devices that the program follows
 */
const sigfox_device_t                   devices[] =
{
    {.id_modem = "12FED", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FEE", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FEF", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FF0", .attribution = 0, .timestamp_attribution = 0},
};


static struct mg_serve_http_opts        s_http_server_opts;
static void                             *s_db_handle    = NULL;
static const struct mg_str              s_get_method    = MG_MK_STR("GET");
static const struct mg_str              s_post_method   = MG_MK_STR("POST");
static const struct mg_str              s_put_method    = MG_MK_STR("PUT");


// static const struct mg_str              s_delele_method = MG_MK_STR("DELETE");


/**
 * \brief Signal number caught (if 0, the program continues)
 */
static int     s_sig_num = 0;


/**
 * \brief      Signal handler
 *
 * \param[in]  sig_num  The signal number
 */
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
    struct http_message             *hm         = (struct http_message *) ev_data;
    struct mg_str     key;


    switch ( ev )
    {
        case MG_EV_HTTP_REQUEST:

            if ( has_prefix(&hm->uri, &api_prefix) )
            {
                API_Operation       op = API_OP_NULL;

                char                body[4096];
                char                uri[1024];
                char                method[10];

                key.p   = hm->uri.p + api_prefix.len;       // Memory chunk pointer
                key.len = hm->uri.len - api_prefix.len;          // Memory chunk length

                snprintf(body, hm->body.len + sizeof(char), "%s", hm->body.p);
                snprintf(uri, hm->uri.len + sizeof(char), "%s", hm->uri.p);
                snprintf(method, hm->method.len + sizeof(char), "%s", hm->method.p);

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


                // else if ( is_equal(&hm->method, &s_delele_method) )
                // {
                // op = API_OP_DEL;
                // }

                iprintf("%s %s %zu %s\n", method, uri, hm->body.len, body);

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
            }

            break;

        default:
            break;
    }
}



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


    /* For each new connection, execute ev_handler in a separate thread */

    // Add a device into the list
    devices_len = sizeof(devices) / sizeof(devices[0]);

    for ( i = 0; i < devices_len; ++i )
    {
        sigfox_insert_devices(&db, devices[i]);
    }


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

#if 0
    sqlite3             *db         = NULL;
    json_object         *jarray     = NULL;
    unsigned char       i           = 0;
    unsigned char       devices_len = 0;

    sigfox_raws_t       raw __attribute__( (unused) ) =
    {
        .timestamp  = time(NULL),
        .id_modem   = "12FED",
        .snr        = 0,
        .station    = "E4FA",
        .ack        = 0,
        .data_str   = "16f000000000000000000000",
        .data_hex   = {0},
        .duplicate  = 0,
        .avg_signal = 0,
        .rssi       = 0,
        .latitude   = 0,
        .longitude  = 0,
        .seq_number = 0
    };


    // Creation of the database
    sigfox_open_db(&db, "sigfox.db");


    // Creation of the tables
    sigfox_create_tables(&db);


    // Add a device into the list
    devices_len = sizeof(devices) / sizeof(devices[0]);

    for ( i = 0; i < devices_len; ++i )
    {
        sigfox_insert_devices(&db, devices[i]);
    }


    // Add a raw structure into the list
    // sigfox_insert_raws(&db, raw);


    // Print devices
    jarray  = sigfox_select_devices(&db);
    gprintf("DEVICES : %d elements\n", json_object_array_length(jarray) );

    #ifdef __DEBUG__
    gprintf("%s\n", json_object_to_json_string(jarray) );
    #endif


    // Print raws
    jarray  = sigfox_select_raws(&db);
    gprintf("RAWS    : %d elements\n", json_object_array_length(jarray) );

    #ifdef __DEBUG__
    gprintf("%s\n", json_object_to_json_string(jarray) );
    #endif


    // Delete tables
    // sigfox_delete_db(&db);

    return (0);
#endif
}