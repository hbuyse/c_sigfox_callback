/**
 * @file database.c
 * @author hbuyse
 * @date 17/06/2016
 *
 * @brief  Functions that reports to the database
 */

#include <sqlite3.h>          // sqlite3_open, sqlite3_exec, sqlite3_free
#include <stdio.h>          // fprintf
#include <stdlib.h>          // exit
// #include <database.h>          // sqlite_open_db, sqlite_create_table, sqlite_insert, sqlite_select, sqlite_update,
                                // sqlite_delete
#include <time.h>          // time_t, struct tm, time, localtime, strftime


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


unsigned char sigfox_open_db(sqlite3 **db, const char* db_name)
{
    unsigned char   res = 0;
    int             rc = 0;

    rc = sqlite3_open(db_name, db);

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db) );
        return 1;
    }
    else
    {
        printf("Opened database successfully\n");
        return 0;
    }
}


unsigned char sigfox_create_tables(sqlite3 **db, const char* sql_script_path)
{
    int         rc      = 0;
    char        *sqlite3_error_msg = NULL;
    
    // 
    FILE        *sql_script = NULL;
    size_t      file_size = 0;
    char        *sql    = NULL;
    size_t      read    = 0;


    sql_script = (sql_script_path != NULL) ? fopen(sql_script_path, "r") : fopen("./sqls/database.sql", "r");

    if (! sql_script)
    {
        fprintf(stderr, "Can not open the file %s\n", (sql_script_path) ? sql_script_path : "./sqls/database.sql");

        return 1;
    }


    // Get the size of the SQL script
    fseek(sql_script, 0, SEEK_END);
    file_size = ftell(sql_script);
    fseek(sql_script, 0, SEEK_SET);


    // Create SQL statement
    sql = (char*) calloc(file_size + 1, sizeof(char));
    read = fread(sql, sizeof(char), file_size, sql_script);

    if (read != file_size)
    {
        fprintf(stderr, "read != file_size\n");
        return 2;
    }

    // Execute SQL statement
    rc  = sqlite3_exec(*db, sql, NULL, NULL, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);

        return 3;
    }
    else
    {
        printf("Tables created successfully\n");
    }

    return 0;
}


#if 0
void sqlite_insert(sqlite3 **db)
{
    char        *sql    = NULL;
    int         rc      = 0;
    char        *sqlite3_error_msg = NULL;


    /* Create SQL statement */
    sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); " \
          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );" \
          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) VALUES (5, 'David', 27, 'Texas', 85000.00 );" \
          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) VALUES (6, 'Kim', 22, 'South-Hall', 45000.00 );" \
          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) VALUES (7, 'James', 24, 'Houston', 10000.00 );";


    /* Execute SQL statement */
    rc  = sqlite3_exec(*db, sql, NULL, NULL, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);
    }
    else
    {
        printf("INSERT done successfully\n");
    }
}



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


    sql = "UPDATE COMPANY set SALARY = 25000.00 where ID=1";


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


void sigfox_delete_db(sqlite3 *db)
{
    char        *sql    = NULL;
    int         rc      = 0;
    char        *sqlite3_error_msg = NULL;


    /* Create merged SQL statement */
    sql = "DELETE from COMPANY where ID=2";


    /* Execute SQL statement */
    rc  = sqlite3_exec(db, sql, NULL, NULL, &sqlite3_error_msg);

    if ( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_error_msg);
        sqlite3_free(sqlite3_error_msg);
    }
    else
    {
        printf("DELETE done successfully\n");
    }
}