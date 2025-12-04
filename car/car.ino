#include <ESP32Servo.h>
#include <DHT.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

long long startTimeSlow=millis();

String carMode="Manual";

const int DHT_PIN = 33;
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);
int hum, temp;

const int LIGHT_STATUS_PIN = 34; 
const int LIGHT_PIN = 15;        
String lightStatus = "off";

const int GAS_PIN = 35; 
bool gasgas = false;

Servo camSide, camAngle;
int camAngleValue = 90;
const int CAM_SIDE_PIN = 26;  
const int CAM_ANGLE_PIN = 25;
bool goingUp = false, goingDown = false;

const int trigPin = 4;
const int echoPinF=2, echoPinR=5, echoPinL = 39; 
float distanceF, distanceL, distanceR;
bool comingOut = false;

long long elapsedTime, startTime = 0;
bool changed = false;

int checkGroundPin = 23;
int checkGroundValue;
bool checkSidesGround = false;
long long startTimeGround = 0;

const int la = 19, lb = 18, ra = 21, rb = 22;//const int la = 19, lb = 18, ra = 21, rb = 22;
int speed = 210;
String currentSpeed="medium";
const int speedPin=13;
byte previousMotor;
bool first=true;

int checkRainPin = 12;
int rainValue;
Servo rainServo;
int rainServoPin = 27;

int moistureServoPin = 14;
Servo moistureServo;
int MOISTURE_SENSOR_PIN = 36;
int moistureValue = 0;

HardwareSerial HC12(1); 
#define HC12_TX 17 
#define HC12_RX 16

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len > 0) {
    int8_t command = data[0];
    byte d=command;
    checkData(d);
    Serial.println(d);
  }
}

void setup() { 
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }
  esp_now_register_recv_cb(onReceive);

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

  pinMode(LIGHT_STATUS_PIN, INPUT); 
  pinMode(LIGHT_PIN, OUTPUT); 
  digitalWrite(LIGHT_PIN, HIGH); 

  pinMode(GAS_PIN, INPUT);
  
  rainServo.attach(rainServoPin);
  moistureServo.attach(moistureServoPin);
  rainServo.write(0);
  HC12.begin(9600, SERIAL_8N1, HC12_RX, HC12_TX);

  pinMode(speedPin, OUTPUT); 
  analogWrite(speedPin,127);

  dht.begin();
  motorStop();
}

void loop() {
  if (HC12.available()>0) {//ako ima dobieno info od kontroler chita se i se pravi toj sto znachi svak byte
    changed=false;
    int x = HC12.read();  
    byte data=x;
    checkData(data);
    Serial.println(data);        
  }else{
    gasCheck();//proverue za gas i prakja ga na kontroler
    dhtCheck();//kd ke stigne tga
    //provere tuja zatoj sto ako nema primeno vrednostr ondak mozhe da prati, uglavnom ne
    //mozhe da prima i prakja u isto vreme zatoj e takoj
  }
  lightCheck("sensor");//proverue dali ima svetlo ako enma pali ga

  if(carMode=="Automatic" && millis()-startTimeGround>500){
    obstacleChecker("front bottom");
    if(checkGroundValue == 1){
      backwards();
      if(!checkSidesGround){
        obstacleChecker("right left");
        checkSidesGround=true;
        startTimeGround=millis();
      }
      if(distanceL <= distanceR){
        turnRight();
      }else if(distanceL >= distanceR){
        turnLeft();
      }
    }else if(distanceF <= 20 && distanceF!=0){
      checkSidesGround=false;
      obstacleChecker("right left");//podeleno e za da ne se troshi vreme i da se vika sve ako distanceF e povishe od 20cm
      if(distanceL<=7 && distanceR<=7){//proverue dali ulaza u slep sokak za da mozhe da iskochi nanad  zatoj sto nema mesto da vrti
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
      checkSidesGround=false;
      forward();
    }
  }else if(carMode=="Manual" && changed){
    motorStop();
  }//changed e za dugme na samoto avtiche za da se menja mod
  //ako nema changed tuj i u serial gore ondak nema da dava da se upravlja preko kontroler zatoj sto celo vreme ke se pushta motorStop()
}

void obstacleChecker(String s){
  if(s=="front bottom"){
    digitalWrite(trigPin, HIGH);//na ist nachin se chita od svi tri ultrasonic senzori i se pretvara u cm
    delayMicroseconds(20);
    digitalWrite(trigPin, LOW);
    distanceF = 0.017 * pulseIn(echoPinF, HIGH);
    delay(50);
    checkGroundValue=digitalRead(checkGroundPin);//bottom check with ir sensor
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
  Serial.println(checkGroundValue);
  Serial.println();
  Serial.println();*/
}

void turnRight() {
  digitalWrite(la, HIGH);
  digitalWrite(lb, LOW);
  digitalWrite(ra, LOW);
  digitalWrite(rb, HIGH);
  
  if (carMode == "Automatic") {
    if (speed == 180) delay(500);
    else if (speed == 210) delay(800);
    else delay(1000); // When speed is 255
  }
}

void turnLeft() {
  digitalWrite(la, LOW);
  digitalWrite(lb, HIGH);
  digitalWrite(ra, HIGH);
  digitalWrite(rb, LOW);
  
  if (carMode == "Automatic") {
    if (speed == 180) delay(500);
    else if (speed == 210) delay(800);
    else delay(1000); // When speed is 255
  }
}

void backwards() {
  digitalWrite(la, HIGH);
  digitalWrite(lb, LOW);
  digitalWrite(ra, HIGH);
  digitalWrite(rb, LOW);
}

void forward() {
  digitalWrite(la, LOW);
  digitalWrite(lb, HIGH);
  digitalWrite(ra, LOW);
  digitalWrite(rb, HIGH);
} 

void motorStop() {
  digitalWrite(la, LOW);
  digitalWrite(lb, LOW);
  digitalWrite(ra, LOW);
  digitalWrite(rb, LOW);
}

void lightCheck(String s){
  int light=digitalRead(LIGHT_STATUS_PIN);
  if(s=="sensor"){   
    if(light==1 && lightStatus=="off"){
        delay(30);
        digitalWrite(LIGHT_PIN,LOW);//svetlo se pali na low a gasi se na high
        lightStatus="on";
    }else if(light==0 && lightStatus=="on"){
        delay(30);
        digitalWrite(LIGHT_PIN,HIGH);
        lightStatus="off";
    }
  }else if(s=="on"){//to turn on 13
    if(lightStatus=="on"){
      byte b=2;
      HC12.write(b);
    }else{
      digitalWrite(LIGHT_PIN,LOW);
    }
  }else if(s=="off"){//to turn off 14
    if(lightStatus=="on"){
      byte b=3;
      HC12.write(b);
    }else{
      digitalWrite(LIGHT_PIN,HIGH);
    }
  }
}

void servoCamStop(){
  camSide.write(90);
}
void upCam(){
  camAngleValue-=3;
  camAngle.write(camAngleValue);
}
void downCam(){
  camAngleValue+=3;
  camAngle.write(camAngleValue);
}
void turnCamRight(){
  camSide.write(1400);
}
void turnCamLeft(){
  camSide.write(100);//doljan servo e continous , goran ne e
}

void gasCheck() {
  /*int gasAnalog=analogRead(GAS_PIN_A);
  int gas;
  //Serial.println(gas);
  if(gasAnalog>250 && gasgas==false){
    gas=1;
    gasgas=true;
  }else if(gasAnalog<250 && gasgas==true){
    gas=0;
    gasgas=false;
  }*/
  static int lastGasState = -1; 
  static unsigned long lastChangeTime = 0;
  const unsigned long stableTime = 30; 
  static int confirmedGasState = -1; 
  int gas = digitalRead(GAS_PIN); 
  if (gas != lastGasState) { 
    lastChangeTime = millis(); 
    lastGasState = gas; 
  }if ((millis() - lastChangeTime) > stableTime && gas != confirmedGasState) {
    byte b = gas ? 0 : 1; 
    HC12.write(b);
    Serial.println(b);
    confirmedGasState = gas; 
  }
}//uglavnom provertue dali stvarno se smenila vrednost i prakja ako se ako ne nisto

void dhtCheck(){
  static long nowTime=0;
  const long long neededTime=5000;
  temp= dht.readTemperature();
  hum = dht.readHumidity();
  byte tempB=temp;
  byte humB=hum;

  if((millis()-nowTime)>neededTime){
    HC12.write(tempB);
    HC12.write(humB);
    nowTime=millis();
  }
 /*Serial.println(temp);
 Serial.println(hum);*/
}//proverue temperaturu i vlazhnost

void changeSpeed(String s){//menja brzinu ss toj sto dobiva info o kontroler preko serial gore i u zavisnost sto dobie menja gu vrednost
  if(s=="slow"){
    speed=80;
    currentSpeed="slow";
  }else if(s=="medium"){
    speed=150;
    currentSpeed="medium";
  }else{
    speed=255;
    currentSpeed="high";
  }
  analogWrite(speedPin, speed);
}
void rainCheck(){
  rainServo.write(90);
  delay(2000);
  rainValue=analogRead(checkRainPin);
  Serial.println(rainValue);
  delay(2000);
  rainServo.write(0);
  if(rainValue<250){
    byte send=101;
    HC12.write(send);//wet
  }else{
    byte send=102;
    HC12.write(send);//dry
  }
}

void groundCheck(){
  moistureServo.write(130);
  delay(1200);
  moistureServo.write(90);
  delay(3000);
  moistureValue=analogRead(MOISTURE_SENSOR_PIN);
  Serial.println(moistureValue);
  delay(1000);
  moistureServo.write(50);
  delay(1200);
  moistureServo.write(90);
}

void checkData(byte data){
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
    else if(data==18) rainCheck();
    else if(data==19) groundCheck();
    else if(carMode=="Manual"){
      previousMotor=data;
      if(data==3) motorStop();//stop
      else if(data==4) forward();//forward
      else if(data==5) backwards();//backward
      else if(data==6) turnRight();//right 
      else if(data==7) turnLeft();//left
      if((previousMotor==3 && currentSpeed=="slow")){//starating- povagja od pgolemo i namalue
        analogWrite(speedPin,150);
        motorStop();
        if(data==4) forward();//forward
        else if(data==5) backwards();//backward
        else if(data==6) turnRight();//right 
        else if(data==7) turnLeft();//left
        startTimeSlow=millis();
        first=true;
        Serial.println("sg pochnue");
      }
      Serial.println(previousMotor);
      Serial.println(currentSpeed);
      Serial.println(data);
      if(millis()-startTimeSlow>700 && first){
        analogWrite(speedPin,80);
        first=false;
        Serial.println("sg zavrshue");
      }
    }
    else motorStop();
}

