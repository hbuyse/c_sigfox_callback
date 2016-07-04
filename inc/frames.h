/**
 * @file frame.h
 * @author hbuyse
 * @date 17/06/2016
 *
 * @brief  Functions that reports to the frames received
 */


#ifndef __FRAME_H__
#define __FRAME_H__

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief Length of a Sigfox data frame in string
 */
#define SIGFOX_DATA_STR_LENGTH  24


/**
 * \brief Length of a Sigfox data frame in hexadecimal
 */
#define SIGFOX_DATA_LENGTH      12


/**
 * \brief Length of a Sigfox station name
 */
#define SIGFOX_STATION_LENGTH   4


/**
 * \brief Length of a Sigfox modem name
 */
#define SIGFOX_DEVICE_LENGTH    8


/**
 * \typedef sigfox_raws_t
 */
typedef struct sigfox_raws_s sigfox_raws_t;


/**
 * \typedef sigfox_device_t
 */
typedef struct sigfox_device_s sigfox_device_t;


/**
 * \brief      Contains all the informations that can be given by the Sigfox backend
 */
struct sigfox_raws_s {
    unsigned char id_modem[SIGFOX_DEVICE_LENGTH + 1];          ///< device identifier (in hexadecimal – up to 8 characters <=> 4 bytes)
    time_t timestamp;                        ///< the event timestamp (in seconds since the Unix Epoch)
    unsigned char duplicate;          ///< «true» if the message is a duplicate one, meaning that the backend has already processed this message from
                                      ///< a different base station, «false» otherwise. To receive duplicate messages, you have to check the «send
                                      ///< duplicate» box in the callback configuration page
    double snr;          ///< the signal to noise ratio (in dB – Float value with two maximum fraction digits)
    unsigned char station[SIGFOX_STATION_LENGTH + 1];          ///< the base station identifier (in hexadecimal – 4 characters <=> 2 bytes)
    unsigned char data_str[SIGFOX_DATA_STR_LENGTH + 1];          ///< the user data (in string)
    unsigned char data_hex[SIGFOX_DATA_LENGTH];          ///< the user data (in hexadecimal)
    double avg_signal;          ///< the average signal to noise ratio computed from the last 25 messages (in dB – Float value with two maximum
                                ///< fraction digits) or «N/A». The device must have send at least 15 messages.
    short latitude;          ///< the latitude, rounded to the nearest integer, of the base station which received the message
    short longitude;          ///< the longitude, rounded to the nearest integer, of the base station which received the message
    double rssi;          ///< the RSSI (in dBm – Float value with two maximum fraction digits). If there is no data to be returned, then the value is
                          ///< null.
    unsigned int seq_number;          ///< the sequence number of the message if available
    unsigned char ack;          ///< true if this message needs to be acknowledged, false else
    unsigned char long_polling;          ///< longPolling : 'true' or 'false'
};


/**
 * \brief      Contains all the informations that defines a Sigfox device
 */
struct sigfox_device_s {
    unsigned char id_modem[SIGFOX_DEVICE_LENGTH];          ///< device identifier (in hexadecimal – up to 8 characters <=> 4 bytes)
    int attribution;
    int timestamp_attribution;
};

#ifdef     __cplusplus
}
#endif

#endif          // __FRAME_H__