C SIGFOX CALLBACK
#################

.. image:: https://travis-ci.org/hbuyse/c_sigfox_callback.svg?branch=master
    :target: https://travis-ci.org/hbuyse/c_sigfox_callback

.. image:: https://codedocs.xyz/hbuyse/c_sigfox_callback.svg
    :target: https://codedocs.xyz/hbuyse/c_sigfox_callback


.. contents::


Synopsis
========

Small API server allowing to list all the SIGFOX events that were received.
It uses a `SQLite3 database <https://www.sqlite.org>`_ to record all events and use `Mongoose <https://github.com/cesanta/mongoose>`_ webserver to list the events on a webpage.


Compilation
===========

In order to compile the program, you simply have to do:

.. code:: bash

    make [OPTIONS]

Options available :
 * OPTIM=NONE|DEBUG|SIZE|SPEED  (dft : DEBUG)
 * STATIC=0|1  (dft : 0)
 * V=0|1  (dft : 0)


Utilization
===========

To use the program:

.. code:: bash

    ./sigfox_callback.out


Contributors
============

* Henri Buyse (henri.buyse@gmail.com)


Licence
=======

C Sigfox Callback is an open source software provided under the `GNU GPLv2 License <./LICENSE>`_.