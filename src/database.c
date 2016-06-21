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
#include <time.h>          // time_t, struct tm, time, localtime, strftime

#include <frames.h>


/**
 * \brief Maximum length of a buffer
 */
#define BUFFER_MAX_LENGTH 4096


/**
 * \brief SQL command to insert data from a sigfox_device_t to the database
 */
#define INSERT_DEVICES "INSERT INTO `devices` VALUES (NULL, '%s', %d, %d);"


/**
 * \brief SQL command to insert data from a sigfox_raws_t to the database
 */
#define INSERT_RAWS "INSERT INTO `raws` VALUES (NULL, %ld, '%s', %.2f, '%s', %u, '%s', %u, %.2f, %.2f, %u, %u, %u);"

static int callback_row(void    *data,
                        int     argc,
                        char    **argv,
                        char    **col_name
                        )
{
    int     i = 0;


    if ( data )
    {
        printf("%s\t", (char *) data);
    }

    for ( i = 0; i < argc; i++ )
    {
        printf("%s = %s\t", col_name[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");

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



unsigned char sigfox_create_tables(sqlite3      **db,
                                   const char   *sql_script_path
                                   )
{
    int         rc = 0;
    char        *sqlite3_error_msg  = NULL;


    // Script SQL
    FILE        *sql_script         = NULL;
    size_t      file_size           = 0;
    char        *sql                = NULL;
    size_t      read                = 0;


    sql_script = (sql_script_path != NULL) ? fopen(sql_script_path, "r") : fopen("./sqls/create_tables.sql", "r");

    if ( ! sql_script )
    {
        fprintf(stderr,
                "[%s] Can not open the file %s\n",
                __func__,
                (sql_script_path) ? sql_script_path : "./sqls/create_tables.sql");

        return (1);
    }

    // Get the size of the SQL script
    fseek(sql_script, 0, SEEK_END);
    file_size   = ftell(sql_script);
    fseek(sql_script, 0, SEEK_SET);


    // Create SQL statement
    sql         = (char *) calloc(file_size + 1, sizeof(char) );
    read        = fread(sql, sizeof(char), file_size, sql_script);

    if ( read != file_size )
    {
        fprintf(stderr, "[%s] read != file_size\n", __func__);

        return (2);
    }

    // Execute SQL statement
    rc          = sqlite3_exec(*db, sql, NULL, NULL, &sqlite3_error_msg);

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



#if 0
void sqlite_select(sqlite3 **db)
{
    char        *sql    = NULL;
    int         rc      = 0;
    char        *sqlite3_error_msg = NULL;


    // #define DATA   "Callback function called"

    sql = "SELECT * from COMPANY";


    /* Execute SQL statement */
    #ifdef DATA
    rc  = sqlite3_exec(*db, sql, callback_row, (void *) DATA, &sqlite3_error_msg);
    #else
    rc  = sqlite3_exec(*db, sql, callback_row, NULL, &sqlite3_error_msg);
    #endif

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);
    }
}



void sqlite_update(sqlite3 **db)
{
    char        *sql    = NULL;
    int         rc      = 0;
    char        *sqlite3_error_msg = NULL;


    sql = "UPDATE COMPANY set South-HallARY = 25000.00 where ID=1";


    /* Execute SQL statement */
    rc  = sqlite3_exec(*db, sql, NULL, NULL, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);
    }
    else
    {
        printf("UPDATE done successfully\n");
    }
}
#endif


unsigned char sigfox_delete_db(sqlite3      **db,
                               const char   *sql_script_path
                               )
{
    int         rc = 0;
    char        *sqlite3_error_msg  = NULL;


    // Script SQL
    FILE        *sql_script         = NULL;
    size_t      file_size           = 0;
    char        *sql                = NULL;
    size_t      read                = 0;


    sql_script = (sql_script_path != NULL) ? fopen(sql_script_path, "r") : fopen("./sqls/delete_tables.sql", "r");

    if ( ! sql_script )
    {
        fprintf(stderr, "Can not open the file %s\n", (sql_script_path) ? sql_script_path : "./sqls/delete_tables.sql");

        return (1);
    }

    // Get the size of the SQL script
    fseek(sql_script, 0, SEEK_END);
    file_size   = ftell(sql_script);
    fseek(sql_script, 0, SEEK_SET);


    // Create SQL statement
    sql         = (char *) calloc(file_size + 1, sizeof(char) );
    read        = fread(sql, sizeof(char), file_size, sql_script);

    if ( read != file_size )
    {
        fprintf(stderr, "read != file_size\n");

        return (2);
    }

    // Execute SQL statement
    rc          = sqlite3_exec(*db, sql, NULL, NULL, &sqlite3_error_msg);

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