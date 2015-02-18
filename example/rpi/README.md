IzoT ShortStack RaspBerry Pi Examples
=====================================

This directory contains IzoT ShortStack example applications and example driver software for use
with the Raspberry Pi mini-computer using the Raspbian Linux operating system, or other targets with similar hard- and software for the host platform. 

These examples have been tried on *Raspberry Pi Model B V1.2* and *Raspberry Pi 2 Model B V1.1*. The examples assume an asynchronous serial link layer connection (SCI), typically at 38400 baud with a 3.3V signal level.

Documentation
-------------

See the *doc* folder for wiring instructions to connect a Raspberry Pi board to a Micro Server using an Echelon FT 5000 or FT 6050 EVB. See the product documentation for connectivity to any other IzoT ShortStack Micro Server.

Driver
------

The *driver* folder contains a serial link-layer driver implementation shared by all example applications within this folder.

Simple Example
--------------

The *simple* example focusses on the general framework, implementing a very simple example application. Start reviewing the technology with this example. Start reviewing this example with the rpi-simple.c main source file.