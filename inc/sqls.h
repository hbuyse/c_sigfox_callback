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


/**
 * \defgroup  Raws_col_name  Raws table column names
 * @{
 */
#define SQL_COL_ID_RAWS "id_raws"          ///< ID raws column name
#define SQL_COL_ID_MODEM "id_modem"          ///< ID modem column name
#define SQL_COL_TIMESTAMP "timestamp"          ///< Timestamp column name
#define SQL_COL_LATITUDE "latitude"          ///< Latitude column name
#define SQL_COL_DATA_STR "data_str"          ///< Data string column name
#define SQL_COL_DATA_HEX "data_hex"          ///< Hexadecimal data column name
#define SQL_COL_LONGITUDE "longitude"          ///< Longitude column name
#define SQL_COL_SEQ_NUMBER "seq_number"          ///< Sequence number column name
#define SQL_COL_ATTRIBUTION "attribution"          ///< Attribution column name
#define SQL_COL_TIMESTAMP_ATTRIBUTION "timestamp_attribution"          ///< Timestamp attribution column name
#define SQL_COL_ACK "ack"          ///< Acknowledge column name
#define SQL_COL_DUPLICATE "duplicate"          ///< Duplicate column name
#define SQL_COL_SNR "snr"          ///< Signal column name
#define SQL_COL_AVG_SIGNAL "avg_signal"          ///< Average signal column name
#define SQL_COL_RSSI "rssi"          ///< RSSI column name


/**@}*/


/**
 * \brief SQL command to create the 'raws' and 'devices' tables in the database
 */
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
    "  `data_str` TEXT NOT NULL,\n" \
    "  `duplicate` INTEGER NOT NULL,\n" \
    "  `avg_signal` REAL NOT NULL,\n" \
    "  `rssi` REAL NOT NULL,\n" \
    "  `latitude` INTEGER NOT NULL,\n" \
    "  `longitude` INTEGER NOT NULL,\n" \
    "  `seq_number` INTEGER NOT NULL\n" \
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


/**
 * \brief SQL command to drop 'raws' and 'devices' tables
 */
#define DROP_SIGFOX_TABLES \
    "-- DELETION OF THE SIGFOX TABLES\n" \
    "\n" \
    "-- Delete the tables if they exists\n" \
    "DROP TABLE IF EXISTS `raws`;\n" \
    "DROP TABLE IF EXISTS `devices`;"


/**
 * \brief SQL command to select all the colums in the 'raws' table
 */
#define SELECT_RAWS "SELECT * FROM `raws`"


/**
 * \brief SQL command to select all the colums in the 'devices' table
 */
#define SELECT_DEVICES "SELECT * FROM `devices`"


/**
 * \brief SQL command to insert data from a sigfox_device_t to the database
 */
#define INSERT_DEVICES "INSERT INTO `devices` VALUES ('%s', %d, %d);"


/**
 * \brief SQL command to insert data from a sigfox_raws_t to the database
 */
#define INSERT_RAWS \
    "INSERT INTO `raws` VALUES (NULL, %ld, '%s', %.2f, '%s', %u, '%s', %u, %.2f, %.2f, %u, %u, %u);"

#ifdef     __cplusplus
}
#endif

#endif          // __SQLS_H__