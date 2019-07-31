IzoT ShortStack Development Kit 4.3
===================================

Product Documentation
---------------------

Adesto’s IzoT® ShortStack Software Development Kit (SDK) enables anyproduct that contains a microprocessor or microcontroller to quickly and inexpensively become a networked and Internet connected smart device. The ShortStack Micro Server provides a simple way to add LON/IP and native LON networking to new or existing smart devices. The IzoT ShortStack SDK is easy to use due to a simple host API, a simple driver, a simple hardware interface, a small host memory footprint, and comprehensive tool support.  See the [*IzoT ShortStack SDK User's Guide*](http://downloads.echelon.com/support/documentation/manuals/devtools/078-0540-01B_IzoT_ShortStack_SDK_User's_Guide.pdf "IzoT ShortStack SDK User's Guide") for further documentation. 


Installation
------------

Clone this repository, then review the installation instructions in the [*IzoT ShortStack SDK User's Guide*](http://downloads.echelon.com/support/documentation/manuals/devtools/078-0540-01B_IzoT_ShortStack_SDK_User's_Guide.pdf "IzoT ShortStack SDK User's Guide").



Change History
--------------

Revision history is maintained within the GIT database. Stable releases
are tagged as **vM.NN**, where *M* and *N* are the mmajor and minor 
version numbers, respectively. 

The first release of IzoT ShortStack to be hosted with github.com is version
4.30, so the first stable release is tagged **v4.30**.

Revisions between stable releases should be considered experimental or under 
development.


Repository Content and Layout
-----------------------------

The **shortstack** repository is organized into the following
directories:  

**api**
contains the ShortStack host API and related source code. You need to adapt 
this code to your compiler toolchain with definitions within *LonPlatform.h* 
and port the API source code to be compiled and linked with your application.

**bin** 
contains binary utilities and installers, including Windows Installers for
the *IzoT Interface Interpreter* (iii) and the *IzoT Resource Editor*.

**doc**
contains further documentation.

**microserver**
contains two subfolders, **microserver/standard** and **microserver/custom**.

**microserver/standard**
contains a set of pre-build standard Micro Server images for use with the
current edition of IzoT ShortStack. Standard Micro Servers are designed for 
use with most typical hardware configurations for a variety of common Neuron 
Chips and Smart Transceivers.

**microserver/custom**
contains a set of pre-build Neuron C libraries and a number of source files. 
You can use these source files and libraries to construct custom Micro Server
images to support requirements not met by any of the standard Micro Servers. 
Note that construction of custom ShortStack Micro Servers requires that you
have a licensed copy of Echelon IzoT NodeBuilder installed. 

**microserver/firmware**
contains updated Neuron firmware for use with some Neuron Chips or Smart Transceivers. Please see the README.md file within for detailed insstructions.

**example**
contains a set of examples for a number of applications and targets. Each 
example includes example-specific documentation. The examples are configured to
reference other files within the root of this repository, such as files 
contained in the *api* or *microserver/standard* directories. 

**example/rpi/simple**
contains a very simple example application with a complete API port and example
driver for use with the Raspberry Pi mini-computer and the Raspbian Linux operating
system (or a compatible platform). 

**example/rpi/simple-isi**
contains a simple example application to demonstrate the use of the Interoperable Self-Installation API (ISI) for use with the Raspberry Pi mini-computer and the Raspbian Linux operating system (or a compatible platform).

**example/rpi/ris**
contains an example for an application implementing runtime interface selection for use with the Raspberry Pi mini-computer and the Raspbian Linux operating system (or a compatible platform).

**example/rpi/properties**
contains an example for an application which implements properties in property files. The example also demonstrates the initialization of complex property types, and demonstrates the use of properties for a configurable value range limitation. 


Reporting Bugs and Feedback
---------------------------

To report bugs or send feedback, open a Support Ticket at
<http://www.echelon.com/support>.


Copyright and License
---------------------

Use of this code is subject to your compliance with the terms of the
Echelon IzoT(tm) Software Developer's Kit License Agreement which is
available in the **LICENSE.md** file in your **shortstack** directory.

Copyright © 2002-2019 Adesto Technologies Corporation. Echelon, LON, the Echelon logo, LonWorks, NodeBuilder, LonTalk, LonPoint, Neuron, 3120, 3150, LNS, i.LON, ShortStack, Pyxos, LonMaker, IzoT, LonLink, LonResponse, LonSupport, OpenLDV, LonScanner, and LonBridge are trademarks of Echelon Corporation that may be registered in the United States and other countries. For a complete list of registered trademarks see the Echelon Web site at www.echelon.com. All rights reserved.
