# Hike Elevation Gain System

## :construction: Still! UNDER CONSTRUCTION :construction: 
## Description
This tool uses a microcomputer connected to a GPS receiver and a barometric pressure sensor to record timestamped hike latitude, longitude, and both GPS and barometric elevation data, and a macro-enabled Excel workbook that processes the recorded data to calculate an accurate elevation gain value for a hike.  The workbook VBA macros can also generate a Keyhole Markup Language (.kml) file that can be used to plot the hike route using mapping software, and calculate the total distance of the hike and the distance from the start to various landmarks along the way.

I developed this tool mainly due to frustration with the wide range of elevation gains reported by various sources, including MapMyHike, Google Maps, Google Earth, and Global Mapper.  Google Maps and Google Earth seemed to report more elevation gain than MapMyHike, and Google Earth and Global Mapper (both of which use, or can be configured to use, the Shuttle Radar Topography Mission (SRTM) terrain elevation data) seemed to be in some agreement.

The tool is a system comprising a recording component and a processing component.  The recording component uses an [Arduino](https://www.arduino.cc/en/Guide/Introduction) (or [Arduino-like](http://wiki.microduinoinc.com/Microduino_Series)) hardware platform that incorporates a GPS receiver such as a u-Blox NEO-6M or NEO-M8N to measure and record GPS time, latitude, longitude, and elevation, and a Bosch BMP180/BMP280 temperature and pressure sensor to measure and record the barometric pressure.  For information on the Arduino platform, visit the [Arduino website](https://www.arduino.cc).  For information on the u-Blox GPS receivers, visit the [u-Blox Positioning Chips and Modules website](https://www.u-blox.com/en/positioning-chips-and-modules).  For information on the Bosch pressure sensors, visit the [Bosch Sensortec website](https://www.bosch-sensortec.com/bst/products/environmental/barometric_pressure_sensors/overview_barometricpressuresensors).

The processing component consists of a macro-enabled Excel workbook that processes the recorded data.  There are four main steps in the elevation data processing:
* Correcting the barometric altimeter data, using altimeter setting data from a local airport.  This is what aircraft pilots use to ensure that they have the correct altitude information for the local area;
* Further correcting the barometric altimeter data, using terrain elevation data, which helps remove any constant bias in the barometric elevation data;
* Filtering (smoothing) and plotting the elevation data; and 
* Computing hike elevation gain/loss statistics.

More specifically, processing the hike elevation data includes the following steps:
* Compute some basic statistics of the hike, which include:
  * Start and end time
  * Average latitude and longitude
* Get the Meteorological Terminal Aviation Routine Weather Report (METAR) data for the airport nearest the hike site from before the beginning of the hike until after the end of the hike, using an [API](https://www.aviationweather.gov/help/webservice?page=metarjson) from [NOAA's Aviation Weather Center](https://aviationweather.gov/metar) that returns data in JSON (JavaScript Object Notation) format.  This gives us the local barometric altimeter setting that aircraft pilots use to set their altimeters to indicate the correct altitude, regardless of the local barometric pressure (within reasonable limits).
* Interpolate the METAR altimeter setting data between METAR sample times for each hike data recording sample time.
* Use the interpolated altimeter setting data to correct the measured barometric-pressure-based elevation data.
* Get the Google Maps elevation data corresponding to the latitude/longitude coordinates, using a [Google Maps API](https://developers.google.com/maps/) that returns data in JSON format.  This gives us (I think) the SRTM elevation data.
* Further correct the recorded barometric elevation data by applying a 1st order correction based on the Google Maps elevation data, to correct any constant bias in the barometric pressure data.  This is done using the Excel Solver add-in to calculate a constant barometric pressure offset value that minimizes the sum of the squared differences between the barometric elevation value and the Google Maps elevation value at each hike data recording sample time.
* Filter the GPS, barometric, and Google Maps elevation data using an [infinite-impulse-response (IIR) filter](https://en.wikipedia.org/wiki/Infinite_impulse_response), the development of which has been facilitated by a [tool](http://www-users.cs.york.ac.uk/~fisher/mkfilter/trad.html) developed by Tony Fisher (who is, unfortunately, deceased).
* Compute and display the hike elevation gain and loss from GPS elevation, barometric elevation, and Google Maps elevation.
* Plot (graph) the data.

My initial findings, based on data from 4-5 hikes, are that the barometric elevation data (once calibrated) are probably more realistic and less "noisy" than the measured GPS elevation data and the Google Maps elevation data.  An additional finding, based on experiments with a counterfeit of a u-Blox NEO-6M receiver, is that some GPS receivers (such as the genuine u-Blox receiver) provide much more accurate results than others (such as the Chinese counterfeit of a u-Blox NEO-6M receiver apparently manufactured by Hangzhou ZhongKe Microelectronics CO., Ltd).

## Installation
### Arduino (like) Code
1.  I've posted three versions of the Arduino (or Arduino-like) code.  These three versions are posted here as Arduino sketchbook (not sketch) folders.  All of them use [William Greiman's SdFat library, which can be obtained at this link](https://github.com/greiman/SdFat).  In addition, they use customized versions of [SlashDevin's NeoGPS library](https://github.com/SlashDevin/NeoGPS), which I placed in the libraries folder inside the Arduino sketchbook folder for each version:
    - A version that runs on a Microduino Core+ with the Microduino GPS, Motion, SD, and USBTTL modules, which requires you to install the Arduino for Microduino Integrated Development Environment (IDE) from [the Microduino download site](https://microduinoinc.com/download/).  This IDE includes libraries for the Microduino Motion (\_06_m_Module_Motion), module, which I use, and GPS module (\_06_m_Module_GPS), which I don't use, except for testing the hardware configuration.  For the GPS receiver, I use SlashDevin's NeoGPS library, with some customization in the header files.  Please refer to the NEO-6M Setup.docx document for explanation of the customization.  The Core+ gives you two hardware serial ports.
    - A version that runs on an Arduino Uno with a shield containing a Chinese counterfeit of a u-Blox NEO-6M GPS receiver, which does not implement the u-Blox protocol, and a microSD card socket, and an [Adafruit BMP280 Barometric Pressure + Temperature Sensor Breakout](https://learn.adafruit.com/adafruit-bmp280-barometric-pressure-plus-temperature-sensor-breakout) that I attached and wired onto the shield, connecting SDA on the BMP280 breakout to A4 (SDA) on the Arduino and SCL on the BMP280 breakout to A5 (SCL) on the Arduino.  For this version, I have used SlashDevin's NeoGPS library with some customization in the header files, and the Adafruit BMP280 library.  I found a Chinese-language protocol specification for the Chinese GPS module labeled as a u-blox NEO-6M at [Hangzhou Zhongke Microelectronics Co., Ltd.](http://www.icofchina.com/d/file/xiazai/2017-05-02/ea0cdd3d81eeebcc657b5dbca80925ee.pdf) and translated it to English (with some expected formatting issues) using an [online document translator](https://www.onlinedoctranslator.com/en/translate-chinese-(simplified)-to-english_zh-CN_en), which worked better than expected.  I put a copy of the translated document in the sketchbook folder for this version.  The Arduino Uno only gives you one hardware serial port, tied to the USB port, so this version uses SlashDevin's NeoSWSerial library to communicate with the GPS module.
    - A version that runs on an Arduino Uno with a [DuinoPeak shield](http://duinopeak.com/index.php?route=product/product&product_id=112) containing a u-Blox NEO-N8M GPS receiver and a microSD card socket, and an [Adafruit BMP280 Barometric Pressure + Temperature Sensor Breakout](https://learn.adafruit.com/adafruit-bmp280-barometric-pressure-plus-temperature-sensor-breakout) that I attached and wired onto the shield, connecting SDA on the BMP280 breakout to A4 (SDA) on the Arduino and SCL on the BMP280 breakout to A5 (SCL) on the Arduino.  I used SlashDevin's NeoGPS library with customized header files and the Adafruit BMP280 library for this version also.
2.  Configure your hardware according to one of the options above.
3.  Download the corresponding sketchbook folder.
4.  In the Arduino IDE, go to File... Preferences... Sketchbook location and set the sketchbook location to the location of the sketchbook folder corresponding to the hardware configuration you chose.  See the notes on Arduino library usage below for an explanation of why I do it this way.
5.  Compile and upload the sketch to your Arduino or Microduino.
#### Some notes on Arduino library usage
When I first started developing the three hardware variants of this project, I put all of the necessary libraries in C:\Users\username\My Documents\Arduino\libraries, where my sketchbook folder was C:\Users\username\My Documents\Arduino.  That was OK for the barometric pressure sensor, because I could use one library for the BMP180 and another library for the BMP280, with no customization required.  But the NeoGPS library is highly configurable via multiple header files for different protocol types (e.g., NMEA vs proprietary), variations on what NMEA sentences different receivers put out, and what information you want out of those sentences (e.g., if you don't need speed and heading, you can configure the library to not parse them, thus saving processing time and memory).  The NeoGPS library has a a src folder containing NMEA sentence processing .h and .cpp files, and subfolder under src called ublox, which contains .h and .cpp files for handling the u-blox proprietary GPS receiver protocol.  In addition, using an Arduino Uno, which only has one hardware serial port, you have one GPS serial port configuration, but with the Microduino Core+, you have a second hardware serial port that you can use, which requires a different GPSPort.h configuration in NeoGPS.  So I started out creating different versions of the various NeoGPS header files with different names for the different configurations.  This (IMO) was a really bad idea, because if I update the NeoGPS library, the Arduino library manager replaces the existing library folder with the updated library folder, thus wiping out any custom header files I created.  (Hooray for daily backups!)  Moreover, it starts to get ugly (inelegant) fast.

After some research, I learned that the Arduino IDE looks for libraries in several places (the following is Windows specific):  (1) the libraries folder in your sketchbook, e.g. C:\Users\username\My Documents\Arduino\libraries; (2) in the libraries folder of the Arduino IDE installation folder, e.g. C:\Program Files (x86)\Arduino\libraries; (3) in the folders below C:\Program Files (x86)\Arduino\hardware, e.g., C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries; and (4) in your sketch folder.  If you want to include a library in one of the first three places, you enclose the appropriate header file name in angle brackets (<>), e.g.,

`#include <SomeLib.h>`,

and if you want to include a library in your sketch (not sketchbook) folder, you enclose the header file name in double quotes, e.g.

`#include "MyLib.h"`

I'm told that if you use the latter syntax, if the Arduino IDE doesn't find the library source file in your sketch folder, it will look in the places listed above for the angle bracket syntax.

In order to be able to have several different configurations of the NeoGPS library, as well as other libraries including Adafruit_BMP280_Library or Adafruit_BMP085_Library, NeoSWSerial (if using an Arduino Uno, not needed for a Microduino Core+), PString, and SDFat, the solution I eventually settled on is to have a different sketchbook folder for each configuration, containing a libraries folder and a sketch folder.  I have a copy of the each of the required libraries that are normally stored in the Arduino sketchbook folder in the libraries folder of the hardware-configuration-specific sketchbook folders.  The main disadvantage of this approach (in addition to multiple copies of libraries that you may need to update) is that you have to remember to point the Arduino IDE to the correct sketchbook.  You can easily change the sketchbook folder that the Arduino IDE uses under File... Preferences... Sketchbook location in the Arduino IDE (I'm using version 1.8.5 right now).

Another thing I learned is that if a library has at least one subfolder containing additional source code (.h and/or .cpp) files, all of the source code files must be within a src folder inside the library folder, or the IDE won't find the source code files within the subfolder.  An exception to this src folder requirement may be if the subfolder is named "utility".  See the Arduino IDE 1.5: Library specfication, under the Layout of folders and files heading.  I quote:  

>The source code found in src folder and all its subfolders is compiled and linked in the user's sketch. Only the src folder is added to the include search path (both when compiling the sketch and the library). When the user imports a library into their sketch (from the "Tools > Import Library" menu), an #include statement will be added for all header (.h) files in the src/ directory (but not its subfolders). As a result, these header files form something of a de facto interface to your library; in general, the only header files in the root src/ folder should be those that you want to expose to the user's sketch and plan to maintain compatibility with in future versions of the library. Place internal header files in a subfolder of the src/ folder.

>For backward compatibility with Arduino 1.0.x, the library author may opt to place source code into the root folder, instead of the folder called src. In this case the 1.0 library format is applied and the source code is searched from the library root folder and the utility folder...

I hope this explanation helps someone avoid wasting the amount of time I did in making my project work and look (I hope) at least a bit elegant.

### Excel Workbook
1.  Of course, you'll need Microsoft Excel to use the Excel macro-enabled workbook.  I used Excel 2016.

2.  I was very happy to find an Excel Visual Basic for Applications (VBA) JSON parser at https://github.com/VBA-tools/VBA-JSON.  It's always nice to find some giants who are willing to let you stand on their shoulders.  I just downloaded JsonConverter.bas from [that site](https://github.com/VBA-tools/VBA-JSON) and imported it into my Visual Basic code as a module.

3.  My Excel macro-enabled workbook also uses the Solver Add-in.  If you haven't already enabled it in your copy of Excel, you'll need to do so.  In my version (2016) of Excel, that's done by going to File... Options... Add-ins... Manage Excel Add-ins... Go, and then checking the Solver box and clicking OK.  Depending on which version of Excel you have, your approach may differ.

4.  Download and open the workbook from this page.  When you open the spreadsheet in Excel, you'll probably have to OK enabling macros.

5.  I hike mainly in Arizona, Maryland, Pennsylvania, Virginia, and West Virginia, so those are the states for which I've included airports that report METAR data in the Airports tab of the workbook.  If you live somewhere else, you'll need to download the airports for your area from the [ADDS Station Table](https://www.aviationweather.gov/docs/metar/stations.txt).  Only include the stations that have a "X" in the "M" column of the table (column 63).  Just append them to the bottom of the Airports sheet below the other airports.


## Operation
### Arduino
There's not much to it.  The way it stands right now, if you plug a [well-formatted SD card](https://www.sdcard.org/downloads/formatter_4/) into the recorder, with less than 99 HikeLgxx.csv files (where xx = 0-99) on it, and apply power, it'll create a new HikeLgxx.csv file with the next available number and start recording every 2 seconds as soon as the GPS receiver starts outputting messages.
### Excel Workbook
#### Elevation Gain Calculations
This is a lot more complicated than the Arduino part, given the current state of the "art"...
1.  Ensure that you have an internet connection, because the macros have to retrieve data using the Aviation Weather Center METAR API and the Google Maps elevation data API;
2.  On the Control sheet of the Excel workbook, enter the following data:
    - Your Google Maps API key, which you can obtain from [this website](https://developers.google.com/maps/documentation/elevation/start), in cell B7;
    - Date of the hike, in YYYY-MM-DD or MM/DD/YYYY format, in cell B14.
    - As far as the other cells on the Control sheet are concerned, you should either not mess with them or they will be automagically filled in by the macros.
2.  Open the .csv file recorded by the Microduino or Arduino as an Excel spreadsheet;
3.  Delete everything in the Data sheet of the Excel workbook below the header row, which is row 1 i.e., delete everything in row 2 and below;
4.  Copy the UTC time, latitude and longitude (+/-DD.DDDDD format), GPS elevation (feet), temperature (degrees C), pressure (HPa), baro elevation (feet), number of satellites, HDOP, and VDOP data from the appropriate Hikelgxx.csv file to the Data sheet starting in Row 2, Column 1, i.e. starting in cell A2 of the Data worksheet;
5.  Press \<CTRL>\<p>  to start processing the data.
6.  Look at the results on the Data sheet, primarily in Row 2, Columns U, Y, and AC.  Also look at the two charts, Unfilt Elev and Filt Elev, to get a sense of how the three data sources (GPS elevation, baro elevation, and Google Maps elevation) match up.
#### KML File Generation
This is somewhat more simple than the elevation gain calculations:
1.  On the Control sheet of the Excel workbook, enter the file location at which you want to store your .kml file, should you choose to generate one, in cells B1-B3;
2.  Press \<CTRL>\<k> to generate the .kml file.
#### Distance Calculations
##### Total Hike Distance Calculation
Press \<CTRL>\<d> to calculate the total distance of the hike.
##### Distance to a Waypoint or Landmark
1. Enter the latitude and longitude of the waypoint in cells B27 and C27 respectively;
2. Press \<CTRL>\<q> to calculate the distance.

##  Brief Description of the Excel Workbook
### Control Tab
This is the worksheet on which you enter some parameters related to generating a Keyhole Markup Language (.kml) file, your Google Maps API keyt, the date of the hike, waypoint latitude and longitude if you want to calculate distance to a waypoint.  It's also where certain intermediate results of the elevation processing show up, and where the distances are displayed.
### Data Tab
This is the worksheet onto which you copy and paste the data from the hike log file recorded by the Arduino (like) machine.
### Airports Tab
This is the worksheet that contains the information (most importantly, the latitude and longitude) of any airports from which you want the workbook to retrieve local altimeter setting data.
### METARS Tab
This is a worksheet whose contents are automatically generated by the macros after retrieving the METAR data for the selected airport.
### GM API Rqsts Tab
This is a worksheet whose contents are automatically generated by the macros in the process of requesting elevation data from Google Maps.
### GM Elev Tab
This is a worksheet whose contents are automatically generated by the macros from the elevation data returned by the Google Maps API.
### Unfilt Elev Chart
This is an automatically generated chart showing the unfiltered barometric, GPS, and Google Maps elevation data.
### Filt Elev Chart
This is an automatically generated chart showing the filtered barometric, GPS, and Google Maps elevation data.  The filter is a digital third-order Butterworth IIR filter implemented in a macro.

