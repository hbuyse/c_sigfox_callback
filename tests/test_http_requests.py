#! /usr/bin/env python

# coding: utf-8

import pytest
import requests
import os
import signal
import json

PORT = 8000
PROCESS_ID = 0


class TestingHTTPRequests:

    def test_get(self):
        l = [
            {
                'url': 'http://127.0.0.1:{}'.format(PORT),
                'status_code': 200,
            },
            {
                'url': 'http://127.0.0.1:{}/'.format(PORT),
                'status_code': 200,
            },
            {
                'url': 'http://127.0.0.1:{}/toto'.format(PORT),
                'status_code': 404,
            },
            {
                'url': 'http://127.0.0.1:{}/Api'.format(PORT),
                'status_code': 404,
            },
            {
                'url': 'http://127.0.0.1:{}/api'.format(PORT),
                'status_code': 200,
            },
            {
                'url': 'http://127.0.0.1:{}/api/toto'.format(PORT),
                'status_code': 200,
            }
        ]

        for d in l:
            r = requests.get(url=d['url'])
            print(d)
            assert (r.status_code == d['status_code'])

    def test_post_no_data(self):
        l = [
            {
                'url': 'http://127.0.0.1:{}'.format(PORT),
                'status_code': 200,
            },
            {
                'url': 'http://127.0.0.1:{}/'.format(PORT),
                'status_code': 200,
            },
            {
                'url': 'http://127.0.0.1:{}/toto'.format(PORT),
                'status_code': 404,
            },
            {
                'url': 'http://127.0.0.1:{}/Api'.format(PORT),
                'status_code': 404,
            },
            {
                'url': 'http://127.0.0.1:{}/api'.format(PORT),
                'status_code': 400,
            },
            {
                'url': 'http://127.0.0.1:{}/api/toto'.format(PORT),
                'status_code': 400,
            }
        ]

        for d in l:
            r = requests.post(url=d['url'])
            print(d)
            assert (r.status_code == d['status_code'])

    def test_post_w_data_no_ack(self):
        l = [
            {
                'url': 'http://127.0.0.1:{}'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': False,
                    'long_polling': False,
                },
                'status_code': 200,
            },
            {
                'url': 'http://127.0.0.1:{}/'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': False,
                    'long_polling': False,
                },
                'status_code': 200,
            },
            {
                'url': 'http://127.0.0.1:{}/toto'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': False,
                    'long_polling': False,
                },
                'status_code': 404,
            },
            {
                'url': 'http://127.0.0.1:{}/Api'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': False,
                    'long_polling': False,
                },
                'status_code': 404,
            },
            {
                'url': 'http://127.0.0.1:{}/api'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': False,
                    'long_polling': False,
                },
                'status_code': 204,
            },
            {
                'url': 'http://127.0.0.1:{}/api/toto'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': False,
                    'long_polling': False,
                },
                'status_code': 204,
            }
        ]

        for d in l:
            r = requests.post(url=d['url'], data=json.dumps(d['data']))
            print(d)
            assert (r.status_code == d['status_code'])


    def test_post_w_data_w_ack(self):
        l = [
            {
                'url': 'http://127.0.0.1:{}'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': True,
                    'long_polling': False,
                },
                'status_code': 200,
            },
            {
                'url': 'http://127.0.0.1:{}/'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': True,
                    'long_polling': False,
                },
                'status_code': 200,
            },
            {
                'url': 'http://127.0.0.1:{}/toto'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': True,
                    'long_polling': False,
                },
                'status_code': 404,
            },
            {
                'url': 'http://127.0.0.1:{}/Api'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': True,
                    'long_polling': False,
                },
                'status_code': 404,
            },
            {
                'url': 'http://127.0.0.1:{}/api'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': True,
                    'long_polling': False,
                },
                'status_code': 201,
            },
            {
                'url': 'http://127.0.0.1:{}/api/toto'.format(PORT),
                'data': {
                    'id_modem': "BEF",
                    'timestamp': 123456,
                    'duplicate': False,
                    'snr': 10.23,
                    'station': "FED",
                    'data_str': "16f000000000000000000000",
                    'avg_signal': 10.23,
                    'latitude': 2,
                    'longitude': 2,
                    'rssi': 23.45,
                    'seq_number': 12 ,
                    'ack': True,
                    'long_polling': False,
                },
                'status_code': 201,
            }
        ]

        for d in l:
            r = requests.post(url=d['url'], data=json.dumps(d['data']))
            print(d)
            assert (r.status_code == d['status_code'])
