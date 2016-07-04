/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include <sqlite3.h>

#include <db_plugin_sqlite.h>
#include <sqls.h>
#include <frames.h>          // sigfox_raws_t
#include <logging.h>          // iprintf, eprintf, gprintf, cprintf


static void op_set(struct mg_connection *nc, const struct http_message *hm, const struct mg_str *key, void *db);


static void op_get(struct mg_connection *nc, const struct http_message *hm, const struct mg_str *key, void *db);


static void op_del(struct mg_connection *nc, const struct http_message *hm, const struct mg_str *key, void *db);


/**
 * \brief      From a JSON structure, we create a raws structure
 *
 * \param      raws  The raws structure
 * \param      jobj  The JSON object
 *
 * \return     0 no error else a number
 */
static unsigned char raws_from_json(sigfox_raws_t *raws, struct json_token *jobj);



/**
 * \brief           Convert a data string to an hexadecimal array
 *
 * \param[in]       data_str  The data as string
 * \param[out]      data_hex  The data as hexadecimal
 */
static void convert_data_str_to_data_hex(const unsigned char    data_str[SIGFOX_DATA_STR_LENGTH + 1],
                                         unsigned char          data_hex[SIGFOX_DATA_LENGTH]);


#ifdef __DEBUG__
    #define MG_PRINTF_200 \
    mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n"); gprintf("200 OK\n");
    #define MG_PRINTF_201 \
    mg_printf(nc, "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n"); gprintf("201 Created\n");
    #define MG_PRINTF_400 \
    mg_printf(nc, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n"); eprintf("400 Bad Request\n");
    #define MG_PRINTF_404 \
    mg_printf(nc, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n"); eprintf("404 Not Found\n");
    #define MG_PRINTF_500 \
    mg_printf(nc, "HTTP/1.1 500 Server Error\r\nContent-Length: 0\r\n\r\n"); eprintf("500 Server Error\n");
    #define MG_PRINTF_501 \
    mg_printf(nc, "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n"); eprintf("501 Not Implemented\n");
#else
    #define MG_PRINTF_200   mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
    #define MG_PRINTF_201   mg_printf(nc, "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n");
    #define MG_PRINTF_400   mg_printf(nc, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
    #define MG_PRINTF_404   mg_printf(nc, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
    #define MG_PRINTF_500   mg_printf(nc, "HTTP/1.1 500 Server Error\r\nContent-Length: 0\r\n\r\n");
    #define MG_PRINTF_501   mg_printf(nc, "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n");
#endif


sqlite3* db_open(const char *db_path)
{
    sqlite3     *db = NULL;


    if ( sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                         NULL) == SQLITE_OK )
    {
        sqlite3_exec(db, CREATE_SIGFOX_TABLES, 0, 0, 0);
    }

    return (db);
}



void db_close(void **db_handler)
{
    if ( (db_handler != NULL) && (*db_handler != NULL) )
    {
        sqlite3_close(*db_handler);
        *db_handler = NULL;
    }
}



void db_op(struct mg_connection         *nc,
           const struct http_message    *hm,
           const struct mg_str          *key,
           void                         *db,
           int                          op
           )
{
    switch ( op )
    {
        case API_OP_GET:
            op_get(nc, hm, key, db);
            break;

        case API_OP_SET:
            op_set(nc, hm, key, db);
            break;

        case API_OP_DEL:
            op_del(nc, hm, key, db);
            break;

        default:
            MG_PRINTF_501
            break;
    }
}



static void op_set(struct mg_connection         *nc,
                   const struct http_message    *hm,
                   const struct mg_str          *key __attribute__((unused)),
                   void                         *db
                   )
{
    sqlite3_stmt        *stmt       = NULL;
    const struct mg_str     *body   = (hm->query_string.len > 0) ? &hm->query_string : &hm->body;
    struct json_token       *root   = NULL;
    sigfox_raws_t           raws;


    root = parse_json2(body->p, body->len);

    if ( ! root )
    {
        MG_PRINTF_400

        return;
    }

    if ( raws_from_json(&raws, root) )
    {
        MG_PRINTF_400

        return;
    }

    // Prepare the SQLite statement
    if ( sqlite3_prepare_v2(db, INSERT_RAWS, -1, &stmt, NULL) == SQLITE_OK )
    {
        sqlite3_bind_int(stmt, SQL_IDX_TIMESTAMP, raws.timestamp);
        sqlite3_bind_text(stmt, SQL_IDX_ID_MODEM, (const char *) raws.id_modem, strlen(
                              (const char *) raws.id_modem), SQLITE_STATIC);
        sqlite3_bind_double(stmt, SQL_IDX_SNR, raws.snr);
        sqlite3_bind_text(stmt, SQL_IDX_STATION, (const char *) raws.station, strlen(
                              (const char *) raws.station), SQLITE_STATIC);
        sqlite3_bind_int(stmt, SQL_IDX_ACK, raws.ack);
        sqlite3_bind_text(stmt, SQL_IDX_DATA_STR, (const char *) raws.data_str, strlen(
                              (const char *) raws.data_str), SQLITE_STATIC);
        sqlite3_bind_blob(stmt, SQL_IDX_DATA_HEX, (void *) raws.data_hex, sizeof(raws.data_hex), SQLITE_STATIC);
        sqlite3_bind_int(stmt, SQL_IDX_DUPLICATE, raws.duplicate);
        sqlite3_bind_double(stmt, SQL_IDX_AVG_SIGNAL, raws.avg_signal);
        sqlite3_bind_double(stmt, SQL_IDX_RSSI, raws.rssi);
        sqlite3_bind_int(stmt, SQL_IDX_LATITUDE, raws.latitude);
        sqlite3_bind_int(stmt, SQL_IDX_LONGITUDE, raws.longitude);
        sqlite3_bind_int(stmt, SQL_IDX_SEQ_NUMBER, raws.seq_number);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    MG_PRINTF_201
}



static void op_get(struct mg_connection         *nc,
                   const struct http_message    *hm __attribute__( (unused) ),
                   const struct mg_str          *key __attribute__( (unused) ),
                   void                         *db
                   )
{
    sqlite3_stmt        *stmt   = NULL;
    int                 result;


    if ( sqlite3_prepare_v2(db, SELECT_RAWS, -1, &stmt, NULL) == SQLITE_OK )
    {
        result  = sqlite3_step(stmt);

        // Send headers
        mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nTransfer-Encoding: chunked\r\n\r\n");

        mg_printf_http_chunk(nc, "[ ");

        while ( ((result  = sqlite3_step(stmt)) == SQLITE_OK) || (result == SQLITE_ROW))
        {
            mg_printf_http_chunk(nc, "{ ");
            mg_printf_http_chunk(nc, "\"timestamp\": %lu, ", sqlite3_column_int(stmt, SQL_IDX_TIMESTAMP));
            mg_printf_http_chunk(nc, "\"id_modem\": \"%s\", ", sqlite3_column_text(stmt, SQL_IDX_ID_MODEM));
            mg_printf_http_chunk(nc, "\"snr\": %f, ", sqlite3_column_double(stmt, SQL_IDX_SNR));
            mg_printf_http_chunk(nc, "\"station\": \"%s\", ", sqlite3_column_text(stmt, SQL_IDX_STATION));
            mg_printf_http_chunk(nc, "\"ack\": %u, ", (sqlite3_column_int(stmt, SQL_IDX_ACK)) ? "true" : "false");
            mg_printf_http_chunk(nc, "\"data_str\": \"%s\", ", sqlite3_column_text(stmt, SQL_IDX_DATA_STR));
            mg_printf_http_chunk(nc, "\"duplicate\": %u, ", (sqlite3_column_int(stmt, SQL_IDX_DUPLICATE)) ? "true" : "false");
            mg_printf_http_chunk(nc, "\"avg_signal\": %f, ", sqlite3_column_double(stmt, SQL_IDX_AVG_SIGNAL));
            mg_printf_http_chunk(nc, "\"rssi\": %f, ", sqlite3_column_double(stmt, SQL_IDX_RSSI));
            mg_printf_http_chunk(nc, "\"latitude\": %lu, ", sqlite3_column_int(stmt, SQL_IDX_LATITUDE));
            mg_printf_http_chunk(nc, "\"longitude\": %lu, ", sqlite3_column_int(stmt, SQL_IDX_LONGITUDE));
            mg_printf_http_chunk(nc, "\"seq_number\": %lu, ", sqlite3_column_int(stmt, SQL_IDX_SEQ_NUMBER));
            mg_printf_http_chunk(nc, "}, ");
        }

        sqlite3_finalize(stmt);

        mg_printf_http_chunk(nc, "]");

        // Send empty chunk, the end of response
        mg_send_http_chunk(nc, "", 0);
        gprintf("200 OK\n");
    }
    else
    {
        MG_PRINTF_500
    }
}



static void op_del(struct mg_connection         *nc,
                   const struct http_message    *hm,
                   const struct mg_str          *key,
                   void                         *db
                   )
{
    sqlite3_stmt        *stmt = NULL;
    int                 result;


    (void) hm;

    if ( sqlite3_prepare_v2(db, "DELETE FROM kv WHERE key = ?;", -1, &stmt, NULL) == SQLITE_OK )
    {
        sqlite3_bind_text(stmt, 1, key->p, key->len, SQLITE_STATIC);
        result = sqlite3_step(stmt);

        if ( (result == SQLITE_OK) || (result == SQLITE_ROW) )
        {
            MG_PRINTF_200
        }
        else
        {
            MG_PRINTF_404
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        MG_PRINTF_500
    }
}



static unsigned char raws_from_json(sigfox_raws_t       *raws,
                                    struct json_token   *jobj
                                    )
{
    struct json_token     *tmp = NULL; // Temporary JSON struct


    memset(raws, 0, sizeof(*raws) );


    // Get the modem
    tmp = find_json_token(jobj, SQL_COL_ID_MODEM);          // Get the JSON object

    if ( tmp )
    {
        unsigned short      length = (tmp->len > SIGFOX_DEVICE_LENGTH) ? SIGFOX_DEVICE_LENGTH : tmp->len;
        char                modem_ascii[length + 1];
        memset(modem_ascii, 0, sizeof(modem_ascii) );

        memcpy(raws->id_modem, tmp->ptr, length);

#ifdef __DEBUG__
        iprintf("raws->id_modem : %s (%hu)\n", raws->id_modem, length);
#endif
    }
    else
    {
        return (1);
    }

    // Get the timestamp
    tmp = find_json_token(jobj, SQL_COL_TIMESTAMP);          // Get the JSON object

    if ( tmp )
    {
        char     timestamp_ascii[tmp->len + 1];
        memset(timestamp_ascii, 0, sizeof(timestamp_ascii) );

        memcpy(timestamp_ascii, tmp->ptr, tmp->len);          // Fill the string
        raws->timestamp = strtol(timestamp_ascii, NULL, 10);          // Transform the string into a time_t

#ifdef __DEBUG__
        iprintf("raws->timestamp : %ld (%lu)\n", raws->timestamp, sizeof(raws->timestamp) );
#endif
    }
    else
    {
        return (2);
    }

    // Get the duplicate
    tmp = find_json_token(jobj, SQL_COL_DUPLICATE);          // Get the JSON object

    if ( tmp )
    {
        char     duplicate_ascii[tmp->len + 1];
        memset(duplicate_ascii, 0, sizeof(duplicate_ascii) );

        memcpy(duplicate_ascii, tmp->ptr, tmp->len);          // Fill the string$

        if ( strncmp(duplicate_ascii, "true", 4) == 0 )
        {
            raws->duplicate = 1;
        }

#ifdef __DEBUG__
        iprintf("raws->duplicate : %u (%lu)\n", raws->duplicate, sizeof(raws->duplicate) );
#endif
    }
    else
    {
        return (3);
    }

    // Get the modem
    tmp = find_json_token(jobj, SQL_COL_SNR);          // Get the JSON object

    if ( tmp )
    {
        char     snr_ascii[tmp->len + 1];
        memset(snr_ascii, 0, sizeof(snr_ascii) );

        memcpy(snr_ascii, tmp->ptr, tmp->len);          // Fill the string
        raws->snr = strtod(snr_ascii, NULL);          // Transform the string into a double

#ifdef __DEBUG__
        iprintf("raws->snr : %f (%lu)\n", raws->snr, sizeof(raws->snr) );
#endif
    }
    else
    {
        return (4);
    }

    // Get the station
    tmp = find_json_token(jobj, SQL_COL_STATION);          // Get the JSON object

    if ( tmp )
    {
        unsigned short      length = (tmp->len > SIGFOX_STATION_LENGTH) ? SIGFOX_STATION_LENGTH : tmp->len;
        char                station_ascii[length + 1];
        memset(station_ascii, 0, sizeof(station_ascii) );

        memcpy(raws->station, tmp->ptr, length);

#ifdef __DEBUG__
        iprintf("raws->station : %s (%hu)\n", raws->station, length);
#endif
    }
    else
    {
        return (5);
    }

    // Get the datas
    tmp = find_json_token(jobj, SQL_COL_DATA_STR);          // Get the JSON object

    if ( tmp )
    {
        unsigned short      length = (tmp->len > SIGFOX_DATA_STR_LENGTH) ? SIGFOX_DATA_STR_LENGTH : tmp->len;
        char                data_ascii[length + 1];
        memset(data_ascii, 0, sizeof(data_ascii) );

        memcpy(raws->data_str, tmp->ptr, length);
        convert_data_str_to_data_hex(raws->data_str, raws->data_hex);

#ifdef __DEBUG__
        iprintf("raws->data_str : %s (%hu)\n", raws->data_str, length);
        iprintf("raws->data_hex : %s (%hu)\n", raws->data_hex, length);
#endif
    }
    else
    {
        return (6);
    }

    // Get the modem
    tmp = find_json_token(jobj, SQL_COL_AVG_SIGNAL);          // Get the JSON object

    if ( tmp )
    {
        char     avg_signal_ascii[tmp->len + 1];
        memset(avg_signal_ascii, 0, sizeof(avg_signal_ascii) );

        memcpy(avg_signal_ascii, tmp->ptr, tmp->len);          // Fill the string
        raws->avg_signal = strtod(avg_signal_ascii, NULL);          // Transform the string into a double

#ifdef __DEBUG__
        iprintf("raws->avg_signal : %f (%lu)\n", raws->avg_signal, sizeof(raws->avg_signal) );
#endif
    }
    else
    {
        return (7);
    }

    // Get the duplicate
    tmp = find_json_token(jobj, SQL_COL_LATITUDE);          // Get the JSON object

    if ( tmp )
    {
        char     latitude_ascii[tmp->len + 1];
        memset(latitude_ascii, 0, sizeof(latitude_ascii) );

        memcpy(latitude_ascii, tmp->ptr, tmp->len);          // Fill the string$
        raws->latitude = strtol(latitude_ascii, NULL, 10);          // Transform the string into a number

#ifdef __DEBUG__
        iprintf("raws->latitude : %u (%lu)\n", raws->latitude, sizeof(raws->latitude) );
#endif
    }

    // Get the duplicate
    tmp = find_json_token(jobj, SQL_COL_LONGITUDE);          // Get the JSON object

    if ( tmp )
    {
        char     longitude_ascii[tmp->len + 1];
        memset(longitude_ascii, 0, sizeof(longitude_ascii) );

        memcpy(longitude_ascii, tmp->ptr, tmp->len);          // Fill the string$
        raws->longitude = strtol(longitude_ascii, NULL, 10);          // Transform the string into a number

#ifdef __DEBUG__
        iprintf("raws->longitude : %u (%lu)\n", raws->longitude, sizeof(raws->longitude) );
#endif
    }

    // Get the modem
    tmp = find_json_token(jobj, SQL_COL_RSSI);          // Get the JSON object

    if ( tmp )
    {
        char     rssi_ascii[tmp->len + 1];
        memset(rssi_ascii, 0, sizeof(rssi_ascii) );

        memcpy(rssi_ascii, tmp->ptr, tmp->len);          // Fill the string
        raws->rssi = strtod(rssi_ascii, NULL);          // Transform the string into a double

#ifdef __DEBUG__
        iprintf("raws->rssi : %f (%lu)\n", raws->rssi, sizeof(raws->rssi) );
#endif
    }
    else
    {
        return (7);
    }

    // Get the timestamp
    tmp = find_json_token(jobj, SQL_COL_SEQ_NUMBER);          // Get the JSON object

    if ( tmp )
    {
        char     seq_number_ascii[tmp->len + 1];
        memset(seq_number_ascii, 0, sizeof(seq_number_ascii) );

        memcpy(seq_number_ascii, tmp->ptr, tmp->len);          // Fill the string
        raws->seq_number = strtol(seq_number_ascii, NULL, 10);          // Transform the string into a time_t

#ifdef __DEBUG__
        iprintf("raws->seq_number : %u (%lu)\n", raws->seq_number, sizeof(raws->seq_number) );
#endif
    }
    else
    {
        return (8);
    }

    // Get the duplicate
    tmp = find_json_token(jobj, SQL_COL_ACK);          // Get the JSON object

    if ( tmp )
    {
        char     ack_ascii[tmp->len + 1];
        memset(ack_ascii, 0, sizeof(ack_ascii) );

        memcpy(ack_ascii, tmp->ptr, tmp->len);          // Fill the string$

        if ( strncmp(ack_ascii, "true", 4) == 0 )
        {
            raws->ack = 1;
        }

#ifdef __DEBUG__
        iprintf("raws->ack : %u (%lu)\n", raws->ack, sizeof(raws->ack) );
#endif
    }
    else
    {
        raws->ack = 0;
    }

    // Get the duplicate
    tmp = find_json_token(jobj, SQL_COL_LONG_POLLING);          // Get the JSON object

    if ( tmp )
    {
        char     long_polling_ascii[tmp->len + 1];
        memset(long_polling_ascii, 0, sizeof(long_polling_ascii) );

        memcpy(long_polling_ascii, tmp->ptr, tmp->len);          // Fill the string$

        if ( strncmp(long_polling_ascii, "true", 4) == 0 )
        {
            raws->long_polling = 1;
        }

#ifdef __DEBUG__
        iprintf("raws->long_polling : %u (%lu)\n", raws->long_polling, sizeof(raws->long_polling) );
#endif
    }
    else
    {
        return (10);
    }

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