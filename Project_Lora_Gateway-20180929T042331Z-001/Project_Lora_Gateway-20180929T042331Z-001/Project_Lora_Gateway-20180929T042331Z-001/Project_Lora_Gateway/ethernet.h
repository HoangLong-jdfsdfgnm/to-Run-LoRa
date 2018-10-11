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
#ifndef _ethernet_H
  #define _ethernet_H
#endif
#ifndef _SPI_H
  #define _SPI_H
#endif
/*=============================================================================
                             INCLUDE
=============================================================================*/
#include <SPI.h>
#include <Ethernet.h>
/*=============================================================================
                             DEFINE
=============================================================================*/
uint8_t ethernetConfigure();
void sendServer(String messenger);
void readSever();
void checkEthernet();
