/**
 * @file: main.c
 * @author: hbuyse
 */

#include <stdio.h>          // fprintf
#include <sqlite3.h>          // sqlite3
#include <time.h>           // time
#include <json/json.h>          // json_object, json_object_array_length

#include <database.h>          // sigfox_open_db

int main(void)
{
    sqlite3             *db     = NULL;
    json_object         *jarray = NULL;

    sigfox_device_t     device  =
    {
        .id_modem               = "12FED",
        .attribution            = 0,
        .timestamp_attribution  = 0
    };

    sigfox_raws_t       raw     =
    {
        .timestamp  = time(NULL),
        .id_modem   = "12FED",
        .snr        = 0,
        .station    = "E4FA",
        .ack        = 0,
        .data       = "16f000000000000000000000",
        .duplicate  = 0,
        .avg_snr    = 0,
        .rssi       = 0,
        .latitude   = 0,
        .longitude  = 0,
        .seq_number = 0
    };


    // Creation of the database
    sigfox_open_db(&db, "sigfox.db");


    // Creation of the tables
    sigfox_create_tables(&db, NULL);


    // Add a device into the list
    sigfox_insert_devices(&db, device);


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
    // sigfox_delete_db(&db, NULL);

    return (0);
}