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
SD card slot for programming/storage uses FatFS and an ini file parced by iniparcer library

# Current Project State
  Working, with issues see ToDo

## ToDo
Document Software

Sort Licencing issues and attribution

Sort DMA 8/16 bit DMA to display

Add Alaw Audio encoding

Add better amp for Microphone

Add Support for JogWheel for destination selection/modes





