#include <SPI.h>
//#include <DigiKeyboard.h>

// addres for memory control
#define writeEnable       0x06 // Address Write Enable
#define writeDisable      0x04 // Address Write Disable
#define chipErase         0xc7 // Address Chip Erase
#define readStatusReg1    0x05 // Address Read Status
#define readData          0x03 // Address Read Data
#define pageProgramStat   0x02 // Address Status Page Program
#define chipCommandId     0x9f // Address Status Read Id

int ledr = 1; //PB1 indicator red led
int ledg = 3; //PB6 indicator green led

byte page_buffer_n[256];
byte page_data[256];

int cont = 0;

/*
print_page_bytes() is a simple helper function that formats 256 bytes
 */
void print_page_bytes(byte *page_buffer) 
{
  //DigiKeyboard.println("Llego a imprimir pagina");
  char buf[10];
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 16; ++j) {
      //sprintf(buf, "%02x", page_buffer[i * 16 + j]);
      //Serial.print(buf);
    }
    //Serial.println();
  }
  //DigiKeyboard.println(page_buffer[0]);
}

/*
This functions map to user commands. wrap the low-level calls with 
print/debug statements to read
*/

/* 
 The chip command id  is fairly generic, just to verify function setup 
 */
void chipCmdId(void) 
{
  //Serial.println("Set Command: chipCmdIda");
  byte b1, b2, b3;
  _chipCmdId(&b1, &b2, &b3);
  //char buf[128];
  //sprintf(buf, "ID: %02xh\nMemory Type: %02xh\nCapacity: %02xh", b1, b2, b3);
  //Serial.println(buf);
  //Serial.println("Ready");
} 

void chip_erase(void) 
{
  //Serial.println("command: chip_erase");
  _chip_erase();
  //Serial.println("Ready");
}

void read_page(unsigned int page_number, byte page_buffer[256]) 
{
  //char buf[80];
  //sprintf(buf, "command: read_page(%04xh)", page_number);
  //Serial.println(buf);
  //byte page_buffer[256];
  _read_page(page_number, page_buffer);
  //print_page_bytes(page_buffer);
  //Serial.println("Ready");
}

void read_all_pages(byte page_buffer[256]) 
{
  //Serial.println("command: read_all_pages");
  //byte page_buffer[256];
  for (int i = 0; i < 6; ++i) // nuestra memoria tiene 16384 paginas, aqui solo hay 6
  {
    _read_page(i, page_buffer);
    //print_page_bytes(page_buffer);
  }
  //Serial.println("Ready");
}

void write_byte(word page, byte offset, byte databyte) 
{
  //char buf[80];
  //sprintf(buf, "command: write_byte(%04xh, %04xh, %02xh)", page, offset, databyte);
  //Serial.println(buf);
  //byte page_data[256];
  _read_page(page, page_data);
  page_data[offset] = databyte;
  _write_page(page, page_data);
  //Serial.println("Ready");
}


/*
 * Functions to manage memory at low level
 */
void _chipCmdId(byte *b1, byte *b2, byte *b3) {
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
void _chip_erase(void) {
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
void _read_page(word page_number, byte *page_buffer) {
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);
  SPI.transfer(readData);

  // Construct the 24-bit address from the 16-bit page
  // number and 0x00, since we will read 256 bytes (one
  // page). Esta memoria es de 4MB, direcciones desde 0x000000h - 0x3FFFFFh
  SPI.transfer((page_number >> 8) & 0xFF);
  SPI.transfer((page_number >> 0) & 0xFF);
  SPI.transfer(0);
  for (int i = 0; i < 256; ++i) 
  {
    page_buffer[i] = SPI.transfer(0);
  }
  digitalWrite(SS, HIGH);
  not_busy();
}
 
/*
 * See the timing diagram in section 9.2.21 of the
 * data sheet, "Page Program (02h)".
 */
void _write_page(word page_number, byte *page_buffer) {
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(writeEnable);
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(pageProgramStat);
  SPI.transfer((page_number >>  8) & 0xFF);
  SPI.transfer((page_number >>  0) & 0xFF);
  SPI.transfer(0);
  for (int i = 0; i < 256; ++i) {
    SPI.transfer(page_buffer[i]);
  }
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
void not_busy(void) {
  digitalWrite(SS, HIGH);  
  digitalWrite(SS, LOW);
  SPI.transfer(readStatusReg1);       
  while (SPI.transfer(0) & 1) {}; 
  digitalWrite(SS, HIGH);  
}

void blink_led(int pin, int iter, int del_time)
{
  for (int i = 0; i < iter; ++i) 
  { 
    digitalWrite(pin, HIGH);
    delay(del_time);              
    digitalWrite(pin, LOW); 
    delay(del_time);
  }
}

void setup(void) 
{
  pinMode(ledr, OUTPUT); //LED on board
  pinMode(ledg, OUTPUT);
  
  blink_led(ledr, 1, 500);
  
  SPI.begin();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);

  delay(1000);

  //chipCmdId(); // verificacion de comunicacion con la memoria
  chip_erase();

  blink_led(ledr, 2, 300);

  delay(4000);
}


void loop(void) 
{
  while (cont < 5)
  {
  //word pageno = cont;
  //read_page(pageno, page_buffer_n);

  //blink_led(ledr, 3, 300);

  //delay(1000);

  word pageno = cont;
  byte offset = 0x55; // 0x00 - 0xff
  byte data = 0x28;
  write_byte(pageno, offset, data);

  blink_led(ledr, 3, 300);
  delay(1000);

  read_page(pageno, page_buffer_n);

  blink_led(ledr, 4, 300);
  delay(1000);
  
  
  if (page_buffer_n[offset] == data)
  {
    blink_led(ledg, 4, 300);
  }
  else
  {
    blink_led(ledg, 2, 300);
  }
  
  delay(2000);
  cont = cont + 1;
  }

// chipCmdId();

// chip_erase();

// word pageno;
// byte offset;
// byte data;

// read_page(pageno);
// write_byte(pageno, offset, data);  

// read_all_pages(); // usar con precaucion

}

