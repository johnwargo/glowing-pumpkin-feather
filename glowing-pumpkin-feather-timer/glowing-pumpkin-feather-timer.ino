/**************************************************
   Glowing Pumpkin Project w/Timer (Feather)

   By John M. Wargo
   September, 2017

  Components & Tutorials
  ======================
  Feather HUZZAH: https://www.adafruit.com/product/2821
  https://learn.adafruit.com/adafruit-feather-huzzah-esp8266

  Adalogger FeatherWing: https://www.adafruit.com/product/2922
  https://learn.adafruit.com/adafruit-adalogger-featherwing

  NeoPixel FeatherWing: https://www.adafruit.com/product/2945
  https://learn.adafruit.com/adafruit-neopixel-featherwing
 **************************************************/

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <RTClib.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

//#include <HttpClient.h>
//#include <ArduinoJson.h>
//#include <WiFi101.h>

//Local libraries
#include "constants.h"
#include "wi-fi-config.h"

//============================================================
// USER CONFIGURABLE OPTIONS
//============================================================
//Change the following options based on your specific needs,
//hardware configuration, and physical location.

//Set the time zone using the time zones listed in constants.h.
//You'll have to manually switch the device from standard time to daylight
//time during the summer if you live in an area that observes daylight time.
const int timeZone = EDT;

//Wi-Fi settings are in an external file: **wifi-config.h**
//Set your Wi-Fi SSID and password there.

//Populate the following variable with the number of rows in the slots array
//I'm doing this way because Arduino doesn't have an easy way to determine
//the size of an array at runtime, especially multidimensional arrays.
#define NUMSLOTS 2
//Use the Slots array to define when the relay(s) go on and off
//Slots array values: {onTime, offTime };
// Examples:
// Turn the relay on at 6:00 AM, turn it off 8:00 AM
// { 600, 800 },
// Turn the relay on at 5:00 PM turn it off at 11:00 PM.
// {1700, 2300}
// Turn the relay on at 5:30 AM, turn the relay off at 7:00 AM.
// {530, 700}
int slots[NUMSLOTS][2] = {
  {600, 800},
  {1900, 2100}
  //BE SURE TO UPDATE THE NUMSLOTS CONSTANT IF YOU ADD/REMOVE
  //ROWS FROM THIS ARRAY
};

/**************************************************************************
   Specify the Feather pin the NeoPixel is connected to.
   According to the guide at:
   https://learn.adafruit.com/adafruit-neopixel-uberguide/neopixel-shields
   "The NeoPixels are normally controlled from digital pin 6, but pads on
   the bottom make this reassignable. In particular, the default pin for
   Feather Huzzah ESP8266 must be moved, try pin #15!"
   As we're using the Huzzah, we'll make this 15.
 **************************************************************************/
#define PIN 15
// Specifies the number of NeoPixels that are attached to the Feather. In
// this case, we're using the NeoPixel FeatherWing, so its constant at 32
#define NUMPIXELS 32
//Specifies how many columns are in the NeoPixel array, this is used by
//the loop iterator in setColumnColor
#define NUMCOLUMNS 8
//Specifies the number of rows in the NeoPixel array, this is also used
//by the loop iterator in setColumnColor  (the array is zero-based, so the
//value here is num_rows - 1)
#define NUMROWS 4

//initialize the pixels object
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN);

// color definitions
uint32_t cBlue = pixels.Color(0, 0, 255);
uint32_t cGreen = pixels.Color(0, 255, 0);
uint32_t cNone = pixels.Color(0, 0, 0);
uint32_t cOrange = pixels.Color(255, 128, 1);
uint32_t cPurple = pixels.Color(128, 0, 128);
uint32_t cRed = pixels.Color(255, 0, 0);
uint32_t cWhite = pixels.Color(255, 255, 255);
uint32_t cYellow = pixels.Color(255, 215, 0);

////colors is an array of all of the colors
int numColors = 7;
uint32_t colors[] = {cBlue, cGreen, cOrange, cPurple, cRed, cWhite, cYellow};
//============================================================

//Used to pretty-up the sketch's output
String HASHES = "############################################";
String SLOTERRORSTR = "\nInvalid SLOT value: ";
bool wasOn;

// Realtime Clock object
RTC_PCF8523 rtc;
DateTime currentTime;

// UDP/NTP settings
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

// local port to listen for UDP packets
unsigned int localPort = 8888;
// time.nist.gov NTP server
char timeServer[] = "time.nist.gov";
// NTP time stamp is in the first 48 bytes of the message
const int NTP_PACKET_SIZE = 48;
//buffer to hold incoming and outgoing packets
byte packetBuffer[NTP_PACKET_SIZE];

void setup() {
  //Open the serial connection
  Serial.begin(115200);
  //Wait a little bit, to let the serial connection initialize
  delay(500);
  //Then write program information to the serial port
  Serial.println(HASHES);
  Serial.println("# Adafruit Feather Glowing Pumpkin w/timer #");
  Serial.println("# By John M. Wargo (www.johnwargo.com)     #");
  Serial.println(HASHES);

  // Initialize the NeoPixel library
  pixels.begin();
  // show any lights that should be on (in this case, none)
  pixels.show();

  //Initialize the random number generator with a random seed pulled from
  //analog pin #3
  randomSeed(analogRead(3));

  //==================================================
  //Check the software configuration
  //==================================================
  Serial.println("\nChecking software configuration");

  // Make sure our slot array is populated with valid times
  for (int i = 0; i < NUMSLOTS; i++) {
    //Get our time values
    int onTime = slots[i][0];
    int offTime = slots[i][1];

    // Is onTime valid?
    if (!isValidTime(onTime)) {
      //That won't work, so display an error
      Serial.print(SLOTERRORSTR);
      Serial.println(onTime);
      while (true);
    }

    // Is offTime valid?
    if (!isValidTime(offTime)) {
      //That won't work, so display an error
      Serial.print(SLOTERRORSTR);
      Serial.println(offTime);
      while (true);
    }

    //Are onTime and offTime the same?
    if (onTime == offTime) {
      //That won't work, so display an error
      Serial.print(SLOTERRORSTR);
      Serial.print("OnTime and OffTime cannot have the same value; slot #");
      Serial.println(i);
      while (true);
    }

    // Is onTime AFTER offTime?
    if (onTime > offTime) {
      //That won't work, so display an error
      Serial.print(SLOTERRORSTR);
      Serial.print("offTime preceeds onTime in slot #");
      Serial.println(i);
      // and hop into an infinite loop
      while (true);
    }
  }

  if (!rtc.begin()) {
    Serial.println("RTC board not found");
    //loop forever
    while (true);
  }

  //Has the RTC board been initialized?
  if (!rtc.initialized()) {
    //RTC hasn't been initialized, so it has no time/date value
    Serial.println("RTC is not initialized");
    //The following line sets the RTC to the date & time this sketch was compiled
    //This assumes initializing the RTC immediately after compile, which you would
    //do when you flash the firmware on the device.
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }  else  {
    Serial.println("RTC is initialized");
  }

  //Let the user know we're trying to connect to the network
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  //Start trying to connect to the local Wi-Fi network
  WiFi.begin(wifi_ssid, wifi_pass);
  //Loop until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Let them know we're connected
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //Start the udp server
  Udp.begin(localPort);
  //Then tell the time library how to check network time
  //The getNTPtime function will execute periodically
  //to keep the clock in sync.
  setSyncProvider(getNTPTime);

  //Initialize the pointer that tells the sketch whether the LEDs were
  //on or off.
  wasOn = false;

  Serial.println("Exiting setup()");
}

void loop() {
  //Are we in an **on** slot?
  if (isInOnPeriod()) {
    wasOn = true;
    //generate a random integer between 1 and 10
    //  if it's a 9 or a 10, ...
    if ((int)random(11) > 8) {
      //do that flicker thing
      flicker();
    } else {
      //otherwise, pick a random color and set each pixel individually to that color
      //with a random time delay between setting each pixel
      stepLights(colors[(int)random(1, numColors + 1)], (int)random(50, 500));
    }
  } else {
    //Use wasOn to make sure we only turn the lights off once (after they've
    //been on)
    if (wasOn) {
      //Then turn the lights off
      setColor(cNone);
      pixels.show();
      wasOn = false;
    }
    //Wait five seconds and check again
    delay(5000);
  }
}

void flicker() {
  // how many times are we going to flash?
  int flashCount = (int)random(1, 7);
  //flash the lights in white flashCount times
  //with a random duration and random delay between each flash
  for (int i = 0; i < flashCount; i++) {
    // Set all pixels to white and turn them on
    setColor(cWhite);
    // Delay for a random period of time (in milliseconds)
    delay((int)random(50, 150));
    //clear the lights (set the color to none)
    setColor(cNone);
    // Delay for a random period of time (in milliseconds)
    delay((int)random(100, 500));
  }
}

// Fill the NeoPixel array with a specific color
void setColor(uint32_t c) {
  for (uint16_t i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
  }
  pixels.show();
}

//Set a single color one column at a time across the array.
//Used to 'fade' into a new color
void stepLights(uint32_t c, int delayVal) {
  //Loop through all of the columns
  for (int i = 0; i < NUMCOLUMNS; i++) {
    //Set a column to the new color
    for (int j = 0; j < NUMROWS; j++) {
      pixels.setPixelColor(i + (j * NUMCOLUMNS), c);
    } //for j
    //Display the new row color
    pixels.show();
    //wait for the specified period of time (in milliseconds)
    delay(delayVal);
  }  //for i
}

int getTime24() {
  // Return the current time as an integer value in 24 hour format
  DateTime theTime = rtc.now();
  return (theTime.hour() * 100) + theTime.minute();
}

bool isValidTime(int timeVal) {
  //Make sure the time value is between 0 and 2359 with
  // no minute value greater than 59
  return ((timeVal > -1) && (timeVal < 2360) && (timeVal % 100 < 60));
}

bool isInOnPeriod() {
  //Loop through the active slots array to see if the relay is
  //supposed to be on. This assumes the device just powered on
  //and it doesn't know whether it's supposed to be on or not.
  int timeVal = getTime24();
  for (int i = 0; i < NUMSLOTS; i++) {
    if ((timeVal >= slots[i][0]) && (timeVal < slots[i][1])) {
      //yep, we're supposed to be on
      return true;
    }
  }
  return false;
}

// send an NTP request to the time server
void sendNTPpacket(char *address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNTPTime() {
  while (Udp.parsePacket() > 0); // discard any previously received packets
  Serial.println("Sending NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)    {
      //We got a response
      Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
      unsigned long secsSince1900, adjustedTime;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      Serial.print("Time: ");
      Serial.println(secsSince1900);
      Serial.println();

      //Determine the local time
      adjustedTime = secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      //Set the time in the RTC
      rtc.adjust(adjustedTime);
      //Return the current time to the time library
      return adjustedTime;
    }
  }
  Serial.println("No NTP Response\n");
  return 0; // return 0 if unable to get the time
}

