/**
 * @file: main.c
 * @author: hbuyse
 */

#include <stdio.h>          // fprintf
#include <sqlite3.h>          // sqlite3
#include <time.h>           // time
#include <json/json.h>          // json_object, json_object_array_length

#include <database.h>          // sigfox_open_db

const sigfox_device_t     devices[] =
{
    {.id_modem = "12FED", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FEE", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FEF", .attribution = 0, .timestamp_attribution = 0},
    {.id_modem = "12FF0", .attribution = 0, .timestamp_attribution = 0},
};


int main(void)
{
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
    sigfox_insert_raws(&db, raw);


    // Print devices
    jarray  = sigfox_select_devices(&db);
    fprintf(stdout, "DEVICES : %d elements\n", json_object_array_length(jarray) );

#ifdef __DEBUG__
    fprintf(stdout, "%s\n", json_object_to_json_string(jarray) );
#endif


    // Print raws
    jarray  = sigfox_select_raws(&db);
    fprintf(stdout, "RAWS    : %d elements\n", json_object_array_length(jarray) );

#ifdef __DEBUG__
    fprintf(stdout, "%s\n", json_object_to_json_string(jarray) );
#endif


    // Delete tables
    // sigfox_delete_db(&db);

    return (0);
}