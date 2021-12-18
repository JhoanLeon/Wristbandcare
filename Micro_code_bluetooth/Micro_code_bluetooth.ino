
String comando = "";
int timebase = 0;
int _timestamp = 0;
String dato = "";

void setup()
{
  Serial.begin(9600);  // comunicación directa con el bluetooth
  timebase = millis(); // tiempo inicial de referencia
}

void loop() 
{
   delay(500); 
  _timestamp = millis() - timebase;

  enviarConfirmacion(_timestamp);
  
  if(_timestamp/1000 >= 32)
  {
    timebase = millis(); // tiempo inicial de referencia
    Serial.print("*REINICIANDO*");
    delay(10);
  }
  
  if (Serial.available())
  {
    comando = Serial.readString();
    Serial.print(comando);
  }    
}

void enviarConfirmacion(int _time)
{
   dato = (String)(_time);
   dato = "TIME: " + dato + " S";
   Serial.print(dato);
   delay(10);
}

