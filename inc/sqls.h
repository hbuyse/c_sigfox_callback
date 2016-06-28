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


#define SQL_COL_ID_RAWS "id_raws"
#define SQL_COL_ID_MODEM "id_modem"
#define SQL_COL_TIMESTAMP "timestamp"
#define SQL_COL_LATITUDE "latitude"
#define SQL_COL_LONGITUDE "longitude"
#define SQL_COL_SEQ_NUMBER "seq_number"
#define SQL_COL_ATTRIBUTION "attribution"
#define SQL_COL_TIMESTAMP_ATTRIBUTION "timestamp_attribution"
#define SQL_COL_ACK "ack"
#define SQL_COL_DUPLICATE "duplicate"
#define SQL_COL_SNR "snr"
#define SQL_COL_AVG_SIGNAL "avg_signal"
#define SQL_COL_RSSI "rssi"


#define CREATE_SIGFOX_TABLES \
    "-- CREATION OF THE SIGFOX TABLES WITH SOME DATA\n" \
    "\n" \
    "--\n" \
    "-- Create 'raws' table\n" \
    "--\n" \
    "CREATE TABLE IF NOT EXISTS `raws` (\n" \
    "  `id_raws` INTEGER PRIMARY KEY AUTOINCREMENT,\n" \
    "  `timestamp` INTEGER NOT NULL,\n" \
    "  `id_modem` TEXT NOT NULL,\n" \
    "  `snr` REAL NOT NULL,\n" \
    "  `station` TEXT NOT NULL,\n" \
    "  `ack` integer,\n" \
    "  `data` TEXT NOT NULL,\n" \
    "  `duplicate` INTEGER NOT NULL,\n" \
    "  `avg_signal` REAL NOT NULL,\n" \
    "  `rssi` REAL NOT NULL,\n" \
    "  `latitude` INTEGER NOT NULL,\n" \
    "  `longitude` INTEGER NOT NULL,\n" \
    "  `seqNumber` INTEGER NOT NULL\n" \
    ");\n" \
    "\n" \
    "\n" \
    "--\n" \
    "-- Create 'devices' table\n" \
    "--\n" \
    "CREATE TABLE IF NOT EXISTS `devices` (\n" \
    "  `id_modem` TEXT NOT NULL UNIQUE,\n" \
    "  `attribution` INTEGER,\n" \
    "  `timestamp_attribution` INTEGER\n" \
    ");"


#define DROP_SIGFOX_TABLES \
    "-- CREATION OF THE SIGFOX TABLES WITH SOME DATA\n" \
    "\n" \
    "-- Delete the tables if they exists\n" \
    "DROP TABLE IF EXISTS `raws`;\n" \
    "DROP TABLE IF EXISTS `devices`;"


#define SELECT_RAWS "SELECT * FROM `raws`"
#define SELECT_DEVICES "SELECT * FROM `devices`"


/**
 * \brief SQL command to insert data from a sigfox_device_t to the database
 */
#define INSERT_DEVICES "INSERT INTO `devices` VALUES ('%s', %d, %d);"


/**
 * \brief SQL command to insert data from a sigfox_raws_t to the database
 */
#define INSERT_RAWS "INSERT INTO `raws` VALUES (NULL, %ld, '%s', %.2f, '%s', %u, '%s', %u, %.2f, %.2f, %u, %u, %u);"

#ifdef     __cplusplus
}
#endif

#endif          // __SQLS_H__