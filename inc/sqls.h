/**
 * @file sqls.h
 * @author hbuyse
 * @date 24/06/2016
 *
 * @brief  SQL commands to create, select and drop the tables
 */


#ifndef __SQLS_H__
#define __SQLS_H__

#ifdef __cplusplus
extern "C" {
#endif


#define CREATE_SIGFOX_TABLES \
    "-- CREATION OF THE SIGFOX TABLES WITH SOME DATA\n" \
    "\n" \
    "--\n" \
    "-- Create 'raws' table\n" \
    "--\n" \
    "CREATE TABLE IF NOT EXISTS `raws` (\n" \
    "  `idraws` INTEGER PRIMARY KEY AUTOINCREMENT,\n" \
    "  `time` INTEGER NOT NULL,\n" \
    "  `idmodem` TEXT NOT NULL,\n" \
    "  `snr` REAL NOT NULL,\n" \
    "  `station` TEXT NOT NULL,\n" \
    "  `ack` integer,\n" \
    "  `data` TEXT NOT NULL,\n" \
    "  `duplicate` INTEGER NOT NULL,\n" \
    "  `avgSignal` REAL NOT NULL,\n" \
    "  `rssi` REAL NOT NULL,\n" \
    "  `lat` INTEGER NOT NULL,\n" \
    "  `lon` INTEGER NOT NULL,\n" \
    "  `seqNumber` INTEGER NOT NULL\n" \
    ");\n" \
    "\n" \
    "\n" \
    "--\n" \
    "-- Create 'devices' table\n" \
    "--\n" \
    "CREATE TABLE IF NOT EXISTS `devices` (\n" \
    "  `iddevices` INTEGER PRIMARY KEY AUTOINCREMENT,\n" \
    "  `idmodem` TEXT NOT NULL UNIQUE,\n" \
    "  `attribution` INTEGER,\n" \
    "  `timestamp_attribution` INTEGER\n" \
    ");"


#define DROP_SIGFOX_TABLES \
    "-- CREATION OF THE SIGFOX TABLES WITH SOME DATA\n" \
    "\n" \
    "-- Delete the tables if they exists\n" \
    "DROP TABLE IF EXISTS `raws`;\n" \
    "DROP TABLE IF EXISTS `events`;\n" \
    "DROP TABLE IF EXISTS `devices`;"


#define SELECT_RAWS "SELECT * FROM `raws`"
#define SELECT_DEVICES "SELECT * FROM `devices`"


/**
 * \brief SQL command to insert data from a sigfox_device_t to the database
 */
#define INSERT_DEVICES "INSERT INTO `devices` VALUES (NULL, '%s', %d, %d);"


/**
 * \brief SQL command to insert data from a sigfox_raws_t to the database
 */
#define INSERT_RAWS "INSERT INTO `raws` VALUES (NULL, %ld, '%s', %.2f, '%s', %u, '%s', %u, %.2f, %.2f, %u, %u, %u);"

#ifdef     __cplusplus
}
#endif

#endif          // __SQLS_H__