#include <Wire.h>           // librería para el manejo del protocolo I2C
#include <SPI.h>            // librería para el manejo del protocolo SPI
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

/*
 * Funciones para el manejo de la memoria de 4MB (16384 páginas, 256 bytes por página)
 * _chip_erase();
 * word pageno; 0 - 1638
 * _read_page(pageno, page_buffer);
 * _write_page(pageno, page_buffer);  
 */


// pines de los led indicadores de nivel de batería
const int ledr_pin = 1; // PB1 indicador de batería led rojo
const int ledv_pin = 4; // PB3 indicador de batería led verde


// pin y variable para la medición de batería
const int batt_level_pin = A5;           // PA7 nivel de batería
const float constante_divisor = 0.82397; // 220kohm/(220kohm + 47kohm)
int batt_level = 0;                      // variable para guardar valor análogo de batería
float batt_voltaje = 0;                  // voltaje medido de la batería


// pin y variables para el sensor de radiación ML8511
const int ml8511_sensor_pin = A9;   // PA3 salida del sensor de radiación 
int ml8511_level = 0;               // variable para guardar el valor análogo del sensor ML8511
float uvIntensity = 0;              // variable donde se almacena la medición real del sensor ML8511


// crea instancia y variables del sensor HTU21D
HTU21D htu21d_sensor;
float humedad = 0;      // variable correspondiente para medir la humedad
float temperatura = 0;  // variable correspondiente para media la temperatura


// pin y variables para el sensor de pulso cardiaco 
const int pulse_sensor_pin = A3;    // Sensor de Pulso conectado al puerto PB6
int pulso = 0;                      // variable para guardar lectura de voltaje del sensor de pulso
// Estas variables son volatiles porque son usadas durante la rutina de interrupcion
volatile int BPM;                   // Pulsaciones por minuto
volatile int Signal;                // Entrada de datos del sensor de pulsos
volatile int IBI = 600;             // tiempo entre pulsaciones
volatile boolean Pulse = false;     // Verdadero cuando la onda de pulsos es alta, falso cuando es Baja
volatile boolean QS = false;        // Verdadero cuando el Arduino busca un pulso del Corazon


// direcciones para el control de la memoria flash
const byte writeEnable = 0x06;      // Address Write Enable
const byte writeDisable = 0x04;     // Address Write Disable
const byte chipErase = 0xc7;        // Address Chip Erase
const byte readStatusReg1 = 0x05;   // Address Read Status
const byte readData = 0x03;         // Address Read Data
const byte pageProgramStat = 0x02;  // Address Status Page Program
const byte chipCommandId = 0x9f;    // Address Status Read Id

char page_buffer_n[32]; // variable donde se guardan los paquetes de datos de la memoria
word package_id = 0;    // variable para guardar los paquetes de datos en memoria

float sum = 0;  // variable global usada en la funcion custom_delay()
byte i = 0;     // variable de iteracion global
float j = 0;    // variable de iteracion global 

boolean conectado = false; // variable de estado de conexion BT del dispositivo


/*
 * Funcion para generar un delay en segundos sin utilizar el delay por defecto de Arduino
 */
void custom_delay(int tiempo_espera) // parametro es tiempo de espera en 0.5 segundos
{
  for(i = 0; i < tiempo_espera; i++)
  {
    sum = 0;
    for(j = 0; j < 55112; j++)
    {
       // solo esperar
       sum = sum + 1;
    }
  }
}


/*
 * Función para depuración con leds
 */
void blink_led(int pin, int iter, int del_time)
{
  for (int k = 0; k < iter; k++) 
  { 
    digitalWrite(pin, HIGH);
    custom_delay(del_time);              
    digitalWrite(pin, LOW); 
    custom_delay(del_time);
  }
}


/*
 * Retorna un promedio de 10 mediciones analogas del pin dado
 */
int averageAnalogRead(int pinToRead)
{
  unsigned int runningValue = 0; 

  for(i = 0; i < 10; i++)
    runningValue += analogRead(pinToRead);
  runningValue /= 10;

  return(runningValue);  
}


/*
 * La funcion map de arduino pero con floats
 */
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


/*
 * Funciones para manejar la memoria
 */  
void _chip_erase(void) 
{
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(writeEnable);
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(chipErase);
  digitalWrite(SS, HIGH);

  not_busy();
}

void _read_page(word page_number, char *page_buffer) 
{
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);
  SPI.transfer(readData);

  // Construct the 24-bit address from the 16-bit page
  // number and 0x00, since we will read 256 bytes (one
  // page). Esta memoria es de 4MB, direcciones desde 0x000000h - 0x3FFFFFh
  SPI.transfer((page_number >> 8) & 0xFF);
  SPI.transfer((page_number >> 0) & 0xFF);
  SPI.transfer(0);
  for (i = 0; i < 32; i++) 
  {
    page_buffer[i] = SPI.transfer(0);
  }
  digitalWrite(SS, HIGH);
  not_busy();
}
 
void _write_page(word page_number, char *page_buffer) 
{
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(writeEnable);
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(pageProgramStat);
  SPI.transfer((page_number >>  8) & 0xFF);
  SPI.transfer((page_number >>  0) & 0xFF);
  SPI.transfer(0);
  for (i = 0; i < 32; i++) 
  {
    SPI.transfer(page_buffer[i]);
  }
  digitalWrite(SS, HIGH);
  
  not_busy();
}

void not_busy(void) 
{
  digitalWrite(SS, HIGH);  
  digitalWrite(SS, LOW);
  SPI.transfer(readStatusReg1);       
  while (SPI.transfer(0) & 1) {}; 
  digitalWrite(SS, HIGH);  
}


/*
 * Función de configuración principal del sistema
 */
void setup() 
{
  // puerto serial configurado a 9600 Baudios para comunicacion con Bluetooth HM-10
  Serial.begin(9600);                                               

  // definición de pines de salida
  pinMode(ledr_pin, OUTPUT);
  pinMode(ledv_pin, OUTPUT);

  // definición de pines de entrada
  pinMode(batt_level_pin, INPUT);
  pinMode(ml8511_sensor_pin, INPUT);
  pinMode(pulse_sensor_pin, INPUT);

  // configura la interrupcion para leer el sensor de pulsos cada 2ms
  interruptSetup();   

  // inicializa el sensor htu21d por I2C
  htu21d_sensor.begin();

  // inicializa comunicación SPI con la memoria flash
  SPI.begin();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);

  // secuencia de iniciación del sistema
  blink_led(ledr_pin, 1, 1);

  //_chip_erase();
  last_package_saved(); // actualiza el package_id deacuerdo a los paquetes almacenados en memoria

  blink_led(ledv_pin, 1, 1);
}


/*
 * Función loop principal
 */
void loop() 
{ 
  // medición del voltaje de batería
  batt_level = analogRead(batt_level_pin);
  batt_voltaje = (3.3 / 1023) * batt_level;
  batt_voltaje = batt_voltaje / constante_divisor;

  // indicador de nivel de batería con los leds en el dispositivo 
  if (batt_voltaje < 3.6)
  {
    digitalWrite(ledr_pin, HIGH); // enciende el led rojo si quedan menos de 3.6V de batería
    digitalWrite(ledv_pin, LOW);
  }
  else
  {
    digitalWrite(ledr_pin, LOW); // enciende el led verde si quedan 3.6V o más de batería
    digitalWrite(ledv_pin, HIGH);
  }


  // medición del sensor de radiación UV
  ml8511_level = averageAnalogRead(ml8511_sensor_pin);
  uvIntensity = (3.3 / 1023) * ml8511_level; 
  uvIntensity = mapfloat(uvIntensity, 0.99, 2.8, 0.0, 15.0); // convierte el voltaje medido a intensidad UV en (mW/cm^2)

  
  // medición del sensor de humedad y temperatura
  humedad = htu21d_sensor.readHumidity();         // valor en porcentaje de humedad relativa
  temperatura = htu21d_sensor.readTemperature();  // valor en grados celsius de temperatura corporal


  // medición del sensor de pulso cardíaco
  pulso = analogRead(pulse_sensor_pin); // lee el valor del pulsometro conectado al puerto Analogo A0
 
  if (QS)                               // bandera del Quantified Self es verdadera cuando el Arduino busca un pulso del corazon
  {                                 
    QS = false;                         // reset a la bandera del Quantified Self 
  }


  // chequea la conexión con el BT para determinar si envía o guarda el paquete
  if (Serial.available())
  {
    while (Serial.available())
    {
      int incombyte = 0;
      incombyte = Serial.read();
    } 
    conectado = !conectado; // cambia el estado de conexión
  }


  // envía el paquete de datos por serial al BT
  if (conectado) 
  { 
    read_allpackages_tosend(); // revisa si hay paquetes guardados en memoria para enviarlos por BT
    
    Serial.print('T' + String(temperatura) + ',');
    Serial.println('H' + String(humedad) + ',');
    custom_delay(1); // delay de 0.5 seg para envío de los subpaquetes
    Serial.print('R' + String(uvIntensity) + ',');
    Serial.println('B' + String(BPM) + ',');
    custom_delay(1); // delay de 0.5 seg para envío de los subpaquetes
    Serial.println('V' + String(batt_voltaje) + ',');
    custom_delay(1); // delay de 0.5 seg para envío de los subpaquetes
  }

  // guarda el paquete de datos en memoria
  else
  {
    page_buffer_n[0] = (package_id >> 8) & 0xff; 
    page_buffer_n[1] = (package_id >> 0) & 0xff;
  
    for (i = 0; i < sizeof(String(temperatura)) - 1; i++)
    {
       page_buffer_n[2+i] = String(temperatura)[i];
    }
  
    for (i = 0; i < sizeof(String(humedad)) - 1; i++)
    {
       page_buffer_n[8+i] = String(humedad)[i];
    }
  
    for (i = 0; i < sizeof(String(uvIntensity)) - 1; i++)
    {
       page_buffer_n[14+i] = String(uvIntensity)[i];
    }
  
    for (i = 0; i < sizeof(String(BPM)) - 1; i++)
    {
       page_buffer_n[20+i] = String(BPM)[i];
    }
  
    for (i = 0; i < sizeof(String(batt_voltaje)) - 1; i++)
    {
       page_buffer_n[26+i] = String(batt_voltaje)[i];
    }
     
    _write_page(package_id, page_buffer_n); // escribe la página correspondiente con el paquete en los primeros 32 bytes [0-31]
  
    package_id = package_id + 1; // aumenta el id para el siguiente paquete a guardar

    custom_delay(3); // delay de 1.5 seg para envío o guardado de los paquetes aproximadamente a 2seg
  }
    
}


/*
 * Actualiza la variable package id con el valor donde se deben empezar a guardar los paquetes, cuando se apague y prenda el microcontrolador
 */
void last_package_saved()
{
  // para hacer esto lee todas las paginas y pregunta por la primera pagina donde el id es vacio (0xff) y ese es el nuevo package_id
  package_id = 0;
  
  while (package_id < 10)
  {
    _read_page(package_id, page_buffer_n);

    if (byte(page_buffer_n[0]) == 0xff) // pregunta si el byte de la página donde se guarda el package_id está vacío
    {
      break; // si es así entonces esta página es la siguiente a la página del último paquete guardado
    }
    
    package_id = package_id + 1;
  }
}


/*
 * Lee todos los paquetes de la memoria hasta la página indicada por la variable global package_id, y los retorna uno a uno por Serial
 */
void read_allpackages_tosend()
{
  if (package_id > 0) // pregunta si hay paquetes guardados en la memoria, para retornarlos cuando haya conexión BT
  {
    Serial.println('D' + String(package_id) + ',');

    custom_delay(1);
    
    for (word x = 0; x < package_id; x++)
    {
      _read_page(x, page_buffer_n);

      Serial.print('T');
      Serial.print(char(page_buffer_n[2])); 
      Serial.print(char(page_buffer_n[3]));
      Serial.print(char(page_buffer_n[4])); 
      Serial.print(char(page_buffer_n[5]));
      Serial.print(char(page_buffer_n[6]));
      //Serial.print(char(page_buffer_n[7]));
      Serial.print(',');
      
      Serial.print('H');
      Serial.print(char(page_buffer_n[8])); 
      Serial.print(char(page_buffer_n[9]));
      Serial.print(char(page_buffer_n[10])); 
      Serial.print(char(page_buffer_n[11]));
      Serial.print(char(page_buffer_n[12]));
      //Serial.print(char(page_buffer_n[13]));
      Serial.println(',');

      custom_delay(1);
      
      Serial.print('R');
      Serial.print(char(page_buffer_n[14]));
      Serial.print(char(page_buffer_n[15]));
      Serial.print(char(page_buffer_n[16])); 
      Serial.print(char(page_buffer_n[17]));
      Serial.print(char(page_buffer_n[18]));
      //Serial.print(char(page_buffer_n[19]));
      Serial.print(',');

      Serial.print('B');
      Serial.print(char(page_buffer_n[20])); 
      Serial.print(char(page_buffer_n[21]));
      Serial.print(char(page_buffer_n[22])); 
      //Serial.print(char(page_buffer_n[23]));
      //Serial.print(char(page_buffer_n[24]));
      //Serial.print(char(page_buffer_n[25]));
      Serial.print(',');

      Serial.print('V');
      Serial.print(char(page_buffer_n[26])); 
      Serial.print(char(page_buffer_n[27]));
      Serial.print(char(page_buffer_n[28])); 
      Serial.print(char(page_buffer_n[29]));
      //Serial.print(char(page_buffer_n[30]));
      //Serial.print(char(page_buffer_n[31]));
      Serial.println(',');
      
      custom_delay(1);
    }
    
    Serial.print('F'); // bandera para detectar en la app que se mandó el último paquete de la memoria
    Serial.println(',');

    custom_delay(1);

    _chip_erase();     // una vez enviados por serial todos los datos de la memoria, esta se limpia
    package_id = 0;    // resetea el id de los paquetes para empezar a guardar desde el principio los nuevos paquetes
  }
}

