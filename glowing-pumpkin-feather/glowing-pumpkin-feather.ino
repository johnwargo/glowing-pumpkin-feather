/**************************************************
   Glowing Pumpkin Project (Feather Version)

   By John M. Wargo
   September, 2017
 **************************************************/

//Adafruit Feather board
// https://www.adafruit.com/product/2821

//===================
// Adafruit Libs
//===================
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

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

//From the docs: "A value indicating the type of NeoPixels that are connected.
//In most cases you can leave this off and pass just two arguments; the example
//code is just being extra descriptive. If you have a supply of classic “V1”
//Flora pixels, those require NEO_KHZ400 + NEO_RGB to be passed here."
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
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

void setup() {
  // Initialize the NeoPixel library
  pixels.begin();
  // show any lights that should be on (in this case, none)
  pixels.show();

  //Initialize the random number generator with a random seed pulled from
  //analog pin #3
  randomSeed(analogRead(3));
}

void loop() {
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

