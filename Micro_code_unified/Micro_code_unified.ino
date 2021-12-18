#include <DigiKeyboard.h> //library for debbuging
#include <Wire.h>
#include <SparkFunHTU21D.h>
#include <SPI.h>

int ledr = 1; //PB1 indicator red led
int ledg = 3; //PB6 indicator green led
int ledb = 4; //PB3 indicator blue led

int batt_level_pin = A5; //PA7 battery level
int batt_level = 0; // variable to save analog value

int ml8511_sensor_pin = A9; //PA3 output from uv sensor
int ml8511_level = 0;
float ml8511_outputVoltage = 0;
float uvIntensity = 0; 

//Create an instance of the object
HTU21D myHumidity;


// the setup routine runs once when you press reset:
void setup() 
{                
  pinMode(ledr, OUTPUT); //LED on board
  pinMode(batt_level_pin, INPUT);
  pinMode(ml8511_sensor_pin, INPUT);
  
  myHumidity.begin();
  //pinMode(ledb, OUTPUT);
  //pinMode(ledg, OUTPUT);
  
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.println("Code for Wristbancare");
}


void loop() 
{
  // battery voltage level
  batt_level = analogRead(batt_level_pin);
  delay(5);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.print("Battery level ");
  DigiKeyboard.println(batt_level);

  // uv sensor
  ml8511_level = averageAnalogRead(ml8511_sensor_pin);
  delay(5);

  ml8511_outputVoltage = (3.3 / 675) * ml8511_level;

  uvIntensity = mapfloat(ml8511_outputVoltage, 0.99, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level

  DigiKeyboard.sendKeyStroke(0);
  //DigiKeyboard.print("Output uv voltage ");
  //DigiKeyboard.print(String(ml8511_outputVoltage));            

  DigiKeyboard.print("UV intensity ");
  DigiKeyboard.println(uvIntensity);   

  // humedity and temperature sensor
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();

  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.print("Temperature ");
  DigiKeyboard.print(temp, 1);
  DigiKeyboard.println("C");
  DigiKeyboard.print("Humidity ");
  DigiKeyboard.print(humd, 1);
  DigiKeyboard.println("%");

  DigiKeyboard.delay(500);  
       
}


//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 

  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);  
}


//The Arduino Map function but for floats
//From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
