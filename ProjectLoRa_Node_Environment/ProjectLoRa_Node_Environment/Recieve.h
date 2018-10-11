// Date: 8:46-21/4
/*
* @file Sdcard.h
* 
* @version        V1.0.0
* | Written By:    Long.N.H
* | Written On:    31 March 2018
* | Last Updated:  Apirl 2018 - P.Đ.Kha
* #
* 
* @description
* | The funciton in this file used to handel information of Node
* | 
* | 
* #
* 
* @license
* 
******************************************************************************
*/
#ifndef _RECIEVE_H
  #define _RECIEVE_H
  #ifndef _SPI_H
    #define _SPI_H
  #endif
  #ifndef _LORA_H
    #define _LORA_H
  #endif
#endif
/*=============================================================================
                             INCLUDE
=============================================================================*/
#include <SPI.h>
#include <LoRa.h>
/*=============================================================================
                             DEFINE
=============================================================================*/
//FRAME
#define   SYMBOL_HEADER                 0x40      //"@"
#define   SYMBOL_HEADER_Ethernet        0x24      //"$"
#define   SYMBOL_START_DATA             0x3B      //";"
#define   SYMBOL_SEPERATE               0x2C      //","
#define   SYMBOL_SLASH                  0x2F      //"/"
#define   SYMBOL_END                    0x21      //"!"
#define   SYMBOL_CHECK                  0x3F      //"?"

//Address
#define   SYMBOL_GATEWAY                0x30      //"0"
#define   SYMBOL_TEMPRATURE_NODE        0x31      //"1"
#define   SYMBOL_CONTROL_NODE           0x3F      //"2"

//Command
#define   JOIN                          1
#define   ACCEPT                        2
#define   CONFIGURE                     3
#define   IMPORMATION                   4
//#define   GET_STATE                     5
//#define   SET_STATE                     6
#define   TEMPERATURE                   7
#define   LIGHT                         8
#define   HUMIDITY                      9
#define   DATA                          10


/*=============================================================================
                             AVARIABLE
=============================================================================*/

/*=============================================================================
                             FUNCTIONS
=============================================================================*/
void readBufferLoRa(uint8_t bufferLoRa[]);
byte checkFrame(uint8_t buffer_Frame[]);
void recieveCommandLoRa(uint8_t bufferLoRa[]);
void recieveTypeCommand(uint8_t bufferLoRa[]);
uint8_t haveDataLora();
uint8_t sizeBufferLora();
