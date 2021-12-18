#include <SPI.h>

// direcciones para el control de la memoria flash
const byte writeEnable = 0x06; // Address Write Enable
const byte writeDisable = 0x04; // Address Write Disable
const byte chipErase = 0xc7; // Address Chip Erase
const byte readStatusReg1 = 0x05; // Address Read Status
const byte readData = 0x03; // Address Read Data
const byte pageProgramStat = 0x02; // Address Status Page Program
const byte chipCommandId = 0x9f; // Address Status Read Id

char page_buffer_n[32]; // variable donde se guarda la última pagina leída de la memoria

word package_id = 0; // variable para guardar los paquetes de datos en memoria

float sum = 0; // variable global usada en la funcion custom_delay()
byte i = 0; // variable de iteracion globales
float j = 0; // variable de iteracion globales 

boolean conectado = false; // variable de estado de conexion BT del dispositivo

byte b1 = 0;
byte b2 = 0;
byte b3 = 0;

/*
 * Funcion para generar un delay en segundos sin utilizar el delay por defecto 
 */
void custom_delay(int tiempo_espera) // parametro es tiempo de espera en segundos
{
  for(i = 0; i < tiempo_espera; i++)
  {
    sum = 0;
    for(j = 0; j < 110225; j++)
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
 * Retorna un promedio de 8 mediciones analogas del pin dado
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
 * La funcion map de arduino pero en floats
 */
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


/*
 * Functions to manage memory at low level
 */
void _chipCmdId(byte *b1, byte *b2, byte *b3) 
{
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);
  SPI.transfer(chipCommandId);
  *b1 = SPI.transfer(0); // manufacturer id
  *b2 = SPI.transfer(0); // memory type
  *b3 = SPI.transfer(0); // capacity
  digitalWrite(SS, HIGH);
  not_busy();
}  

/*
 See the timing diagram in section 9.2.26 of the data sheet, "Chip Erase (C7h / 06h)".
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

  /* See notes on rev 2 
  digitalWrite(SS, LOW);  
  SPI.transfer(writeDisable);
  digitalWrite(SS, HIGH);
  */
  not_busy();
}

/*
 * See the timing diagram in section 9.2.10 of the
 * data sheet located below, "Read Data (03h)".
 */
void _read_page(word page_number, byte *page_buffer) 
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
  for (i = 0; i < 32; ++i) 
  {
    page_buffer[i] = SPI.transfer(0);
  }
  digitalWrite(SS, HIGH);
  not_busy();
}
 
void _write_page(word page_number, byte *page_buffer) 
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
  for (i = 0; i < 32; ++i) 
  {
    SPI.transfer(page_buffer[i]);
  }
//  for (i = 32; i < 256; i++)
//  {
//    SPI.transfer(0xff); 
//  }
  digitalWrite(SS, HIGH);
  /* See notes on rev 2
  digitalWrite(SS, LOW);  
  SPI.transfer(writeDisable);
  digitalWrite(SS, HIGH);
  */
  not_busy();
}

/* 
 * See section 9.2.8 of the datasheet
 */
void not_busy(void) 
{
  digitalWrite(SS, HIGH);  
  digitalWrite(SS, LOW);
  SPI.transfer(readStatusReg1);       
  while (SPI.transfer(0) & 1) {}; 
  digitalWrite(SS, HIGH);  
}

/*
print_page_bytes() is a simple helper function that formats 256 bytes
 */
void print_page_bytes(byte *page_buffer) 
{
  char buf[10];
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 16; ++j) {
      sprintf(buf, "%02x", page_buffer[i * 16 + j]);
      Serial.print(buf);
    }
    Serial.println();
  }
  Serial.println("new page");
}

void setup(void) 
{
  SPI.begin();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);
  Serial.begin(9600);
  //_chip_erase();
  last_package_saved();
  Serial.println("Ready"); 
}

/*
 */
void loop(void) 
{
  float temperatura = 27.56;
  float humedad = 80.23;
  float uvIntensity = 22.31;
  int BPM = 90;
  float batt_voltaje = 3.78;

 read_allpackages_tosend();
//  Serial.print('T' + String(temperatura) + ',');
//  Serial.print('H' + String(humedad) + ',');
//  Serial.print('R' + String(uvIntensity) + ',');
//  Serial.print('B' + String(BPM) + ',');
//  Serial.print('V' + String(batt_voltaje) + ',');
  Serial.println('P' + String(package_id));
  //else if (sino está conectado a BT, guarda el paquete en la memoria)
  
  //save_package(String(temperatura), String(humedad), String(uvIntensity), String(BPM), String(batt_voltaje));

  custom_delay(1); // delay de 1 seg para envío de información

}

/*
 * Actualiza la variable package id con el valor donde se deben empezar a guardar los paquetes, cuando se apague y prenda el microcontrolador
 */
void last_package_saved()
{
  // para hacer eso tendría que leer todas las paginas y preguntar por la primer pagina donde el id es vacio (0xff) y ese es el nuevo package_id
  package_id = 0;

  while (package_id < 16843)
  {
    _read_page(package_id, page_buffer_n);

    if (page_buffer_n[0] != char(0xff)) // pregunta si el byte de la página donde se guarda el package_id está vacío
    {
      package_id = package_id + 1;
      // si es así entonces esta página es la siguiente a la página del último paquete guardado
    }
    else
    {
      break;
    }
    
  }
}


/*
 * Guarda el paquete de datos pasado por parametro en la pagina correspondiente al package_id
 */
void save_package(String temp, String hume, String uv, String beats, String batt)
{
  /*
   * orden de los 32 bytes de cada paquete a guardar en la memoria, desde el id hasta la ultima coma
   * (package_id >> 8) & 0xFF - (package_id >> 0) & 0xFF
   * char 'T'
   * (temp >> 24) & 0xFF - (temp >> 16) & 0xFF - (temp >> 8) & 0xFF - (temp >> 0) & 0xFF
   * char 'H'
   * (hume >> 24) & 0xFF - (hume >> 16) & 0xFF - (hume >> 8) & 0xFF - (hume >> 0) & 0xFF
   * char 'R'
   * (uv >> 24) & 0xFF - (uv >> 16) & 0xFF - (uv >> 8) & 0xFF - (uv >> 0) & 0xFF
   * char 'B'
   * (beats >> 8) & 0xFF - (beats >> 0) & 0xFF
   * char 'V'
   * (batt >> 24) & 0xFF - (batt >> 16) & 0xFF - (batt >> 8) & 0xFF -(batt >> 0) & 0xFF
   */

   // guarda los 32 bytes del paquete en el arreglo de la página, char por char en bytes
   Serial.println("Entro a guardar");
   
   page_buffer_n[0] = (package_id >> 8) & 0xFF; // para mandar un byte
   page_buffer_n[1] = (package_id >> 0) & 0xFF;

   for (i = 0; i < sizeof(temp) - 1; i++)
   {
      page_buffer_n[2+i] = temp[i];
      Serial.println(temp[i]);
   }

   for (i = 0; i < sizeof(hume) - 1; i++)
   {
      page_buffer_n[8+i] = hume[i];
      Serial.println(hume[i]);
   }

   for (i = 0; i < sizeof(uv) - 1; i++)
   {
      page_buffer_n[14+i] = uv[i];
      Serial.println(uv[i]);
   }

   for (i = 0; i < sizeof(beats) - 1; i++)
   {
      page_buffer_n[20+i] = beats[i];
      Serial.println(beats[i]);
   }

   for (i = 0; i < sizeof(batt) - 1; i++)
   {
      page_buffer_n[26+i] = batt[i];
      Serial.println(batt[i]);
   }

   Serial.println("Empieza for");

//   for (int k = 0; k < 32; k++)
//   {
//    //Serial.println(char(page_buffer_n[k]));
//    Serial.println(sizeof(page_buffer_n[k]));
//   }

   Serial.println("Termina for");
   
   _write_page(package_id, page_buffer_n); // escribe la página correspondiente con el paquete en los primeros 32 bytes [0-31]

  Serial.println("hizo el write page");

   package_id = package_id + 1; // aumenta el id del siguiente paquete a guardar

}


/*
 * Lee los paquetes de la memoria hasta la página indicada por la variable global package_id, y los retorna uno a uno por Serial
 */
void read_allpackages_tosend()
{
  if (package_id > 0) // pregunta si hay paquetes guardados en la memoria, para retornarlos cuando haya conexión BT
  {
    for (word x = 0; x < package_id; x++)
    {
      _read_page(x, page_buffer_n);

      Serial.print("Pagina actual ");
      Serial.println(x);
      
      Serial.print('T');
      Serial.print(char(page_buffer_n[2])); 
      Serial.print(char(page_buffer_n[3]));
      Serial.print(char(page_buffer_n[4])); 
      Serial.print(char(page_buffer_n[5]));
      Serial.print(char(page_buffer_n[6]));
      Serial.print(char(page_buffer_n[7]));
      Serial.print(',');
      
      Serial.print('H');
      Serial.print(char(page_buffer_n[8])); 
      Serial.print(char(page_buffer_n[9]));
      Serial.print(char(page_buffer_n[10])); 
      Serial.print(char(page_buffer_n[11]));
      Serial.print(char(page_buffer_n[12]));
      Serial.print(char(page_buffer_n[13]));
      Serial.print(',');

      Serial.print('R');
      Serial.print(char(page_buffer_n[14])); 
      Serial.print(char(page_buffer_n[15]));
      Serial.print(char(page_buffer_n[16])); 
      Serial.print(char(page_buffer_n[17]));
      Serial.print(char(page_buffer_n[18]));
      Serial.print(char(page_buffer_n[19]));
      Serial.print(',');

      Serial.print('B');
      Serial.print(char(page_buffer_n[20])); 
      Serial.print(char(page_buffer_n[21]));
      Serial.print(char(page_buffer_n[22])); 
      Serial.print(char(page_buffer_n[23]));
      Serial.print(char(page_buffer_n[24]));
      Serial.print(char(page_buffer_n[25]));
      Serial.print(',');

      Serial.print('V');
      Serial.print(char(page_buffer_n[26])); 
      Serial.print(char(page_buffer_n[27]));
      Serial.print(char(page_buffer_n[28])); 
      Serial.print(char(page_buffer_n[29]));
      Serial.print(char(page_buffer_n[30]));
      Serial.print(char(page_buffer_n[31]));
      Serial.println(',');
      
      //Serial.print("H" + char(page_buffer_n[8]) + char(page_buffer_n[9]) + char(page_buffer_n[10]) + char(page_buffer_n[11]) + char(page_buffer_n[12]) + char(page_buffer_n[13]) + ',');
      //Serial.print("R" + char(page_buffer_n[14]) + char(page_buffer_n[15]) + char(page_buffer_n[16]) + char(page_buffer_n[17]) + char(page_buffer_n[18]) + char(page_buffer_n[19]) + ',');
      //Serial.print("B" + char(page_buffer_n[20]) + char(page_buffer_n[21]) + char(page_buffer_n[22]) + char(page_buffer_n[23]) + char(page_buffer_n[24]) + char(page_buffer_n[25]) + ',');
      //Serial.println("V" + String(page_buffer_n[26]) + String(page_buffer_n[27]) + String(page_buffer_n[28]) + String(page_buffer_n[29]) + String(page_buffer_n[30]) + String(page_buffer_n[31]) + ",");

      custom_delay(1);
    }
  
    //_chip_erase(); // una vez enviados por serial todos los datos de la memoria, esta se vacía
    package_id  = 0; // resetea el id de los paquetes para empezar a guardar desde el principio
  }
}


