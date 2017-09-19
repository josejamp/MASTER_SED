#include <Wire.h>

/*
 * Codigo realizado por Cristina Valentina Espinosa Victoria y Jose Javier Martinez Pages.
 * Partes del codigo basadas en los tutoriales Public Domain de https://www.arduino.cc/ 
 * Para ver que tutoriales se han usado visitar la parte de Bibliografia de la Memoria.
 */

// 1 minuto para calibrarse
int calibrationTime = 60;  

//pin de entrada de la salida del sensor
int inPin = 2;
//pin de salida, corresponde a un LED, se enciende si el sensor detecta sonido
int ledPin = 13;

//tiempo en el que no se detecta sonido
long unsigned int undetectedTime; 

//numero de milisegundos que el sensor tiene que estar a la baja una vez se haya detectado movimiento
//para que deje de considerarse que hay movimiento
long unsigned int pause = 2000;  

//booleano que indica que el sensor esta a la baja
boolean undetected = true;
//variable para saber si hay que almacenar el tiempo que esta a la baja
boolean saveTime;  


//booleano para saber si debe informar al PC o no
boolean activo = false;

//direccion del bus para conectarse con el otro arduino
int i2cBus_address = 8;

//modo de ejecucion, por defecto es el Modo 3
int modo = 3;


/*
 * En el setup():
 *   - Determinar los pines a utilizar
 *   - Inicializar el bus i2c para comunicarse con el otro Arduino
 *   - Calibrar el sensor
 *   - Iniciar la comunicacion con el PC
 */
void setup(){
  pinMode(inPin, INPUT);// definimos el pin 2 como entrada
  pinMode(ledPin, OUTPUT);// definimos el pin 13 como salida
  Serial.begin(9600);

  Wire.begin(i2cBus_address); // entra en el bus
  Wire.onReceive(receiveEvent); // ejecuta el procedimiento "receiveEvent" cuando llegue un dato por el bus

  //calibra el sensor
  calibrate();

  //inicia comunicacion con el PC
  establishContact();
}

/*
 * En el metodo loop() se comprueba el modo en el que estamos, si estamos activos
 * y comrpobamos si hay sonido
 */
void loop(){

  if(modo == 1){
    if(activo || !undetected){
      detect_sound(true);
    }
  }
  else if(modo == 2){
    detect_sound(false);
  }
  else if(modo == 3){
    detect_sound(true);
  }
     
}

/*
 * Calibracion del sensor, cada iteración del bucle consume aprox. 1 segundo
 */
void calibrate(){

  // Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    delay(50);

}

/*
 * Establecimiento de contacto con el PC, se envia el caracter "B"
 */
void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println("B");   // send a capital B
    delay(300);
  }
}

/*
 * Procedimiento que detecta sonido.
 * Si independent es true el Arduino actua de manera independiente y no avisa al otro,
 * si por el contrario es false, ordena al otro Arduino activarse o desactivarse cuando
 * detecta o deja de detectar sonido, respectivamente
 */
void detect_sound(boolean independent){

  //Si se ha detectado movimiento (el sensor se activa a la baja)
    if(digitalRead(inPin) == LOW){
       digitalWrite(ledPin, HIGH);   //el led se enciende
       if(undetected){  
         //indicamos que se ha encontrado sonido
         undetected = false;

         //indicamos al PC que ha empezado a oirse sonido
         if(independent) Serial.println("BeginS");
         else{
          //iniciamos una transmision con el otro Arduino y le mandamos un 'true' para que se active
            Wire.beginTransmission(i2cBus_address); // transmit to device #8
            Wire.write(true);              // sends one byte
            Wire.endTransmission();
            Serial.println("BeginS");
         }
         
         delay(50);
         }         
         saveTime = true;
       }

    // Si no hay sonido
     if(digitalRead(inPin) == HIGH){       
       digitalWrite(ledPin, LOW);  //el led se apaga

      //si antes habia sonido
       if(saveTime){
        undetectedTime = millis(); //guardamos el tiempo
        saveTime = false;
        }
        
       //si no se detecta sonido durante el tiempo 'pause' consideramos el siguiente sonido que se detecte
       //como distinto al anteriormente encontrado
       if(!undetected && millis() - undetectedTime > pause){  //si antes habia movimiento
           
           undetected = true;     
           //mandamos al PC que se ha dejado de detectar sonido
           if(independent) Serial.println("EndedS");
           else{
               //iniciamos una transmision con el otro Arduino y le mandamos un 'true' para que se active
              Wire.beginTransmission(i2cBus_address); // transmit to device #8
              Wire.write(false);              // sends one byte
              Wire.endTransmission();
              Serial.println("EndedS");
           } 
           delay(50);
           }
       }


}

/*
 * Metodo que se ejecuta cuando llega un evento desde el PC
 */
void serialEvent(){

  String content = "";
  content.reserve(200);
  char character;

  //mientras haya caracteres disponibles los leemos y formamos el String completo
  while(Serial.available()) {
      character = (char)Serial.read();
      if(character != '\n') content.concat(character);
      delay(10);
  }


//si el mensaje es aceptable, lo aceptamos y formamos un nuevo modo
  if(content.equals("M1") || content.equals("M2") || content.equals("M3")) {
    modo = modoToInt(content);
    Serial.println(modo);
  }
  
}

/*
 * Convertimos el modo en formato String a Int
 */
int modoToInt(String m){

  if(m.equals("M1"))
    return 1;
  else if(m.equals("M2"))
    return 2;
  else if(m.equals("M3"))
    return 3;
  else return 3;
  
}

/*
 * Procedimiento que se ejecuta cuando llega un mensaje desde el otro Arduino
 */
void receiveEvent(int howMany) {
  while (Wire.available()) {
    activo = Wire.read(); //nos activamos o nos desactivamos, segun nos ordenen
    Serial.println(activo);
  }
}

