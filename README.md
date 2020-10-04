# Wristband LilyGo

## Download the files from github

Go to this repository and download it as a .zip file so you have everything
https://github.com/Xinyuan-LilyGO/LilyGo-T-Wristband 

## Board version 
We are working with the LSM9DS1 accelerometer chip and are following this version of the tutorial. 

https://github.com/Xinyuan-LilyGO/LilyGo-T-Wristband/blob/master/examples/T-Wristband-LSM9DS1/README.MD

![](./images/version.jpg) *Note the difference to confirm which version you are using*

## Drivers

The CP21 driver: if you have it, something like this would exist in your machine: 

	Macintosh HD/Library/Extensions/SiLabsUSBDriver.kext

**The CH340 driver**

Open terminal and navigate to: 

	cd /Library/Extensions

Then list the usb drivers

	ls | grep usb

If you see *usb.kext* or *usbserial.kext* it means you have TCH340 driver files installed. If not: go through this tutorial to install (uninstalling is also described here): 

https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all#mac-osx

### Install board files for ESP32 (if necessary)
 
 Check if you find *ESP32 Dev Module* under 

> Tools > Boards. 

![What you should see if all is installed](./images/board.png)
 
If not: add this url to your Additional Board Manager URLS. To do that, in the Arduino IDO go to: 

> File > Preferences

And paste the following link into the box that says "Additional Board Manager URLs:
	
	https://dl.espressif.com/dl/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
	
![adding an URL to the board manager](./images/boardsmanager.png)

Then go to 

> Tools > Board > Boards Manager ...

and search for *ESP32*. A library by Espressif systems should show up and allow you to install or update. 

Restart Arduino and check if you can find the board now

## Install the libraries dependencies

Find the folder called *libdeps*, and copy paste its contents to your Arduino Library folder. Mine is in: 

	Macintosh HD > Users > [username] > Documents > Arduino > sketches > libraries

![](./images/libraries_location.png)*Path to arduino libraries*


* **pcf8563.h** controls the clock chip
* **SparkFun\_LSM9DS1\_Arduino\_Library**  to control the sensor
* **TFT\_eSPI** library to put the graphics on the display (using SPI communication protocol)
* **WiFiManager** allows you to program the board via wifi (not explained here).

If you are not sure where to find these, alternatively you can compress each library folder into its own .zip file. 

> Sketch > Include Library > Add .ZIP library

and then going into the library manager via 

> Sketch > Include Library > Library manager

Where you search for the library and click to install it. 

![](./images/tft-espi_lib.png) *Installing a library with the library manager*

### User Definitions for the TFT_eSPI lib

Open the file called *User\_Setup\_Select.h* that is inside the TFT_eSPI library. 

Locate line 20 that says:

	// Only ONE line below should be uncommented.  Add extra lines and files as needed.

Make sure all the lines starting with **#include** are commented out, except this one: 

	#include <User_Setups/Setup26_TTGO_T_Wristband.h>  // Setup file for ESP32 and TTGO T-Wristband ST7735 SPI bus TFT

![](./images/usersetup_TFTeSPI.png) *The file should look like this as a result*

Save the file and close. 

## Programming the board

Take the device out of the bracelet and open it up: with the screen facing up, open it on the right side, where the touch button is.

![](./images/openingup.jpg)

With a fine tweezer, you can lift out the board by sticking a leg under it in the tiny notch on the top right

![](./images/liftout.jpg)

Connect the programmer cable to the board by opening the latch, sliding in the flexible ribbon wire, and closing the latch. Check the which port pops up in the Arduino IDE, and select the one that becomes active when you plug the cables into your USB port. 

![](./images/cables.jpg)

Watch this vid to see how https://www.youtube.com/watch?v=fNtdHYDsplc 

**_NOTE_**: Look carefully how the device goes into the casing, if you put it upside down it will get stuck and you'll likely tear the cables loose to take it out. 

### Upload an example sketch

Compile and try uploading one of the example sketches, and see if the screen is programmed correctly. 

**Simple examples**

- *helloworld* : wakes up the screen when you press the button
- *blink*: turn the on-board LED on/off with the button
- *touchpad_led*: turn on LED with feedback in serial
-*animation*: displays text and a bouncing ball

**More advanced examples**

- *dot* : features three mini programs you can loop through with the touch button (incl accelerometer use)
- 


### Upload via Wifi

>>> hmmm I don't know if this is right. It works once, but when you overwrite it it cannot connect anymore soooooo

Open the BasicOTA sketch and find line 6 and 7 where you are asked to put in your network's name and password:

	const char* ssid = "INSERT NETWORK NAME";
	const char* password = "INSERT PASSWORD"; 

Put in your details, and upload the sketch. Open the serial monitor. If all goes well, the IP address of the device will be displayed in the serial monitor. 

![](./images/serial_ipaddress.png)

Close the sketch and open another example that uses the display. Then go to: 

> Tools > Port > Network Ports

![](./images/networkport.png)

And you will see a new port listed there, e.g. *ESP32 XXXXX at 192.168.xxx.xxx* Select that port and upload the sketch. It will now upload via the network instead of the serial. Handy if you prefer to keep the board in the casing, but will drain the battery since it's not charging at the same time.

Source: https://www.youtube.com/watch?v=GR2ZXyPaqMo

## Options to explore

### On-board stuff to play with

| Name              | Pin    |
| ----------------- | ------ |
| Red LED        | 4 |
| Display backlight        | 27 |
| Touchpad          | 33     |
| Touchpad Power (pullup)   | 25     |

See also example sketches

### Orientation

You can set the orientation with increments of 90 degrees, where 0 = portrait, 1 = landscape and so on. The 0,0 point is on the top left. 

	  tft.setRotation(1);   	// where 0 = portrait, 1 = landscape 

### Dimensions

* short edge = 80 pixels
* long edge = 160 pixels

### Images

To display an image you have to convert an image to a UTFT format required into special code that can be displayed. 

First crop your image to a 160 x 80 px image. 

Go to this website to upload and convert it: 
http://www.rinkydinkelectronics.com/t_imageconverter565.php

then copy the code into the *image.h* tab of your sketch. For an example, try the **Image** example

![](./images/image.jpg)*Display an image*

**Reduce brightness of the display**

put this in your setup :

	  //   Reduce the brightness for longer battery life
	   ledcSetup(0, 1000, 8);
	   ledcAttachPin(TFT_BL, 0);
	   ledcWrite(0, 10);

**Lower MCU frequency**

to reduce current consumption & heat:

	// Lower MCU frequency can effectively reduce current 	consumption and heat
	setCpuFrequencyMhz(80);

### Color 

You can define colors in the *TFT_eSPI.h* file, which you can now find in the library folder. These are already specified: 

* TFT_BLACK
* TFT_NAVY
* TFT_DARKGREEN
* TFT_DARKCYAN
* TFT_MAROON
* TFT_PURPLE
* TFT_OLIVE
* TFT_LIGHTGREY
* TFT_DARKGREY
* TFT_BLUE
* TFT_GREEN
* TFT_CYAN
* TFT_RED
* TFT_MAGENTA
* TFT_YELLOW
* TFT_WHITE
* TFT_ORANGE
* TFT_GREENYELLOW
* TFT_PINK
* TFT_BROWN
* TFT_GOLD
* TFT_SILVER
* TFT_SKYBLUE
* TFT_VIOLET


**PIN connections**

| Name              | Pin    |
| ----------------- | ------ |
| Red LED        | 4 |
| Display backlight        | 27 |
| Touchpad          | 33     |
| Touchpad Power (pullup)   | 25     |
| RTC Interrupt     | 34     |
| Battery ADC       | 35     |
| I2C_SDA           | 21     |
| I2C_SCL           | 22     |
| CHARGE Indication | 32     |
| IMU INT1    sensor      | 38     |
| IMU INT2   sensor       | 39     |
| IMU INTM   sensor       | 37     |
| IMU RDY   sensor        | 36     |
| TFT Driver        | ST7735 |
| TFT_MISO          | N/A    |
| TFT_MOSI          | 19     |
| TFT_SCLK          | 18     |
| TFT_CS            | 5      |
| TFT_DC            | 23     |
| TFT_RST           | 26     |




##To Find Out

### How to add new fonts
### List devices/networks
### Send each other coordinates?

