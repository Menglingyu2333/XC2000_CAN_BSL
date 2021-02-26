/**************************************************************************
**                                                                        *
**  INFINEON XC20007XE16x AP16164 , CAN_loader, Version 1.0               *
**                                                                        *
**  Id: FlashDev.c 2009-10-1 YS                                           *
**                                                                        *
**  DESCRIPTION :                                                         *
**        - commands for handshake between ’host?and ’slave?board       *
**        - Defines a port pin to signal the process status               *
**        - Flash Programming Functions for XC2xxx (On-chip Flash)        *
**        - Code location: the internal PSRAM at address 0xE0,0000        *
**************************************************************************/
#include  "main.h"
#include  "XC22xxREGS.H"

struct FlashBlocks  {
  ulong   szBlock;     // Size in Bytes
  ulong AddrBlock;     // Address of Blocks
};

union uniIMB_FSR_BUSY{
  uword v;
  struct b {
    uword busyfm1  : 1;
    uword busyfm2  : 1;
    uword busyfm3  : 1;
    uword res3     : 1;
    uword res4     : 1;
    uword res5     : 1;
    uword res6     : 1;
    uword res7     : 1;
    uword pagem1   : 1;
    uword pagem2   : 1;
    uword pagem3   : 1;
    //uword reserved_10_15: 6;
  } b;
};


union  uniIMB_FSR_OP{                // Flash State Operations
  uword v;
  struct o{
    uword prog   : 1;
    uword erase  : 1;
    uword power  : 1;
    uword mar    : 1;
    uword sqer   : 1;
    uword oper   : 1;
    //uword reserved_6_15: 10;
  } b;
};

union uniIMB_FSR_PROT {                // Flash State Protection
  uword v;
  struct p {
    uword proin  : 1;
    uword proiner: 1;
    uword rprodis: 1;
    uword wprodis: 1;
    uword proer  : 1;
    //uword reserved_5_15: 11;
  } b;
};

ulong base;               // Device Base Address

/////////////////////////////////////////////////////////
// Function to Check Status of Flash Device.
// *    Parameter:      adr:  Block Address
// *    Return Value:   0 - OK,  1 - Failed
////////////////////////////////////////////////////////
uword Flash_Check (void) {
  union uniIMB_FSR_BUSY fbr;
  union uniIMB_FSR_OP fop;
  union uniIMB_FSR_PROT fpr;

  do {
    fbr.v =  (IMB_FSR_BUSY);         // Read Busy Register
  } while (fbr.b.busyfm1 || fbr.b.busyfm2 || fbr.b.busyfm3); // Wait until Flash is Busy

  fop.v =  (IMB_FSR_OP);         //Read Operation Register
  fpr.v =  (IMB_FSR_PROT);       //Read Protection Register

  if (fop.b.prog || fop.b.erase) {
    (*((uword far *) (base |0xAA))) = 0xF5;  // Clear Status
  }

  if (fop.b.oper || fop.b.sqer || fpr.b.proer) {
    (*((uword far *) (base |0xAA))) = 0xF0;  // Reset to Read Mode
    return (1);                              // Failed
  }
  return (0);                                // Done
}


/////////////////////////////////////////////////////////
// Function to Initialize Flash Programming Functions
// *    Return Value:   0 - OK,  1 - Failed
////////////////////////////////////////////////////////
uword Flash_Init (ulong adr) {
  base = adr;
  return (0);
}


/////////////////////////////////////////////////////////
// Function to De-Initialize Flash Programming Functions
// *    Return Value:   0 - OK,  1 - Failed
////////////////////////////////////////////////////////
uword Flash_UnInit (void) {
  return (0);
}


/////////////////////////////////////////////////////////
// Function to erase one sector of the flash.
// *    Parameter:      adr:  Block Address
// *    Return Value:   0 - OK,  1 - Failed
////////////////////////////////////////////////////////
uword Flash_erase_sector (ulong ulSector_Addr) {
  (*((uword far *) (base |0xAA))) = 0x80;       // Erase Sector (1. Cycle)
  (*((uword far *) (base |0x54))) = 0xAA;       // Erase Sector (2. Cycle)
  (*((uword far *) (ulSector_Addr))) = 0x33;    // Erase Sector (3. Cycle)
  return (Flash_Check());                             // Check until Device Ready
}

/////////////////////////////////////////////////////////
// Function to erase one page of the flash.
// *    Parameter:      adr:  page Address
// *    Return Value:   0 - OK,  1 - Failed
////////////////////////////////////////////////////////
uword Flash_erase_page (ulong ulPage_Addr) {
  (*((uword far *) (base |0xAA))) = 0x80;       // Erase Sector (1. Cycle)
  (*((uword far *) (base |0x54))) = 0xAA;       // Erase Sector (2. Cycle)
  (*((uword far *) (ulPage_Addr))) = 0x03;      // Erase Sector (3. Cycle)
  return (Flash_Check());                             // Check until Device Ready
}

////////////////////////////////////////////////////////
// Function to Program one flash data page.
// *    Parameter:      adr:  Block Address
// *                    buf:  Block Data
// *    Return Value:   0 - OK,  1 - Failed
////////////////////////////////////////////////////////
uword Flash_ProgramPage (ulong page_adr, void *buf) {
  uword cnt, uwTmp;  
  // Enter Page Mode
  (*((uword far *) (base |0x00AA))) = 0x50;       // 1st cycle: write 50h to xx'00AAh
  (*((uword far *) (page_adr))) = 0xAA;     // 2nd cycle: write AAh to page start address
  uwTmp = (Flash_Check());                            // Check until Device Ready

  if (uwTmp == 0) 
  {   
    // Load Page
    for (cnt = 0; cnt < 64; cnt++)  {
      (*((uword far *) (base |0x00F2))) = (*(uword *)buf);       // write word from R4 to xx'00F2h
      buf = (uword *) buf+1;
    }
    // Write Page
    (*((uword far *) (base |0x00AA))) = 0xA0;       // 1st cycle: write A0h to xx'00AAh
    (*((uword far *) (base |0x5A))) = 0xAA;         // 2nd cycle: : write AAh to xx'005A
    uwTmp = Flash_Check();                          // Check until Device Ready
  }
  return (uwTmp); 
}

/////////////////////////////////////////////////////////
// Function to calculate sector start address from sector number.
// physical sectors all of 4k size are used
// *    Parameter:      adr:  Block Address
// *    Return Value:   
////////////////////////////////////////////////////////
ulong calculate_sector (ubyte ubSectorNr) {
    uword uwAddrH, uwAddrL;
    ulong ulAddr;
    uwAddrL = 0;
    uwAddrH = 0x00C0;
    while (ubSectorNr!=0)
    {
       ubSectorNr--; 
       uwAddrL += 0x1000; 
    }
    ulAddr =  (((ulong) uwAddrH << 16) | uwAddrL); 
    return ulAddr;
}
/////////////////////////////////////////////////////////
// void delay(void)
//    
////////////////////////////////////////////////////////
void delay(void)
{
  long counter;
  for (counter =0; counter <0xFFFFFFFF; counter++) {;} // delay for hardware
}
