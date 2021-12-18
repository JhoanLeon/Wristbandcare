#include <Wire.h> // librería para el manejo del protocolo I2C
#include <SPI.h> // librería para el manejo del protocolo SPI
#include <SparkFunHTU21D.h> // librería para el manejo del sensor HTU21D

/*
 * Protocolo para envío de datos a la aplicación
 * 5 Variables a enviar: Temperatura, Humedad, Radiación, Ritmo Cardíaco y Voltaje de Batería
 * Temperatura: "TVALOR, " 
 * Humedad: "HVALOR, "
 * Radiación: "RVALOR, "
 * Ritmo cardiaco: "BVALOR, "
 * Voltaje Batería: "VVALOR, " 
 */

// pines de los led indicadores de nivel de batería
const int ledr_pin = 1; //PB1 indicator red led
const int ledb_pin = 4; //PB3 indicator blue led

// pin y variable para la medición de batería
const int batt_level_pin = A5; //PA7 battery level
int batt_level = 0; //variable to save analog value
float batt_voltaje = 0; //voltaje de la batería
const float constante_divisor = 0.82397; // 220kohm/(220kohm + 47kohm)

// pin y variables para el sensor de radiación ML8511
const int ml8511_sensor_pin = A9; //PA3 output from uv sensor
int ml8511_level = 0;
float ml8511_outputVoltage = 0;
float uvIntensity = 0; 

// crea instancia y variables del sensor HTU21D
HTU21D htu21d_sensor;
float humedad = 0;
float temperatura = 0;

// pin y variables para el sensor de pulso cardiaco 
const int pulse_sensor_pin = A3;                   // Sensor de Pulso conectado al puerto PB6
int pulso = 0;                      // variable para guardar lectura de voltaje del sensor de pulso
// Estas variables son volatiles porque son usadas durante la rutina de interrupcion
volatile int BPM;                   // Pulsaciones por minuto
volatile int Signal;                // Entrada de datos del sensor de pulsos
volatile int IBI = 600;             // tiempo entre pulsaciones
volatile boolean Pulse = false;     // Verdadero cuando la onda de pulsos es alta, falso cuando es Baja
volatile boolean QS = false;        // Verdadero cuando el Arduino busca un pulso del Corazon

float sum = 0; // variable utilizada para la función custom_delay()


void custom_delay(int tiempo_espera) // parametro es tiempo de espera en segundos
{
  for(int t=0; t < tiempo_espera; t++)
  {
    sum = 0;
    for(int i=0; i < 332; i++)
    {
      for (int j=0; j < 332; j++)
      {
        // solo esperar
        sum = sum + 1;
      }
    }
  }
}


void setup() 
{
  Serial.begin(9600);                // Puerto serial configurado a 9600 Baudios para comunicacion con Bluetooth HM-10
  
  interruptSetup();                  // Configura la interrupcion para leer el sensor de pulsos cada 2ms                

  // definición de pines de salida
  pinMode(ledr_pin, OUTPUT);
  pinMode(ledb_pin, OUTPUT);

  // definición de pines de entrada
  pinMode(batt_level_pin, INPUT);
  pinMode(ml8511_sensor_pin, INPUT);
  pinMode(pulse_sensor_pin, INPUT);

  htu21d_sensor.begin();
  
  digitalWrite(ledr_pin, HIGH);
  digitalWrite(ledb_pin, HIGH);
  custom_delay(1);
  digitalWrite(ledr_pin, LOW);
  digitalWrite(ledb_pin, LOW);
  custom_delay(1);
  
  Serial.println("Dispositivo inicializado");
}


void loop() 
{ 
  // medición del voltaje de batería
  batt_level = analogRead(batt_level_pin);
  batt_voltaje = (3.3 / 1023) * batt_level;
  batt_voltaje = batt_voltaje / constante_divisor;

  // indicador de nivel de batería con los leds en el dispositivo 
  if (batt_voltaje < 3.4)
  {
    digitalWrite(ledr_pin, HIGH);
    digitalWrite(ledb_pin, LOW);
  }
  else
  {
    digitalWrite(ledr_pin, LOW);
    digitalWrite(ledb_pin, HIGH);
  }


  // medición del sensor de radiación UV
  ml8511_level = averageAnalogRead(ml8511_sensor_pin);

  ml8511_outputVoltage = (3.3 / 1023) * ml8511_level; 

  uvIntensity = mapfloat(ml8511_outputVoltage, 0.99, 2.8, 0.0, 15.0); //convierte el voltaje medido a intensidad UV en (mW/cm^2)

  
  // medición del sensor de humedad y temperatura
  humedad = htu21d_sensor.readHumidity(); // valor en porcentaje de humedad relativa
  temperatura = htu21d_sensor.readTemperature(); // valor en grados celsius


  // medición del sensor de pulso cardíaco
  pulso = analogRead(pulse_sensor_pin); //lee el valor del pulsometro conectado al puerto Analogo A0
 
  if (QS)                               //bandera del Quantified Self es verdadera cuando el Arduino busca un pulso del corazon
  {                                 
    QS = false;                         //reset a la bandera del Quantified Self 
  }


  // arma y publica el mensaje de las mediciones a la aplicación bluetooth  
  Serial.print("T" + String(temperatura) + ",");
  Serial.print("H" + String(humedad) + ",");
  Serial.print("R" + String(uvIntensity) + ",");
  Serial.print("B" + String(BPM) + ",");
  Serial.print("V" + String(batt_voltaje) + ",");

  custom_delay(1); // delay de 1 seg para envío de información
  
  // REVISAR ALIMENTACION DE MICRO A 3.3V
  // VERIFICAR CONEXIÓN ENTRE LA APLICACION Y LA MANILLA PARA LUEGO DETECTAR CUANDO ESTÁN DESCONECTADOS
       
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
