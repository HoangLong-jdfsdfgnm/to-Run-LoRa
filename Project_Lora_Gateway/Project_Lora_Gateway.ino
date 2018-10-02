#include "RTC.h"
#include "Recieve.h"
#include "ethernet.h"
#include "EEPROM.h"
//#include <Wire.h> 

/*************************************************/
int request = 0;
int serialCommand = 0;
byte LoraCommand = 0;
byte typeCommand = 0;
byte numberNode = 0;
String messenger;
extern uint8_t bufferLoRa[25];
volatile int a = 0;
unsigned long timeOut1 = 0;
unsigned long timeOut2 = 0;
volatile uint8_t  timeAlarm_Hour = 0;
volatile uint8_t  timeAlarm_Minute = 0;
uint8_t timeCheck = 0;
extern int second, minute, hour, day, wday, month, year;

byte role1,role2,role3;
byte temperature,humidity,light, groundHumidity;
unsigned int tempratureAlarm, humidityAlarm, lightAlarm, groundHuidityAlarm;
uint8_t Origin_DataNode[20][14];
uint8_t Process_DataNode[20][14];
uint8_t activeNode[20];
bool Flag_checkTime = 0;
//Command
#define   JOIN                          1
#define   ACCEPT                        2
#define   CONFIGURE                     3
#define   IMFORMATION                   4
#define   STATE                         5
#define   TEMPERATURE                   6
#define   LIGHT                         7
#define   HUMIDITY                      8
#define   DATA                          9
#define   CONTROL                       10
#define   ALARM                         11

#define   ALARM_TEMPERATURE             1
#define   ALARM_LIGHT                   2
#define   ALARM_HUMIDITY                3
#define   ALARM_GROUND_HUMIDITY         4
#define   ALARM_ROLE_1                  5
#define   ALARM_ROLE_2                  6
#define   ALARM_ROLE_3                  7

#define   DEFAULT_ALARM_TEMPERATURE             1
#define   DEFAULT_ALARM_LIGHT                   2
#define   DEFAULT_ALARM_HUMIDITY                3
#define   DEFAULT_ALARM_GROUND_HUMIDITY         4
#define   DEFAULT_ALARM_ROLE_1                  5
#define   DEFAULT_ALARM_ROLE_2                  6
#define   DEFAULT_ALARM_ROLE_3                  7

#define   ADDR_ADDRESS                0
#define   ADDR_TEMPERATURE            ADDR_ADDRESS + 40
#define   ADDR_HUMIDITY               ADDR_TEMPERATURE + 40
#define   ADDR_LIGHT                  ADDR_HUMIDITY + 40
#define   ADDR_GROUND_HUMIDITY        ADDR_LIGHT + 40
#define   ADDR_ROLE_1                 ADDR_GROUND_HUMIDITY +20
#define   ADDR_ROLE_2                 ADDR_ROLE_1 + 20
#define   ADDR_ROLE_3                 ADDR_ROLE_2 +20

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  while (!Serial);
  Serial.println("LoRa Gateway");
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, irqLoRa, RISING);
  if (!LoRa.begin(436E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
//  LoRa.setTxPower(17);
  LoRa.setSpreadingFactor(10);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
  setTime(15, 00, 45, 1, 25, 9, 18);
  readDS1307();
  digitalClockDisplay();
  defaultSetup();
  listCommandSerial();
  while(1){
    while(Serial.available() > 1){
      uint8_t typeFunction = determineCommandSerial();
      Serial.print("type funcition: ");
      Serial.println(typeFunction);
      switch(typeFunction){
        
      }
    }
  }
}

void irqLoRa(){
  Serial.println("Co ngat xay ra!");
}

/****************** LOOP  ***********************/
void loop() {
  
  // ------->> Quan ly Alarm
  readDS1307();
  if(second == 0){                                   // kiểm tra theo từng phút
    if(Flag_checkTime == 0){
      digitalClockDisplay();
      Serial.println("Vao ham quan ly time");
      Flag_checkTime = 1;
      LoRa.setFrequency(434E6);
      managerAlarm();
      Serial.println("xong!");
      LoRa.setFrequency(436E6);

    }
  }
  else
    Flag_checkTime = 0;

  // ----->> Quan ly lenh PC
  while(Serial.available() > 1){
    serialCommand = readCommandSerial();
    Serial.print("serialCommand: ");
    Serial.println(serialCommand);
    if((serialCommand > 0) && (serialCommand <= 11)){
      LoRa.setFrequency(434E6);
      process_Serial_Command();
      wattingNode();
      LoRa.setFrequency(436E6);
  //    while(Serial.available()){
  //      char c = Serial.read();
  //    }
    }
  }
  
  //--->> Serial Command
  while(Serial.available() > 1){
    uint8_t typeFunction = readBufferSerial();
    switch(typeFunction)
  }

  // ---->> Quan ly lenh LoRa
  if(bufferLoRaAvailable()){
    processLoRaCommand();
  }
}



//-------------------- Các hàm và câu lệnh ----------------------


/***********/
uint8_t readCommandSerial(){
  char serialSymbol;
  int command = -1;
  if (Serial.available()){
    serialSymbol = char(Serial.read());
    Serial.print("serialSymbol: ");
    Serial.println(serialSymbol);
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
//      case 'S':
//        command = STATE;
//        break;
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
      case 'S':
        command = ALARM;
        break;
      default:
        command = 0;
    }
  }
  return command;
}
/***************/
void defaultSetup(){
  timeCheck = second;
  uint8_t z = 0;
  // Value default Alarm-array
  for(byte i = 0; i<20; i++){
    for(byte j = 0; j<7; j++){
      Origin_DataNode[i][j] = 0;
      Process_DataNode[i][j] = 0;
    }
  }
  // Active Node
  for(byte i = 0; i<20; i++){
    activeNode[i] = 0;
  }
}
/**************/
void managerAlarm(){
  bool sendLoRa = 0;
  byte i = 0; 
  byte j = 0;
  for( i = 0; i<2; i++){
    for( j = 0; j<14; j= j+2){
      if(Process_DataNode[i][j] >= 100){
        if(Process_DataNode[i][j+1] == minute)
          if(Process_DataNode[i][j] == (hour+100))
            sendLoRa = 1;
      }
      else{
        if(Process_DataNode[i][j] < 25){
          if(Process_DataNode[i][j] == 0){
            if(Process_DataNode[i][j+1] != 0){
              Process_DataNode[i][j+1]--;
              if(Process_DataNode[i][j+1] == 0){
                sendLoRa = 1;
                Process_DataNode[i][j] = Origin_DataNode[i][j];
                Process_DataNode[i][j+1] = Origin_DataNode[i][j+1];
              }
            }
          }
        }
        if(Process_DataNode[i][j] != 0){
          if(Process_DataNode[i][j+1] != 0){
            Process_DataNode[i][j+1]--;
          }
          else{
            Process_DataNode[i][j] --;
            Process_DataNode[i][j+1] = 59;
          }
        }
      }
      Serial.print("Data[");Serial.print(i);
      Serial.print("][");Serial.print(j);Serial.print("]: ");
      Serial.println(Process_DataNode[i][j]);
      Serial.print("Data[");Serial.print(i);
      Serial.print("][");Serial.print(j+1);Serial.print("]: ");
      Serial.println(Process_DataNode[i][j+1]);
    }
  }
      
      
//      if(Process_DataNode[i][j] != 0){
//        Process_DataNode[i][j]--;
//        Serial.print("Data[");Serial.print(i);
//        Serial.print("][");Serial.print(j);Serial.print("]: ");
//        Serial.println(Process_DataNode[i][j]);
//        if(Process_DataNode[i][j] == 0){
        if(sendLoRa == 1){
          messenger = "@;0;";
          messenger += char(i+49);
          Serial.print("j/2 +1: ");
          Serial.println(j/2 +1);
          switch(j/2 +1){
            case ALARM_TEMPERATURE:
                messenger += ";T:;!";
                Serial.println("Lay du lieu nhiet do");
              break;
            case ALARM_LIGHT:
                messenger += ";L:;!";
                Serial.println("Lay du lieu anh sang");
              break;
            case ALARM_HUMIDITY:
                messenger += ";H:;!";
                Serial.println("Lay du lieu do am");
              break;
            case ALARM_GROUND_HUMIDITY:
                messenger += ";G:;!";
                Serial.println("Lay du lieu do am dat");
              break;
            case ALARM_ROLE_1:
                messenger += ";W:1;!";
                Serial.println("Lay du lieu role 1");
              break;
            case ALARM_ROLE_2:
                messenger += ";W:2;!";
                Serial.println("Lay du lieu role 2");
              break;
            case ALARM_ROLE_3:
                messenger += ";W:3;!";
                Serial.println("Lay du lieu role 3");
                
              break;
          }
          Serial.print("messenger: ");
          Serial.println(messenger);
          LoRa.beginPacket();
          LoRa.print(messenger);
          LoRa.endPacket();
          Process_DataNode[i][j] = Origin_DataNode[i][j];
          Serial.println(Process_DataNode[i][j]);
          timeOut1 = millis();
          while ((millis()-timeOut1) <= 10000){
            if(bufferLoRaAvailable()){
              processLoRaCommand();
            }
          }
        }
      }
//    }
//  }
//}
/**************/
void getDataEnvironment(uint8_t Node){
  //temperature
  Serial.println("Node Environment gia nhap mang ...");
//  write_EEPROM(int addr, Node -48)
  Serial.print("Cam bien: ");
  if(bufferLoRa[8] == '1'){
    temperature = 1;
    Serial.print("Temperature");
    Origin_DataNode[Node -49][0] = DEFAULT_ALARM_TEMPERATURE;
    Process_DataNode[Node -49][0] = DEFAULT_ALARM_TEMPERATURE;
  }
    else{
       if(bufferLoRa[8] == '0')
        temperature = 0;
        else{
          temperature = -1;
          Serial.print("Temperature (bi loi)");
        }
    }
    
    //humidity
  if(bufferLoRa[10] == '1'){
    humidity = 1;
    Serial.print("; Humidity");
    Origin_DataNode[Node -49][1] = DEFAULT_ALARM_HUMIDITY;
    Process_DataNode[Node -49][1] = DEFAULT_ALARM_HUMIDITY;
  }
    else{
     if(bufferLoRa[10] == '0')
      humidity = 0;
      else{
        humidity = -1;
        Serial.print("; Humidity (bi loi)");
      }
    }
    
    //light
  if(bufferLoRa[12] == '1'){
    light = 1;
    Serial.print("; Light");
    Origin_DataNode[Node -49][2] = DEFAULT_ALARM_LIGHT;
    Process_DataNode[Node -49][2] = DEFAULT_ALARM_LIGHT;
  }
    else{
     if(bufferLoRa[12] == '0')
      light = 0;
      else{
        light = -1;
        Serial.print("; Light (bi loi)");
      }
    }
    
  if(bufferLoRa[14] == '1'){
    groundHumidity = 1;
    Serial.print("; GroundHumidity");
    Origin_DataNode[Node -49][3] = DEFAULT_ALARM_GROUND_HUMIDITY;
    Process_DataNode[Node -49][3] = DEFAULT_ALARM_GROUND_HUMIDITY;
  }
    else{
     if(bufferLoRa[8] == '0')
      groundHumidity = 0;
      else{
        groundHumidity = -1;
        Serial.print("; GroundHumidity (bi loi)");
      }
  }
  Serial.println();
  Serial.println("----------");
}
/////////////////////////////
void getDataControl(uint8_t Node){
  Serial.println("Node Control gia nhap mang ...");
  Serial.print("Role_1: ");
  if(bufferLoRa[8] == '1'){
    role1 = 1;
    Serial.println("ON");
    Origin_DataNode[Node -49][4] = DEFAULT_ALARM_ROLE_1;
    Process_DataNode[Node -49][4] = DEFAULT_ALARM_ROLE_1;
  }
    else{
     if(bufferLoRa[8] == '0'){
      role1 = 0;
      Serial.println("OFF");
     }
      else{
        role1 = -1;
        Serial.println("error");
      }
  }
  
  Serial.print("Role_2: ");
  if(bufferLoRa[10] == '1'){
    role2 = 1;
    Serial.println("ON");
    Origin_DataNode[Node -49][5] = DEFAULT_ALARM_ROLE_2;
    Process_DataNode[Node -49][5] = DEFAULT_ALARM_ROLE_2;
  }
    else{
     if(bufferLoRa[12] == '0'){
      role2 = 0;
      Serial.println("OFF");
     }
      else{
        role2 = -1;
        Serial.println("error");
      }
  }

  Serial.print("Role_3: ");
  if(bufferLoRa[12] == '1'){
    role3 = 1;
    Serial.println("ON");
    Origin_DataNode[Node -49][6] = DEFAULT_ALARM_ROLE_3;
    Process_DataNode[Node -49][6] = DEFAULT_ALARM_ROLE_3;
  }
    else{
     if(bufferLoRa[12] == '0'){
      role3 = 0;
      Serial.println("OFF");
     }
     else{
        role3 = -1;
        Serial.println("error");
     }
  }
}
///////////////////////////////
void processLoRaCommand(){
  readBufferLoRa(bufferLoRa);
  uint8_t b = checkFrame(bufferLoRa);
  Serial.println(b);
  if(b == 0){
    LoraCommand = bufferLoRa[6];
    typeCommand = bufferLoRa[8];
    numberNode = bufferLoRa[4];
    Serial.println(" ***** Ban tin LoRa ******");
    Serial.print("Lenght: ");
    Serial.println(sizeBufferLoRa());
    Serial.print("Payload: ");
    for(byte i = 8; i< (sizeBufferLoRa()-2); i++){
      Serial.print(char(bufferLoRa[i]));
    }
    Serial.println();
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    Serial.print("command: ");
    Serial.println(char(LoraCommand));
    Serial.print("Loai lenh: ");
    Serial.println("Node -> Gateway");
    Serial.print("Node: ");
    if(numberNode == '1'){
      Serial.println(" Environment");
    }
    if(numberNode == '0'){
      Serial.println("Control");
    }
  }
  switch(LoraCommand){
    case 'J':
        Serial.println(" ..... Lenh JOIN: gui ban tin ACCEPT");
        messenger = "@;0;";
        messenger += numberNode - 48;
        messenger += ";A:;!";
        if((numberNode >= '1')&&(numberNode <= '9')){
          activeNode[numberNode - 49] = 1;
          Serial.print("activeNode[");
          Serial.print(numberNode - 49);
          Serial.print("]: ");
          Serial.println(activeNode[numberNode - 49]);
        }
        if(numberNode == '1'){
          getDataEnvironment(numberNode);
        }
        else{
          getDataControl(numberNode);
        }
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;
    case STATE:
        role1 = bufferLoRa[10];
        role2 = bufferLoRa[12];
        role3 = bufferLoRa[14];
      break;
    case TEMPERATURE:
      Serial.print("Gia tri nhiet do hien tai:");
      for(uint8_t i = 8; i<13; i++){
        if(bufferLoRa[i] == ';'){
          Serial.println();
          break;
        }
        else{
          Serial.print(char(bufferLoRa[i]));
        }
      }
      break;
    case LIGHT:
      Serial.print("Gia tri anh sang hien tai:");
      for(uint8_t i = 8; i<13; i++){
        if(bufferLoRa[i] == ';'){
          Serial.println();
          break;
        }
        else{
          Serial.print(char(bufferLoRa[i]));
        }
      }
      break;
    case HUMIDITY:
      Serial.print("Gia tri do am kk hien tai:");
      for(uint8_t i = 8; i<13; i++){
        if(bufferLoRa[i] == ';'){
          Serial.println();
          break;
        }
        else{
          Serial.print(char(bufferLoRa[i]));
        }
      }
      break;
    case ACCEPT:
        Serial.println("Da nhan duoc ban tin");
        messenger = "@;0;1;J:;!";
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;
    default:
      break;
  }
}
////////////////////////////////////
void process_Serial_Command(){
  switch(serialCommand){
    case CONTROL:
        delay(5);
        timeOut1 = millis();
        Serial.println(timeOut1);
        while((millis() - timeOut1) < 5000){
          Serial.println("An phim 123 (symbolRole):");
          while(Serial.available()){
            char symbolRole = char(Serial.read());
            if((symbolRole >= '1')&&(symbolRole <= '3')){
              Serial.println("An phim O|F (stateRole):");
              timeOut2 = millis();
              while((millis() - timeOut2) < 5000){
                Serial.print("An phim O|F (stateRole):");
                char stateRole = char(Serial.read());
                Serial.println(char(stateRole));
                if((stateRole == 'O')||(stateRole == 'F')){
                  messenger = "@;0;2;C:";
                  messenger += symbolRole;
                  messenger += ",";
                  messenger += stateRole;
                  messenger += ";!";
                  Serial.println(messenger);
                  LoRa.beginPacket();
                  LoRa.print(messenger);
                  LoRa.endPacket();
                  timeOut2 = millis()-6000;
                }
              }
              timeOut1 = millis()-6000;
            }

            else{
              break;
            }
          }
        }
//        Serial.println("OutTime");
      break;
    case IMFORMATION:
        messenger = "@;0;1;I:;!";
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;
    case TEMPERATURE:
        messenger = "@;0;1;T:;!";
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;
    case LIGHT:
        messenger = "@;0;1;L:;!";
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;
    case HUMIDITY:
        messenger = "@;0;1;H:;!";
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;
    case ALARM:
        Serial.println("*** Chon loai Node can hen gio:");
        Serial.println("1: Node Environment");
        Serial.println("2: Node Control");
        delay(5);
        timeOut1 = millis();
        
        while(1){
          uint8_t i,j;
          i=j=0;
          volatile uint8_t x = 0;
          if((millis() - timeOut1) < 10000){
            if(Serial.available()){
              i = Serial.read();
              delay(50);
              while(Serial.available()){
                char c = Serial.read();
              }
              if(i == '1'){
                i = i-49;
                x = 1;
                messenger = "@;0;1;S:";
                Serial.println("*** Chon sensor can hen gio:");
                Serial.println("1: Temperature");
                Serial.println("2: Humidity");
                Serial.println("3: Light");
                Serial.println("4: Ground Humidity");
                timeOut2 = millis();
                while ((millis() - timeOut2) < 10000){
                  if(Serial.available()){
                    j = Serial.read();
//                      Serial.println(d);
                    delay(50);
                    while(Serial.available()){
                      char c = Serial.read();
//                        Serial.println("xoa bo dem");
                    }
                    Serial.println("Hen gio theo:");
                    Serial.println("1. Kieu chu ky");
                    Serial.println("2. Kieu thoi gian co dinh");
                    while (!Serial.available());
                    delay(10);
                    char c = Serial.read();
                    if(c =='2'){
                      timeAlarm_Hour = 100;
                      timeAlarm_Minute = 0;
                    }
                    else
                      if(c =='1'){
                        timeAlarm_Hour = 0;
                        timeAlarm_Minute = 1;
                      }
//                    Serial.print("timeAlarm: ");
//                    Serial.println(timeAlarm);
                    while(Serial.available()){
                      char c = Serial.read();
//                        Serial.println("xoa bo dem");
                    }
                    Serial.println(">>>>> Nhap gio: ");
                    while (!Serial.available());
                    timeAlarm_Hour += char2decima();
                    Serial.print("timeAlarm_Hour: ");
                    Serial.println(timeAlarm_Hour);

                    Serial.println(">>>>> Nhap phut: ");
                    while (!Serial.available());
                    timeAlarm_Minute += char2decima();
                    Serial.print("timeAlarm_Minute: ");
                    Serial.println(timeAlarm_Minute);
                    
//                    timeAlarm_Minute = 1;
//                    while (timeAlarm_Minute == 1){
//                      while(Serial.available()){
//                        delay(50);
//                        uint8_t a = Serial.available();
//                        Serial.print("Serial.available(): ");
//                        Serial.println(a);
//                        for(byte i = a-1; i> 0; i--){
//                          uint8_t x = Serial.read() -48;
//                          Serial.print("x: ");
//                          Serial.println(x);
//                          Serial.print("i: ");
//                          Serial.println(i);
//                          if((x >= 0)&&(x<=9)){
//                            
//                            timeAlarm += x*pow(10,(i-1));
//                            Serial.print("timeAlarm_Minute: ");
//                            Serial.println(timeAlarm_Minute);
//                          }
//                        }
//                        while(Serial.available()){
//                          char c = Serial.read();
//  //                        Serial.println("xoa bo dem");
//                        }
//                      }
//                    }
                    
//                      Serial.println(timeAlarm);
                        Serial.print("Address Node: ");
                        Serial.println(i+1);
//                        Serial.print("j: ");
//                        Serial.println(j);
                    switch(j){
                      case '1':
                          Origin_DataNode[i][0] = timeAlarm_Hour;
                          Origin_DataNode[i][1] = timeAlarm_Minute;
                          Process_DataNode[i][0] = timeAlarm_Hour;
                          Process_DataNode[i][1] = timeAlarm_Minute;
                          if(Origin_DataNode[i][0] >= 100){
                            Serial.println("Kieu hen gio: co dinh");
                            Serial.print("Hen gio nhiet do=: ");
                            Serial.print(Origin_DataNode[i][0] -100);
                          }
                          else{
                            Serial.println("Kieu hen gio: chu ky");
                            Serial.print("Hen gio nhiet do: ");
                            Serial.print(Origin_DataNode[i][0]);
                          }
                          
                          Serial.print("h");
                          Serial.println(Origin_DataNode[i][1] -1);
                        break;
                      case '2':
                          Origin_DataNode[i][2] = timeAlarm_Hour;
                          Origin_DataNode[i][3] = timeAlarm_Minute;
                          Process_DataNode[i][2] = timeAlarm_Hour;
                          Process_DataNode[i][3] = timeAlarm_Minute;
                          if(Origin_DataNode[i][2] >= 100){
                            Serial.println("Kieu hen gio: co dinh");
                            Serial.print("Hen gio do am kk: ");
                            Serial.print(Origin_DataNode[i][2] -100);
                          }
                          else{
                            Serial.println("Kieu hen gio: chu ky");
                            Serial.print("Hen gio do am kk: ");
                            Serial.print(Origin_DataNode[i][2]);
                          }
                          Serial.print("h");
                          Serial.println(Origin_DataNode[i][3] -1);
                        break;
                      case '3':
                          Origin_DataNode[i][4] = timeAlarm_Hour;
                          Origin_DataNode[i][5] = timeAlarm_Minute;
                          Process_DataNode[i][4] = timeAlarm_Hour;
                          Process_DataNode[i][5] = timeAlarm_Minute;
                          if(Origin_DataNode[i][4] >= 100){
                            Serial.println("Kieu hen gio: co dinh");
                            Serial.print("Hen gio anh sang: ");
                            Serial.print(Origin_DataNode[i][4] -100);
                          }
                          else{
                            Serial.println("Kieu hen gio: chu ky");
                            Serial.print("Hen gio anh sang: ");
                            Serial.print(Origin_DataNode[i][4]);
                          }
                          Serial.print("h");
                          Serial.println(Origin_DataNode[i][5] -1);
                        break;
                      case '4':
                          Origin_DataNode[i][6] = timeAlarm_Hour;
                          Origin_DataNode[i][7] = timeAlarm_Minute;
                          Process_DataNode[i][6] = timeAlarm_Hour;
                          Process_DataNode[i][7] = timeAlarm_Minute;
                          if(Origin_DataNode[i][6] >= 100){
                            Serial.println("Kieu hen gio: co dinh");
                            Serial.print("Hen gio do am dat: ");
                            Serial.print(Origin_DataNode[i][6] -100);
                          }
                          else{
                            Serial.println("Kieu hen gio: chu ky");
                            Serial.print("Hen gio do am dat: ");
                            Serial.print(Origin_DataNode[i][6]);
                          }
                          Serial.print("h");
                          Serial.println(Origin_DataNode[i][7] -1);
                        break;
                      default:
                        break;
                    }
                  }
                }
              }
              if (i == '2'){
                i= i-49;
                x = 2;
                messenger = "@;0;2;S:";
                Serial.println("*** Chon role can hen gio:");
                Serial.println("1: Role 1");
                Serial.println("2: Role 2");
                Serial.println("3: Role 3");
                timeOut2 = millis();
                while ((millis() - timeOut2) < 5000){
                  if(Serial.available()){
                    j = Serial.read();
                    delay(50);
                    while(Serial.available()){
                      char c = Serial.read();
                    }
                    Serial.println("Hen gio theo:");
                    Serial.println("1. Kieu chu ky");
                    Serial.println("2. Kieu thoi gian co dinh");
                    while (!Serial.available());
                    delay(10);
                    char c = Serial.read();
                    if(c =='2'){
                      timeAlarm_Hour = 100;
                      timeAlarm_Minute = 0;
                    }
                    else
                      if(c =='1'){
                        timeAlarm_Hour = 0;
                        timeAlarm_Minute = 1;
                      }
//                    Serial.print("timeAlarm: ");
//                    Serial.println(timeAlarm);
                    while(Serial.available()){
                      char c = Serial.read();
//                        Serial.println("xoa bo dem");
                    }
                    Serial.println(">>>>> Nhap gio: ");
                    while (!Serial.available());
                    timeAlarm_Hour += char2decima();
                    Serial.print("timeAlarm_Hour: ");
                    Serial.println(timeAlarm_Hour);

                    Serial.println(">>>>> Nhap phut: ");
                    while (!Serial.available());
                    timeAlarm_Minute += char2decima();
                    Serial.print("timeAlarm_Minute: ");
                    Serial.println(timeAlarm_Minute);
                    
                    
                    switch(j){
                      case '1':
                          Origin_DataNode[i][8] = timeAlarm_Hour;
                          Origin_DataNode[i][9] = timeAlarm_Minute;
                          Process_DataNode[i][8] = timeAlarm_Hour;
                          Process_DataNode[i][9] = timeAlarm_Minute;
                          if(Origin_DataNode[i][8] >= 100){
                            Serial.println("Kieu hen gio: co dinh");
                            Serial.print("Hen gio role 1: ");
                            Serial.print(Origin_DataNode[i][8] -100);
                          }
                          else{
                            Serial.println("Kieu hen gio: chu ky");
                            Serial.print("Hen gio role 1: ");
                            Serial.print(Origin_DataNode[i][8]);
                          }
                          Serial.print("h");
                          Serial.println(Origin_DataNode[i][9] -1);
                        break;
                      case '2':
                          Origin_DataNode[i][10] = timeAlarm_Hour;
                          Origin_DataNode[i][11] = timeAlarm_Minute;
                          Process_DataNode[i][10] = timeAlarm_Hour;
                          Process_DataNode[i][11] = timeAlarm_Minute;
                          if(Origin_DataNode[i][10] >= 100){
                            Serial.println("Kieu hen gio: co dinh");
                            Serial.print("Hen gio role 2: ");
                            Serial.print(Origin_DataNode[i][10] -100);
                          }
                          else{
                            Serial.println("Kieu hen gio: chu ky");
                            Serial.print("Hen gio role 2: ");
                            Serial.print(Origin_DataNode[i][10]);
                          }
                          Serial.print("h");
                          Serial.println(Origin_DataNode[i][11] -1);
                        break;
                      case '3':
                          Origin_DataNode[i][12] = timeAlarm_Hour;
                          Origin_DataNode[i][13] = timeAlarm_Minute;
                          Process_DataNode[i][12] = timeAlarm_Hour;
                          Process_DataNode[i][13] = timeAlarm_Minute;
                          if(Origin_DataNode[i][12] >= 100){
                            Serial.println("Kieu hen gio: co dinh");
                            Serial.print("Hen gio role 3: ");
                            Serial.print(Origin_DataNode[i][12] -100);
                          }
                          else{
                            Serial.println("Kieu hen gio: chu ky");
                            Serial.print("Hen gio role 3: ");
                            Serial.print(Origin_DataNode[i][12]);
                          }
                          Serial.print("h");
                          Serial.println(Origin_DataNode[i][13] -1);
                        break;
                      default:
                        break;
                    }
                  }
                }
              }
            }
          }
          else{
           if(x == 0)
            Serial.println("Time Out");
           break;
          }
          //viết waitting o day
        }
      break;
    default:
      break;
  }
}

///////////////////////////
void listCommandSerial(){
  Serial.println("******* Danh sach tap lenh :");
  Serial.println("1. T - lay du lieu nhiet do");
  Serial.println("2. H - lay du lieu do am");
  Serial.println("3. L - lay du lieu anh sang");
  Serial.println("4. C - dieu khien role");
  Serial.println("5. S - cai dat hen gio");
}

///////////////////////////
void load_Data_EEPROM(){
  for(byte i=0; i<20; i++){
    if(read_EEPROM(i) != 0){
      Process_DataNode[i][0] = read_EEPROM(ADDR_TEMPERATURE + i);
      Process_DataNode[i][1] = read_EEPROM(ADDR_HUMIDITY + i);
      Process_DataNode[i][2] = read_EEPROM(ADDR_LIGHT + i);
      Process_DataNode[i][3] = read_EEPROM(ADDR_GROUND_HUMIDITY + i);
      Process_DataNode[i][4] = read_EEPROM(ADDR_ROLE_1 + i);
      Process_DataNode[i][5] = read_EEPROM(ADDR_ROLE_2 + i);
      Process_DataNode[i][6] = read_EEPROM(ADDR_ROLE_3 + i);
      messenger = "@;0;";
      messenger += (i+1);
      messenger += ";I:;!";
      LoRa.beginPacket();
      LoRa.print(messenger);
      LoRa.endPacket();
    }
  }
}

///////////////////////
uint8_t char2decima(){
  uint8_t a[10];
  uint8_t value = 0;
  while (Serial.available()){
    delay(50);
    uint8_t a = Serial.available();
    Serial.print("Serial.available(): ");
    Serial.println(a);
    for(byte i = a-1; i> 0; i--){
      uint8_t x = Serial.read() -48;
      Serial.print("x: ");
      Serial.println(x);
      Serial.print("i: ");
      Serial.println(i);
      if((x >= 0)&&(x<=9)){
        
        value += x*pow(10,(i-1));
        Serial.print("value: ");
        Serial.println(value);
      }
    }
    while(Serial.available()){
      char c = Serial.read();
    }
  }
  return value;
}
///////////////////////
uint8_t wattingNode(){
  bool reponse = 0;
  Serial.println("wattingNode(): ");
  unsigned long timeWait = millis();
  while((millis() - timeWait) <= 10000){
    if(bufferLoRaAvailable()){
      processLoRaCommand();
      reponse = 1;
      break;
    }
  }
  return reponse;
}
