#include <HX711.h>

/*
 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
 OUTDATED 
 Arduino pin 6 -> HX711 CLK
 Arduino pin 5 -> HX711 DOUT
 Arduino pin 5V -> HX711 VCC
 Arduino pin GND -> HX711 GND 
*/

#include "HX711.h"

HX711 scale(30, 32); // data pin, clock pin

float calibration_factor = -23.00; // this calibration factor is adjusted according to my load cell
float units;
float ounces;
float newtons;
float time;

void setup() {
  Serial.begin(9600);
//  Serial.println("HX711 calibration sketch");
//  Serial.println("Remove all weight from scale");
//  Serial.println("After readings begin, place known weight on scale");
//  Serial.println("Press + or a to increase calibration factor");
//  Serial.println("Press - or z to decrease calibration factor");

  scale.set_scale();
  scale.tare();  //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
//  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
//  Serial.println(zero_factor);
}

void loop() {
  scale.set_scale(calibration_factor); //Adjust to this calibration factor

//  Serial.print("Reading: ");

  time = millis();
// units are in grams, python code prints Newtons, so units * 0.009807
  units = scale.get_units(), 10;
  if (units < 0)
  {
    units = 0.00;
  }
  ounces = units * 0.035274;
  // newtons = units * 0.0098;
  Serial.print(time);
  Serial.print('\t');
  Serial.print(units);
//  Serial.print(units);
//  Serial.print(" grams"); 
//  Serial.print(" calibration_factor: ");
//  Serial.print(calibration_factor);
  Serial.println();

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 10;
  }
}
