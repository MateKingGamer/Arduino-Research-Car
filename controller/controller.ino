#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 2);

const int SWITCH_BUTTON_PIN=6;
const int LIGHT_BUTTON_PIN=7;
const int SPEED_BUTTON_PIN=5;
const int RAIN_BUTTON_PIN=9;
const int GROUND_BUTTON_PIN=8;

const int X_PIN=A2, Y_PIN=A3;
int xValue, yValue;
byte movementSend=3;
bool movementChanged=false;

const int CAM_X_PIN=A1, CAM_Y_PIN=A0;
int xCamValue,yCamValue;
byte camSend=8;
bool camChanged=false;

String carMode="Manual";
int buttonMode=0;

int lightButton=0;
String currentLightMode="off";

int speedButton=0;
String currentSpeed="Medium";

int rainButton=0;
long long lastRainButton=0;

int groundButton=0;
long long lastGroundButton=0;

byte send=0;

String gasState="gas_low";
long long startTime,neededTime=4000,elapsedTime;
int counter=0;

float temperature=0, humidity=0;
SoftwareSerial HC12(3, 2); // TX, RX

long long lastLightButton=0, lastSpeedButton=0, lastModeButton=0;

void setup() { 
  lcd.init();                      
  lcd.backlight();
  lcd_write("car_mode");

  pinMode(SWITCH_BUTTON_PIN, INPUT);
  pinMode(LIGHT_BUTTON_PIN, INPUT);
  pinMode(SPEED_BUTTON_PIN, INPUT);
  pinMode(RAIN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(GROUND_BUTTON_PIN, INPUT_PULLUP);

  HC12.begin(9600);
  Serial.begin(9600);
}

void loop() {
  elapsedTime=millis()-startTime;
  if(elapsedTime>neededTime && startTime!=0){
    if(gasState=="gas_high" )
      lcd_write(gasState);
    else{
      if(counter==0){
        lcd_write(gasState);
        counter=1;
      }else if(counter==1){
        lcd_write("temperature");
        counter=2;
      }else if(counter==2){
        lcd_write("humidity");
        counter=0;
      } 
    }
    startTime=millis();
  }//lcd writer for info
  
  delay(100);
  if(HC12.available()>0){ 
    int x=HC12.read();
    Serial.println(x);
    if(x==0){
      gasState="gas_low";
      lcd_write(gasState);
    }else if(x==1){
      gasState="gas_high";
      lcd_write(gasState);
    }else if(x>15 && x<35){
      temperature=x;
    }else if(x>=35 && x<100){
      humidity=x;
    }else if(x==2){// kd e vekj ukljucheno svetlo na avtiche od sensor
      lcd_write("error_1");
    }else if(x==3){//kd ne mozhe da se iskljuchi zatoj sto e tmno
      lcd_write("error_2");
      currentLightMode="on";
    }else if(x==101){
      lcd_write("raining");
    }else if(x==102){//raincheck
      lcd_write("not_raining");
    }else if(x==103){
      lcd_write("dry_ground");
    }else if(x==104){//groundcheck
      lcd_write("wet_ground");
    }
  }
  
  xValue=analogRead(X_PIN);
  yValue=analogRead(Y_PIN);

  xCamValue=analogRead(CAM_X_PIN);
  yCamValue=analogRead(CAM_Y_PIN);
  /*Serial.println(xValue);
  Serial.println(yValue);
  Serial.println(xCamValue);
  Serial.println(yCamValue);*/

  buttonMode=digitalRead(SWITCH_BUTTON_PIN);//reading values
  lightButton=digitalRead(LIGHT_BUTTON_PIN);
  speedButton=digitalRead(SPEED_BUTTON_PIN);
  rainButton=digitalRead(RAIN_BUTTON_PIN);
  groundButton=digitalRead(GROUND_BUTTON_PIN);

  if(lightButton==1 && millis()-lastLightButton>300){
    if(currentLightMode=="off"){
      send=13;
      currentLightMode="on";
      lcd_write("light_on");
    }else if(currentLightMode=="on"){
      send=14;
      currentLightMode="off";
      lcd_write("light_off");
    }
    HC12.write(send);
    lastLightButton=millis();
  }//turn on/off car light

  if(speedButton==1 && millis()-lastSpeedButton>300){
    if(currentSpeed=="Low"){
      currentSpeed="Medium";
      send=15;
      lcd_write("speed_medium");
    }else if(currentSpeed=="Medium"){
      send=16;
      currentSpeed="High";
      lcd_write("speed_high");
    }else if(currentSpeed=="High"){
      send=17;
      currentSpeed="Low";
      lcd_write("speed_low");
    }
    HC12.write(send);
    lastSpeedButton=millis();
  }//change speed

  if(buttonMode==1 && millis()-lastModeButton>300){
    if(carMode=="Automatic"){
      carMode="Manual";
      send=2;
    }else{
      carMode="Automatic"; 
      send=1;     
    }
    HC12.write(send);
    lcd.clear();
    lcd_write("car_mode");
    lastModeButton=millis();
  }

  if(rainButton==0 && millis()-lastRainButton>1000){
    send=18;
    HC12.write(send);
    Serial.println("rain");
    lastRainButton=millis();
  }//for rain sensor

  if(groundButton==0 && millis()-lastGroundButton>1000){
    send=19;
    HC12.write(send);
    Serial.println("ground");
    lastGroundButton=millis();
  }//for ground sensor

  if(carMode=="Manual"){
    movementChanged=false;
    if(yValue>400 && yValue<700 && xValue>400 && xValue<700 && movementSend!=3){
      movementSend=3;
      movementChanged=true;
    }else if(yValue>700 && movementSend!=4){
      movementSend=5;
      movementChanged=true;
    }else if(yValue<300 && movementSend!=5){
      movementSend=4;
      movementChanged=true;
    }else if(xValue>700 && yValue>400 && yValue<700 && movementSend!=6){
      movementSend=7;
      movementChanged=true;
    }else if(xValue<300 && yValue>400 && yValue<700 && movementSend!=7){
      movementSend=6;
      movementChanged=true;
    }
    if(movementChanged){
      HC12.write(movementSend);
      Serial.println(movementSend);
    }//sistem za da uglavnom ne prakja celo vreme nego samo ako nova vrednost e razlichna od proshlu

  }//prakja info za mrdanje ako e manual mode

  camChanged=false;
  if(yCamValue>400 && yCamValue<700 && xCamValue>400 && xCamValue<700 && camSend!=8){
    camSend=8;
    camChanged=true;
  }else if(yCamValue>700){
    camSend=10;
    HC12.write(camSend);
  }else if(yCamValue<300){
    camSend=9;
    HC12.write(camSend);
  }else if(xCamValue>700 && yCamValue>400 && yCamValue<700 && camSend!=11){
    camSend=12;
    camChanged=true;
  }else if(xCamValue<300 && yCamValue>400 && yCamValue<700 && camSend!=12){
    camSend=11;
    camChanged=true;
  }
  if(camChanged){
      HC12.write(camSend);
      Serial.println(camSend);
  }
}

void lcd_write(String s){
  if(s=="car_mode"){
    lcd.setCursor(2,0);
    lcd.print("Current Mode: ");
    if(carMode=="Manual"){
      lcd.setCursor(5,1);
    }else{
      lcd.setCursor(3,1);
    }
    lcd.print(carMode);
    startTime=millis();
  }else if(s=="gas_high"){
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("WARNING!");
    lcd.setCursor(1,1);
    lcd.print("HIGH GAS LEVEL");
  }else if(s=="gas_low"){
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("SECURE!");
    lcd.setCursor(0,1);
    lcd.print("NO GAS DETECTED");
  }else if(s=="speed_low"){
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Current Speed: ");
    lcd.setCursor(6,1);
    lcd.print(currentSpeed);
    startTime=millis();
  }else if(s=="speed_medium"){
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Current Speed: ");
    lcd.setCursor(5,1);
    lcd.print(currentSpeed);
    startTime=millis();
  }else if(s=="speed_high"){
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Current Speed: ");
    lcd.setCursor(6,1);
    lcd.print(currentSpeed);
    startTime=millis();
  }else if(s=="error_1"){
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("DARK OUTSIDE");
    lcd.setCursor(0,1);
    lcd.print("LIGHT ALREADY ON");
    startTime=millis();
  }else if(s=="error_2"){
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("DARK OUTSIDE");
    lcd.setCursor(0,1);
    lcd.print("CANT TURN OFF");
    startTime=millis();
  }else if(s=="temperature"){
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Temperature: ");
    lcd.setCursor(4,1);
    lcd.print(temperature);
    lcd.setCursor(9,1);
    lcd.print((char)223);
    lcd.setCursor(10,1);
    lcd.print("C");
  }else if(s=="humidity"){
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Air Humidity: ");
    lcd.setCursor(5,1);
    lcd.print(humidity);
    lcd.setCursor(10,1);
    lcd.print("%");
  }else if(s=="light_on"){
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Light State: ");
    lcd.setCursor(7,1);
    lcd.print("ON");
    startTime=millis();
  }else if(s=="light_off"){ 
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Light State: ");
    lcd.setCursor(6,1);
    lcd.print("OFF");
    startTime=millis();
  }else if(s=="raining"){ 
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Rain Status:");
    lcd.setCursor(4,1);
    lcd.print("RAINING");
    startTime=millis();
  }else if(s=="not_raining"){ 
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Rain Status:");
    lcd.setCursor(2,1);
    lcd.print("NOT RAINING");
    startTime=millis();
  }else if(s=="dry_ground"){ 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Moisture Status:");
    lcd.setCursor(6,1);
    lcd.print("DRY");
    startTime=millis();
  }else if(s=="wet_ground"){ 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Moisture Status:");
    lcd.setCursor(6,1);
    lcd.print("WET");
    startTime=millis();
  }
}
