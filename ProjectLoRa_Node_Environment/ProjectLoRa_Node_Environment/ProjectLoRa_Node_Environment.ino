#include "Recieve.h"
#include "DHT.h"
#include <Wire.h>
#include "BH1750.h"


BH1750 lightMeter;

#define DHTPIN 2
#define DHTTYPE DHT21
#define LED_INDICATOR    3

volatile byte z = 0;
//bool stateLED = 0;
byte acceptLoRa = 0;
unsigned long timeOut = millis();
int request = 0;
int serialCommand = 0;
byte LoraCommand = 0;
byte typeCommand = 0;
byte numberNode = 0;
String messenger;
extern uint8_t bufferLoRa[25];
volatile int a = 0;
byte role1,role2,role3;
float temperature = 0.0;
float humidity = 0.0;
uint16_t light = 0.0;

//Command
#define   JOIN                          1
#define   ACCEPT                        2
#define   CONFIGURE                     3
#define   IMFORMATION                   4
#define   STATE                         5
#define   TEMPERATURE                   6
#define   LIGHT                         7
#define   HUMIDITY                      8
#define   GET_STATE                     5
#define   DATA_RATE                     9
#define   SET_STATE                     10
#define   CONTROL                       10
#define   BATTERY                       14
#define   QUIT                          15
#define   POWER                         16
#define   FREQ_JOIN_ENVIRONMENT         17
#define   FREQ_JOIN_CONTROL             18 

DHT dht(DHTPIN, DHTTYPE);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_INDICATOR, OUTPUT);
  digitalWrite(LED_INDICATOR, 0);
  while (!Serial);
  Serial.println("Node Environment");

  //sửa lại hàm này để kiểm tra node có hoạt động ko
  dht.begin();
  lightMeter.begin();
  
  if (!LoRa.begin(436E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //  LoRa.setTxPower(17);
  LoRa.setSpreadingFactor(10);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
}

void loop() {
  /* Phần gia nhập mạng */
  // các biến sử dụng:
  //      _ acceptLoRa (0/1) : báo gia nhập mạng
  //      _ countResend [0;5] : số lượng lần gửi lặp lại
  //      _ availableSensor [2-array]: quản lý sensor trên từng Node
  //      _ id_Node [1-array] : lưu địa chỉ ID Node
  //      _ typeNode [1-array] : lưu loại các Node hiện có
  //      _ powerTransmitNode [1_array]: 

  //----------------------------------------- Phần Code
//  LoRa.setFrequency(436E6);
//  LoRa.setSyncWord(44);
  volatile uint8_t countResend = 0;
  uint8_t errorFrame = 99;
  volatile uint8_t packet = 0;
  uint8_t randomTime = 0;
  while(acceptLoRa == 0)
  {
    timeOut = millis();
    Serial.println("Gui ban tin");
    Serial.print("timeOut: ");
    Serial.println(timeOut);
    LoRa.beginPacket();
    LoRa.print("@;0;1;J:1,1,1,0;!");
    LoRa.endPacket();
    while((millis() - timeOut) < 10000){
      packet = LoRa.parsePacket();
      if(packet > 0){
        Serial.println("Co ban tin");
        readBufferLoRa(bufferLoRa);
        errorFrame = checkFrame(bufferLoRa);
        Serial.print("errorFrame: ");
        Serial.println(errorFrame);
        if(errorFrame == 0){
          LoraCommand = bufferLoRa[6];
          if(LoraCommand == 'A'){
            acceptLoRa = 1;
            Serial.println("Frequency 434Mhz");
            LoRa.setFrequency(434E6);
            delay(100);
          }
        }
      }
    }
    Serial.println("Gui lai ban tin");
    randomTime = random(4,5);
//    delay(randomTime * 1000);
    delay(5000);
  }
//  Serial.println("Frequency 434Mhz");

  
  // put your main code here, to run repeatedly:
//  if (acceptLoRa == 0){
//    if((millis() - timeOut) > 5000){
//      messenger = "@;0;1;J:7,17,81,1,1,0;!";
//      LoRa.beginPacket();
//      LoRa.print(messenger);
//      LoRa.endPacket();
//      timeOut = millis();
//    }
//  }



/* Phần xử lý bản tin */
  a = LoRa.parsePacket();
  if(a > 0){
//    a = 0;
    readBufferLoRa(bufferLoRa);
    uint8_t b = checkFrame(bufferLoRa);
    Serial.print("checkFrame: ");
    Serial.println(b);
    if(b == 0){
      numberNode = bufferLoRa[4];
      LoraCommand = bufferLoRa[6];
      typeCommand = bufferLoRa[8];
      Serial.println(" ***** Ban tin LoRa ******");
      Serial.print("Lenght: ");
      Serial.println(sizeBufferLora());
      Serial.print("Payload: ");
      for(byte i = 10; i< (sizeBufferLora()-2); i++){
        Serial.print(char(bufferLoRa[i]));
      }
      Serial.println();
      Serial.print("command: ");
      Serial.println(char(LoraCommand));
      Serial.print("Loai lenh: ");
      Serial.println("Gateway -> Node");
      z = readCommandLora(LoraCommand);
      Serial.print("type command: ");
      Serial.println(z);
      for(byte i = 0; i< (LoraCommand -70); i++){
        digitalWrite(LED_INDICATOR, HIGH);
        delay(100);
        digitalWrite(LED_INDICATOR, LOW);
        delay(100);
      }
//      digitalWrite(LED_INDICATOR, LOW);
//    delay(3000);
      byte c = readCommandLora(LoraCommand);
      switch(c){
        case ACCEPT:
          acceptLoRa = 1;
          break;
  //      case CONTROL:
  //          role1 = bufferLoRa[10];
  //          role2 = bufferLoRa[12];
  //          role3 = bufferLoRa[14];
  //        break;
        case TEMPERATURE:
            messenger = "@;0;1;T:";
            temperature = dht.readTemperature();
            messenger += temperature;
            messenger += ";!";
            LoRa.beginPacket();
            LoRa.print(messenger);
            LoRa.endPacket();
          break;
        case LIGHT:
            messenger = "@;0;1;L:";
            light = lightMeter.readLightLevel();
            messenger += light;
            messenger += ";!";
            LoRa.beginPacket();
            LoRa.print(messenger);
            LoRa.endPacket();
          break;
        case HUMIDITY:
            messenger = "@;0;1;H:";
            humidity = dht.readHumidity();
            messenger += humidity;
            messenger += ";!";
            LoRa.beginPacket();
            LoRa.print(messenger);
            LoRa.endPacket();
          break;
          
        default:
            
          break;
      }
    }
  }
}

uint8_t readCommandLora(uint8_t Symbol){
  int command = -1;
  switch (Symbol){
    case 'J':
      command = JOIN;
      break;
    case 'A':
      command = ACCEPT;
      break;
    case 'F':
      command = CONFIGURE;
      break;
    case 'I':
      command = IMPORMATION;
      break;
    case 'S':
      command = STATE;
      break;
    case 'T':
      command = TEMPERATURE;
      break;
    case 'L':
      command = LIGHT;
      break;
    case 'H':
      command = HUMIDITY;
      break;
    case 'C':
      command = CONTROL;
      break;
    default:
      command = 0;
  }
  return command;
}
///////////////////
/*
void createMessenger(byte ){
  messenger += command;
  messenger += ";S:";
  messenger += ;
}

void LoRaSender(String messenger){
  LoRa.beginPacket();
  LoRa.print(messenger);
  LoRa.endPacket();
}

void LoRaRecieve(){
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
  while (LoRa.available()) {
    Serial.print((char)LoRa.read());
  }
}

*/
