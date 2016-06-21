/**
 * @file database.h
 * @author hbuyse
 * @date 17/06/2016
 *
 * @brief  Functions that reports to the database
 */


#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <frames.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief      Open a sqlite database
 * \details    Default name for the sqlite database is "sigfox.db"
 *
 * \param      db       The database
 * \param[in]  db_name  The database name
 *
 * \return     0: no error / other: error
 */
unsigned char sigfox_open_db(sqlite3 **db, const char *db_name);


/**
 * \brief      Creates tables into the sqlite database
 *
 * \param      db               The database
 * \param[in]  sql_script_path  Path to the sql script that creates the tables
 *
 * \return     0: no error / other: error
 */
unsigned char sigfox_create_tables(sqlite3 **db, const char *sql_script_path);


/**
 * \brief      Insert a device into the device table of the database
 *
 * \param      db      The database
 * \param[in]  device  The device
 *
 * \return     0: no error / other: error
 */
unsigned char sigfox_insert_devices(sqlite3 **db, const sigfox_device_t device);


/**
 * \brief      Insert a raws structure into the raws table of the database
 *
 * \param      db    The database
 * \param[in]  raws  The device
 *
 * \return     0: no error / other: error
 */
unsigned char sigfox_insert_raws(sqlite3 **db, const sigfox_raws_t raws);


/**
 * \brief      Delete tables from the sqlite database
 *
 * \param      db               The database
 * \param[in]  sql_script_path  Path to the sql script that deletes the tables
 *
 * \return     0: no error / other: error
 */
unsigned char sigfox_delete_db(sqlite3 **db, const char *sql_script_path);


#ifdef     __cplusplus
}
#endif

#endif          // __DATABASE_H__