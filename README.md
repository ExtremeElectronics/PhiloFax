# PhiloFax
Farnsworth - Sound and audio over UDP for Pi PicoW - written in C

### Software to implement a Farnsworth (see Warehouse 13) using a Pi Picow, GC9A01 35mm circular display and a cheap OV7670 Camera

## Camera
The camera is implemented using a parallel to serial shift register to reduce the GPIO pin count on the Pico
PIO's are used to de serialise the camera data. The Camera is fixed at 240x320 RGB565, Control is via i2c

## Display
The display uses a GC9A01 35mm circular display module via SPI
Datafrom the camera or from UDP is streamed line by line via DMA

## Audio
PWM direct to a speaker for output, single transistor amp from an electret microphone via the picos a2d

## Networking
Currently very simple UDP with no encoding,compression,error or loss correction. Video in 240 pixel lines, sent as available. 
Only currently supports IP to IP connection set by ini file in SD Card

## SDCard
SD card slot for programming/storage uses FatFS and an ini file parced by iniparcer library. An Example INI file is in SDCardContents folder. Currently the softwar wont start without this. 

## USB Serial 
USB serial has debug output

## UDP Repeater (Strowger server)
Strowger.py is a simple UDP repeater that allows two Farnsworths to connect on a "Channel". This means that both the farnsworths can use the same destination address (the Strowger server's) 

It also enables the possibity of dialing other users on the same server (not yet implimented) 

# Current Project State
  Working, with issues, see ToDo

## ToDo
Document Software - blog via ExtremeElectronics. 

Sort Licencing issues and attribution

Sort DMA 8/16 bit DMA to display

Add Alaw Audio encoding

Add better amp for Microphone

Add Support for JogWheel for destination selection/modes

Add ring and connect tones

# The current Farnsworth Project - Artifact Creation.

## Security 
WARNING, THERE IS NONE. Everything you do and say is transmitted in the raw to the internet. NO Passwords, NO Encryption. YOU HAVE BEEN WARNED

![2a2deadc2fc2ff37](https://github.com/ExtremeElectronics/PhiloFax/assets/102665314/86b3b2e3-a4dd-43d1-9928-bf8416e8bee0)


![c6f870eb24ed178d](https://github.com/ExtremeElectronics/PhiloFax/assets/102665314/91131192-dc36-4a74-999a-d2169781ccfb)

## Other usefull code 
Gimp to RGB565 array in C https://github.com/MrPozor/gimp_export565array/blob/master/export565array.py


