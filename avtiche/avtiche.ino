#include <Servo.h>
#include <DHT.h>
#include <SoftwareSerial.h>

const int DHT_PIN=A0;
#define DHTTYPE DHT22   
DHT dht(DHT_PIN, DHTTYPE); 
int  hum,temp;  
const int LIGHT_STATUS_PIN=5,LIGHT_PIN =A5;
//#define LIGHT_PIN_ESP32CAM A5
String lightStatus="off";//light status

const int GAS_PIN=12;
int lastsend=0;

Servo camSide, camAngle;
int camAngleValue=90;
const int CAM_SIDE_PIN=A2, CAM_ANGLE_PIN=A3;
bool goingUp=false, goingDown=false;

const int trigPin=2;    // TRIG pin
const int echoPinF=0, echoPinR=4, echoPinL=A4, echoPinB=A1;  // ECHO pins
float distanceF, distanceL, distanceR, distanceB;
bool comingOut=false;
bool useAnalog=true; // Set to true for analog, false for digital

String carMode="Manual";
const int MODE_BUTTON_PIN=8;
int modeButton;

long long elapsedTime, startTime=0;
bool changed=false;

SoftwareSerial HC12(7, 6); // TX, RX,,, tx od ovoj na 7, rx na 6

const int la=11,lb=10,ra=9,rb=3;
int speed=170;

void setup() { 
  pinMode(la, OUTPUT);  
  pinMode(lb, OUTPUT);  
  pinMode(ra, OUTPUT); 
  pinMode(rb, OUTPUT); 

  camSide.attach(CAM_SIDE_PIN);
  camAngle.attach(CAM_ANGLE_PIN);
  camAngle.write(camAngleValue);

  pinMode(trigPin, OUTPUT); 
  pinMode(echoPinF, INPUT);  
  pinMode(echoPinL, INPUT);  
  pinMode(echoPinR, INPUT); 
  pinMode(echoPinB, INPUT); 

  pinMode(LIGHT_STATUS_PIN, INPUT); 
  pinMode(LIGHT_PIN, OUTPUT); 
  digitalWrite(LIGHT_PIN, HIGH);
  //digitalWrite(LIGHT_PIN_ESP32CAM, LOW);

  pinMode(MODE_BUTTON_PIN, INPUT); 

  pinMode(INPUT, GAS_PIN);
  HC12.begin(9600);//postavuev se svi pinovi
  Serial.begin(9600);//postavuev se svi pinovi

  dht.begin();
  motorStop();
}

void loop() {
  //delay(100);
  modeButton=digitalRead(MODE_BUTTON_PIN);
  if(modeButton==1){
    elapsedTime=millis()-startTime;
    if(carMode=="Manual" && elapsedTime>500){
      carMode="Automatic";
      startTime=millis();
      elapsedTime=0;
    }else if(carMode=="Automatic" && elapsedTime>500){
      carMode="Manual";
      startTime=millis();
      elapsedTime=0;
      changed=true;
    }      
  }//ovoj e napraveno zatoj sto mnogo brzo vrti void loop a ne mozhe da se stavi delay zatoj sto trba sto pobrz respone time da ima
  //i dok se klikne i pushti dugmeto ono loop izvrti mnogo puta i carMode se smeni mnogo puta i zatoj imam implementirano moj delay koj ne e mnogo efikasan ama vrshi rabotu

  lightCheck("sensor");//proverue dali ima svetlo ako enma pali ga

  if (HC12.available()>0) {//ako ima dobieno info od kontroler chita se i se pravi toj sto znachi svak byte
    changed=false;
    int x = HC12.read();    
    byte data = x;           
    Serial.println(x);        
    if(data==1){
      carMode="Automatic";
      changed=true;
    }else if(data==2){
      carMode="Manual";
      changed=true;
    }else if(data==8) servoCamStop(); 
    else if(data==9) upCam();
    else if(data==10) downCam();
    else if(data==11) turnCamRight();
    else if(data==12) turnCamLeft(); 
    else if(data==13) lightCheck("on");
    else if(data==14) lightCheck("off");
    else if(data==15) changeSpeed("medium");
    else if(data==16) changeSpeed("high");
    else if(data==17) changeSpeed("slow"); 
    else if(carMode=="Manual"){
      if(data==3) motorStop();//stop
      else if(data==4) forward();//forward
      else if(data==5) backwards();//backward
      else if(data==6) turnRight();//right 
      else if(data==7) turnLeft();//left
    }
    else motorStop();
   }else{
      gasCheck();//proverue za gas i prakja ga na kontroler
      dhtCheck();//kd ke stigne tga
      //provere tuja zatoj sto ako nema primeno vrednostr ondak mozhe da prati, uglavnom ne
      //mozhe da prima i prakja u isto vreme zatoj e takoj
   }
  if(carMode=="Automatic"){
    obstacleChecker("front bottom");
    if(distanceB >=50){
        backwards();
        comingOut=true;
    }else if(distanceF <= 20){
      obstacleChecker("right left");//podeleno e za da ne se troshi vreme i da se vika sve ako distanceF e povishe od 20cm
      if(distanceL<=10 && distanceR<=10){//proverue dali ulaza u slep sokak za da mozhe da iskochi nanad  zatoj sto nema mesto da vrti
        backwards();
        comingOut=true;
      }else if(comingOut){
        if(distanceL <= distanceR){
          turnRight();
        }else if(distanceL >= distanceR){
          turnLeft();
        }
        comingOut=false;
      }else if(distanceL <= distanceR){//ako ne e u slep sokak ondak najnormalno ke proverue nakude e bolje da zavrti
        turnRight();
      }else if(distanceL >= distanceR){
        turnLeft();
      }
    }else{
      forward();
    }
  }else if(carMode=="Manual" && changed){
    motorStop();
  }//changed e za dugme na samoto avtiche za da se menja mod
  //ako nema changed tuj i u serial gore ondak nema da dava da se upravlja preko kontroler zatoj sto celo vreme ke se pushta motorStop()

  if(goingUp){
    upCam();
  }else if(goingDown){
    downCam();
  }//proverue dali ide nagore ili nadole, zatoj sto se prakja edn signal a treba celo vreme da ide dok e joystick nagore
}

void obstacleChecker(String s){
  if(s=="front bottom"){
    digitalWrite(trigPin, HIGH);//na ist nachin se chita od svi tri ultrasonic senzori i se pretvara u cm
    delayMicroseconds(20);
    digitalWrite(trigPin, LOW);
    distanceF = 0.017 * pulseIn(echoPinF, HIGH);
    delay(50);
    digitalWrite(trigPin, HIGH);//na ist nachin se chita od svi tri ultrasonic senzori i se pretvara u cm
    delayMicroseconds(20);
    digitalWrite(trigPin, LOW);
    distanceB = 0.017 * pulseIn(echoPinB, HIGH);
    delay(50);
  }else{
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(20);
    digitalWrite(trigPin, LOW);
    distanceL = 0.017 * pulseIn(echoPinL, HIGH);
    delay(50);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(20);
    digitalWrite(trigPin, LOW);
    distanceR = 0.017 * pulseIn(echoPinR, HIGH);
    delay(50);
  }
    
  /*Serial.print("front ");
  Serial.println(distanceF);
  Serial.print("left ");
  Serial.println(distanceL);
  Serial.print("right ");
  Serial.println(distanceR);
  
  Serial.print("bottom ");
  Serial.println(distanceB);
  Serial.println();
  Serial.println();*/
}

void turnRight() {
  if (useAnalog) {
    analogWrite(la, speed);
    analogWrite(lb, 0);
    analogWrite(ra, 0);
    analogWrite(rb, speed);
  } else {
    digitalWrite(la, HIGH);
    digitalWrite(lb, LOW);
    digitalWrite(ra, LOW);
    digitalWrite(rb, HIGH);
  }
  
  if (carMode == "Automatic") {
    if (speed == 170) delay(500);
    else if (speed == 210) delay(250);
    else delay(100); // When speed is 255
  }
}

void turnLeft() {
  if (useAnalog) {
    analogWrite(la, 0);
    analogWrite(lb, speed);
    analogWrite(ra, speed);
    analogWrite(rb, 0);
  } else {
    digitalWrite(la, LOW);
    digitalWrite(lb, HIGH);
    digitalWrite(ra, HIGH);
    digitalWrite(rb, LOW);
  }
  
  if (carMode == "Automatic") {
    if (speed == 170) delay(500);
    else if (speed == 210) delay(250);
    else delay(100);
  }
}

void backwards() {
  if (useAnalog) {
    analogWrite(la, 0);
    analogWrite(lb, speed);
    analogWrite(ra, 0);
    analogWrite(rb, speed);
  } else {
    digitalWrite(la, LOW);
    digitalWrite(lb, HIGH);
    digitalWrite(ra, LOW);
    digitalWrite(rb, HIGH);
  }
}

void forward() {
  if (useAnalog) {
    analogWrite(la, speed);
    analogWrite(lb, 0);
    analogWrite(ra, speed);
    analogWrite(rb, 0);
  } else {
    digitalWrite(la, HIGH);
    digitalWrite(lb, LOW);
    digitalWrite(ra, HIGH);
    digitalWrite(rb, LOW);
  }
}

void motorStop() {
  if (useAnalog) {
    analogWrite(la, 0);
    analogWrite(lb, 0);
    analogWrite(ra, 0);
    analogWrite(rb, 0);
  } else {
    digitalWrite(la, LOW);
    digitalWrite(lb, LOW);
    digitalWrite(ra, LOW);
    digitalWrite(rb, LOW);
  }
}

void lightCheck(String s){
  int light=digitalRead(LIGHT_STATUS_PIN);
  if(s=="sensor"){   
    if(light==1 && lightStatus=="off"){
        delay(30);
        digitalWrite(LIGHT_PIN,LOW);//svetlo se pali na low a gasi se na high
        //digitalWrite(LIGHT_PIN_ESP32CAM, HIGH);
        lightStatus="on";
    }else if(light==0 && lightStatus=="on"){
        delay(30);
        digitalWrite(LIGHT_PIN,HIGH);
        //digitalWrite(LIGHT_PIN_ESP32CAM, LOW);
        lightStatus="off";
    }
  }else if("on"){//to turn on 13
    if(lightStatus=="on"){
      byte b=2;
      HC12.write(b);
    }else{
      digitalWrite(LIGHT_PIN,LOW);
      //digitalWrite(LIGHT_PIN_ESP32CAM, HIGH);
    }
  }else if("off"){//to turn off 14
    if(lightStatus=="on"){
      byte b=3;
      HC12.write(b);
    }else{
      digitalWrite(LIGHT_PIN,HIGH);
      //digitalWrite(LIGHT_PIN_ESP32CAM, LOW);
    }
  }
  
}

void servoCamStop(){
  camSide.write(90);
  goingDown=false;
  goingUp=false;
}
void upCam(){
  camAngleValue-=3;
  camAngle.write(camAngleValue);
  goingUp=true;
}
void downCam(){
  camAngleValue+=3;
  camAngle.write(camAngleValue);
  goingDown=true;
}
void turnCamRight(){
  camSide.write(1400);
}
void turnCamLeft(){
  camSide.write(100);//doljan servo e continous , goran ne e
}

void gasCheck() {
  static int lastGasState = -1; 
  static unsigned long lastChangeTime = 0;
  const unsigned long stableTime = 30; 
  static int confirmedGasState = -1; 
  int gas = digitalRead(GAS_PIN); 
  if (gas != lastGasState) { 
    lastChangeTime = millis(); 
    lastGasState = gas; 
  }if ((millis() - lastChangeTime) > stableTime && gas != confirmedGasState) {
    byte b = gas ? 1 : 0; 
    HC12.write(b);
    confirmedGasState = gas; 
  }
}//uglavnom provertue dali stvarno se smenila vrednost i prakja ako se ako ne nisto

void dhtCheck(){
  static long nowTime=0;
  const long long neededTime=15000;
  temp= dht.readTemperature();
  hum = dht.readHumidity();
  byte tempB=temp;
  byte humB=hum;

  if((millis()-nowTime)>neededTime){
    HC12.write(tempB);
    HC12.write(humB);
    nowTime=millis();
  }
}//proverue temperaturu i vlazhnost

void changeSpeed(String s){//menja brzinu ss toj sto dobiva info o kontroler preko serial gore i u zavisnost sto dobie menja gu vrednost
  if(s=="slow"){
    speed=170;
  }else if(s=="medium"){
    speed=210;
  }else{
    speed=255;
  }
}