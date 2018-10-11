#ifndef DEBUG
  #define DEBUG
#endif

#include "Recieve.h"

byte acceptLoRa = 0;
unsigned int timeOut = millis();
int serialCommand = 0;
byte LoraCommand = 0;
byte typeCommand = 0;
byte numberNode = 0;
String messenger;
extern uint8_t bufferLoRa[25];
volatile int a = 0;
byte role1,role2,role3;
byte typeRole, stateRole;
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
#define   _OK                           11
#define   BATTERY                       14
#define   QUIT                          15
#define   POWER                         16
#define   FREQ_JOIN_ENVIRONMENT         17
#define   FREQ_JOIN_CONTROL             18               

//Relay
#define RELAY_1         48
#define RELAY_2         46
#define RELAY_3         44
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Node Control");
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  digitalWrite(RELAY_1, HIGH);
//  delay(3000);
//  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  
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
//  LoRa.setFrequency(434E6);
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
    LoRa.print("@;0;2;J:1,1,1;!");
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
          if(LoraCommand == 'A')
            acceptLoRa = 1;
            LoRa.setFrequency(434E6);
        }
        else{
          Serial.print("Loi ban tin!!!");
        }
      }
    }
    Serial.println("Gui lai ban tin");
    randomTime = random(4,5);
    delay(randomTime * 1000);
//    delay(3000);
  }

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
  a = LoRa.parsePacket();
  if(a > 0){
//    a = 0;
    readBufferLoRa(bufferLoRa);
    uint8_t b = checkFrame(bufferLoRa);
    Serial.println(b);
    if(b == 0){
      LoraCommand = bufferLoRa[6];
//      typeCommand = bufferLoRa[8];
      numberNode = bufferLoRa[4];
      typeRole = bufferLoRa[8];
      stateRole = bufferLoRa[10];
      
#ifdef DEBUG
      Serial.println(" ***** Ban tin LoRa ******");
      Serial.print("Type Node recieved: ");
      switch(numberNode){
        case '1':
            Serial.println("Environment");
          break;
        case '2':
            Serial.println("Control");
          break;
        default:
            Serial.println("Type node WRONG!!!");
          break;
      }
      Serial.print("Lenght: ");
      Serial.println(sizeBufferLora());
      Serial.print("Payload: ");
      for(byte i = 8; i< (sizeBufferLora()-2); i++){
        Serial.print(char(bufferLoRa[i]));
      }
      Serial.println();
      Serial.print("command: ");
      Serial.println(char(LoraCommand));
      Serial.print("Loai lenh: ");
      Serial.println("Gateway -> Node");
      Serial.print("Control: ");
      
      switch(typeRole){
        case '1':
            Serial.print("Role 1");
          break;
        case '2':
            Serial.print("Role 2");
          break;
        case '3':
            Serial.print("Role 3");
          break;
        default:
            Serial.print("Sai role");
          break;
      }
      Serial.print("; State: ");
      switch(stateRole){
        case 'O':
            Serial.println("ON");
          break;
        case 'F':
            Serial.println("OFF");
          break;
        default:
            Serial.print("Sai trang thai: ");
            Serial.println(char(stateRole));
          break;
      }
#endif
    
      byte c = readCommandLora(LoraCommand);
      Serial.print("c: ");
      Serial.println(c);
      switch(c){
        // accept Node
        case ACCEPT:
          acceptLoRa = 1;
          break;
  
        // control role
        case SET_STATE:
            // select role
            switch(typeRole){
            case '1':
                Serial.println("1");
                if(stateRole == 'O'){
                  role1 = 1;
                  digitalWrite(RELAY_1, HIGH);
                }
                if(stateRole == 'F'){
                  role1 = 0;
                  digitalWrite(RELAY_1, LOW);
                }
              break;
            case '2':
                Serial.println("2");
                if(stateRole == 'O'){
                  role2 = 1;
                  digitalWrite(RELAY_2, HIGH);
                }
                if(stateRole == 'F'){
                  role2 = 0;
                  digitalWrite(RELAY_2, LOW);
                }
              break;
            case '3':
                Serial.println("3");
                if(stateRole == 'O'){
                  role3 = 1;
                  digitalWrite(RELAY_3, HIGH);
                }
                if(stateRole == 'F'){
                  role3 = 0;
                  digitalWrite(RELAY_3, LOW);
                }
              break;
              
            default:
              break;
            }
            
            LoRa.beginPacket();
            LoRa.print("@;0;2;O:ok;!");
            LoRa.endPacket();
          break;
  
        // get state role
        case GET_STATE:
            Serial.println("Get_state...");
            messenger = "@;0;2;G:";
            messenger += char(typeRole);
            messenger += ',';
            switch(typeRole){
              case '1':
                  messenger += role1;
                break;
              case '2':
                  messenger += role2;
                break;
              case '3':
                  messenger += role3;
                break;
                
              default:
                break;
            }
            
            messenger += ";!";
            //truyen ban tin
            LoRa.beginPacket();
            LoRa.print(messenger);
            LoRa.endPacket();
            Serial.println(messenger);
          break;
        
        // no type command
        default:
            
          break;
      }
    }
  }
}

uint8_t readCommandLora(char serialSymbol){
  int command = -1;
  switch (serialSymbol){
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
    case 'T':
      command = TEMPERATURE;
      break;
    case 'L':
      command = LIGHT;
      break;
    case 'H':
      command = HUMIDITY;
      break;
//    case 'C':
//      command = CONTROL;
      break;
    case 'S':
        command = SET_STATE;
      break;
    case 'G':
        command = GET_STATE;
      break;
    case 'B':
        command = BATTERY;
      break;
    case 'P':
        command = POWER;
      break;
    case 'D':
        command = DATA_RATE;
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
