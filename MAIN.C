/**************************************************************************
**                                                                        *
**  INFINEON XC20007XE16x AP16164 , CAN_loader, Version 1.0               *
**                                                                        *
**  Id: Main.c 2009-10-1 YS                                           *
**                                                                        *
**  DESCRIPTION :                                                         *
**        - commands for handshake between ’host?and ’slave?board       *
**        - Defines a port pin to signal the process status               *
**        - Flash Programming Functions for XC2xxx (On-chip Flash)        *
**        - Code location: the internal PSRAM at address 0xE0,0000        *
**************************************************************************/
#include "MAIN.H"
#define ADD_OK               0x01
#define PROCESS_OK           0x01
#define CAN_BSL_EXECUTION    0x8F
#define PLLSTAT_VCOLOCK      0x0008
#define PLLSTAT_VCOBYST      0x0001
#define K1_ACK_SET           1U
#define K2_ACK_SET           1U
#define P_ACK_SET            1U
#define N_ACK_SET            1U
#define K1DIV_SET_1          0U
#define CLOCK_EN             1U
#define CLOCK_SEL_FSYS       0U
#define F_OSC                8
#define OUTPUT_PUSH_PULL_GPIO  8U
#define OUTPUT_PUSH_PULL_ALT1  9U
#define OUTPUT_PUSH_PULL_ALT2  10U
#define OUTPUT_PUSH_PULL_ALT3  11U

#define VCO_BY_SET             1U
#define HPOSCCON_MODE          0 // ext.crystal no power-saving
#define SYSCON0_CLKSEL_F_OSC   1 // CLKSEL 0: fWU, 1: fOSC, 2: fPLL, 3: Reserved

uword uwDatPage_128Byte[64];
ubyte ubgCommand;

//****************************************************************************
// @Function      void main(void) 
//****************************************************************************
void main(void)
{
  ubyte inx;
  ubgCommand=0;
 
  P0_OUT       =   0x0000;             // output level = high at begin
  P0_IOCR04    =   0x0080;             // general purpose output for pin 4
  P0_IOCR05    =   0x0080;             // general purpose output for pin 5
  P0_IOCR06    =   0x0080;             // general purpose output for pin 6
  P0_IOCR07    =   0x0080;             // general purpose output for pin 7
 
  CAN_HWOBJ[1].uwMOFCRH = 0x0200;       //DLC=2
  SendAck(CAN_BSL_EXECUTION, 0);
   
  while(1)
  {                                     
    if(CAN_HWOBJ[0].uwMOCTRL & 0x0008)                    // if NEWDAT 
    {
      ubgCommand = CAN_HWOBJ[0].MODATAn32bit.uwData[0];
      CAN_HWOBJ[0].uwMOCTRL = 0x0008;                     // clear NEWDAT
    }
            
    process_command(); 

	  P0_OUT = ~P0_OUT; 
	  delay();
	  delay();
	  delay();
	  delay();
  }
} //  End of function main


void delay_ms(int ms)
{
	int i,j;
	char k;
 	for(i=0;i<ms;i++)
	{
		for(k=0;k<10;k++)
			for(j=0;j<8000;j++)
				;
	}
}
///////////////////////////////////////////////////////////////////////////////
// ReceiveDataBlock
///////////////////////////////////////////////////////////////////////////////
void ReceiveDataBlock(void)        // receive data and store it to 0xC000 
{
  uword i;
  for (i = 0; i < (16*4); i+=4)   // 128 bytes, 64 words
  {
    while (!(CAN_HWOBJ[0].uwMOCTRL & 0x0008)); // wait until NEWDAT
    {
      // store the datas from the CAN Messages
      uwDatPage_128Byte[0+i]=CAN_HWOBJ[0].MODATAn32bit.uwData[0];
      uwDatPage_128Byte[1+i]=CAN_HWOBJ[0].MODATAn32bit.uwData[1];
      uwDatPage_128Byte[2+i]=CAN_HWOBJ[0].MODATAn32bit.uwData[2];
      uwDatPage_128Byte[3+i]=CAN_HWOBJ[0].MODATAn32bit.uwData[3];
    }
    CAN_HWOBJ[0].uwMOCTRL = 0x0008;                     // clear NEWDAT
  }           
}

void ProgramBlock(ulong ulBaseAddr, ubyte ubPageNr)
{
  ulong ulAddr; 
  ubyte ubTmp=1;
  ReceiveDataBlock();                               // receive data from CAN interface and store it to 0xC000
  
  ulAddr = ulBaseAddr + 0x80 * ubPageNr;  
  ubTmp = Flash_erase_page(ulAddr);
  while (ubTmp); 
  if (ubTmp==0)
  {
    ubTmp = Flash_ProgramPage(ulAddr, &uwDatPage_128Byte); // data from 0xC000
    SendAck(ubTmp, ubPageNr);  // '00'= ok, '01'= fail
  }
}

///////////////////////////////////////////////////////////////////////////////
// Decode the Command receive in the CAN message and perform it
///////////////////////////////////////////////////////////////////////////////
void process_command(void)
{
  uword uwAddressValid=0; 
  ulong ulBaseAddr;
  ubyte ubTmp=1, ubPageNr;

  switch(ubgCommand)      // scan the type of the operation requested
  {
    case 0x01: //programming page operation
    {
         ulBaseAddr = (CAN_HWOBJ[0].MODATAn32bit.ubData[3]);//upper bits
         ulBaseAddr = (ulBaseAddr<<8)|CAN_HWOBJ[0].MODATAn32bit.ubData[2];//middle bits
         ulBaseAddr = (ulBaseAddr<<8)|CAN_HWOBJ[0].MODATAn32bit.ubData[1];//middle bits
         Flash_Init (ulBaseAddr);
         ProgramBlock(ulBaseAddr, ubPageNr); 
         break;
    }
   case 0x03://sector erasing operation 
   {
        ubyte ubSector_Nr, uwTmp; 
        ubSector_Nr = CAN_HWOBJ[0].MODATAn32bit.ubData[1];
        ulBaseAddr = calculate_sector (ubSector_Nr);
        Flash_Init (ulBaseAddr);
        uwTmp = Flash_erase_sector(ulBaseAddr);
        SendAck(uwTmp, 0);  // '00'= ok, '01'= fail
        break;
   }      
   case 0x20:// Done downloading so do a software reset      for standard start
   {
        delay(); 
        SCU_SWRSTCON = 0x0000;       //SWCFG =0000,0000b (standard start)
        SCU_RSTCON0 = 0xC000;        //.SW=application reset            
        SCU_SWRSTCON|=0x0003;         // generate reset
        break;
   }
   case 0x21:// Done downloading so do a software reset      for CAN BSL
   {
        SendAck(0x00, 0);                  // Send back an Ack to trasmitter
        delay(); 
        SCU_SWRSTCON = 0x0D00;       //SWCFG =0000,1101b (CAN BSL)
        SCU_RSTCON0 = 0xC000;        //.SW=application reset            
        SCU_SWRSTCON|=0x0003;        // generate reset
        break;
   }
   case 0x51: //16k application code (from UConnect C0,0000-C0,3FFF)
   {
        uword uwPageNr;
        ulBaseAddr = 0x00C00000;
        Flash_Init (ulBaseAddr);
        SendAck(0x00, 0);                                 // Send back an Ack to trasmitter

        for (uwPageNr=0; uwPageNr<=(4*8*4 -1); uwPageNr++) // receive 16K   
        {
          ProgramBlock(ulBaseAddr, (ubyte)uwPageNr);
        }
        break;
     }
     default:// if a bad commamd is received
       break;
   }
   ubgCommand = 0x00; 
}
