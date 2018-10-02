#include "ATcommand.h"
#include "Recieve.h"
#include <avr/interrupt.h>


#define LENGHT_BUFFER_SERIAL      64
#define LINEFEED                  10

#define ERROR_LINE_FEED           
#define ERROR_AT
#define ERROR_PLUS
#define ERROR_TYPE_DEVICE

#define GET_COMMAND               1
#define SET_COMMAND               0

/*** Biến hàm ngắt ISR ***/
volatile uint8_t flagISR = 0;
volatile uint8_t flag_timeOut = 0;
volatile uint8_t countISR = 0;

/*** Biến vị trí dấu câu ***/
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

  return size
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
      for(uint8_t j = pos_T; j< pos_lineFeed; j++){
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
  for(uint8_t i = (pos_plus +1) ; i< pos_lineFeed; i++){
    if(buf[i] == '='){
      pos_equal = i;
    }
  }



  
  //--- Determine '?'
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
      break
      
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
    getStateRole(equal, questMark);
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

void readAddress(uint8_t equal, uint8_t lineFeed){

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

void getLight(uint8_t equal, uint8_t questMark){

  //--- read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< lineFeed; i++){
    address = readAddr(equal, questMark);
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

void getTemperature(uint8_t equal, uint8_t questMark){

  //--- read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< lineFeed; i++){
    address = readAddr(equal, questMark);
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

void getHumidity(uint8_t equal, uint8_t questMark){

  //--- read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< lineFeed; i++){
    address = readAddr(equal, questMark);
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

void getGroundHumidity(uint8_t equal, uint8_t questMark){

  //--- read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< lineFeed; i++){
    address = readAddr(equal, questMark);
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

void getBattery(uint8_t equal, uint8_t questMark){

  // read address
  uint8_t address = 0;
  for(uint8_t i = equal; i< lineFeed; i++){
    address = readAddr(equal, questMark);
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
  for(uint8_t i = equal; i< lineFeed; i++){
    address = readAddr(equal, questMark);
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
      setupTimer();
    }


    // LoRa response
    if(bufferLoRaAvailable()){
      uint8_t typeCommand = typeCommandLoRa();
      
      if(typeCommand == GET_POWER_LORA){
        return readDataLoRa(GET_POWER_LORA);
      }
      
    }
    
  }
  
}




//--------------------------------------------------------

uint8_t setTimer(uint16_t millis){
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
      setupTimer();
    }


    // LoRa response
    if(bufferLoRaAvailable()){
      uint8_t typeCommand = typeCommandLoRa();
      
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
