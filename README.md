# XaserImage
Converts an image to GCODE for laser cutting, uses S0..1 for power setting.

Mostly working app (Still in test) for my home built laser cutter and smoothieboard, or any GCODE 
that uses The S parameter of a G1 move to set the laser output power

scale       - sets the scale of the image , 1 pixel = 1 mm
upper/lower - 0-100 minimum and maximum power to use, lower would be the least amount to fire the laser
              upper would be the maximum you want to use, converted to S0 to S1
invert      - switch between black being all on, or white being all on

RGB image is converted to HSV then the V is used converted from 0..255 to lower..upper then to 0..1

Supports png/jpg/bmp/gif

MFC + Visual Studio 2015, though should work with earlier versions just change the toolset

TODO:
  conversion is linear
  conversion works on image in buffer versus always from source, clone loaded image
  
