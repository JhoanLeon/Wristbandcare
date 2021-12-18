#include <Wire.h>
#include "SparkFunHTU21D.h"

//Create an instance of the object
HTU21D myHumidity;

void setup()
{
  Serial.begin(9600);
  Serial.println("HTU21D Example!");

  myHumidity.begin();
}

void loop()
{
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();

  //Serial.print("Time:");
  //Serial.print(millis());
  //Serial.print(" Temperature:");
  Serial.print(temp, 1);
  //Serial.print("C");
  Serial.print(",");
  //Serial.print(" Humidity:");
  Serial.print(humd, 1);
  //Serial.print("%");

  Serial.println();
  delay(200);
}
