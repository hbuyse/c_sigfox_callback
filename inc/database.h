/**
 * @file database.h
 * @author hbuyse
 * @date 17/06/2016
 *
 * @brief  Functions that reports to the database
 */


#ifndef __DATABASE_H__
#define __DATABASE_H__

#ifdef __cplusplus
extern "C" {
#endif

unsigned char sigfox_open_db(sqlite3 **db, const char* db_name);

unsigned char sigfox_create_tables(sqlite3 **db, const char* sql_script_path);

unsigned char sigfox_delete_db(sqlite3 *db);


#ifdef     __cplusplus
}
#endif

#endif          // __DATABASE_H__