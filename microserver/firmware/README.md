This folder contains Neuron firmware to support some versions of the IzoT ShortStack Micro Server.

For example, the IzoT ShortStack Micro Server images for the FT 6050 Smart Transceiver are built using version 21.04.07 Neuron firmware. Your IzoT ShortStack hardware or prototyping platform for the FT 6050 Smart Transceiver may need upgrading to version 21.04.07 Neuron firmware to allow loading of the IzoT ShortStack Micro Server over the network. 

To upgrade your existing FT 6050-based ShortStack device to use the latest IzoT ShortStack Micro Server for the FT 6050 Smart Transceiver, use in-circuit programming tools to program the SS430_FT6050_SYS20000kHz.nmf (or SS430_FT6050ISI_SYS20000kHz.nmf) image file into the serial flash memory device. 

Alternatively, you can prepare your FT 6050 Smart Transceiver over the network following these steps:

1. Make sure to get the current version of the NodeLoad or Nodeutil utility.

If you do not already have a current version of NodeLoad or NodeUtil, download and install one of the utilities from https://enoceanwiki.atlassian.net/wiki/spaces/LON/pages/2425007/NodeUtil+and+NodeLoad.  If you are downloading NodeUtil, also download the latest NodeUtil Node Utility User’s Guide.  NodeLoad is easier to use for just downloading images to device, whereas NodeUtil provides other useful tools for device developers, including tools for verifying Neuron firmware version numbers.

2.	Download the bla6000.ndl file with NodeLoad or NodeUtil into your FT 6050 Smart Transceiver.

This file is in the microserver/firmware/ver21 folder within the ShortStack repository.

To download with NodeLoad, enter NodeLoad /? to display help.  
To download with NodeUtil, see the NodeUtil documentation for the Y command.
    
3.	Download the b6050v4.ndl Neuron firmware image file with NodeLoad or NodeUtil.

Use the same location and procedures as detailed under the previous step.

4.	If you are using NodeUtil, you can verify the firmware version number with the S command.

5.  Load your SS430_FT6050_20000kHz.ndl (or SS430_FT6050ISI_20000kHz.ndl) IzoT ShortStack Micro Server image file using the same procedure. 

