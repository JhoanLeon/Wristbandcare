#include <Wire.h>
#include <SparkFunHTU21D.h>
#include <DigiKeyboard.h> //library for debbuging

//Create an instance of the object
HTU21D myHumidity;

void setup()
{
  //DigiKeyboard.sendKeyStroke(0);
  //DigiKeyboard.println("Example code for HTU21D");      
  Serial.begin(9600);
  myHumidity.begin();
}

void loop()
{
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();

//  DigiKeyboard.sendKeyStroke(0);
//  DigiKeyboard.print("Temperature ");
//  DigiKeyboard.print(temp, 1);
//  DigiKeyboard.print("C");
//  DigiKeyboard.print("  Humidity ");
//  DigiKeyboard.print(humd, 1);
//  DigiKeyboard.println("%");
//  
//  DigiKeyboard.delay(200);
  String comd = String(temp) + " , " + String(humd);
  Serial.print(comd);

  delay(200);
}

