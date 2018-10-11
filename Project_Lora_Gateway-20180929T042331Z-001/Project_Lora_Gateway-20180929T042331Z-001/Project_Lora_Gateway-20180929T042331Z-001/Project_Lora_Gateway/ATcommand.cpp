#include "ATcommand.h"
#include "Recieve.h"
#include <avr/interrupt.h>


#define LENGHT_BUFFER_SERIAL      64
#define LINEFEED                  10
#define QUESTMARK                 63
#define COMMA                     44

#define ERROR_LINE_FEED           1
#define ERROR_AT                  2
#define ERROR_PLUS                  3
#define ERROR_TYPE_DEVICE             4
#define ERROR_PARAM             4

#define TIME_OUT                   18

#define WORKING                   10
#define GET_COMMAND               1
#define SET_COMMAND               0

#define GET_VER                   18
#define SET_RST                   18
#define GET_POWER_LORA            1
#define GET_DATA_ENVIRONMENT      2
#define SET_DATA_ENVIRONMENT      11
#define GET_STATE_ROLE            12
#define SET_STATE_ROLE            13
#define GET_PARAMETER_NETWORK     14
#define SET_PARAMETER_NETWORK     15
#define GET_PARAMETER_SYSTEM      16
#define SET_PARAMETER_SYSTEM      17

#define GET_LIGHT                   18
#define GET_TEMP                   18
#define GET_HUM                   18
#define GET_GND                   18
#define GET_BAT                   18

/*** Biến hàm ngắt ISR ***/
volatile uint8_t flagISR = 0;
volatile uint8_t flag_timeOut = 0;
volatile uint8_t countISR = 0;

/*** Biến vị trí dấu câu ***/
uint8_t buf[25];
//--------------------------------------------------------

uint8_t readBufferSerial(){
  
  // Delete buffer 1-array
  for(uint8_t i=0; i< LENGHT_BUFFER_SERIAL ; i++){
    buf[i] = 0;
  }


  // read buffer Serial
  uint8_t count = 0;
  while(Serial.available()){
    buf[count] = Serial.read();
    count ++;
    
    if(buf[count] == LINEFEED){
      break;
    }
    
  }

  
  return count;
}






//--------------------------------------------------------

uint8_t sizeofBuf(){
  uint8_t size = 0;

  // count element diference 0
  for(uint8_t i=0; i< LENGHT_BUFFER_SERIAL ; i++){
    if(buf[i] != 0){
      size = i;
    }
  }

  return size;
}







//--------------------------------------------------------
/* đọc bộ đệm
 * 
 * trả về các lỗi <LF>, <AT>, <+>, <?>, <=>
 * trả về dạng lệnh get/set, xác định loại Node
 * ko có chức năng đọc lệnh và thông số
 */

uint8_t determineCommandSerial(){
  readBufferSerial();
  
  //--- Determine <LF>
  uint8_t pos_lineFeed = 0;
  for(uint8_t i = (LENGHT_BUFFER_SERIAL -1); i>0 ; i--){
    if(buf[i] == LINEFEED){
      pos_lineFeed = i;
    }
  }

  // error Line Feed
  if(pos_lineFeed == 0){
    return ERROR_LINE_FEED;
  }


  
  //--- Determine 'AT'
  uint8_t pos_A = 0;
  uint8_t pos_AT = 0;
  for(uint8_t i = 0; i< pos_lineFeed; i++){
    if((buf[i] == 'A') && (buf[i+1] == 'T')){
      pos_A = i;
      for(uint8_t j = pos_A; j< pos_lineFeed; j++){
        if(buf[j] == 'T'){
          pos_AT = j;
        }
      }
    }
  }

  // error 'AT'
  if(pos_AT == 0){
    return ERROR_AT;
  }



  //--- Determine Working
  if(buf[pos_AT +1] == LINEFEED){
    return WORKING;
  }

  if(buf[pos_AT +1] == QUESTMARK){
    return WORKING;
  }




  //--- Determine '+'
  uint8_t pos_plus = 0;
  for(uint8_t i = pos_AT; i< pos_lineFeed; i++){
    if(buf[i] == '+'){
      pos_plus = i;
    }
  }

  // error '+'
  if(pos_plus == 0){
    return ERROR_PLUS;
  }



  //--- Determine type device
  uint8_t typeDevice = 0;
  for(uint8_t i = pos_lineFeed; i > pos_plus; i-- ){
    if((buf[i] >= 'A') && (buf[i] <= 'Z')){
      typeDevice = buf[i];
    }
  }

  // error type deivce
  if(typeDevice == 0){
    return ERROR_TYPE_DEVICE;
  }




  //--- Determine '='
  uint8_t pos_equal = 0;
  for(uint8_t i = (pos_plus +1) ; i< pos_lineFeed; i++){
    if(buf[i] == '='){
      pos_equal = i;
    }
  }



  
  //--- Determine '?'
  uint8_t pos_questMark = 0;
  for(uint8_t i = pos_equal ; i< pos_lineFeed; i++){
    if(buf[i] == '?'){
      pos_questMark = i;
    }
  }



  //--- Determine type command
  bool typeCommand;
  if((pos_equal == 0) && (pos_questMark == 0)){
    typeCommand = GET_COMMAND;
  }
  
  if((pos_equal == 0) && (pos_questMark != 0)){
    typeCommand = GET_COMMAND;
  }
  
  if((pos_equal != 0) && (pos_questMark == 0)){
    typeCommand = SET_COMMAND;
  }
  
  if((pos_equal != 0) && (pos_questMark != 0)){
    typeCommand = GET_COMMAND;
  }


  
  //--- Determine type function
  switch(typeDevice){
    
    case 'E':
        if(typeCommand == GET_COMMAND){
          return GET_DATA_ENVIRONMENT;
        }
        else
          return SET_DATA_ENVIRONMENT;
      break;

      
    case 'C':
        if(typeCommand == GET_COMMAND){
          return GET_STATE_ROLE;
        }
        else
          return SET_STATE_ROLE;
      break;

      
    case 'N':
        if(typeCommand == GET_COMMAND){
          return GET_PARAMETER_NETWORK;
        }
        else
          return SET_PARAMETER_NETWORK;
      break;

      
    case 'S':
        if(typeCommand == GET_COMMAND){
          return GET_PARAMETER_SYSTEM;
        }
        else
          return SET_PARAMETER_SYSTEM;
      break;

      
    case 'V':
        return GET_VER;
      break;

      
    case 'R':
        return SET_RST;
      break;
      
    default:
      break;
  }
}






//--------------------------------------------------------
/* chỉ có chức năng xác định lệnh gì
 * thực hiện các hàm con chức năng ở bên trong
 * cần làm tiếp xác định địa chỉ Node
 */

uint8_t getDataEnvironment(uint8_t plus, uint8_t equal, uint8_t questMark){
  
  String command = "";
  for(uint8_t i = (plus+1); i< equal; i++){
    command += char(buf[i]);
  }


  //--- Type Data
  if(command == "ELIGHT"){
    getLight(equal, questMark);
  }

  if(command == "ETEMP"){
    getTemperature(equal, questMark);
  }

  if(command == "EHUM"){
    getHumidity(equal, questMark);
  }

  if(command == "EGND"){
    getGroundHumidity(equal, questMark);
  }

  if(command == "EBAT"){
    getBattery(equal, questMark);
  }
  
  if(command == "EPWLORA"){
    getPowerLoRa(equal, questMark);
  }

}




//--------------------------------------------------------

uint8_t setDataEnvironment(uint8_t plus, uint8_t equal, uint8_t lineFeed){
  String command = "";
  for(uint8_t i = (plus+1); i< equal; i++){
    command += char(buf[i]);
  }


  //--- Type parameter
  if(command == "ERST"){
    resetNode(equal, lineFeed);
  }

  if(command == "EFRQJOIN"){
    setFreqJoin(equal, lineFeed);
  }

  if(command == "EFRQACCEPT"){
    setFreqAccept(equal, lineFeed);
  }

  if(command == "EPWLORA"){
    setPowerLoRa(equal, lineFeed);
  }

  if(command == "EDRLORA"){
    setDataRateLoRa(equal, lineFeed);
  }
  
  if(command == "EID"){
    setID(equal, lineFeed);
  }
}



//--------------------------------------------------------

uint8_t getStateRole(uint8_t plus, uint8_t equal, uint8_t questMark){
  String command = "";
  for(uint8_t i = (plus+1); i< equal; i++){
    command += char(buf[i]);
  }


  //--- Type Role
  if(command == "CROLE"){
    getState(equal, questMark);
  }
  
  if(command == "CBAT"){
    getBattery(equal, questMark);
  }
  
  if(command == "CPWLORA"){
    getPowerLoRa(equal, questMark);
  }
}




//--------------------------------------------------------

uint8_t setStateRole(uint8_t plus, uint8_t equal, uint8_t lineFeed){
  String command = "";
  for(uint8_t i = (plus+1); i< equal; i++){
    command += char(buf[i]);
  }


  //--- Type parameter
  if(command == "CROLE"){
    controlRole(equal, lineFeed);
  }
  
  if(command == "CRST"){
    resetNode(equal, lineFeed);
  }

  if(command == "CFRQJOIN"){
    setFreqJoin(equal, lineFeed);
  }

  if(command == "CFRQACCEPT"){
    setFreqAccept(equal, lineFeed);
  }

  if(command == "CPWLORA"){
    setPowerLoRa(equal, lineFeed);
  }

  if(command == "CDRLORA"){
    setDataRateLoRa(equal, lineFeed);
  }
  
  if(command == "CID"){
    setID(equal, lineFeed);
  }
  
}




//--------------------------------------------------------

uint8_t readAddress(uint8_t equal, uint8_t lineFeed){

  //--- determine comma
  uint8_t comma = lineFeed;
  for(uint8_t i = lineFeed; i> equal; i--){
    if(buf[i] == COMMA){
      comma = i;
    }
  }

  
  //--- error comma
  uint8_t exponential = 0; 
  uint8_t sum = 0;
  for(uint8_t i = comma; i> equal; i--){
    if((buf[i] >= '0') && (buf[i] <= '9')){
      if(exponential != 0){
        sum += (buf[i] - 48)* pow(10,exponential);
      }
      else{
        sum += buf[i] - 48;
      }
      exponential ++;
    }
  }
}




//--------------------------------------------------------

uint8_t getLight(uint8_t equal, uint8_t questMark){

  //--- read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< questMark; i++){
    address = readAddress(equal, questMark);
  }


  //--- send messenger
  String messenger = "@;0;";
  messenger += address;
  messenger += ";L:;!";
  LoRa_transmit(messenger);

  //--- watting resposen
  wattingReponse(GET_LIGHT);
}


//--------------------------------------------------------

uint8_t getTemperature(uint8_t equal, uint8_t questMark){

  //--- read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< questMark; i++){
    address = readAddress(equal, questMark);
  }


  //--- send messenger
  String messenger = "@;0;";
  messenger += address;
  messenger += ";T:;!";
  LoRa_transmit(messenger);

  //--- watting resposen
  wattingReponse(GET_TEMP);
}



//--------------------------------------------------------

uint8_t getHumidity(uint8_t equal, uint8_t questMark){

  //--- read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< questMark; i++){
    address = readAddress(equal, questMark);
  }


  //--- send messenger
  String messenger = "@;0;";
  messenger += address;
  messenger += ";H:;!";
  LoRa_transmit(messenger);

  //--- watting resposen
  wattingReponse(GET_HUM);
}




//--------------------------------------------------------

uint8_t getGroundHumidity(uint8_t equal, uint8_t questMark){

  //--- read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< questMark; i++){
    address = readAddress(equal, questMark);
  }


  //--- send messenger
  String messenger = "@;0;";
  messenger += address;
  messenger += ";G:;!";
  LoRa_transmit(messenger);


  //--- watting resposen
  wattingReponse(GET_GND);
}





//--------------------------------------------------------

uint8_t getBattery(uint8_t equal, uint8_t questMark){

  // read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< questMark; i++){
    address = readAddress(equal, questMark);
  }


  // send messenger
  String messenger = "@;0;";
  messenger += address;
  messenger += ";B:;!";
  LoRa_transmit(messenger);


  //--- watting resposen
  wattingReponse(GET_BAT);
  
}





//--------------------------------------------------------

uint8_t getPowerLoRa(uint8_t equal, uint8_t questMark){

  //--- read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< questMark; i++){
    address = readAddress(equal, questMark);
  }


  //--- send messenger
  String messenger = "@;0;";
  messenger += address;
  messenger += ";P:;!";
  LoRa_transmit(messenger);



  //--- watting resposen
  
  while(1){

    // timeout
    if(flag_timeOut == 1){
      TIMSK1 = 0;
      cli();
      flag_timeOut == 0;
      return TIME_OUT;
    }

    
    // interrupt
    if(flagISR == 1){
      flagISR = 0;
      setupTimer(1000);
    }


    // LoRa response
    if(bufferLoRaAvailable()){
      uint8_t typeCommand = 0;//typeCommandLoRa();
      
      if(typeCommand == GET_POWER_LORA){
        return readDataLoRa(GET_POWER_LORA);
      }
      
    }
    
  }
  
}




//--------------------------------------------------------

uint8_t setupTimer(uint16_t millis){
  if(millis < 250){
    return 0;
  }

  cli();                                  // tắt ngắt toàn cục
  
  /* Reset Timer/Counter1 */
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  
  /* Setup Timer/Counter1 */
  
  TCCR1B |= (1 << CS11) | (1 << CS10);    // prescale = 64
  TCNT1 = 65536 - (millis/4);
  TIMSK1 = (1 << TOIE1);                  // Overflow interrupt enable 
  sei();                                  // cho phép ngắt toàn cục
}




//--------------------------------------------------------

ISR (TIMER1_OVF_vect){
    countISR ++;
    
    if (countISR == (timeOut/200)){
      flag_timeOut = 1;
    }
    else
      flagISR = 1;
      
}


uint8_t wattingReponse(uint8_t commandSerial){
  while(1){

    // timeout
    if(flag_timeOut == 1){
      TIMSK1 = 0;
      cli();
      flag_timeOut == 0;
      return TIME_OUT;
    }

    
    // interrupt
    if(flagISR == 1){
      flagISR = 0;
      setupTimer(1000);
    }


    // LoRa response
    if(bufferLoRaAvailable()){
      uint8_t typeCommand = 0;//typeCommandLoRa();
      
      if(typeCommand == GET_POWER_LORA){
        return readDataLoRa(GET_POWER_LORA);
      }
      
    }
    
  }
}




//--------------------------------------------------------

uint8_t determineParameter(uint8_t index, uint8_t type, uint8_t equal, uint8_t lineFeed){
  uint8_t comma[5] = {0, 0, 0, 0, 0};
  uint8_t index_param = 0;
  uint8_t j = 0;

  //--- determine comma
  for(uint8_t i = equal; i< lineFeed; i++){
    if(buf[i] == COMMA){
      comma[j] = i;
      j ++;
    }
  }

  //--- error parameter
  if(j< index){
    return ERROR_PARAM;
  }
  else{
    
  }
  
}

//----------------------------------------------------------

void resetNode(uint8_t equal, uint8_t lineFeed){
  
}

//-----------------------------------------------------------
void setFreqJoin(uint8_t equal, uint8_t lineFeed){
  
}

//-----------------------------------------------------------
void setFreqAccept(uint8_t equal, uint8_t lineFeed){
  
}

//-----------------------------------------------------------
void setPowerLoRa(uint8_t equal, uint8_t lineFeed){
  
}

//-----------------------------------------------------------
void setDataRateLoRa(uint8_t equal, uint8_t lineFeed){
  
}

//-----------------------------------------------------------
void setID(uint8_t equal, uint8_t lineFeed){
  
}

//-----------------------------------------------------------
uint8_t getState(uint8_t equal, uint8_t questMark){
  
}


//-----------------------------------------------------------
void controlRole(uint8_t equal, uint8_t lineFeed){
  
}

//-----------------------------------------------------------

void LoRa_transmit(String messenger){
  
}

uint8_t readDataLoRa(uint8_t command){
  
}

//uint8_t bufferLoRaAvailable(){
//  
//}
