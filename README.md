# cncrouter
Firmware for a simple CNC router built based on an old FashForge Creator 3D Printer
The three steppers, stepper drivers, end stops, 24V power supply, cooling fan and lighting (both directly attached to the power source) were re-used from the printer
All other components are new: Arduino Mega as controller, 16x2 LCD Display, Rotary encoder, SD card reader, relais for spindle, 775 spindle motor

The CNC router will be able to run in manual mode (control axes by rotary encoder; show position in mm; reset position to show relative distances)
The automatic mode reads a specific file from the SD card.
The format of the file is a very simple custom format ".CNC" with only a few single letter commands and numbers to be interpreted by the firmware

Next step will be to program a "slicer" (probably in SCILAB) to generate the .CNC files
