
#include <SoftwareSerial.h>     /* Allows Pin Change Interrupt Vector Sharing */

#define RxD 5
#define TxD 6

SoftwareSerial BTSerial(RxD, TxD);

String comando="";
char confirmacion="";
int envconfirmation=0;
int timebase=0;
int _timestamp=0;


char d ="s";

void setup(){
  BTSerial.begin(9600); // comunicación directa al Modulo.
  Serial.begin(9600);  // comunicación directa al Monitor.
  Serial.println("*EMPEZANDO A ESCUCHAR*");
  tomarTiempoInicial();
}

void loop() {
  _timestamp=millis()-timebase;
  leerPuertoSerial();
  enviarConfirmacion(_timestamp);  
  //envconfirmation=1;
  //delay(1000);
}


void leerPuertoSerial(){
  if (BTSerial.available()){
  comando  = BTSerial.readString();
  Serial.print("DATA FROM APP:");
  Serial.println(comando);
  envconfirmation = 1;
  }
}

void enviarConfirmacion(int _time){
  if(envconfirmation){
    Serial.print("BT CONFIRMATION:");
    //BTSerial.print("ans ");
    Serial.print(comando);
    Serial.print("-> ");
    Serial.println("OK ");
    envconfirmation=0;
    BTSerial.print(comando);
    }
  }

void tomarTiempoInicial(void){
  timebase = millis(); // tiempo inicial de referencia
}
