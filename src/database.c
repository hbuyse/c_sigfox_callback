/**
 * @file database.c
 * @author hbuyse
 * @date 17/06/2016
 *
 * @brief  Functions that reports to the database
 */

#include <sqlite3.h>          // sqlite3_open, sqlite3_exec, sqlite3_free
#include <stdio.h>          // fprintf
#include <stdlib.h>          // calloc, NULL
#include <string.h>          // strcmp
#include <time.h>          // time_t, struct tm, time, localtime, strftime
#include <json/json.h>          // json_object

#include <logging.h>          // gprintf, eprintf, cprintf
#include <frames.h>          // sigfox_raws_t, sigfox_device_t
#include <sqls.h>          // CREATE_SIGFOX_TABLES, DROP_SIGFOX_TABLES, SELECT_RAWS, SELECT_DEVICES, INSERT_RAWS, INSERT_DEVICES


/**
 * \brief Maximum length of a buffer
 */
#define BUFFER_MAX_LENGTH 4096


/**
 * \brief      Print the each row of the dabase into stdout
 *
 * \param      data      Pointer to the data we want to get back
 * \param[in]  argc      Number of colums
 * \param      argv      List of columns' values in the columns
 * \param      col_name  List of columns' names of the column
 *
 * \return     0
 */
static int callback_raw(void *data, int argc, char **argv, char **col_name);



/**
 * \brief      Get a JSON object from the row
 *
 * \param      data      Pointer to the data we want to get back
 * \param[in]  argc      Number of columns
 * \param      argv      List of columns' values in the columns
 * \param      col_name  List of columns' names of the column
 *
 * \return     0
 */
static int callback_json(void *data, int argc, char **argv, char **col_name);



/**
 * \brief           Convert a data string to an hexadecimal array
 *
 * \param[in]       data_str  The data as string
 * \param[out]      data_hex  The data as hexadecimal
 */
static void convert_data_str_to_data_hex(const unsigned char    data_str[SIGFOX_DATA_STR_LENGTH + 1],
                                         unsigned char          data_hex[SIGFOX_DATA_LENGTH]);



void sigfox_raws_from_json(sigfox_raws_t        *raws,
                           const json_object    *jobj
                           )
{
    json_object     *tmp = NULL;


    // Clear the structure.
    memset(raws, 0, sizeof(sigfox_raws_t) );

    // Get the timestamp
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_TIMESTAMP, &tmp) )
    {
        raws->timestamp = json_object_get_int(tmp);
    }

    // Get the modem identification
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_ID_MODEM, &tmp) )
    {
        const char     *id_modem = json_object_get_string(tmp);
        memcpy(&(raws->id_modem), id_modem, SIGFOX_DEVICE_LENGTH);
    }

    // Get the latitude
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_LATITUDE, &tmp) )
    {
        raws->latitude = json_object_get_int(tmp);
    }

    // Get the longitude
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_LONGITUDE, &tmp) )
    {
        raws->longitude = json_object_get_int(tmp);
    }

    // Get the sequence number
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_SEQ_NUMBER, &tmp) )
    {
        raws->seq_number = json_object_get_int(tmp);
    }

    // Get the acknowledge variable
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_ACK, &tmp) )
    {
        raws->ack = json_object_get_boolean(tmp);
    }

    // Get the duplicate variable
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_DUPLICATE, &tmp) )
    {
        raws->duplicate = json_object_get_boolean(tmp);
    }

    // Get the signal variable
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_SNR, &tmp) )
    {
        raws->snr = json_object_get_double(tmp);
    }

    // Get the average signal variable
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_AVG_SIGNAL, &tmp) )
    {
        raws->avg_signal = json_object_get_double(tmp);
    }

    // Get the RSSI
    if ( json_object_object_get_ex( (json_object *) jobj, SQL_COL_RSSI, &tmp) )
    {
        raws->rssi = json_object_get_double(tmp);
    }
}



unsigned char sigfox_open_db(sqlite3    **db,
                             const char *db_name
                             )
{
    int     rc = 0;
    unsigned char     res = 0;


    rc = sqlite3_open(db_name, db);

    switch ( rc )
    {
        case SQLITE_OK:
#ifdef __DEBUG__
            gprintf("Opened database successfully\n");
#endif
            res = 0;
            break;

        default:
            eprintf("Can't open database: %s\n", sqlite3_errmsg(*db) );
            res = 1;
            break;
    }

    return (res);
}



unsigned char sigfox_create_tables(sqlite3 **db)
{
    int         rc = 0;
    char        *sqlite3_error_msg = NULL;


    // Execute SQL statement
    rc = sqlite3_exec(*db, CREATE_SIGFOX_TABLES, NULL, NULL, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        eprintf("SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);

        return (3);
    }

#ifdef __DEBUG__
    else
    {
        gprintf("Tables created successfully\n");
    }
#endif

    return (0);
}



unsigned char sigfox_insert_devices(sqlite3                 **db,
                                    const sigfox_device_t   device
                                    )
{
    unsigned char       res = 0;
    int                 rc  = 0;
    char                *sqlite3_error_msg = NULL;
    char                sql[BUFFER_MAX_LENGTH];


    sprintf(sql, INSERT_DEVICES, device.id_modem, device.attribution, device.timestamp_attribution);


    // Execute SQL statement
    rc = sqlite3_exec(*db, sql, NULL, NULL, &sqlite3_error_msg);

    switch ( rc )
    {
        case SQLITE_OK:
#ifdef __DEBUG__
            gprintf("Device %s inserted into devices table\n", device.id_modem);
#endif
            res = 0;
            break;

        case SQLITE_CONSTRAINT:
            cprintf("Device %s already in the database (SQL Error: %s)\n",
                    device.id_modem,
                    sqlite3_error_msg);
            sqlite3_free(sqlite3_error_msg);
            res = 1;
            break;

        default:
            eprintf("SQL error (%d): %s\n", rc, sqlite3_error_msg);
            sqlite3_free(sqlite3_error_msg);
            res = 2;
            break;
    }

    return (res);
}



unsigned char sigfox_insert_raws(sqlite3        **db,
                                 sigfox_raws_t  raws
                                 )
{
    unsigned char       res = 0;
    int                 rc  = 0;
    char                *sqlite3_error_msg = NULL;
    char                sql[BUFFER_MAX_LENGTH];


    // Convert string datas to hex datas
    convert_data_str_to_data_hex(raws.data_str, raws.data_hex);

    sprintf(sql, INSERT_RAWS, raws.timestamp, raws.id_modem, raws.snr, raws.station, raws.ack, raws.data_str,
            raws.duplicate, raws.avg_signal, raws.rssi, raws.latitude, raws.longitude, raws.seq_number);


    // Execute SQL statement
    rc = sqlite3_exec(*db, sql, NULL, NULL, &sqlite3_error_msg);

    switch ( rc )
    {
        case SQLITE_OK:
#ifdef __DEBUG__
            gprintf("Raw %s inserted into raws table\n", raws.data_str);
#endif
            res = 0;
            break;

        default:
            eprintf("SQL error (%d): %s\n", rc, sqlite3_error_msg);
            sqlite3_free(sqlite3_error_msg);
            res = 1;
            break;
    }

    return (res);
}



json_object* sigfox_select_raws(sqlite3 **db)
{
    int         rc = 0;
    char        *sqlite3_error_msg  = NULL;
    json_object     *jarray         = NULL;


    // Create the JSON array
    jarray  = json_object_new_array();


    // Execute SQL statement
    rc      = sqlite3_exec(*db, SELECT_RAWS, callback_json, (void *) jarray, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        eprintf("SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);
    }

    return (jarray);
}



json_object* sigfox_select_devices(sqlite3 **db)
{
    int         rc = 0;
    char        *sqlite3_error_msg  = NULL;
    json_object     *jarray         = NULL;


    // Create the JSON array
    jarray  = json_object_new_array();


    // Execute SQL statement
    rc      = sqlite3_exec(*db, SELECT_DEVICES, callback_json, (void *) jarray, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        eprintf("SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);
    }

    return (jarray);
}



unsigned char sigfox_delete_db(sqlite3 **db)
{
    int         rc = 0;
    char        *sqlite3_error_msg = NULL;


    // Execute SQL statement
    rc = sqlite3_exec(*db, DROP_SIGFOX_TABLES, NULL, NULL, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        eprintf("SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);

        return (3);
    }

#ifdef __DEBUG__
    else
    {
        gprintf("Tables deleted successfully\n");
    }
#endif

    if ( *db )
    {
        sqlite3_close(*db);
        *db = NULL;
    }

    return (0);
}



static int callback_raw(void    *data __attribute__( (unused) ),
                        int     argc,
                        char    **argv,
                        char    **col_name
                        )
{
    int     i = 0;


    for ( i = 0; i < argc; i++ )
    {
        // Pass if do not have a column name or if it is the rows' IDs
        if ( (col_name[i] == NULL) || (strcmp(col_name[i], SQL_COL_ID_RAWS) == 0) ||
             (strcmp(col_name[i], SQL_COL_ID_MODEM) == 0) )
        {
            continue;
        }

        fprintf(stdout, "%s: %s\t", col_name[i], (argv[i]) ? argv[i] : NULL);
    }

    fprintf(stdout, "\n");

    return (0);
}



static int callback_json(void   *data,
                         int    argc,
                         char   **argv,
                         char   **col_name
                         )
{
    int     i = 0;
    json_object     *jobj   = NULL;


    // Cast the JSON array
    json_object     *jarray = (json_object *) data;


    // Create a new JSON pbject
    jobj = json_object_new_object();

    for ( i = 0; i < argc; i++ )
    {
        json_object     *value = NULL;

        // Pass if do not have a column name
        if ( col_name[i] == NULL )
        {
            continue;
        }

        if ( argv[i] == NULL )
        {
            json_object_object_add(jobj, col_name[i], NULL);
            continue;
        }

        // We do not give the ids
        if ( (strcmp(col_name[i], SQL_COL_ID_RAWS) == 0) )
        {
            continue;
        }

        // Fill the JSON object in function of the key, we create a speficic JSON object type
        if ( (strcmp(col_name[i], SQL_COL_TIMESTAMP) == 0) || (strcmp(col_name[i], SQL_COL_LATITUDE) == 0) ||
             (strcmp(col_name[i], SQL_COL_LONGITUDE) == 0) || (strcmp(col_name[i], SQL_COL_SEQ_NUMBER) == 0) ||
             (strcmp(col_name[i], SQL_COL_ATTRIBUTION) == 0) ||
             (strcmp(col_name[i], SQL_COL_TIMESTAMP_ATTRIBUTION) == 0) )
        {
            value = json_object_new_int(atoi(argv[i]) );
        }
        else if ( (strcmp(col_name[i], SQL_COL_ACK) == 0) || (strcmp(col_name[i], SQL_COL_DUPLICATE) == 0) )
        {
            value = json_object_new_boolean(atoi(argv[i]) );
        }
        else if ( (strcmp(col_name[i], SQL_COL_SNR) == 0) || (strcmp(col_name[i], SQL_COL_AVG_SIGNAL) == 0) ||
                  (strcmp(col_name[i], SQL_COL_RSSI) == 0) )
        {
            value = json_object_new_double(atof(argv[i]) );
        }
        else if ( strcmp(col_name[i], SQL_COL_DATA_STR) == 0 )
        {
            unsigned char       j   = 0;
            size_t              len = 0;
            json_object         *data_hex       = NULL;
            json_object         *data_hex_array = NULL;


            // Create the array that will store the datas in integer
            data_hex        = json_object_new_object();
            data_hex_array  = json_object_new_array();


            // Get the length of the string
            len = strlen(argv[i]);

            for ( j = 0; j < len; j += 2 )
            {
                json_object         *data_hex_val   = NULL;
                unsigned char       s[]             = {argv[i][j], (argv[i][j + 1]) ? argv[i][j + 1] : 0, 0};


                // Create the json value to store into the array
                data_hex_val = json_object_new_int(strtol( (const char *) s, NULL, 16) );


                // Store in the array
                json_object_array_add(data_hex_array, data_hex_val);
            }

            json_object_object_add(jobj, SQL_COL_DATA_HEX, data_hex_array);


            // Add the data_str value to the JSON
            value = json_object_new_string(argv[i]);
        }
        else
        {
            value = json_object_new_string(argv[i]);
        }

        json_object_object_add(jobj, col_name[i], value);
    }

    // Add the JSON object into the JSON array
    json_object_array_add(jarray, jobj);

    return (0);
}



static void convert_data_str_to_data_hex(const unsigned char    data_str[SIGFOX_DATA_STR_LENGTH + 1],
                                         unsigned char          data_hex[SIGFOX_DATA_LENGTH]
                                         )
{
    unsigned char       i   = 0;
    unsigned char       j   = 0;
    size_t              len = 0;


    // Get the length of the string
    len = strlen( (const char *) data_str);

    for ( i = 0, j = 0; i < len; i += 2, ++j )
    {
        unsigned char     s[] = {data_str[i], data_str[i + 1], 0};

        data_hex[j] = (unsigned char) strtol( (const char *) s, NULL, 0);
    }
}