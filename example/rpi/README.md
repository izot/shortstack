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

IO
--

The *io* folder contains some utilities for general purpose I/O on this platform. The GPIO bit I/O is used by the driver module, but applications may also use these routines for application-specific input or output. 


Simple Example
--------------

The *simple* example focusses on the general framework, implementing a very simple example application. Start reviewing the technology with this example. Start reviewing this example with the rpi-simple.c main source file.

Simple ISI Example
------------------

The *simple-isi* example focusses on using the IzoT ShortStack ISI engine and API. The example requires an ISI-enabled Micro Server, for example SS430_FT6050ISI_SYS20000kHz. Start reviewing this example with the rpi-simple-isi.c source file. 

In addition to the general project settings discussed in the Eclipse Configuration document, this example requires that you define the following symbols in your project settings or makefile for all configurations:

ISIGETWIDTH_HANDLED
ISIGETNVINDEX_HANDLED
ISIGETASSEMBLY_HANDLED
ISICREATECSMO_HANDLED
ISIUPDATEUSERINTERFACE_HANDLED


Runtime Interface Selection Example
-----------------------------------

The *ris* example demonstrates an advanced feature, the implementation of an application with runtime interface selection. Start reviewing this example with the rpi-ris.c main source file.
