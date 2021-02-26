/**************************************************************************
**                                                                        *
**  INFINEON XC20007XE16x AP16164 , CAN_loader, Version 1.0               *
**                                                                        *
**  Id: CAN.c 2009-10-1 YS                                                *
**                                                                        *
**  DESCRIPTION :                                                         *
**        - commands for handshake between ’host’ and ’slave’ board       *
**        - Defines a port pin to signal the process status               *
**        - Flash Programming Functions for XC2xxx (On-chip Flash)        *
**        - Code location: the internal PSRAM at address 0xE0,0000        *
**************************************************************************/
#include "MAIN.H"

//****************************************************************************
// @Function      void CAN_vTransmit(ubyte ubObjNr) 
//****************************************************************************
void CAN_vTransmit(ubyte ubObjNr)
{
  CAN_HWOBJ[ubObjNr].uwMOCTRH = 0x0720;  // set TXRQ,TXEN0,TXEN1,MSGVAL
}   

///////////////////////////////////////////////////////////////////////////////
// SendAck
///////////////////////////////////////////////////////////////////////////////
void SendAck(ubyte ubACK, ubyte ubTmp)
{
  CAN_HWOBJ[1].uwMOCTRH = 0x0008;       // set NEWDAT
  CAN_HWOBJ[1].MODATAn32bit.ubData[0] = ubACK;
  CAN_HWOBJ[1].MODATAn32bit.ubData[1] = ubTmp; 
  CAN_HWOBJ[1].uwMOCTRL = 0x0040;       // reset RTSEL
  while(!CAN_ubRequestMsgObj(1));
  CAN_HWOBJ[1].uwMOCTRH = 0x0720;  // set TXRQ,TXEN0,TXEN1,MSGVAL

}


