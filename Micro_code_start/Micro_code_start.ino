#include <ATtinySerialOut.h>

int ledr = 1; //PB1 indicator red led
int ledg = 3; //PB6 indicator green led
int ledb = 4; //PB3 indicator blue led

int batt_level_pin = A5; //PA7 battery level
int batt_level = 0; // variable to save analog value

// the setup routine runs once when you press reset:
void setup() 
{                
  // initialize the digital pin as an output.
  //Serial.begin(115200);
  pinMode(ledr, OUTPUT); //LED on board
  pinMode(ledb, OUTPUT);
  pinMode(ledg, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() 
{
  batt_level = analogRead(batt_level_pin);
  delay(10);
  analogWrite(ledb, batt_level);
  delay(10);
  
  //Serial.print("Estamos probando");
//  digitalWrite(ledr, HIGH);
//  delay(250);              
//  digitalWrite(ledr, LOW); 
//  delay(250); 
//  digitalWrite(ledb, HIGH);
//  delay(250);              
//  digitalWrite(ledb, LOW); 
//  delay(250);
//  digitalWrite(ledg, HIGH);
//  delay(250);              
//  digitalWrite(ledg, LOW); 
//  delay(250);

//  for (int i = 0; i <= 255; i++) 
//  {
//    analogWrite(ledb, i);
//    delay(10);
//  }   
       
}
