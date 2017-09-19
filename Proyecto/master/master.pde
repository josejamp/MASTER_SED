
import processing.serial.*;
import processing.video.*;

// capturadora de la webcam
Capture cam;

//puertos serie
Serial myPortMov;  
Serial myPortSound; 

//los datos que recibamos por los puertos serie se guardan aqui
String val;

//booleanos para indicar si hemos establecido contacto con los Arduino
boolean firstContactMov = false;
boolean firstContactSound = false;

//booleanos para indicar si estamos recibiendo sonido y movimiento
boolean sonido = false;
boolean movimiento = false;

//booleanos para saber cuantas fotos llevamos hechas
int numFoto = 0;

/*
 * Enumerado que nos indica el modo
 */
enum Modo {
    M1,M2,M3
};

//modo en el que estamos, por defecto es el Modo 3
Modo modo = Modo.M3;


/*
 * En el procedimiento setup():
 * - Se genera una ventana 600x600
 * - Se abren los puertos serie para comunicarse con los Arduino
 * - Se inicializa la cámara
 */
void setup(){
  
  size(600,600);
  
  String portNameMov = Serial.list()[0]; // movimiento
  String portNameSound = Serial.list()[1]; // sonido
  
  myPortMov = new Serial(this, portNameMov, 9600); 
  myPortMov.bufferUntil('\n');
  
  myPortSound = new Serial(this, portNameSound, 9600); 
  myPortSound.bufferUntil('\n');
  
  
  String[] cameras = Capture.list();
  
  if (cameras.length == 0) {
    println("No hay camaras disponibles.");
    exit();
  } else {
    cam = new Capture(this, cameras[0]);    
  }
}

/*
 * En el procedimiento draw se comprueba si se ha recibido algun mensaje
 * que indique que se ha detectado sonido o mivimiento
 */
void draw(){
  
      boolean videoStarted = false;
  
      // Si se han detectado los dos, se graba video y se muestra en la ventana
      if(movimiento && sonido){
          println("AAAAMBOOOS");
          if(!videoStarted){
              cam.start();   
              videoStarted = true;
          }
          if (cam.available() == true) {
              cam.read();
              image(cam, 0, 0);
          }
          delay(100);
      } //si solo se detecta movimiento, se hacen fotos y se guardan
      else if(movimiento){
          if(!videoStarted){
              cam.start();   
              videoStarted = true;
          }
          if (cam.available() == true) {
              cam.read();
              image(cam, 0, 0);
              saveFrame("captura" + numFoto + ".png");
              numFoto++;
          }
          println("MOVIMIEEEENTOOO");
          delay(3000);
      } //si solo se detecta sonido no se hace nada
      else if(sonido){
          if(videoStarted){
             cam.stop();
             videoStarted = false;
          }
        
          println("SONAAAANDOOO");
          delay(1000);
      }
      else{
        if(videoStarted){
             cam.stop();
             videoStarted = false;
          }
        
      }
  
}

/*
 * Procedimiento que se dispara cuando llega un evento desde uno de los puertos serie
 */
void serialEvent( Serial myPort) {
  
  if(myPort == myPortSound){
    val = myPort.readStringUntil('\n');
    if (val != null) {
      
      //separa los caracteres por espacios y caracteres especiales
      val = trim(val);
      
      //si no hemos establecido contacto busca una "B"
      if (firstContactSound == false && val.equals("B")) {
          myPort.clear(); //borramos todo lo que nos llegara antes
          firstContactSound = true;
          myPort.write("B");
          println("contact with Sound device");
      }
      else { //si ya hemos establecido contacto, comprobamos si hemos detectado sonido o no
        
        sonido = (val.equals("BeginS"));
        
        println(val);
        println("Recibido");
        
        }
      }
  }
  else if(myPort == myPortMov){
    val = myPort.readStringUntil('\n');
    if (val != null) {
      
     //separa los caracteres por espacios y caracteres especiales
      val = trim(val);
    
      //si no hemos establecido contacto busca una "A"
      if (firstContactMov == false && val.equals("A")) {
          myPort.clear();
          firstContactMov = true;
          myPort.write("A");
          println("contact with Movement device");
      }
      else { //si ya hemos establecido contacto, comprobamos si hemos detectado movimiento o no
        
        movimiento = (val.equals("BeginM"));
        
        println(val);
        println("Recibido");
        
        }
      }
    
  }
}

/*
 * Funcion que dado un modo devuelve su correspondiente representacion en String
 */
String modeToString(Modo m){
  
  if(m == Modo.M1)
    return "M1\n";
  else if(m == Modo.M2)
    return "M2\n";
  else if(m == Modo.M3)
    return "M3\n";
  else return "-";
  
}

/*
 * Procedimiento que se ejecuta cuando se presiona una letra del teclado,
 * se utiliza para cambiar el modo y enviarlo a los Arduino
 */
void keyPressed(){
  
  if(key == '1'){
    modo = Modo.M1;
  }
  else if(key == '2'){
    modo = Modo.M2;
  }
  else if(key == '3'){
    modo = Modo.M3;
  }
  
  enviaNuevoModo(modeToString(modo));  
  
}

/*
 * Procedimiento que envia el modo a los Arduino
 */
void enviaNuevoModo(String mode){
 
  myPortMov.write(mode);
  myPortSound.write(mode);
  
}