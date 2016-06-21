/**
 * @file: main.c
 * @author: hbuyse
 */

#include <stdio.h>          // fprintf
#include <sqlite3.h>          // sqlite3

#include <database.h>          // sigfox_open_db

int main(void)
{
    sqlite3     *db = NULL;


    // Creation of the database
    sigfox_open_db(&db, "sigfox.db");


    // Creation of the tables
    sigfox_create_tables(&db, NULL);


    // Delete tables
    sigfox_delete_db(&db, NULL);

    return (0);
}