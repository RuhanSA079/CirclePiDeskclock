# CirclePiDeskclock
A baremetal C++ application using Circle SDK for Raspberry Pi (https://github.com/rsta2/circle) 
to drive a ST7735 LCD and display time from a DS1307 RTC. 

# Note
This is a code experiment, and old hardware (Raspberry Pi Zero) was used to prove a baremetal concept.  
I will further develop this, as soon as I have some free time.

# Installation
Please use the latest source version of rsta2/circle.  
Include this code into a folder inside the "app" folder on the Circle SDK folder.  

Compile with make.  

# Hardware used
- A ST7735 display (Must have been the green-tab - Got from my parts bin)  
- DS1307 or DS3231 (Used a DS3231 - From parts bin)  
- VEML7700 (Code work in progress - Bought new)  

# Wiring
Wire-up peripherals accordingly: (BCM pins) -- Your hardware may differ!  
RTC and VEML7700:  
GPIO2 -> SDA  
GPIO3 -> SCL  
  
ST7735 LCD:  
GPIO22 -> DC  
GPIO27 -> RST  
GPIO18 -> BL (PWM pin driving a MOSFET on display)  
GPIO8  -> CS  
GPIO10 -> DATA/SDA  
GPIO11 -> CLK/SCL  

The whole setup has been running from the 3.3V & GND rail on the Pi.

# Idea
The idea was to create a nice-looking, desk/nightstand digital clock, that will use WiFi to sync its time with NTP, RTC and display the time,  
dim the display accordingly to the amount of light input.  

Code is very early in development, so basic code implementation have been done to prove the concept.  

Please use this code at your own risk.
