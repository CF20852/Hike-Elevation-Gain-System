/*
  SD card datalogger

  This example shows how to log data from an ATGM332D GPS receiver and BMP280 barometric altimeter to an SD card.

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  adapted for ATGM332D GPS and BMP280 14 Apr-18 May 2018
  by Chip Fleming

  This code is developed for the Wanderbirds hiking club.
*/

#define recordingInterval 2  //seconds between successive samples

//#define SERIALPORT
#undef SERIALPORT

#include <PString.h>

char buffer[80];
PString dataString(buffer, sizeof(buffer));

// BEGIN SD CARD SECTION *****************************************************************************
#include <SPI.h>
#include <SdFat.h>

// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = 4;

// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "HikeLg"

// File system object.
SdFat sd;

// Log file.
SdFile file;

#define error(msg) sd.errorHalt(F(msg))
// END SD CARD SECTION *******************************************************************************

// BEGIN GPS SECTION *********************************************************************************
/*
   In NMEAGPS_cfg.h of SlashDevin’s NeoGPS library, make sure NMEAGPS_PARSE_SATELLITES, NMEAGPS_PARSE_GGA,
   NMEAGPS_PARSE_GSA, NMEAGPS_COMMA_NEEDED, and LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_TXT are #defined.
   The other NMEAGPS_PARSE_XXX should not be defined, because those messages should not be sent by the NEO-6M
   with the message configuration shown below in setup().
*/

/*In GPSfix_cfg.h of SlashDevin’s NeoGPS library, #define some parameters and comment out the definitions of others, as follows:
  //#define GPS_FIX_DATE
  #define GPS_FIX_TIME
  #define GPS_FIX_LOCATION
  //#define GPS_FIX_LOCATION_DMS
  #define GPS_FIX_ALTITUDE
  //define GPS_FIX_SPEED
  //#define GPS_FIX_VELNED
  //define GPS_FIX_HEADING
  #define GPS_FIX_SATELLITES
  #define GPS_FIX_HDOP
  #define GPS_FIX_VDOP
  //#define GPS_FIX_PDOP
  //#define GPS_FIX_LAT_ERR
  //#define GPS_FIX_LON_ERR
  //#define GPS_FIX_ALT_ERR
  //#define GPS_FIX_GEOID_HEIGHT
*/

#include <NMEAGPS.h>
#include <ublox/ubxGPS.h>

#include <GPSport.h>
#include <Streamers.h>

static NMEAGPS  nmeaGPS; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values

// END GPS SECTION ***********************************************************************************

// BEGIN BMP280 SECTION ******************************************************************************
#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;

// END BMP280 SECTION ********************************************************************************


void setup() {
  // Open serial communications and wait for port to open:
  DEBUG_PORT.begin(115200);
  while (!Serial)
    ;
  DEBUG_PORT.println(F("Starting up SD card, ATGM332D, and BMP280....."));

  // BEGIN SD CARD SECTION *****************************************************************************
#if defined (SERIALPORT)
  DEBUG_PORT.println(F("Initializing log file name and SD card..."));
#endif
  // set up file name
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";

  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }

  // Find an unused file name.
  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      error("Can't create file name");
    }
  }
  if (!file.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {
    error("file.open");
  }
  else {
    DEBUG_PORT.print(fileName);
    DEBUG_PORT.println(F(" opened OK"));
  }
  // END SD CARD SECTION *******************************************************************************

  // BEGIN GPS SECTION *********************************************************************************
  //the ATGM332D starts up at 9600 bps, so start the GPS serial port there and wait a moment
  gpsPort.begin(9600);
  delay(100);

  //tell the ATGM332D to operate its serial port at 38400 bps and wait a moment
  nmeaGPS.send_P( &gpsPort, F("PCAS01,3") );
  delay(500);

  //restart the Arduino GPS serial port at 38400 bps and wait a moment
  gpsPort.begin(38400);
  delay(100);

  //configure the ATGM332D to send only GGA and GSA messages
  nmeaGPS.send_P( &gpsPort, F("PCAS03,1,0,1,0,0,0,0,0") );

  // END GPS SECTION ***********************************************************************************

  // BEGIN BMP280 SECTION ******************************************************************************
  if (!bmp.begin()) {
    DEBUG_PORT.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  }
  else {
    DEBUG_PORT.println(F("Found a BMP280 sensor, setup is complete."));
  }
  // END BMP280 SECTION ********************************************************************************
}  // end of setup()

uint8_t lastGPSSeconds = 0;
uint32_t timer = millis();
float BMP280Temperature, BMP280Pressure, BMP280Altitude;


void loop() {
  // read data from the GPS in the 'main loop'

  while (nmeaGPS.available( gpsPort )) {
    fix = nmeaGPS.read();
    //DEBUG_PORT.print(".");
  }

  // if the inter-sample interval has elapsed, (optionally) display and record the GPS time,
  //
  if ((fix.dateTime.seconds % recordingInterval == 0) && (fix.dateTime.seconds != lastGPSSeconds)) {
    lastGPSSeconds = fix.dateTime.seconds;

    // clear the dataString
    dataString.begin();

#if defined SERIALPORT
    Serial.println();

    DEBUG_PORT.print( F("Location: ") );
    if (fix.valid.location) {
      DEBUG_PORT.print( fix.latitude(), 6 );
      DEBUG_PORT.print( ", " );
      DEBUG_PORT.print( fix.longitude(), 6 );
    }

    DEBUG_PORT.print( F(", Altitude: ") );
    if (fix.valid.altitude)
      DEBUG_PORT.print( fix.altitude_ft() );

    DEBUG_PORT.print( F(", Satellites: ") );
    if (fix.valid.satellites)
      DEBUG_PORT.print( fix.satellites );

    DEBUG_PORT.print( F(", HDOP: ") );
    if (fix.valid.hdop)
      DEBUG_PORT.print( fix.hdop );

    DEBUG_PORT.print( F(", VDOP: ") );
    if (fix.valid.vdop)
      DEBUG_PORT.print( fix.vdop );

    DEBUG_PORT.println();
#endif

    dataString.print(fix.dateTime.hours, DEC);
    dataString.print(F(":"));
    dataString.print(fix.dateTime.minutes, DEC);
    dataString.print(F(":"));
    dataString.print(fix.dateTime.seconds, DEC);
    dataString.print(F(", "));

    dataString.print(fix.latitude(), 6);
    dataString.print(F(", "));
    dataString.print(fix.longitude(), 6);
    dataString.print(F(", "));
    dataString.print(fix.altitude() * 3.28084, 1);
    dataString.print(F(", "));

    dataString.print(fix.satellites);
    dataString.print(F(", "));
    dataString.print(fix.hdop / 1000.0);
    dataString.print(F(", "));
    dataString.print(fix.vdop / 1000.0);
    dataString.print(F(", "));

    BMP280Temperature = bmp.readTemperature();
    BMP280Pressure = bmp.readPressure();
    BMP280Altitude = bmp.readAltitude(1013.25);

#if defined (SERIALPORT)
    DEBUG_PORT.print(F("Temperature = "));
    DEBUG_PORT.print(BMP280Temperature);
    DEBUG_PORT.println(F(" *C"));

    DEBUG_PORT.print(F("Pressure = "));
    DEBUG_PORT.print(BMP280Pressure);
    DEBUG_PORT.println(F(" Pa"));

    DEBUG_PORT.print(F("Altitude = "));
    DEBUG_PORT.print(BMP280Altitude * 3.28084);
    DEBUG_PORT.println(F(" feet"));
#endif

    dataString.print(BMP280Temperature, 1);
    dataString.print(F(", "));
    dataString.print(BMP280Pressure, 1);
    dataString.print(F(", "));
    dataString.print(BMP280Altitude * 3.28084, 1);

    file.println(dataString);

    // Force data to SD and update the directory entry to avoid data loss.
    if (!file.sync() || file.getWriteError()) {
#if defined (SERIALPORT)
      error("write error");
#endif
    }

    // print to the serial port too:
    //#if defined (SERIALPORT)
    DEBUG_PORT.println(dataString);
    //#endif
  } // end if

}  // end of loop()
