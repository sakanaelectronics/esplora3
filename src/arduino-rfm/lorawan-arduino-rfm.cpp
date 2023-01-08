/*
    loraid-arduino-rfm.cpp
    Copyright © 2018 lora.id. All right reserved.

    Author: Andri Rahmadhani
    Date: 2018-04-25

    Encapsulate Ideetron LoRaWAN simple node demonstrator
    *This fimrware supports
        *Over The Air Activation
        *Activation By Personalization
        *Class switching between Class A and Class C motes
        *Channel hopping
        *
        *The following settings can be done
        *Channel Receive and Transmit
        *Datarate Receive and Transmit
        *Transmit power
        *Confirmed or unconfirmed messages
        *Device Address
        *Application Session Key
        *Network Session Key
        *Device EUI
        *Application EUI
        *Application key
        *Mote Class

    Use of this source code is governed by the MIT license that can be found in the LICENSE file.
*/


#include "lorawan-arduino-rfm.h"
#include "Conversions.h"

LoRaWANClass::LoRaWANClass()
{

}

LoRaWANClass::~LoRaWANClass()
{

}

bool LoRaWANClass::init(void)
{
    // Lora Setting Class
    dev_class = CLASS_A;
    // Random seed
    randomSeed(analogRead(0));

    // Status
    RFM_Command_Status = NO_RFM_COMMAND;
    Rx_Status = NO_RX;

    // current channel
    currentChannel = MULTI;

    // Initialise session data struct (Semtech default key)
    memset(Address_Tx, 0x00, 4);
    memset(NwkSKey, 0x00, 16);
    memset(AppSKey, 0x00, 16);
    
    Frame_Counter_Tx = 0x0000;
    Session_Data.NwkSKey = NwkSKey;
    Session_Data.AppSKey = AppSKey;
    Session_Data.DevAddr = Address_Tx;
    Session_Data.Frame_Counter = &Frame_Counter_Tx;

    //Initialize OTAA data struct
    memset(DevEUI, 0x00, 8);
    memset(AppEUI, 0x00, 8);

    memset(AppKey, 0x00, 16);
    memset(DevNonce, 0x00, 2);
    memset(AppNonce, 0x00, 3);
    memset(NetID, 0x00, 3);
    OTAA_Data.DevEUI = DevEUI;
    OTAA_Data.AppEUI = AppEUI;
    OTAA_Data.AppKey = AppKey;
    OTAA_Data.DevNonce = DevNonce;
    OTAA_Data.AppNonce = AppNonce;
    OTAA_Data.NetID = NetID;
    
    // Device Class
    LoRa_Settings.Mote_Class = 0x00; //0x00 is type A, 0x01 is type C
    
    // Rx
#if defined(AS_923)
    LoRa_Settings.Datarate_Rx = SF10BW125;   //set to SF10 BW 125 kHz
    LoRa_Settings.Channel_Rx = 0x00;    // set to recv channel
#endif
     LoRa_Settings.Frame_Port_Tx =0x01; //set frameport tx 1

    // Tx
    LoRa_Settings.Channel_Tx = 0x00;    // set to channel 0

    LoRa_Settings.Confirm = 0x00; //0x00 unconfirmed, 0x01 confirmed
    LoRa_Settings.Channel_Hopping = 0x00; //0x00 no channel hopping, 0x01 channel hopping

    // Initialise buffer for data to transmit
    memset(Data_Tx, 0x00, sizeof(Data_Tx));
    Buffer_Tx.Data = Data_Tx;
    Buffer_Tx.Counter = 0x00;
    // Initialise buffer for data to receive
    memset(Data_Rx, 0x00, sizeof(Data_Rx));
    Buffer_Rx.Data = Data_Rx;
    Buffer_Rx.Counter = 0x00;
    Message_Rx.Direction = 0x01; //Set down direction for Rx message

    //Initialize I/O pins
    pinMode(RFM_pins.DIO0,INPUT);
    pinMode(RFM_pins.DIO1,INPUT);
    #ifdef BOARD_DRAGINO_SHIELD
    pinMode(RFM_pins.DIO5,INPUT);
    #endif
    pinMode(RFM_pins.DIO2,INPUT);
    pinMode(RFM_pins.CS,OUTPUT);
    pinMode(RFM_pins.RST,OUTPUT);
    
    digitalWrite(RFM_pins.CS,HIGH);
    
    // Reset
    digitalWrite(RFM_pins.RST,HIGH);
    delay(10);
    digitalWrite(RFM_pins.RST,LOW);
    delay(10);
    digitalWrite(RFM_pins.RST,HIGH);

    //Initialise the SPI port
    SPI.begin();
    
    /*** This prevents the use of other SPI devices with different settings ***/
    //SPI.beginTransaction(SPISettings(4000000,MSBFIRST,SPI_MODE0)); 


    //Wait until RFM module is started
    delay(50);
    
    //Initialize RFM module
    if(!RFM_Init()){
    return 0;
    }
    return 1;
}

bool LoRaWANClass::join(void)
{    
    bool join_status;
    const unsigned long timeout = 6000;
    unsigned long prev_millis;
    unsigned char i = 0; 
    unsigned char freq_idx;

    if (currentChannel == MULTI) {
        
        freq_idx=random(2,4); //921.4MHz and 921.6MHz channel
        LoRa_Settings.Channel_Tx =freq_idx;
        LoRa_Settings.Channel_Rx =freq_idx;
    }
    // join request
    LoRa_Send_JoinReq(&OTAA_Data, &LoRa_Settings);

    // loop for <timeout> wait for join accept
    prev_millis = millis();
    do {
        join_status = LORA_join_Accept(&Buffer_Rx, &Session_Data, &OTAA_Data, &Message_Rx, &LoRa_Settings);

    }while ((millis() - prev_millis) < timeout && !join_status);

    return join_status;
}

void LoRaWANClass::setDevEUI(const char *devEUI_in)
{
    for(byte i = 0; i < 8; ++i)
        DevEUI[i] = ASCII2Hex(devEUI_in[i*2],devEUI_in[(i*2) + 1]);
    //Reset frame counter
    Frame_Counter_Tx = 0x0000;

    //Reset RFM command status
    RFM_Command_Status = NO_RFM_COMMAND;
}

void LoRaWANClass::setAppEUI(const char *appEUI_in)
{
    for(byte i = 0; i < 8; ++i)
        AppEUI[i] = ASCII2Hex(appEUI_in[i*2],appEUI_in[(i*2) + 1]);
    //Reset frame counter
    Frame_Counter_Tx = 0x0000;

    //Reset RFM command status
    RFM_Command_Status = NO_RFM_COMMAND;
}

void LoRaWANClass::setAppKey(const char *appKey_in)
{
    for(byte i = 0; i < 16; ++i)
        AppKey[i] = ASCII2Hex(appKey_in[i*2],appKey_in[(i*2) + 1]);
    //Reset frame counter
    Frame_Counter_Tx = 0x0000;

    //Reset RFM command status
    RFM_Command_Status = NO_RFM_COMMAND;

}

void LoRaWANClass::setNwkSKey(const char *NwkKey_in)
{
    for (uint8_t i = 0; i < 16; ++i)
        NwkSKey[i] = ASCII2Hex(NwkKey_in[i*2],NwkKey_in[(i*2)+1]);

    //Reset frame counter
    Frame_Counter_Tx = 0x0000;

    //Reset RFM command status
    RFM_Command_Status = NO_RFM_COMMAND;
}

void LoRaWANClass::setAppSKey(const char *ApskKey_in)
{
    for (uint8_t i = 0; i < 16; ++i)
        AppSKey[i] = ASCII2Hex(ApskKey_in[i*2],ApskKey_in[(i*2)+1]);
    
    //Reset frame counter
    Frame_Counter_Tx = 0x0000;

    //Reset RFM command status
    RFM_Command_Status = NO_RFM_COMMAND;
}

void LoRaWANClass::setDevAddr(const char *devAddr_in)
{
    memset(Session_Data.DevAddr, 0x30, sizeof(Session_Data.DevAddr));

    //Check if it is a set command and there is enough data sent
    Address_Tx[0] = ASCII2Hex(devAddr_in[0],devAddr_in[1]);
    Address_Tx[1] = ASCII2Hex(devAddr_in[2],devAddr_in[3]);
    Address_Tx[2] = ASCII2Hex(devAddr_in[4],devAddr_in[5]);
    Address_Tx[3] = ASCII2Hex(devAddr_in[6],devAddr_in[7]);

    //Reset frame counter
    Frame_Counter_Tx = 0x0000;

    //Reset RFM command status
    RFM_Command_Status = NO_RFM_COMMAND;
}

void LoRaWANClass::setDeviceClass(devclass_t dev_class)
{
    LoRa_Settings.Mote_Class = (dev_class == CLASS_A)? CLASS_A : CLASS_C;

    if (LoRa_Settings.Mote_Class == CLASS_A) {
        RFM_Switch_Mode(RFM_MODE_STANDBY);
    } else {
        RFM_Continuous_Receive(&LoRa_Settings);
    }

    //Reset RFM command
    RFM_Command_Status = NO_RFM_COMMAND;
}

void LoRaWANClass::sendUplink(char *data, unsigned int len, unsigned char confirm)
{
    if (currentChannel == MULTI) {
        randomChannel();
    }

    if(((LoRa_Settings.Channel_Tx == 0)||(LoRa_Settings.Channel_Tx == 1)||(LoRa_Settings.Channel_Tx == 2)||(LoRa_Settings.Channel_Tx == 3)||(LoRa_Settings.Channel_Tx == 4)||(LoRa_Settings.Channel_Tx == 6))&&(drate_common>5))
        LoRa_Settings.Datarate_Tx=0x05;
 
    else if((LoRa_Settings.Channel_Tx==5)&&(drate_common>6))
        LoRa_Settings.Datarate_Tx=0x06;

    else LoRa_Settings.Datarate_Tx=drate_common;

    if(LoRa_Settings.Mote_Class==CLASS_C)
    {
        LoRa_Settings.Channel_Rx = 0x00;
         LoRa_Settings.Datarate_Rx=SF10BW125;
    }

    LoRa_Settings.Confirm = (confirm == 0) ? 0 : 1;

    //Set new command for RFM
    RFM_Command_Status = NEW_RFM_COMMAND;
    
    Buffer_Tx.Counter = len;
    memcpy(Buffer_Tx.Data,data,len);
}

void LoRaWANClass::sendUplinkHex(byte *data, unsigned int len, unsigned char confirm)
{
    if (currentChannel == MULTI) {
        randomChannel();
    }

    if(((LoRa_Settings.Channel_Tx == 0)||(LoRa_Settings.Channel_Tx == 1)||(LoRa_Settings.Channel_Tx == 2)||(LoRa_Settings.Channel_Tx == 3)||(LoRa_Settings.Channel_Tx == 4)||(LoRa_Settings.Channel_Tx == 6))&&(drate_common>5))
        LoRa_Settings.Datarate_Tx=0x05;
 
    else if((LoRa_Settings.Channel_Tx==5)&&(drate_common>6))
        LoRa_Settings.Datarate_Tx=0x06;

    else LoRa_Settings.Datarate_Tx=drate_common;

    if(LoRa_Settings.Mote_Class==CLASS_C)
    {
        LoRa_Settings.Channel_Rx = 0x00;
         LoRa_Settings.Datarate_Rx=SF10BW125;
    }

    LoRa_Settings.Confirm = (confirm == 0) ? 0 : 1;

    //Set new command for RFM
    RFM_Command_Status = NEW_RFM_COMMAND;
    
    Buffer_Tx.Counter = len;
    memcpy(Buffer_Tx.Data,data,len);
}

void LoRaWANClass::setDataRate(unsigned char data_rate)
{
    drate_common = data_rate;
    if(drate_common <= 0x04){
        LoRa_Settings.Datarate_Tx = drate_common;
        LoRa_Settings.Datarate_Rx = data_rate + 0x0A;
    }
    RFM_Command_Status = NO_RFM_COMMAND;
}

void LoRaWANClass::setChannel(unsigned char channel)
{
    if (channel <= 7) {
        currentChannel = channel;
        LoRa_Settings.Channel_Tx = channel;
    } else if (channel == MULTI) {
        currentChannel = MULTI;
    }
}

unsigned char LoRaWANClass::getChannel()
{
    return LoRa_Settings.Channel_Tx;
}

int LoRaWANClass::getChannelFreq(unsigned char channel)
{
    if(channel==0)return 921400000;
    else if(channel==1)return 921600000;
    else if(channel==2)return 921200000;
    else if(channel==3)return 921800000;
    else if(channel==4)return 922000000;
    else if(channel==5)return 922200000;
    else if(channel==6)return 922400000;
    else if(channel==7)return 922600000;
}


unsigned char LoRaWANClass::getChannelRx()
{
    return LoRa_Settings.Channel_Rx;
}

int LoRaWANClass::getChannelRxFreq(unsigned char channel)
{
    if(channel==0)return 921400000;
    else if(channel==1)return 921600000;
    else if(channel==2)return 921200000;
    else if(channel==3)return 921800000;
    else if(channel==4)return 922000000;
    else if(channel==5)return 922200000;
    else if(channel==6)return 922400000;
    else if(channel==7)return 922600000;
}

unsigned char LoRaWANClass::getDataRate() {
    return LoRa_Settings.Datarate_Tx;
}

unsigned char LoRaWANClass::getDataRateRx() {
    return LoRa_Settings.Datarate_Rx;
}

void LoRaWANClass::setTxPower(unsigned char power_idx)
{
    unsigned char RFM_Data;
    LoRa_Settings.Transmit_Power = (power_idx > 0x0F) ? 0x0F : power_idx; 
    RFM_Data = LoRa_Settings.Transmit_Power + 0x80;
    RFM_Write(RFM_REG_PA_CONFIG, RFM_Data);
}

int LoRaWANClass::getTxPower()
{
    return LoRa_Settings.Transmit_Power;
}

int LoRaWANClass::readData(char *outBuff)
{
    int res = 0;
    //If there is new data
    if(Rx_Status == NEW_RX)
    {
        res = Buffer_Rx.Counter;
        memset(outBuff, 0x00, res + 1);
        memcpy(outBuff, Buffer_Rx.Data, res);
        
        // Clear Buffer counter
        Buffer_Rx.Counter = 0x00;
        
        Rx_Status = NO_RX;
    }

    return res;
}

int LoRaWANClass::readDataByte(byte *outBuff)
{
    int res = 0;
    //If there is new data
    if(Rx_Status == NEW_RX)
    {
        res = Buffer_Rx.Counter;
        memset(outBuff, 0x00, res + 1);
        memcpy(outBuff, Buffer_Rx.Data, res);
         
        // Clear Buffer counter
        Buffer_Rx.Counter = 0x00;
        Rx_Status = NO_RX;
    }

    return res;
}



void LoRaWANClass::update(void)
{
    //Type A mote transmit receive cycle
    if((RFM_Command_Status == NEW_RFM_COMMAND || RFM_Command_Status == JOIN) && LoRa_Settings.Mote_Class == CLASS_A)
    {
      //LoRa cycle
      LORA_Cycle(&Buffer_Tx, &Buffer_Rx, &RFM_Command_Status, &Session_Data, &OTAA_Data, &Message_Rx, &LoRa_Settings);
      if(Buffer_Rx.Counter != 0x00)
      {
        Rx_Status = NEW_RX;
      }
      
      RFM_Command_Status = NO_RFM_COMMAND;
    }

    //Type C mote transmit and receive handling
    if(LoRa_Settings.Mote_Class == CLASS_C)
    {
       //Transmit
      if(RFM_Command_Status == NEW_RFM_COMMAND)
      {     
        //Lora send data
        LORA_Send_Data(&Buffer_Tx, &Session_Data, &LoRa_Settings);

        RFM_Command_Status = NO_RFM_COMMAND;
      }

      //Receive
      if(digitalRead(RFM_pins.DIO0) == HIGH)
      {
        //Get data
        LORA_Receive_Data(&Buffer_Rx, &Session_Data, &OTAA_Data, &Message_Rx, &LoRa_Settings);
        if(Buffer_Rx.Counter != 0x00)
        {
            Rx_Status = NEW_RX;
        }
      }
      RFM_Command_Status = NO_RFM_COMMAND;
    }

}

void LoRaWANClass::randomChannel()
{
    unsigned char freq_idx;
#ifdef AS_923
    freq_idx = random(0,7);
    // limit drate, ch 5-> Datarate 6 (sf7bw250)
    // LoRa_Settings.Datarate_Tx = freq_idx == 0x05? 0x06 : drate_common;

   
#endif
    if(LoRa_Settings.Mote_Class==CLASS_A)
    {
        LoRa_Settings.Channel_Rx = freq_idx;
    }
    LoRa_Settings.Channel_Tx = freq_idx;
}

unsigned int LoRaWANClass::getFrameCounter() {
    return Frame_Counter_Tx;
}

unsigned int LoRaWANClass::getFrameCounterRx() {
     return Message_Rx.Frame_Counter;
}

void LoRaWANClass::setFrameCounter(unsigned int FrameCounter) {
    Frame_Counter_Tx = FrameCounter;
}

int LoRaWANClass::getFramePortRx()
{
    int port;
    port=Message_Rx.Frame_Port;
    return port;      
}

int LoRaWANClass::getFramePortTx()
{
    int port;
    port=LoRa_Settings.Frame_Port_Tx;
    return port;
}

void LoRaWANClass::setFramePortTx(unsigned int port) {
   LoRa_Settings.Frame_Port_Tx=port;
}


// define lora objet 
LoRaWANClass lora;
