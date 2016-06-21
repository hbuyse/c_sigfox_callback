/**
 * @file frame.h
 * @author hbuyse
 * @date 17/06/2016
 *
 * @brief  Functions that reports to the frames received
 */


#ifndef __FRAME_H__
#define __FRAME_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SIGFOX_DATA_LENGTH 12
#define SIGFOX_STATION_LENGTH 2
#define SIGFOX_DEVICE_LENGTH 4


typedef struct sigfox_callback_s sigfox_callback_info_t;

struct sigfox_callback_info_s {
    unsigned char device[SIGFOX_DEVICE_LENGTH];          ///< device identifier (in hexadecimal – up to 8 characters <=> 4 bytes)
    unsigned char duplicate;          ///< «true» if the message is a duplicate one, meaning that the backend has already processed this message from
                                      // a
                                      // different base station, «false» otherwise. To receive duplicate messages, you have to check the «send
                                      // duplicate» box
                                      // in the callback configuration page
    double snr;          ///< the signal to noise ratio (in dB – Float value with two maximum fraction digits)
    double rssi;          ///< the RSSI (in dBm – Float value with two maximum fraction digits). If there is no data to be returned, then the value is
                          // null.
    double avgSnr;          ///< the average signal to noise ratio computed from the last 25 messages (in dB – Float value with two maximum fraction
                            // digits) or
                            // «N/A». The device must have send at least 15 messages.
    unsigned char station[SIGFOX_STATION_LENGTH];          ///< the base station identifier (in hexadecimal – 4 characters <=> 2 bytes)
    unsigned char data[SIGFOX_DATA_LENGTH];          ///< the user data (in hexadecimal)
    double lat;          ///< the latitude, rounded to the nearest integer, of the base station which received the message
    double lng;          ///< the longitude, rounded to the nearest integer, of the base station which received the message
    unsigned int seqNumber;          ///< the sequence number of the message if available
}


#ifdef     __cplusplus
}
#endif

#endif          // __FRAME_H__