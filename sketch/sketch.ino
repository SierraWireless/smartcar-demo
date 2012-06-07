/*******************************************************************************
 * Copyright (c) 2012 Sierra Wireless and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     Sierra Wireless - initial API and implementation
 *******************************************************************************/

/*******************************************************************************
 * This sketch aims at simulating a simplified car.
 *
 * The "car" has 10 different itineraries stored in its internal memory. 
 * The user can use the DeuLigne connected to the board to select what road
 * he/she wants to follow. When the trip has begun, the "car" will start sending
 * simulated CANBus-like frames to the Serial output.
 *
 * Format of these frames is: Lat-Lon-Speed-MPG
 *   - Lat: the latitude of the car. Example: 41.9202
 *   - Lon: the longitude of the car. Example: 1.4281
 *   - Speed: the average speed since the previous sampling, in mph. Example: 43
 *   - MPG: the average gas consumption since the previous sampling, in mpg. Example: 53
 *
 * -= Pin usage =- 
 *
 *   TBD
 *
 *
 *******************************************************************************/

#include "ColorLCDShield.h"
#include "Stream.h"
#include "trips.h"
#include "bitmaps.h"

#define MAX_SPEED 100.0

LCDShield lcd;
int buttons[3] = {
  3, 4, 5};  // S1 = 3, S2 = 4, S3 = 5
#define DEFAULT_CONTRAST 40

float simulatedSpeed = 50.0 ; // varies between 0 and 100 mph 
int selectedTrip ; // a number between 1 and 10 indicating the current trip
int tripPosition = 0 ; // the index in the trip array (incremented after each sampling)
unsigned long lastSampleTaken = -1 ; // when was the last sample sent to the CANbus

char buffer[30] ;

void setup()
{
  randomSeed(analogRead(6));

  // start serial port at 9600 bps:
  Serial.begin(9600);

  // init LCD
  lcd.init(EPSON);  // Initialize the LCD, try using PHILLIPS if it's not working
  lcd.contrast(DEFAULT_CONTRAST);  // Initialize contrast
  lcd.clear(WHITE);  // Set background to white
  for (int i=0; i<3; i++)
  {
    pinMode(buttons[i], INPUT);  // Set buttons as inputs
    digitalWrite(buttons[i], HIGH);  // Activate internal pull-up
  }
  drawIcons();

  // Ask the user to select a trip
  selectedTrip = tripSelection();
}

void loop()
{
  // adjust speed 
  /*
  int key = lcd.get_key();
   if(key == 1 && simulatedSpeed < MAX_SPEED)
   simulatedSpeed += 0.05;
   else if(key == 2 && simulatedSpeed > 10)
   simulatedSpeed -= 0.05;
   */
  int c = Serial.read() ;
  if (c == '+' || c == '-') {
    if (c == '+')
      simulatedSpeed += 0.25;
    else   
      simulatedSpeed -= 0.25;
  }

  if (!digitalRead(buttons[0]))
    simulatedSpeed -= 0.25;
  if (!digitalRead(buttons[2]))
    simulatedSpeed += 0.25;

  if (simulatedSpeed < 0) simulatedSpeed = 0 ;
  if (simulatedSpeed > MAX_SPEED) simulatedSpeed = MAX_SPEED ;

  // calculate an approximated mpg
  float estimatedMpg = (MAX_SPEED - simulatedSpeed) * .5 + 20 + random(10)/10. - 5. ;

  dtostrf(simulatedSpeed, 2, 2, buffer) ;
  lcd.setStr(buffer , 25, 60, BLUE, WHITE);

  dtostrf(estimatedMpg, 2, 2, buffer) ;
  lcd.setStr(buffer, 65, 60, BLUE, WHITE);

  if(millis() - lastSampleTaken > ((MAX_SPEED - simulatedSpeed + 1) * 100))
  {
    writeCanFrame(pgm_read_float(&(trips[selectedTrip][tripPosition])), pgm_read_float(&(trips[selectedTrip][tripPosition+1])), simulatedSpeed, estimatedMpg) ;
    lastSampleTaken = millis() ;
    tripPosition+=2 ;
  }
}

void writeCanFrame(float lat, float lon, float spd, float mpg) {
  dtostrf(lat, 2, 5, buffer) ;
  Serial.print(buffer) ; 
  Serial.print('#') ;
  dtostrf(lon, 2, 5, buffer) ;
  Serial.print(buffer) ; 
  Serial.print('#') ;
  Serial.print(spd) ; 
  Serial.print('#') ;
  Serial.print(mpg) ; 
  Serial.println() ;
}

int tripSelection() {
  Serial.println("Select a trip (0-9)");
  int i = Serial.read();
  while(i < 48 || i > 57) {
    i = Serial.read();
  }
  i = i - 48;
  Serial.print("Selection: ");
  strcpy_P(buffer, (char*)pgm_read_word(&(trips_names[i])));
  Serial.println(buffer);
  Serial.flush() ;
  return i ;
}

void drawIcons() {
  short color1 = 0 ;
  short color2 = 0 ;
  for(int x= 0 ; x < 32; x++) {
    for(int y= 0 ; y < 32; y++) {
      color1 = pgm_read_word(&speedometer_bmp[x * 32 + y]);
      color2 = pgm_read_word(&gas_bmp[x * 32 + y]) ;
      lcd.setPixel(color1, 20 + y, 15 + x);
      lcd.setPixel(color2, 60 + y, 15 + x);
    }
  }    
}


// this function will return the number of bytes currently free in RAM
// written by David A. Mellis
// based on code by Rob Faludi http://www.faludi.com
int availableMemory() {
  int size = 1024; // Use 2048 with ATmega328
  byte *buf;

  while ((buf = (byte *) malloc(--size)) == NULL)
    ;

  free(buf);

  return size;
}


















