#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_MPU6050 mpu;
uint8_t carAddress[] = {0x00, 0x4B, 0x12, 0xEB, 0xEB, 0xCC}; // REPLACE with your car ESP MAC
int8_t command = 0;
int lastCommand=1;

int buttonPin1=3, buttonPin2=4;
String mode="drive";
long long startTime=0, startTime2=0, startTime3=0, startTime4=0;

String modeCar="manual";
String light="off";
String speed="medium";

bool changedBack=true;

Adafruit_SSD1306 display(128, 32, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(7, 6); // SDA,SCL

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found. Check wiring.");
    while (1);
  }

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, carAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.begin(115200);
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don’t continue, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2);             // Big text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(32, 0);
  display.setRotation(2);
  display.println(F("Mode:"));
  display.setCursor(30, 16);
  display.println(F("Drive"));
  display.display();  

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  Serial.println("Hand ESP Ready");
}

void loop() {
  if(millis()-startTime4>3000 && changedBack==false){
    display.clearDisplay();
    display.setCursor(32, 0);
    display.println(F("Mode:"));
    if(mode=="drive"){
      display.setCursor(30, 16);
      display.println(F("Drive"));
    }else if(mode=="cam"){
      display.setCursor(25, 16);
      display.println(F("Camera"));
    }else if(mode=="functions"){
      display.setCursor(10, 16);
      display.println(F("Functions"));
    }
    display.display();  
    changedBack=true;
  }
  
  checkButtons();
  checkingAngle();

  if(command==9 || command==10){
    Serial.println(command);
    esp_now_send(carAddress, (uint8_t*)&command, sizeof(command));
  }else if(command!=lastCommand && command!=0){
    lastCommand=command;
    Serial.println(command);
    esp_now_send(carAddress, (uint8_t*)&command, sizeof(command));
  }
  

}

void checkButtons(){
  int button1=digitalRead(buttonPin1);
  int button2=digitalRead(buttonPin2);
  if(button1==LOW && millis()-startTime>400 && mode!="functions"){
    display.clearDisplay();
    display.setCursor(32, 0);
    display.println(F("Mode:"));
    startTime=millis();
    if(mode=="drive"){
      display.setCursor(25, 16);
      display.println(F("Camera"));
      mode="cam";
    }else if(mode=="cam"){
      display.setCursor(30, 16);
      display.println(F("Drive"));
      mode="drive";
    }
    display.display();
  }
  if(button2==LOW && millis()-startTime2>400 && mode!="cam"){
    display.clearDisplay();
    display.setCursor(32, 0);
    display.println(F("Mode:"));
    startTime2=millis();
    if(mode=="drive"){
      display.setCursor(10, 16);
      display.println(F("Functions"));
      mode="functions";
    }else if(mode=="functions"){
      display.setCursor(30, 16);
      display.println(F("Drive"));
      mode="drive";
    }
    display.display(); 
  }
}

void checkingAngle(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  if(mode=="drive"){
    if (a.acceleration.y > 5.0) command = 5;  // Backward
    else if (a.acceleration.y < -5.0) command = 4;  // Forward
    else if (a.acceleration.x < -5.0) command = 6;  // Right
    else if (a.acceleration.x > 5.0)  command = 7;  // Left
    else command = 3;  // Neutral
  }else if(mode=="cam"){
    if (a.acceleration.y > 5.0) command = 10;  // cam Backward
    else if (a.acceleration.y < -5.0) command = 9;  //  cam Forward
    else if (a.acceleration.x < -5.0) command = 11;  // cam Right
    else if (a.acceleration.x > 5.0)  command = 12;  // cam Left
    else command = 8;  // Neutral cam
  }else if(mode=="functions" && millis()-startTime3>700){
    startTime3=millis();
    if (a.acceleration.y > 5.0) command=18;//rain check
    else if (a.acceleration.y < -5.0){
      display.clearDisplay();
      display.setCursor(10, 0);
      display.println(F("Car Mode:"));
      startTime4=millis();
      changedBack=false;
      if(modeCar=="manual"){
        modeCar="automatic";
        command=1;
        display.setCursor(10, 16);
        display.println(F("Automatic"));
      }else{
        modeCar="manual";
        command=2;  
        display.setCursor(10, 16);
        display.println(F("Manual"));
      }
      display.display();
    }
    else if (a.acceleration.x < -5.0){
      display.clearDisplay();
      display.setCursor(50, 0);
      display.println(F("Light:"));
      startTime4=millis();
      changedBack=false;
      if(light=="off"){
        light="on";
        command=13;
        display.setCursor(50, 16);
        display.println(F("ON"));
      }else{
        light="off";
        command=14;
        display.setCursor(45, 16);
        display.println(F("OFF"));
      }
      display.display();
    }
    else if (a.acceleration.x > 5.0){
      display.clearDisplay();
      display.setCursor(50, 0);
      display.println(F("Speed:"));
      startTime4=millis();
      changedBack=false;
      if(speed=="medium"){
        speed="high";
        command=16;
        display.setCursor(50, 16);
        display.println(F("High"));
      }else if(speed=="high"){
        speed="slow";
        command=17;
        display.setCursor(50, 16);
        display.println(F("Slow"));
      }else{
        speed="medium";
        command=15;
        display.setCursor(45, 16);
        display.println(F("Medium"));
      }
      display.display();
    }
  }
}
