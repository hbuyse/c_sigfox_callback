/**
 * @file: main.c
 * @author: hbuyse
 */

#include <stdio.h>          // fprintf
#include <sqlite3.h>          // sqlite3
#include <time.h>           // time
#include <json/json.h>          // json_object, json_object_array_length
#include <pthread.h>

#include <database.h>          // sigfox_open_db
#include <logging.h>            // gprintf
#include <fcgi_sigfox.h>        // Thread_arg_t, fastcgi_thread_function


#define THREAD_COUNT 20
#define PORT_FASTCGI ":8082"

/**
 * \brief List of devices that the program follows
 */
const sigfox_device_t     devices[] =
{
    {.id_modem = "12FED", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FEE", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FEF", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FF0", .attribution = 0, .timestamp_attribution = 0},
};


int main(void)
{
    int             i;
    pthread_t       threads[THREAD_COUNT];
    int             socket_id = 0;
    sqlite3         *db = NULL;
    unsigned char       devices_len = 0;


    // Initialize the FCGX library.
    FCGX_Init();


    // Getting the FCGX request on port PORT_FASTCGI
    socket_id = FCGX_OpenSocket(PORT_FASTCGI, 2000);

    if ( socket_id < 0 )
    {
        fprintf(stderr, "Socket could not be opened\n");

        return (1);
    }


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


    // Creating the threads
    for ( i = 0; i < THREAD_COUNT; i++ )
    {
        // Allocating and setting to zeros the structure
        Thread_arg_t     *thread_arg = (Thread_arg_t *) calloc(2, sizeof(Thread_arg_t) );


        // Putting the arguments into the structure
        thread_arg->socket_id   = socket_id;
        thread_arg->thread_id   = i;
        thread_arg->sigfox_db   = db;


        // Creating a thread and passing the arguments
        pthread_create(&threads[i], NULL, fastcgi_thread_function, (void *) thread_arg);
    }


    // Joining the threads
    for ( i = 0; i < THREAD_COUNT; i++ )
    {
        pthread_join(threads[i], NULL);
    }

    return (0);

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