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

#include <frames.h>
#include <sqls.h>       // CREATE_SIGFOX_TABLES, DROP_SIGFOX_TABLES, SELECT_RAWS, SELECT_DEVICES, INSERT_RAWS, INSERT_DEVICES


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
        if ( (col_name[i] == NULL) || (strcmp(col_name[i], "idraws") == 0) ||
             (strcmp(col_name[i], "iddevices") == 0) )
        {
            continue;
        }

        fprintf(stdout, "%s: %s\t", col_name[i], (argv[i]) ? argv[i] : NULL);
    }

    fprintf(stdout, "\n");

    return (0);
}



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
static int callback_json(void   *data,
                         int    argc,
                         char   **argv,
                         char   **col_name
                         )
{
    int             i       = 0;
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
        if ( (strcmp(col_name[i], "idraws") == 0) || (strcmp(col_name[i], "iddevices") == 0) )
        {
            continue;
        }


        // Fill the JSON object in function of the key, we create a speficic JSON object type
        if ( (strcmp(col_name[i], "idraws") == 0) || (strcmp(col_name[i], "time") == 0) ||
             (strcmp(col_name[i], "lat") == 0) || (strcmp(col_name[i], "lon") == 0) ||
             (strcmp(col_name[i], "seqNumber") == 0) || (strcmp(col_name[i], "iddevices") == 0) ||
             (strcmp(col_name[i], "attribution") == 0) || (strcmp(col_name[i], "timestamp_attribution") == 0) )
        {
            value = json_object_new_int(atoi(argv[i]) );
        }
        else if ( (strcmp(col_name[i], "ack") == 0) || (strcmp(col_name[i], "duplicate") == 0) )
        {
            // fprintf(stdout, "Find %s", argv[i]);
            value = json_object_new_boolean(atoi(argv[i]) );
        }
        else if ( (strcmp(col_name[i], "snr") == 0) || (strcmp(col_name[i], "avgSignal") == 0) ||
                  (strcmp(col_name[i], "rssi") == 0) )
        {
            value = json_object_new_double(atof(argv[i]) );
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
            fprintf(stdout, "[%s] Opened database successfully\n", __func__);
#endif
            res = 0;
            break;

        default:
            fprintf(stderr, "[%s] Can't open database: %s\n", __func__, sqlite3_errmsg(*db) );
            res = 1;
            break;
    }

    return (res);
}



unsigned char sigfox_create_tables(sqlite3 **db)
{
    int         rc                  = 0;
    char        *sqlite3_error_msg  = NULL;


    // Execute SQL statement
    rc = sqlite3_exec(*db, CREATE_SIGFOX_TABLES, NULL, NULL, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "[%s] SQL error: %s\n", __func__, sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);

        return (3);
    }

#ifdef __DEBUG__
    else
    {
        fprintf(stdout, "[%s] Tables created successfully\n", __func__);
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
            fprintf(stdout, "[%s] Device %s inserted into devices table\n", __func__, device.id_modem);
#endif
            res = 0;
            break;

        case SQLITE_CONSTRAINT:
            fprintf(stderr,
                    "[%s] Device %s already in the database (SQL Error: %s)\n",
                    __func__,
                    device.id_modem,
                    sqlite3_error_msg);
            sqlite3_free(sqlite3_error_msg);
            res = 2;
            break;

        default:
            fprintf(stderr, "[%s] SQL error (%d): %s\n", __func__, rc, sqlite3_error_msg);
            sqlite3_free(sqlite3_error_msg);
            res = 1;
            break;
    }

    return (res);
}



unsigned char sigfox_insert_raws(sqlite3                **db,
                                 const sigfox_raws_t    raws
                                 )
{
    unsigned char       res = 0;
    int                 rc  = 0;
    char                *sqlite3_error_msg = NULL;
    char                sql[BUFFER_MAX_LENGTH];


    sprintf(sql, INSERT_RAWS, raws.timestamp, raws.id_modem, raws.snr, raws.station, raws.ack, raws.data,
            raws.duplicate, raws.avg_snr, raws.rssi, raws.latitude, raws.longitude, raws.seq_number);


    // Execute SQL statement
    rc = sqlite3_exec(*db, sql, NULL, NULL, &sqlite3_error_msg);

    switch ( rc )
    {
        case SQLITE_OK:
#ifdef __DEBUG__
            fprintf(stdout, "[%s] Raw %s inserted into raws table\n", __func__, raws.data);
#endif
            res = 0;
            break;

        default:
            fprintf(stderr, "[%s] SQL error (%d): %s\n", __func__, rc, sqlite3_error_msg);
            sqlite3_free(sqlite3_error_msg);
            res = 1;
            break;
    }

    return (res);
}



json_object* sigfox_select_raws(sqlite3 **db)
{
    int             rc      = 0;
    char            *sqlite3_error_msg = NULL;
    json_object     *jarray = NULL;


    // Create the JSON array
    jarray  = json_object_new_array();


    // Execute SQL statement
    rc      = sqlite3_exec(*db, SELECT_RAWS, callback_json, (void *) jarray, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);
    }

    return (jarray);
}



json_object* sigfox_select_devices(sqlite3 **db)
{
    int             rc      = 0;
    char            *sqlite3_error_msg = NULL;
    json_object     *jarray = NULL;


    // Create the JSON array
    jarray  = json_object_new_array();


    // Execute SQL statement
    rc      = sqlite3_exec(*db, SELECT_DEVICES, callback_json, (void *) jarray, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);
    }

    return (jarray);
}



unsigned char sigfox_delete_db(sqlite3 **db)
{
    int         rc                  = 0;
    char        *sqlite3_error_msg  = NULL;


    // Execute SQL statement
    rc = sqlite3_exec(*db, DROP_SIGFOX_TABLES, NULL, NULL, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);

        return (3);
    }

#ifdef __DEBUG__
    else
    {
        fprintf(stdout, "Tables deleted successfully\n");
    }
#endif

    if ( *db )
    {
        sqlite3_close(*db);
        *db = NULL;
    }

    return (0);
}