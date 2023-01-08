/******************************************************************************************
* Copyright 2017 Ideetron B.V.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
/****************************************************************************************
* File:     RFM95.cpp
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         06-01-2017
* Supported Hardware: ID150119-02 Nexus board with RFM95
****************************************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include "RFM95.h"
#include "Config.h"

/**
 *  Lora Frequencies
 */

#if defined(AS_923)
  static const PROGMEM unsigned char  LoRa_Frequency[9][3] = {//[921.2 - 922.6] MHz
    { 0xE6, 0x59, 0xC0 }, //Channel [0], 921.4 MHz / 61.035 Hz = x = 0xE659C0
    { 0xE6, 0x66, 0x8D }, //Channel [1], 921.6 MHz / 61.035 Hz = x = 0xE6668D
    { 0xE6, 0x4C, 0xF3 }, //Channel [2], 921.2 MHz / 61.035 Hz = x = 0xE64CF3
    { 0xE6, 0x73, 0x5A }, //Channel [3], 921.8 MHz / 61.035 Hz = x = 0xE6735A
    { 0xE6, 0x80, 0x27 }, //Channel [4], 922.0 MHz / 61.035 Hz = x = 0xE68027
    { 0xE6, 0x8C, 0xF3 }, //Channel [5], 922.2 MHz / 61.035 Hz = x = 0xE68CF3
    { 0xE6, 0x99, 0xC0 }, //Channel [6], 922.4 MHz / 61.035 Hz = x = 0xE699C0
    { 0xE6, 0xA6, 0x8D }, //Channel [7], 922.6 MHz / 61.035 Hz = x = 0xE6A68D
  };
#endif

/*
*****************************************************************************************
* Description : Function that reads a register from the RFM and returns the value
*
* Arguments   : RFM_Address Address of register to be read
*
* Returns   : Value of the register
*****************************************************************************************
*/

static unsigned char RFM_Read(unsigned char RFM_Address)
{
  unsigned char RFM_Data;

  //Add transactions in Read and Write methods
  SPI.beginTransaction(SPISettings(4000000,MSBFIRST,SPI_MODE0));
  
  //Set NSS pin low to start SPI communication
  digitalWrite(RFM_pins.CS,LOW);

  //Send Address
  SPI.transfer(RFM_Address);
  //Send 0x00 to be able to receive the answer from the RFM
  RFM_Data = SPI.transfer(0x00);

  //Set NSS high to end communication
  digitalWrite(RFM_pins.CS,HIGH);

  //End the transaction so that other hardware can use the bus
  SPI.endTransaction();

  #ifdef DEBUG
  Serial.print("SPI Read ADDR: ");
  Serial.print(RFM_Address, HEX);
  Serial.print(" DATA: ");
  Serial.println(RFM_Data, HEX);
  #endif

  //Return received data
  return RFM_Data;
}
/********************************************************************************************
* Description : Change Spread Factor and Band Width
* 
* Arguments:    _SF = {6,7,8,9,10,11,12}
*               _BW = {0x00 -> 7.8khz   , 0x01 -> 10.4khz, 0x02 -> 15.6khz, 0x03 -> 20.8khz,
*                      0x04 -> 31.25khz , 0x05 -> 41.7khz, 0x06 -> 62.5khz, 0x07 -> 125khz, 
*                      0x08 -> 250khz   , 0x09 -> 500khz}
********************************************************************************************/
static void RFM_change_SF_BW(unsigned char _SF, unsigned char _BW)
{
	RFM_Write(RFM_REG_MODEM_CONFIG2, (_SF << 4) | 0b0100); //SFx CRC On
	RFM_Write(RFM_REG_MODEM_CONFIG1,(_BW << 4) | 0x02); //x kHz 4/5 coding rate explicit header mode

  if(_SF==12 || _SF==11)
    RFM_Write(RFM_REG_MODEM_CONFIG3, 0x0C); //Low datarate optimization on AGC auto on
  else
    RFM_Write(RFM_REG_MODEM_CONFIG3, 0x04); //Mobile node, low datarate optimization on AGC acorging to register LnaGain
}
/*
*****************************************************************************************
* Description : Function to change the datarate of the RFM module. Setting the following
*				register: Spreading factor, Bandwidth and low datarate optimisation.
*
* Arguments   : Datarate the datarate to set
*****************************************************************************************
*/
static void RFM_Change_Datarate(unsigned char Datarate)
{
#if defined(AS_923)
  switch (Datarate) {
  case 0x00:  // SF12BW125
    RFM_change_SF_BW(12,0x07);
    break;
  case 0x01:  // SF11BW125
    RFM_change_SF_BW(11,0x07);
    break;
  case 0x02:  // SF10BW125
    RFM_change_SF_BW(10,0x07);
    break;
  case 0x03:  // SF9BW125
    RFM_change_SF_BW(9,0x07);
    break;
  case 0x04:  // SF8BW125
    RFM_change_SF_BW(8,0x07);
    break;
  case 0x05:  // SF7BW125
    RFM_change_SF_BW(7,0x07);
    break;
  case 0x06:  // SF7BW250
    RFM_change_SF_BW(7,0x08);
    break;
  }
#endif
}
/*
*****************************************************************************************
* Description : Function to change the channel of the RFM module. Setting the following
*				register: Channel
*
* Arguments   : Channel the channel to set
*****************************************************************************************
*/
static void RFM_Change_Channel(unsigned char Channel)
{
#if defined(AS_923)
  if (Channel <= 0x08)
    for(unsigned char i = 0 ; i < 3 ; ++i)
      RFM_Write(RFM_REG_FR_MSB + i, pgm_read_byte(&(LoRa_Frequency[Channel][i])));
#endif
}

/*
*****************************************************************************************
* Description: Function used to initialize the RFM module on startup
*****************************************************************************************
*/
bool RFM_Init()
{
  
  uint8_t ver = RFM_Read(0x42);
  if(ver!=18){
    return 0;
  }
  //Switch RFM to sleep
  //DON'T USE Switch mode function
  RFM_Write(RFM_REG_OP_MODE, RFM_MODE_SLEEP);

  //Wait until RFM is in sleep mode
  delay(50);
 
  //Set RFM in LoRa mode
  //DON'T USE Switch mode function
  RFM_Write(RFM_REG_OP_MODE ,RFM_MODE_LORA);
  //Switch RFM to standby
  RFM_Switch_Mode(RFM_MODE_STANDBY);
  //Set channel to channel 0
  RFM_Change_Channel(CH0);

  RFM_Write(RFM_REG_PA_DAC,0x87);
  //PA pin (minimal power)
  //RFM_Write(0x09,0xF0);
  //set to 17dbm
  RFM_Write(RFM_REG_PA_CONFIG,0x8F);
  RFM_Write(RFM_REG_OCP,0x00);

  //Switch LNA boost on
  RFM_Write(RFM_REG_LNA,0x23);

  //Set RFM To datarate 0 SF12 BW 125 kHz
  RFM_Change_Datarate(0x00);

  //Rx Timeout set to 37 symbols
  RFM_Write(RFM_REG_SYM_TIMEOUT_LSB, 0x25);

  //Preamble length set to 8 symbols
  //0x0008 + 4 = 12
  RFM_Write(RFM_REG_PREAMBLE_MSB,0x00);
  RFM_Write(RFM_REG_PREAMBLE_LSB,0x08);

  //Set LoRa sync word
  RFM_Write(RFM_REG_SYNC_WORD, 0x34);

  //Set FIFO pointers
  //TX base address
  RFM_Write(0x0E,0x80);
  //Rx base address
  RFM_Write(0x0F,0x00);
  return 1;
}

/*
*****************************************************************************************
* Description : Function for sending a package with the RFM
*
* Arguments   : *RFM_Tx_Package pointer to buffer with data and counter of data
*               *LoRa_Settings pointer to sSettings struct
*****************************************************************************************
*/

void RFM_Send_Package(sBuffer *RFM_Tx_Package, sSettings *LoRa_Settings)
{
  unsigned char i;
  unsigned char RFM_Tx_Location = 0x00;

  //Set RFM in Standby mode
  RFM_Switch_Mode(RFM_MODE_STANDBY);

  //Switch Datarate
  RFM_Change_Datarate(LoRa_Settings->Datarate_Tx);

  //Switch Channel
  RFM_Change_Channel(LoRa_Settings->Channel_Tx);

  //Switch DIO0 to TxDone
  RFM_Write(RFM_REG_DIO_MAPPING1, 0x40);

  //Set IQ to normal values
  RFM_Write(RFM_REG_INVERT_IQ,0x27);
  RFM_Write(RFM_REG_INVERT_IQ2,0x1D);

  //Set payload length to the right length
  RFM_Write(RFM_REG_PAYLOAD_LENGTH,RFM_Tx_Package->Counter);

  //Get location of Tx part of FiFo
  RFM_Tx_Location = RFM_Read(0x0E);

  //Set SPI pointer to start of Tx part in FiFo
  RFM_Write(RFM_REG_FIFO_ADDR_PTR, RFM_Tx_Location);

  //Write Payload to FiFo
  for (i = 0;i < RFM_Tx_Package->Counter; i++)
  {
    RFM_Write(RFM_REG_FIFO, RFM_Tx_Package->Data[i]);
  }

  //Switch RFM to Tx
  RFM_Write(RFM_REG_OP_MODE,0x83);

  //Wait for TxDone
  while(digitalRead(RFM_pins.DIO0) == LOW);

  //Clear interrupt
  RFM_Write(RFM_REG_IRQ_FLAGS,0x08);

  //Switch RFM back to receive if it is a type C mote
  if(LoRa_Settings->Mote_Class == CLASS_C)
  {
    //Switch Back to Continuous receive
    RFM_Continuous_Receive(LoRa_Settings);
  }
}

/*
*****************************************************************************************
* Description : Function to switch RFM to single receive mode, used for Class A motes
*
* Arguments   : *LoRa_Settings pointer to sSettings struct
*
* Return	  : message_t Status of the received message
*****************************************************************************************
*/

message_t RFM_Single_Receive(sSettings *LoRa_Settings)
{
  message_t Message_Status = NO_MESSAGE;

  //Change DIO 0 back to RxDone
  RFM_Write(RFM_REG_DIO_MAPPING1, 0x00);

  //Invert IQ Back
  RFM_Write(RFM_REG_INVERT_IQ, 0x67);
  RFM_Write(RFM_REG_INVERT_IQ2, 0x19);

  //Change Datarate
  RFM_Change_Datarate(LoRa_Settings->Datarate_Rx);

  //Change Channel
  RFM_Change_Channel(LoRa_Settings->Channel_Rx);

  //Switch RFM to Single reception
  RFM_Switch_Mode(RFM_MODE_RXSINGLE);

  //Wait until RxDone or Timeout
  //Wait until timeout or RxDone interrupt
  while((digitalRead(RFM_pins.DIO0) == LOW) && (digitalRead(RFM_pins.DIO1) == LOW));

  //Check for Timeout
  if(digitalRead(RFM_pins.DIO1) == HIGH)
  {
    //Clear interrupt register
    RFM_Write(RFM_REG_IRQ_FLAGS,0xE0);
    Message_Status = TIMEOUT;
  }

  //Check for RxDone
  if(digitalRead(RFM_pins.DIO0) == HIGH)
  {
	  Message_Status = NEW_MESSAGE;
  }

  return Message_Status;
}


/*
*****************************************************************************************
* Description : Function to switch RFM to continuous receive mode, used for Class C motes
*
* Arguments   : *LoRa_Settings pointer to sSettings struct
*****************************************************************************************
*/
void RFM_Continuous_Receive(sSettings *LoRa_Settings)
{
  //Change DIO 0 back to RxDone and DIO 1 to rx timeout
  RFM_Write(RFM_REG_DIO_MAPPING1,0x00);

  //Invert IQ Back
  RFM_Write(RFM_REG_INVERT_IQ, 0x67);
  RFM_Write(RFM_REG_INVERT_IQ2, 0x19);
  
	//Change Datarate and channel.
  // This depends on regional parameters

	RFM_Change_Datarate(LoRa_Settings->Datarate_Rx);
	RFM_Change_Channel(LoRa_Settings->Channel_Rx);


	//Switch to continuous receive
	RFM_Switch_Mode(RFM_MODE_RXCONT);
}

/*
*****************************************************************************************
* Description : Function to retrieve a message received by the RFM
*
* Arguments   : *RFM_Rx_Package pointer to sBuffer struct containing the data received
*				and number of bytes received
*
* Return	  : message_t Status of the received message
*****************************************************************************************
*/

message_t RFM_Get_Package(sBuffer *RFM_Rx_Package)
{
  unsigned char i;
  unsigned char RFM_Interrupts = 0x00;
  unsigned char RFM_Package_Location = 0x00;
  message_t Message_Status;

  //Get interrupt register
  RFM_Interrupts = RFM_Read(RFM_REG_IRQ_FLAGS);

  //Check CRC
  if((RFM_Interrupts & 0x20) != 0x20)
  {
	  Message_Status = CRC_OK;
  }
  else
  {
	  Message_Status = WRONG_MESSAGE;
  }

  RFM_Package_Location = RFM_Read(0x10); /*Read start position of received package*/
  RFM_Rx_Package->Counter = RFM_Read(0x13); /*Read length of received package*/

  RFM_Write(RFM_REG_FIFO_ADDR_PTR,RFM_Package_Location); /*Set SPI pointer to start of package*/

  for (i = 0x00; i < RFM_Rx_Package->Counter; i++)
  {
    RFM_Rx_Package->Data[i] = RFM_Read(RFM_REG_FIFO);
  }

  //Clear interrupt register
  RFM_Write(RFM_REG_IRQ_FLAGS,0xE0);

  return Message_Status;
}

/*
*****************************************************************************************
* Description : Function that writes a register from the RFM
*
* Arguments   : RFM_Address Address of register to be written
*         RFM_Data    Data to be written
*****************************************************************************************
*/

void RFM_Write(unsigned char RFM_Address, unsigned char RFM_Data)
{
  // br: SPI Transfer Debug
  #ifdef DEBUG
    Serial.print("SPI Write ADDR: ");
    Serial.print(RFM_Address, HEX);
    Serial.print(" DATA: ");
    Serial.println(RFM_Data, HEX);
  #endif

  //Add transactions in Read and Write methods
  SPI.beginTransaction(SPISettings(4000000,MSBFIRST,SPI_MODE0));

  //Set NSS pin Low to start communication
  digitalWrite(RFM_pins.CS,LOW);

  //Send Address with MSB 1 to make it a writ command
  SPI.transfer(RFM_Address | 0x80);
  //Send Data
  SPI.transfer(RFM_Data);

  //Set NSS pin High to end communication
  digitalWrite(RFM_pins.CS,HIGH);

  //End the transaction so that other hardware can use the bus
  SPI.endTransaction();
}

/*
*****************************************************************************************
* Description : Function to change the operation mode of the RFM. Switching mode and wait
*				for mode ready flag
*               DO NOT USE FOR SLEEP
*
* Arguments   : Mode the mode to set
*****************************************************************************************
*/
void RFM_Switch_Mode(unsigned char Mode)
{
    Mode = Mode | 0x80; //Set high bit for LoRa mode

    //Switch mode on RFM module
    RFM_Write(RFM_REG_OP_MODE,Mode);
}

