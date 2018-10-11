#ifndef DEBUG
  #define DEBUG
#endif

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
uint8_t bufferSerial[25];
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
volatile uint8_t Origin_DataNode[20][14];
volatile uint8_t Process_DataNode[20][14];
uint8_t activeNode[20];
bool Flag_checkTime = 0;
//Command
#define   INVALID_PARAM                 -1
#define   JOIN                          1
#define   ACCEPT                        2
#define   CONFIGURE                     3
#define   IMFORMATION                   4
#define   GET_STATE                     5
#define   TEMPERATURE                   6
#define   LIGHT                         7
#define   HUMIDITY                      8
#define   DATA_RATE                     9
#define   SET_STATE                     10
#define   ALARM_ENVIRONMENT             11
#define   ALARM_CONTROL                 12
#define   GROUND_HUMIDITY               13
#define   BATTERY                       14
#define   QUIT                          15
#define   POWER                         16
#define   FREQ_JOIN_ENVIRONMENT         17
#define   FREQ_JOIN_CONTROL             18


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
  Serial.begin(57600);
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
//  while(1){
//    while(Serial.available() > 1){
//      uint8_t typeFunction = determineCommandSerial();
//      Serial.print("type funcition: ");
//      Serial.println(typeFunction);
//      switch(typeFunction){
//        
//      }
//    }
//  }
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
      Serial.println("Check Alarm...");
      Flag_checkTime = 1;
      LoRa.setFrequency(434E6);
      managerAlarm();
      Serial.println("Done!");
      LoRa.setFrequency(436E6);

    }
  }
  else
    Flag_checkTime = 0;

  // ----->> Quan ly lenh PC
  while(Serial.available() > 1){
    serialCommand = readCommandSerial();

    // read Serial
    if((serialCommand > 0) && (serialCommand <= 14)){
      LoRa.setFrequency(434E6);
      process_Serial_Command();
      if((serialCommand != ALARM_ENVIRONMENT)&&(serialCommand != ALARM_CONTROL)){
        wattingNode();
      }
      LoRa.setFrequency(436E6);
    }

    // error
    if(serialCommand == -1){
      Serial.println("inval_parameter");
    }
    
  }

  
  // ---->> Quan ly lenh LoRa
  if(bufferLoRaAvailable()){
    processLoRaCommand();
  }
}



//-------------------- Các hàm và câu lệnh ----------------------


/***********/
int8_t readCommandSerial(){
/*  char serialSymbol;
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
*/

  //--- read serial
  uint8_t size =0;
  while(Serial.available()){
    delay(100);
    bufferSerial[size] = Serial.read();
#ifdef DEBUG
    Serial.print(char(bufferSerial[size]));
#endif
    size ++;
  }
  
  
  
  
  //--- determine equal
  uint8_t equal = 0;
  for(uint8_t i = 0; i< size; i++){
    if(bufferSerial[i] == '='){
      equal = i;
    }
  }

  
  
  // error equal
  if(equal < 3){
    return -1;
  }


  //--- determine questMark
  uint8_t questMark = 0;
  for(uint8_t i = equal; i< size; i++){
    if(bufferSerial[i] == '?'){
      questMark = i;
    }
  }
  
  //--- determine command
  String messenger = "";
  for(uint8_t i = 0; i< equal; i++){
    messenger += char(bufferSerial[i]);
  }

  
  
  //--- choose
  int command = INVALID_PARAM;

  // get command
  if(questMark != 0){
    if(messenger == "AT+ELIGHT"){
      command = LIGHT;
    }
    if(messenger == "AT+ETEMP"){
      command = TEMPERATURE;
    }
    if(messenger == "AT+EHUM"){
      command = HUMIDITY;
    }
    if(messenger == "AT+EGND"){
      command = GROUND_HUMIDITY;
    }
    if(messenger == "AT+CROLE"){
      command = GET_STATE;
    }
  }

  // set command
  else{
    if(messenger == "AT+CROLE"){
      command = SET_STATE;
    }
    if(messenger == "AT+MEALM"){
      command = ALARM_ENVIRONMENT;
    }
    if(messenger == "AT+MCALM"){
      command = ALARM_CONTROL;
    }
  }

#ifdef DEBUG
  Serial.println();
  Serial.print("size: ");
  Serial.println(size);
  Serial.print("equal: ");
  Serial.println(equal);
  Serial.print("questMark: ");
  Serial.println(questMark);
  Serial.print("messenger: ");
  Serial.println(messenger);
  Serial.print("command: ");
  Serial.println(command);
#endif
  
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
  byte node = 0; 
  byte parameter = 0;
  byte  state = 0;

 
  //--- compare time
  for(uint8_t i = 0; i<2; i++){
    for( uint8_t j = 0; j<14; j= j+2){

      // If Time = constans
      if(Process_DataNode[i][j] >= 100){
        if(Process_DataNode[i][j+1] == minute){
          if(Process_DataNode[i][j] == (hour+100)){
            node = i+1;
            parameter = j;
            sendLoRa = 1;
            state = 0;
          }     
          if(Process_DataNode[i][j] == (hour+200)){
            node = i+1;
            parameter = j;
            sendLoRa = 1;
            state = 1;
          }
        }
      }

      // If time = cycle
      else{
        
        if(Process_DataNode[i][j] < 25){              // đảm bảo thời gian nhỏ hơn 24h
          if(Process_DataNode[i][j] == 0){            // nếu giời = 0 thì xử lý tiếp
            if(Process_DataNode[i][j+1] != 0){        // nếu số phút = 0 tức là ko có cảm biến
              Process_DataNode[i][j+1]--;
              if(Process_DataNode[i][j+1] == 0){                  // sau khi trừ 1 bằng 0 tức là đến giờ quét
                node = i+1;
                parameter = j;
                sendLoRa = 1;                                       // cờ báo truyền tin
                Process_DataNode[i][j] = Origin_DataNode[i][j];                 // gán lại các giá trị chu kỳ
                Process_DataNode[i][j+1] = Origin_DataNode[i][j+1];
              }
            }
          }
        }

        // xử lý giảm giờ
        if(Process_DataNode[i][j] != 0){              
          if(Process_DataNode[i][j+1] != 0){          // nếu phút khác 0 thì giảm phút
            Process_DataNode[i][j+1]--;
          }
          else{                                       // nếu phút = 0 thì giảm giờ, phút = 59
            Process_DataNode[i][j] --;
            Process_DataNode[i][j+1] = 59;
          }
        }
        
      }

#ifdef DEBUG
      // debug by serial
      Serial.print("Data[");Serial.print(i);
      Serial.print("][");Serial.print(j);Serial.print("]: ");
      Serial.println(Process_DataNode[i][j]);
      Serial.print("Data[");Serial.print(i);
      Serial.print("][");Serial.print(j+1);Serial.print("]: ");
      Serial.println(Process_DataNode[i][j+1]);
      Serial.print("node: ");
      Serial.println(node);
      Serial.print("parameter: ");
      Serial.println(parameter);
#endif

      
      
//      if(Process_DataNode[i][j] != 0){
//        Process_DataNode[i][j]--;
//        Serial.print("Data[");Serial.print(i);
//        Serial.print("][");Serial.print(j);Serial.print("]: ");
//        Serial.println(Process_DataNode[i][j]);
//        if(Process_DataNode[i][j] == 0){



        //--- Transmits LoRa
      if(sendLoRa == 1){
        messenger = "@;0;";
        messenger += char(node+48);
        switch(parameter/2+1){
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
              if(state == 0){
                messenger += ";G:1;O!";
                Serial.println("Role 1: OFF");
              }
              if(state == 1){
                messenger += ";G:1;F!";
                Serial.println("Role 1: ON");
              }
            break;
          case ALARM_ROLE_2:
              if(state == 0){
                messenger += ";G:2;O!";
                Serial.println("Role 2: OFF");
              }
              if(state == 1){
                messenger += ";G:2;F!";
                Serial.println("Role 2: ON");
              }
            break;
          case ALARM_ROLE_3:
              if(state == 0){
                messenger += ";G:3;O!";
                Serial.println("Role 3: OFF");
              }
              if(state == 1){
                messenger += ";G:3;F!";
                Serial.println("Role 3: ON");
              }
              
            break;
        }

#ifdef DEBUG         
        Serial.print("messenger: ");
        Serial.println(messenger);
#endif
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
//        Process_DataNode[node -1][parameter] = Origin_DataNode[node -1][parameter];
//        Serial.println(Process_DataNode[node -1][parameter +1]);
        
        timeOut1 = millis();
        while ((millis()-timeOut1) <= 10000){
          if(bufferLoRaAvailable()){
            processLoRaCommand();
            timeOut1 = 0;
          }
          if(timeOut1 == 0){
            break;
          }
        }
        sendLoRa = node = parameter = 0;
      }
    }
  }
}

/**************/
void getDataEnvironment(uint8_t Node){
  //temperature
  Serial.println("Node Environment gia nhap mang ...");
//  write_EEPROM(int addr, Node -48)
  Serial.print("Cam bien: ");
  if(bufferLoRa[8] == '1'){
    temperature = 1;
    Serial.print("Temperature");
    Origin_DataNode[Node -49][1] = DEFAULT_ALARM_TEMPERATURE;
    Process_DataNode[Node -49][1] = DEFAULT_ALARM_TEMPERATURE;
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
    Origin_DataNode[Node -49][3] = DEFAULT_ALARM_HUMIDITY;
    Process_DataNode[Node -49][3] = DEFAULT_ALARM_HUMIDITY;
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
    Origin_DataNode[Node -49][5] = DEFAULT_ALARM_LIGHT;
    Process_DataNode[Node -49][5] = DEFAULT_ALARM_LIGHT;
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
    Origin_DataNode[Node -49][7] = DEFAULT_ALARM_GROUND_HUMIDITY;
    Process_DataNode[Node -49][7] = DEFAULT_ALARM_GROUND_HUMIDITY;
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
    Origin_DataNode[Node -49][9] = DEFAULT_ALARM_ROLE_1;
    Process_DataNode[Node -49][9] = DEFAULT_ALARM_ROLE_1;
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
    Origin_DataNode[Node -49][11] = DEFAULT_ALARM_ROLE_2;
    Process_DataNode[Node -49][11] = DEFAULT_ALARM_ROLE_2;
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
    Origin_DataNode[Node -49][13] = DEFAULT_ALARM_ROLE_3;
    Process_DataNode[Node -49][13] = DEFAULT_ALARM_ROLE_3;
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
//  Serial.print("b: ");
//  Serial.println(b);
  if(b == 0){
    LoraCommand = bufferLoRa[6];
    typeCommand = bufferLoRa[8];
    numberNode = bufferLoRa[4];


    // error
#ifdef DEBUG
    Serial.println(b);
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
    if(numberNode == '2'){
      Serial.println("Control");
    }
#endif

  }
//  if(activeNode[numberNode - 48] == 0){
//    Origin_DataNode[numberNode -49][1] = DEFAULT_ALARM_TEMPERATURE;
//    Process_DataNode[numberNode -49][1] = DEFAULT_ALARM_TEMPERATURE;
//    activeNode[numberNode - 48] = 1;
//  }

  //--- Xử lý lệnh
  switch(LoraCommand){
    case 'O':
        Serial.println("+ Reponse: {ok}");
//        Serial.print("abc");
      break;

      
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
      
//    case GET_STATE:
//        role1 = bufferLoRa[10];
//        role2 = bufferLoRa[12];
//        role3 = bufferLoRa[14];
//      break;
      
    case 'T':
        Serial.print("+ Response: ");
        messenger = "{Temperature:";
        for(uint8_t i = 8; i<13; i++){
          if(bufferLoRa[i] == ';'){
            break;
          }
          else{
            messenger += char(bufferLoRa[i]);
          }
        }
        readDS1307();
        messenger += "*C ;Time:";
        Serial.print(messenger);
        Serial.print(hour);
        Serial.print(", ");
        Serial.print(minute);
        Serial.print(", ");
        Serial.print(day);
        Serial.print(", ");
        Serial.print(month);
        Serial.print(", ");
        Serial.print(year);
        Serial.println("}");
      break;
      
    case 'L':
        Serial.print("+ Response: ");
        messenger = "{Light:";
        for(uint8_t i = 8; i<13; i++){
          if(bufferLoRa[i] == ';'){
            break;
          }
          else{
            messenger += char(bufferLoRa[i]);
          }
        }
        readDS1307();
        messenger += ";Time:";
        Serial.print(messenger);
        Serial.print(hour);
        Serial.print(", ");
        Serial.print(minute);
        Serial.print(", ");
        Serial.print(day);
        Serial.print(", ");
        Serial.print(month);
        Serial.print(", ");
        Serial.print(year);
        Serial.println("}");
      break;
      
    case 'H':
        Serial.print("+ Response: ");
        messenger = "{Humidity:";
        for(uint8_t i = 8; i<13; i++){
          if(bufferLoRa[i] == ';'){
            break;
          }
          else{
            messenger += char(bufferLoRa[i]);
          }
        }
        readDS1307();
        messenger += ";Time:";
        Serial.print(messenger);
        Serial.print(hour);
        Serial.print(", ");
        Serial.print(minute);
        Serial.print(", ");
        Serial.print(day);
        Serial.print(", ");
        Serial.print(month);
        Serial.print(", ");
        Serial.print(year);
        Serial.println("}");
      break;

    case 'G':
        uint8_t x = 0;
        
        Serial.print("+ Response: ");
        for(uint8_t i = 8; i<13; i++){
          if(bufferLoRa[i] == ','){
            x = i;
          }
        }
        
        if(x == 0){
          messenger = "{Ground Humidity:";
          for(uint8_t i = 8; i<13; i++){
            if(bufferLoRa[i] == ';'){
              break;
            }
            else{
              messenger += char(bufferLoRa[i]);
            }
          }
        }
        
        else{
          messenger = "{Role:";
          messenger += char(bufferLoRa[x-1]);
          messenger += ";Tpye:";
          switch(bufferLoRa[x+1]){
            case '1':
                messenger += "ON";
              break;
            case '0':
                messenger += "OFF";
              break;
            default:
                messenger += "inval_param";
              break;
          }
        }
        
        readDS1307();
        messenger += ";Time:";
        Serial.print(messenger);
        Serial.print(hour);
        Serial.print(", ");
        Serial.print(minute);
        Serial.print(", ");
        Serial.print(day);
        Serial.print(", ");
        Serial.print(month);
        Serial.print(", ");
        Serial.print(year);
        Serial.println("}");
      break;
      
    case 'A':
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
  uint8_t comma[5]={0,0,0,0,0};
  uint8_t j = 0;
  uint8_t equal = 0;
  uint8_t lineFeed = 0;
  uint8_t alarmAddr = 0;
  uint8_t alarmRole = 0;
  uint8_t alarmParam = 0;
  uint8_t alarmType = 0;
  uint8_t alarmMinute = 0;
  uint8_t alarmHour = 0;
  uint8_t alarmState = 0;
  switch(serialCommand){
    case GET_STATE:
        // error quest mark
        if(bufferSerial[12] != '?'){
          Serial.println("inval_param");
          break;
        }

        // error type role
        if((bufferSerial[11] < '1') || (bufferSerial[11] > '3')){
          Serial.println("inval_param");
          break;
        }

        // send command
        if(bufferSerial[9] == '2'){
          messenger = "@;0;2;G:";
          messenger += char(bufferSerial[11]);
          messenger += ";!";
          Serial.println(messenger);
          LoRa.beginPacket();
          LoRa.print(messenger);
          LoRa.endPacket();
        }
        else{
          Serial.println("inval_param");
        }
          
      break;

      
    case SET_STATE:
        //error state
        if((bufferSerial[13] != 'O') &&(bufferSerial[13] != 'F')){
            Serial.println("inval_param");
            break;
        }

        // send command
        if(bufferSerial[9] == '2'){
          messenger = "@;0;2;S:";
          messenger += char(bufferSerial[11]);
          messenger += ',';
          messenger += char(bufferSerial[13]);
          messenger += ";!";
//          Serial.println(messenger);
          LoRa.beginPacket();
          LoRa.print(messenger);
          LoRa.endPacket();
        }
        else
          Serial.println("inval_param");
      break;

      
    case IMFORMATION:
        messenger = "@;0;1;I:;!";
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;

      
    case TEMPERATURE:
        messenger = "@;0;";
        messenger += char(bufferSerial[9]);
        messenger += ";T:;!";
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;

      
    case LIGHT:
        messenger = "@;0;";
        messenger += char(bufferSerial[10]);
        messenger += ";L:;!";
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;

      
    case HUMIDITY:
        messenger = "@;0;";
        messenger += char(bufferSerial[8]);
        messenger += ";H:;!";
        LoRa.beginPacket();
        LoRa.print(messenger);
        LoRa.endPacket();
      break;

      
    case ALARM_ENVIRONMENT:
        
        //--- determine data
        for(uint8_t i =0; i< sizeof(bufferSerial); i++){
          if(bufferSerial[i] == ','){
            comma[j] = i;
            j ++;
          }
          if(bufferSerial[i] == '='){
            equal = i;
          }
          if(bufferSerial[i] == 10){
            lineFeed = i;
          }
        }
#ifdef DEBUG
        for(uint8_t i = 0; i< sizeof(bufferSerial); i++){
          Serial.print("buffer[");
          Serial.print(i);
          Serial.print("]: ");
          Serial.println(char(bufferSerial[i]));
        }
#endif        
        // --- set parameter
        alarmAddr = readParameter(equal, comma[0]);
        alarmParam = readParameter(comma[0], comma[1]);
        alarmType = readParameter(comma[1], comma[2]);
        alarmMinute = readParameter(comma[2], comma[3]);
        alarmHour = readParameter(comma[3], lineFeed);

        if(alarmType == 1){
          alarmHour += 100;
        }

        // --- set parameter
        Origin_DataNode[alarmAddr -1][(alarmParam -1)*2] = alarmHour;
        Origin_DataNode[alarmAddr -1][(alarmParam -1)*2 +1] = alarmMinute;
        Process_DataNode[alarmAddr -1][(alarmParam -1)*2] = alarmHour;
        Process_DataNode[alarmAddr -1][(alarmParam -1)*2 +1] = alarmMinute;

        Serial.println("Setup alarm complete!");

#ifdef DEBUG
        //--- debug by serial
        Serial.print("equal: ");
        Serial.println(equal);
        Serial.print("lineFeed: ");
        Serial.println(lineFeed);

        Serial.print("alarmAddr: ");
        Serial.println(alarmAddr);
        Serial.print("alarmParam: ");
        Serial.println(alarmParam);
        Serial.print("alarmType: ");
        Serial.println(alarmType);
        Serial.print("alarmMinute: ");
        Serial.println(alarmMinute);
        Serial.print("alarmHour: ");
        Serial.println(alarmHour);
        Serial.print("alarmState: ");
        Serial.println(alarmState);

        for(uint8_t i = 0; i< sizeof(comma); i++){
          Serial.print("comma[");
          Serial.print(i);
          Serial.print("]: ");
          Serial.println(comma[i]);
        }

        Serial.println("Origin_DataNode:");
        for(uint8_t i = 0; i<20; i++){
          for(uint8_t j = 0; j<2; j++){
            Serial.print("[");
            Serial.print(i);
            Serial.print("][");
            Serial.print(j);
            Serial.print("]: ");
            Serial.print(Origin_DataNode[i][j]);
            Serial.print(", ");
          }
          Serial.println();
        }

        Serial.println("Process_DataNode:");
        for(uint8_t i = 0; i<20; i++){
          for(uint8_t j = 0; j<2; j++){
            Serial.print("[");
            Serial.print(i);
            Serial.print("][");
            Serial.print(j);
            Serial.print("]: ");
            Serial.print(Process_DataNode[i][j]);
            Serial.print(", ");
          }
          Serial.println();
        }
      break;
#endif
      
    case ALARM_CONTROL: 

        //--- determine data
        for(uint8_t i =0; i< sizeof(bufferSerial); i++){
          if(bufferSerial[i] == ','){
            comma[j] = i;
            j ++;
          }
          if(bufferSerial[i] == '='){
            equal = i;
          }
          if(bufferSerial[i] == 10){
            lineFeed = i;
          }
        }
        
        alarmAddr = readParameter(equal, comma[0]);
        alarmRole = readParameter(comma[0], comma[1]);
        alarmType = readParameter(comma[1], comma[2]);
        alarmMinute = readParameter(comma[2], comma[3]);
        alarmHour = readParameter(comma[3], comma[4]);
        alarmState = readParameter(comma[4], lineFeed);


        if(alarmType == 1){
          alarmHour += 100;
        }

        if(alarmState == 'O'){
          alarmHour += 100;
        }

        // --- set parameter
        Origin_DataNode[alarmAddr -1][(alarmRole -1 +4)*2] = alarmHour;
        Origin_DataNode[alarmAddr -1][(alarmRole -1 +4)*2 +1] = alarmMinute;
        Process_DataNode[alarmAddr -1][(alarmRole -1 +4)*2] = alarmHour;
        Process_DataNode[alarmAddr -1][(alarmRole -1 +4)*2 +1] = alarmMinute;

#ifdef DEBUG
        Serial.print("equal: ");
        Serial.println(equal);
        Serial.print("lineFeed: ");
        Serial.println(lineFeed);

        Serial.print("alarmAddr: ");
        Serial.println(alarmAddr);
        Serial.print("alarmRole: ");
        Serial.println(alarmRole);
        Serial.print("alarmType: ");
        Serial.println(alarmType);
        Serial.print("alarmMinute: ");
        Serial.println(alarmMinute);
        Serial.print("alarmHour: ");
        Serial.println(alarmHour);
        Serial.print("alarmState: ");
        Serial.println(alarmState);

        for(uint8_t i = 0; i< sizeof(comma); i++){
          Serial.print("comma[");
          Serial.print(i);
          Serial.print("]: ");
          Serial.println(comma[i]);
        }
#endif
     
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
uint8_t wattingNode(){
  bool reponse = 0;
  Serial.println("wattingNode... ");
  unsigned long timeWait = millis();
  while((millis() - timeWait) <= 10000){
    if(bufferLoRaAvailable()){
      processLoRaCommand();
      reponse = 1;
      break;
    }
  }
  if(reponse == 0){
    Serial.println("TIME OUT: Node no reponse!");
  }
  return reponse;
}



////////////////////////
uint8_t readParameter(uint8_t pos_begin, uint8_t pos_end){
  uint8_t sum = 0;
  uint8_t exponent = 0;
  
  for(uint8_t i = pos_end; i >= pos_begin; i--){
    
    if((bufferSerial[i] >= '1') && (bufferSerial[i] <= '9')){
      
      if(exponent == 0){
        sum += bufferSerial[i] -48;
      }
      else{
        sum += (bufferSerial[i] -48)* pow(10,exponent);
      }

      exponent ++;
    }

    if((bufferSerial[i] >= 'O') || (bufferSerial[i] >= 'F')){
      return bufferSerial[i];
    }
    
  }

  return sum;
}
