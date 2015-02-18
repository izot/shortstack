IzoT (tm) ShortStack Development Kit Release 4.30
=================================================

Product Documentation
---------------------

Please refer to the *ShortStack User's Guide*, contained in the 
*doc* directory within this repository, for detailed product documentation.


Installation
------------

Clone this repository, then review the installation instructions and 
other considerations in the *ShortStack Users' Guide*, found in the 
*doc* directory within this repository.


Change History
--------------

Revision history is maintained within the GIT database. Stable releases
are tagged as **vM.NN**, where *M* and *N* are the mmajor and minor 
version numbers, respectively. 

The first release of IzoT ShortStack to be hosted with github.com is version
4.30, so the first stable release will be tagged **v4.30**.

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

**example**
contains a set of examples for a number of applications and targets. Each 
example includes example-specific documentation. The examples are configured to
reference other files within the root of this repository, such as files 
contained in the *api* or *microserver/standard* directories. 

**example/rpi-simple**
contains a very simple example application with a complete API port and example
driver for use with the Raspberry Pi mini-computer and the Raspbian Linux operating
system. 


Reporting Bugs and Feedback
---------------------------

To report bugs or send feedback, open a Support Ticket at
<http://www.echelon.com/support>.


Copyright and License
---------------------

Use of this code is subject to your compliance with the terms of the
Echelon IzoT(tm) Software Developer's Kit License Agreement which is
available in the **LICENSE.md** file in your **shortstack** directory.

Copyright © 2002-2015 Echelon Corporation. The Echelon logo, Echelon, 
LON, LONWORKS, Digital Home, i.LON, LNS, LONMARK, LonMaker, LonPoint, 
LonTalk, Neuron, NodeBuilder, Pyxos, and ShortStack, are trademarks 
of Echelon Corporation registered in the United States and other 
countries. A full list of Echelon trademarks is available at 
<http://www.echelon.com>. All other trademarks are owned by their 
respective owners. All rights reserved.
