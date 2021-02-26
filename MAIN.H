/**************************************************************************
**                                                                        *
**  INFINEON XC20007XE16x AP16164 , CAN_loader, Version 1.0               *
**                                                                        *
**  Id: MAIN.H 2009-10-1 YS                                               *
**                                                                        *
**  DESCRIPTION :                                                         *
**        - commands for handshake between ’host’ and ’slave’ board       *
**        - Defines a port pin to signal the process status               *
**        - Flash Programming Functions for XC2xxx (On-chip Flash)        *
**        - Code location: the internal PSRAM at address 0xE0,0000        *
**************************************************************************/
#ifndef _MAIN_H_
#define _MAIN_H_

#define TASKING_CLASSIC
#include  "XC22xxREGS.H"
#include  "IO.H"

 // Structure for a single MultiCAN object
 // A total of 128 such object structures exists
#define BEGIN_APP_ROM 0x10000
#define END_APP_ROM 0x3FFFE

#define CAN_HWOBJ ((struct stCanObj volatile far*) 0x201000)             
#define CAN_ubRequestMsgObj(ObjNr) (((CAN_HWOBJ[ObjNr].uwMOCTRL & 0x0100) == 0x0000) \
   ? CAN_HWOBJ[ObjNr].uwMOCTRL = 0x0020, 1 : 0)
#define CAN_ubNewData(ObjNr) (((CAN_HWOBJ[ObjNr].uwMOCTRL & 0x0008) == 0x0008) \
   ? 1 : 0)

 struct stCanObj
 {
   uword  uwMOFCRL;    // Function Control Register Low
   uword  uwMOFCRH;    // Function Control Register High
   uword  uwMOFGPRL;   // FIFO/Gateway Pointer Register Low
   uword  uwMOFGPRH;   // FIFO/Gateway Pointer Register High
   uword  uwMOIPRL;    // Interrupt Pointer Register Low
   uword  uwMOIPRH;    // Interrupt Pointer Register High
   uword  uwMOAMRL;    // Acceptance Mask Register Low
   uword  uwMOAMRH;    // Acceptance Mask Register High
   //ubyte  ubData[8];   // Message Data 0..7
  union
  {
    ubyte   ubData[8];  // Message Data 0..7
    uword  uwData[4];  // Message Data 0..7
    struct
    {
      uword uwDataLow;
      uword uwDataHigh;
    } stData;
  } MODATAn32bit;

   uword  uwMOARL;     // Arbitration Register Low
   uword  uwMOARH;     // Arbitration Register High
   uword  uwMOCTRL;    // Control Register Low
   uword  uwMOCTRH;    // Control Register High
 };

 typedef struct
 {
     ubyte ubMOCfg;    // message object configuration
     ulong ulID;       // standard (11-bit)/extended (29-bit) identifier
     ulong ulMask;     // standard (11-bit)/extended (29-bit) mask
     ubyte ubData[8];  // 8-bit data bytes
     uword uwCounter;  // frame counter(MOIPRnH[15-0])
 }stCAN_SWObj;

void MAIN_vUnlockProtecReg(void);
void MAIN_vLockProtecReg(void);
void MAIN_vChangeFreq(void);
void CAN_vInit(void);
void CAN_vGetMsgObj(ubyte ubObjNr, stCAN_SWObj *pstObj);
void CAN_vTransmit(ubyte ubObjNr);
uword Flash_erase_sector (ulong ulSector_Addr);
uword Flash_erase_page(ulong ulAddr);

ulong calculate_sector (ubyte ubSectorNr);

uword Flash_ProgramPage (ulong page_adr, void *buf);
void SendAck(ubyte ubACK, ubyte ubTmp);
void ReceiveDataBlock(void);        // receive data and store it to 0xC000 
void process_command(void);
void delay(void);
void delay_ms(int);
uword Flash_Init (ulong adr);

#endif  // ifndef _MAIN_H_
