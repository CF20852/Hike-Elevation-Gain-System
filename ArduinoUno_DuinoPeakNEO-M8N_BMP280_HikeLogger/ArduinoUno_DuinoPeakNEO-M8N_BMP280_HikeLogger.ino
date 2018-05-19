/*
  SD card datalogger

  This example shows how to log data from a u-Blox NEO-6M GPS receiver and BMP280 barometric altimeter to an SD card.

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  adapted for GPS and BMP180 14-30 Apr 2018
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
#include "SdFat.h"

// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = 8;

// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "HikeLg"

// File system object.
SdFat sd;

// Log file.
SdFile file;

#define error(msg) sd.errorHalt(F(msg))
// END SD CARD SECTION *******************************************************************************

// BEGIN GPS SECTION *********************************************************************************
//In NMEAGPS_cfg.h of SlashDevin’s NeoGPS library, make sure NMEAGPS_PARSE_SATELLITES, NMEAGPS_PARSE_GGA,
//NMEAGPS_PARSE_GSA, NMEAGPS_COMMA_NEEDED, and LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_TXT are #defined.
//The other NMEAGPS_PARSE_XXX should not be defined, because those messages sent by the NEO-M8N
//do not provide any information that we need and GGA and GSA do not provide.

//In GPSfix_cfg.h of SlashDevin’s NeoGPS library, #define some parameters and comment out the definitions of others, as follows:
//#define GPS_FIX_DATE
//#define GPS_FIX_TIME
//#define GPS_FIX_LOCATION
////#define GPS_FIX_LOCATION_DMS
//#define GPS_FIX_ALTITUDE
////define GPS_FIX_SPEED
////#define GPS_FIX_VELNED
////define GPS_FIX_HEADING
//#define GPS_FIX_SATELLITES
//#define GPS_FIX_HDOP
//#define GPS_FIX_VDOP
////#define GPS_FIX_PDOP
////#define GPS_FIX_LAT_ERR
////#define GPS_FIX_LON_ERR
////#define GPS_FIX_ALT_ERR
////#define GPS_FIX_GEOID_HEIGHT

#include <NMEAGPS.h>
#include <ublox/ubxGPS.h>

#include <GPSport.h>
#include <Streamers.h>

static ubloxGPS gps( &gpsPort );

static NMEAGPS  nmeaGPS; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values

// END GPS SECTION ***********************************************************************************

// BEGIN BMP280 SECTION ******************************************************************************
#include <Wire.h>
#include "Adafruit_BMP280.h"

Adafruit_BMP280 bmp;
// END BMP280 SECTION ********************************************************************************

void sendUBX( const unsigned char *progmemBytes, size_t len )
//This sends a u-blox-formatted command to the NEO-M8N GPS module, and computes and appends the checksum.
{
  //write the SYNC word
  gpsPort.write( 0xB5 ); // SYNC1
  gpsPort.write( 0x62 ); // SYNC2

  //write the payload, which is in program memory and update the checksum
  uint8_t a = 0, b = 0;
  while (len-- > 0) {
    uint8_t c = pgm_read_byte( progmemBytes++ );
    a += c;
    b += a;
    gpsPort.write( c );
  }

  //write the checksum
  gpsPort.write( a ); // CHECKSUM A
  gpsPort.write( b ); // CHECKSUM B

} // sendUBX

const unsigned char ubxSetUART138400[] PROGMEM =
{ 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xC0, 0x08, 0x00, 0x00, 0x00, 0x96, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 };

void setup() {
  // Open serial communications and wait for port to open:
  DEBUG_PORT.begin(115200);
  while (!Serial)
    ;
  DEBUG_PORT.println(F("Starting up SD card, NEO-M8N, and BMP280....."));

#if defined (SERIALPORT)
  DEBUG_PORT.println(F("Initializing log file name and SD card..."));
#endif
  // BEGIN SD CARD SECTION *****************************************************************************
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
  //start the GPS module port at 9600 bps, which is the default for the u-blox NEO-M8N
  gpsPort.begin(9600);
  delay(1000);

  //tell the NEO-M8N to switch to 38.4 kbps
  sendUBX( ubxSetUART138400, sizeof(ubxSetUART138400) );
  delay(1000);

  //restart the Arduino GPS port at 38.4 kbps
  gpsPort.begin(38400);

  //Turn on NMEA messages GxGGA and GxGSA and turn everything else off
  NMEAGPS::nmea_msg_t msgType[6] =
  {NMEAGPS::NMEA_GGA, NMEAGPS::NMEA_GLL, NMEAGPS::NMEA_GSA, NMEAGPS::NMEA_GSV, NMEAGPS::NMEA_RMC, NMEAGPS::NMEA_VTG};
  uint8_t rate[6] = {1, 0, 1, 0, 0, 0};

  int maxI = (sizeof(msgType) / sizeof(NMEAGPS::nmea_msg_t));

  for (int i = 0; i < maxI; i++) {
    DEBUG_PORT.print(F("Setting message type "));
    DEBUG_PORT.print(msgType[i]);
    DEBUG_PORT.print(F(" to rate "));
    DEBUG_PORT.print(rate[i]);
    while (!ublox::configNMEA(gps, msgType[i], rate[i])) {
      DEBUG_PORT.print( '.' );
      delay( 500 );
    }
    DEBUG_PORT.println();
  }

  DEBUG_PORT.println("Completed setting the Msg Mode!");
  DEBUG_PORT.println();


  //  THIS COMMAND SETS THE NAV5 MODES using the more readable and precise method
  ublox::cfg_nav5_t navCfg;
  navCfg.apply.dyn_model = true;                       // set the mask bit indicating we're changing the dynamic model...
  navCfg.dyn_model = ublox::UBX_DYN_MODEL_PEDESTRIAN;  //and change the model to Pedestrian, for hikers
  //
  //  // the following two lines should be commented out for the NEO-M8N, for which the ded(uced) reckoning mode is reserved
  //  navCfg.apply.dr_limit = true;                        // set the mask bit indicating we're changing the ded reckoning limit...
  //  navCfg.dr_limit = 20;                                //and change the limit to 20 s, for hikers whose signals are sometimes blocked
  //
  DEBUG_PORT.print( F("Setting uBlox nav mode") );
  while (!gps.send( navCfg )) {
    DEBUG_PORT.print( '.' );
    delay( 500 );
  }
  DEBUG_PORT.println();
  DEBUG_PORT.println("Completed setting the Nav Mode!");
  DEBUG_PORT.println();
  //

  // END GPS SECTION ***********************************************************************************

  // BEGIN BMP280 SECTION ******************************************************************************
  //see if we can start the BMP280
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

  BMP280Temperature = 0;
  BMP280Pressure = 0;
  BMP280Altitude = 0;

  // read data from the GPS in the 'main loop'
  while (nmeaGPS.available( gpsPort )) {
    fix = nmeaGPS.read();
    //DEBUG_PORT.print(".");
  }

  // if the inter-sample interval has elapsed, (optionally) display and record the GPS time,
  //
  if ((fix.dateTime.seconds % recordingInterval == 0) && (fix.dateTime.seconds != lastGPSSeconds) && (fix.latitude() != 0.0)) {
    lastGPSSeconds = fix.dateTime.seconds;

    // clear the dataString
    dataString.begin();

    //print some readable GPS info, if we're debugging
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

    //add the GPS fix info to the csv data string to be stored on the microSD card
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

    //print readable BMP280 measurements, if we're debugging
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

    //add the BMP280 sensor info to the csv data string to be stored on the microSD card
    dataString.print(BMP280Temperature, 1);
    dataString.print(F(", "));
    dataString.print(BMP280Pressure, 1);
    dataString.print(F(", "));
    dataString.print(BMP280Altitude * 3.28084, 1);

    //write the data string to the microSD card
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
