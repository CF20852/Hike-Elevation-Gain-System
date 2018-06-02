#ifndef GPSport_h
#define GPSport_h

//  Copyright (C) 2014-2017, SlashDevin
//
//  This file is part of NeoGPS
//
//  NeoGPS is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  NeoGPS is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with NeoGPS.  If not, see <http://www.gnu.org/licenses/>.

//-----------------------------------------------------------
//  There are 2 ways this file can be used:
//
//  I.  AS IS, which tries to *guess* which port a beginner should use.
//      If you include <SoftwareSerial.h>, <NeoSWSerial.h>, <AltSoftSerial.h>, 
//      <NeoICSerial.h>  *OR*  <NeoHWSerial.h> before this file (in the INO) 
//      or on line 152 below, one of those ports will be used for the GPS.  
//      This file cannot always guess what you want, so you may need to use it 
//      in the 2nd way...
//
//  II. *REPLACE EVERYTHING* in this file to *specify* which port and 
//      which library you want to use for that port.  Just declare the
//      port here.  You must declare three things: 
//
//         1) the "gpsPort" variable (used by all example programs),
//         2) the string for the GPS_PORT_NAME (displayed by all example programs), and
//         3) the DEBUG_PORT to use for Serial Monitor print messages (usually Serial).
//
//      The examples will display the device you have selected during setup():
//
//         DEBUG_PORT.println( F("Looking for GPS device on " GPS_PORT_NAME) );
//
//      Choosing the serial port connection for your GPS device is probably
//      the most important design decision, because it has a tremendous impact
//      on the processor load.  Here are the possible configurations, 
//      from BEST to WORST:
//
// ---  The BEST choice is to use a HardwareSerial port. You could use Serial 
//      on any board, but you will have to disconnect the Arduino RX pin 0 
//      from the GPS TX pin to upload a new sketch over USB.  This is a very 
//      reliable connection:
//
#define gpsPort Serial1
#define GPS_PORT_NAME "Serial1"
#define DEBUG_PORT Serial

#endif
