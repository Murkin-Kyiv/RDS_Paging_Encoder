/*  Full option SI4713 library with advanced RDS and MPX settings
 *   
 *  v1.0 - Sjef Verhoeven PE5PVB
 *  *  Website: https://www.pe5pvb.nl/
 *  Github:  https://github.com/PE5PVB
 * 
 *  v2.0 - Added functions for 1A, 4A, 7A Groups   
 *  
*/

#include <Wire.h>
#include "tools.h" // some tools for project

// -------------------------------------------------------- TYPE DEFINITIONS
/**
 * Type Block Converts 16 bits word to two bytes
 */
typedef union
{
    struct
    {
      uint8_t LowByte;
      uint8_t HighByte;
    } refined;
    uint16_t raw;
} Block;


/**
 * Group 4A ( RDS Date and Time)
 *
 * To make it compatible with 8, 16 and 32 bits platforms and avoid Crosses boundary, it was necessary to
 * use uint32_t data type.
 */
typedef union
{
    struct
    {
        uint32_t offset : 5;       // Local Time Offset from 0 to 15:30
        uint32_t offset_sense : 1; // Local Offset Sign ( 0 = + , 1 = - )
        uint32_t minute : 6;       // UTC Minutes - 2 bits less significant (void “Crosses boundary”).
        uint32_t hour : 5;         // UTC Hours - 4 bits less significant (void “Crosses boundary”)
        uint32_t mjd : 17;         // Modified Julian Day 
        uint32_t spr : 3;          // Spare bits
        uint32_t pty : 5;          // PTY, program type, 01000(8d) = Science
        uint32_t TP : 1;           // Traffic Programm bit, 0=Non TP, 1=TP
        uint32_t Bo : 1;           // RDS Version, 0=A, 1=B
        uint32_t type : 4;         // Group type 4=Date and Time
        uint32_t pid : 16;           // Programm Identification
     } refined;
  
    uint16_t raw[4]; // 4 16 bit words for blocks A,B,C,D 
    
} type_4A;

/**
 * Group 2A ( RDS Radio Text)
 *
 * To make it compatible with 8, 16 and 32 bits platforms and avoid Crosses boundary, it was necessary to
 * use uint32_t data type.
 */
typedef union
{
    struct
    {
        uint32_t text2 : 16;       // 3 and 4 symbols 
        uint32_t text1 : 16;       // 1 and 2 symbols  
        uint32_t counter : 4;      // Message blocks counter,  
        uint32_t ABflag : 1;       // Text A/B flag. I haven't studied this flag, I always use 0
        uint32_t pty : 5;          // PTY, program type, 01000(8d) = Science
        uint32_t TP : 1;           // Traffic Programm bit, 0=Non TP, 1=TP
        uint32_t Bo : 1;           // RDS Version, 0=A, 1=B
        uint32_t type : 4;         // Group type 4=Date and Time
        uint32_t pid : 16;         // Programm Identification
     } refined;
  
    uint16_t raw[4]; // 4 16 bit words for blocks A,B,C,D 
    
} type_2A;
//--------------------------------------------------

/**
 * Group 1A ( RDS Programm Item Number and slow labeling Codes)
 * 
 * Use for pafing synchronization, must be send each second 
 *
 * To make it compatible with 8, 16 and 32 bits platforms and avoid Crosses boundary, it was necessary to
 * use uint32_t data type.
 */
typedef union
{
    struct
    {
        uint32_t pinc : 16;        // Programme Item Number Code (see 3.2.1.7) 
        uint32_t slc : 16;         // Slow Labeling Code (see 3.1.5.2) 
        uint32_t rpc : 5;          // Radio Paging Codes (see Annex M)
        uint32_t pty : 5;          // PTY, program type, 01000(8d) = Science
        uint32_t TP : 1;           // Traffic Programm bit, 0=Non TP, 1=TP
        uint32_t Bo : 1;           // RDS Version, 0=A, 1=B
        uint32_t type : 4;         // Group type 4=Date and Time
        uint32_t pid : 16;         // Programm Identification
     } refined;
  
    uint16_t raw[4]; // 4 16 bit words for blocks A,B,C,D 
    
} type_1A;

/**
 * Group 7A ( RDS Paging)
 *
 * To make it compatible with 8, 16 and 32 bits platforms and avoid Crosses boundary, it was necessary to
 * use uint32_t data type.
 */
typedef union
{
    struct
    {
        uint32_t data : 16;        // address or data 
        uint32_t address: 16;      // address or data  
        uint32_t psac : 4;         // Paging Segment Address Code (have 4 variants for Tone/10/18/Alpha  
        uint32_t ABflag : 1;       // Text A/B flag. For new message A/B flag must be changed from 0 to 1, or from 0 to 1
        
        uint32_t pty : 5;          // PTY, program type, 01000(8d) = Science
        uint32_t TP : 1;           // Traffic Programm bit, 0=Non TP, 1=TP
        uint32_t Bo : 1;           // RDS Version, 0=A, 1=B
        uint32_t type : 4;         // Group type 7A paging
        uint32_t pid : 16;         // Programm Identification
     } refined;
  
    uint16_t raw[4]; // 4 16 bit words for blocks A,B,C,D 
    
} type_7A;
//--------------------------------------------------

/**
 * Group 7A ( RDS Paging) Adress Block C,D
 *
 * To make it compatible with 8, 16 and 32 bits platforms and avoid Crosses boundary, it was necessary to
 * use uint32_t data type.
 */
typedef union
{
    struct
    {
        uint32_t d_2 : 4;        // 2 symbol 
        uint32_t d_1 : 4;        // 1 symbol 
        uint32_t a_6 : 4;         // 6 digit  
        uint32_t a_5 : 4;         // 5 digit  
        uint32_t a_4 : 4;         // 4 digit  
        uint32_t a_3 : 4;         // 3 digit  
        uint32_t a_2 : 4;         // 2 digit  
        uint32_t a_1 : 4;         // 1 digit  
     } refined;
  
    uint16_t raw[2]; // 2 16 bit words for blocks C,D 
    
} type_7A_adress;

/**
 * Group 7A ( RDS Paging) Adress Block D FOR ALPHA
 * 
 */
typedef union
{
    struct
    {
        uint8_t data_hi : 4;        // High Bits 
        uint8_t data_lo : 4;        // Low Bits 
     } refined;
  
    uint8_t raw; // 2 16 bit words for blocks C,D 
    
} type_7A_AplusD;

/**
 * Group 7A ( RDS Paging) Last byte in D Block for alpha messages (page 114)
 * For future, not used now 
 */
typedef union
{
    struct
    {
        uint8_t CallCounter        : 4;    // Use for check missing messages 
        uint8_t RepeatFlag         : 1;    // Repeated Message
        uint8_t InternationalFlag  : 1;    // 0=National; 1=International
        uint8_t MessageType        : 2;    // 00=Alpha; 01=Variable Length Numeric Message; 10=Reserved ; 11=Variable-length function message
    } refined;
  
    uint8_t raw; //  
    
} type_7A_X1X2;

//-----------------------------------------------------------------------------------------------
// PI (programm_identification) union 
// 0x62E7 6=Country Ukraine; 2=National Network; E7=Programm Reference Number (Jazz FM)
// Country code (CC) is important !!! See in RDS Protocol Annex D and description of EEPROM DUMP
// Also you can try CC from 1 to F, or use Internetional Mode if pager supports this Mode
typedef union
{
    struct
    {
        uint8_t pi_prn     : 8; // Programm Reference Number (does NOT affect for paging)
        uint8_t pi_area    : 4; // Programm Area Coverage (does NOT affect for paging)
        uint8_t pi_country : 4; // Country Code ! IMPORTANT ! 
     } refined;
    uint16_t All; // PI Structure, RDS block A
} type_pi;
//============================================================================================

// UTC Offset union ---------------------------------------------------------------------------
typedef union
{
    struct
    {
        uint8_t offset_minute : 1; // 0-00, 1-30
        uint8_t offset_hour   : 4; // 0-00, FF-15
        uint8_t offset_sign   : 1; // 0-"+", 1-"-"
     } refined;
    uint8_t All; // Offset for 4A group 
} type_Offset;
//================================

#include "config.h" //Settings
//=========================================== END TYPE DEFINITIONS =======================================

uint8_t buf[10];
uint16_t component;
uint16_t acomp;
uint16_t misc;
int addr;
uint8_t GPO;

class SI4713
{
  public:
    void Init(uint8_t RST, uint16_t clk, uint8_t address);
    void Output(uint8_t level, uint8_t cap);
    void Freq(uint16_t freq);
    void RDS_PI(uint16_t RDSPI);
    void RDS_PTY(uint8_t RDSPTY);
    void RDS_COMP(bool ONOFF);
    void RDS_ART(bool ONOFF);
    void RDS_MS(bool ONOFF);
    void RDS_TP(bool ONOFF);
    void RDS_TA(bool ONOFF);
    void RDS_AF(uint16_t AF);
    void RDS_PS(String PS, uint8_t number);
    void RDS_RT(String RT);
    void RDS_PS_MIX(uint16_t PS_MIX); // Sets the ratio of RDS PS (group 0A) and circular buffer/FIFO groups.

    void RDS_MUSP(bool ONOFF);
    void RDS_PSCOUNT(uint8_t count, uint8_t speed);
    void RDS_Enable(bool ONOFF);
    void MPX_Enable(bool ONOFF);
    void Audio_Mute(bool ONOFF);
    void MPX_Deviation(uint16_t dev);
    void RDS_Deviation(uint16_t dev);
    void Audio_PreEmphasis(uint16_t emp);
    void MPX_Freq(uint16_t freq);
    void Audio_Limiter(bool ONOFF);
    void Audio_AGC(bool ONOFF);
    void Audio_Comp_Threshold(int16_t threshold);
    void Audio_Comp_Attack(uint16_t attack);
    void Audio_Comp_Release(uint16_t rel);
    void Audio_Comp_Gain(uint16_t gain);
    void Audio_Limiter_Release(uint16_t rel);
    void Audio_Deviation(uint16_t dev);
    void ASQ(bool &overmod, int8_t &inlevel);
    void Rev(uint8_t &pn, uint8_t &chiprev);
    void GPO(bool GPO1, bool GPO2, bool GPO3);

    // PLAB Updates
    void RDS_SEND_BUFFER (Block A, Block B, Block C, Block D, String Group, byte Monitor); //send RDS buffer 
    void RDS_4A_TIME (uint16_t rds_pi, byte Bo, byte TP, byte PTY, uint16_t Year, byte Month, byte Day, byte Hour, byte Minute, byte O_Sign, byte O_Hour, byte O_Minute, byte Monitor); // Send 4A/4B group: Date and Time
    void RDS_2A_RT   (uint16_t rds_pi, byte Bo, byte TP, byte PTY, byte ABflag, String RT, byte Monitor); //Send RadioText (old RDS_RT)
    void RDS_1A_PIN (uint16_t rds_pi, byte Bo, byte TP, byte PTY, byte rpc, uint16_t slc, uint16_t pinc, byte Monitor);  //Send 1A group PIN ans SLC
    void RDS_7A_PAGING (uint16_t rds_pid, byte Bo, byte TP, byte PTY, byte ABflag, byte Type, uint32_t Address, String M_Text, byte Monitor); //Send Message
    // End PLAB 

  private:
    bool WriteBuffer(uint8_t len);
    bool ReadBuffer(uint8_t len);
    bool Set_Property(uint16_t arg1, uint16_t arg2);
};
// =============================================== End Class ======================================

bool SI4713::ReadBuffer(uint8_t len)
{
}

bool SI4713::WriteBuffer(uint8_t len)
{
  Wire.beginTransmission(addr);
  for (uint8_t i = 0; i < len; i++) {
    Wire.write(buf[i]);
    if (buf[i] < 0x10) {
    }
  }
  Wire.endTransmission();
  delay(54);
  Wire.requestFrom(addr, 1);
  while (bitRead(Wire.read(), 7) != 1) {}
}

bool SI4713::Set_Property(uint16_t arg1, uint16_t arg2)
{
  buf[0] = 0x12;
  buf[1] = 0x00;
  buf[2] = highByte(arg1);
  buf[3] = lowByte(arg1);
  buf[4] = highByte(arg2);
  buf[5] = lowByte(arg2);
  WriteBuffer(6);
}

void SI4713::Output(uint8_t level, uint8_t cap)
{
  buf[0] = 0x31;
  buf[1] = 0x00;
  buf[2] = 0x00;
  buf[3] = level;
  buf[4] = cap;
  WriteBuffer(5);
}

void SI4713::Freq(uint16_t freq)
{
  buf[0] = 0x30;
  buf[1] = 0x00;
  buf[2] = highByte(freq);
  buf[3] = lowByte(freq);
  WriteBuffer(4);
}

void SI4713::RDS_PI(uint16_t RDSPI)
{
  Set_Property(0x2c01, RDSPI);
}

void SI4713::RDS_PSCOUNT(uint8_t count, uint8_t speed)
{
  Set_Property(0x2c05, count);
  Set_Property(0x2c04, speed);
}

void SI4713::RDS_PTY(uint8_t RDSPTY)
{
  bitWrite(misc, 5, bitRead(RDSPTY, 0));
  bitWrite(misc, 6, bitRead(RDSPTY, 1));
  bitWrite(misc, 7, bitRead(RDSPTY, 2));
  bitWrite(misc, 8, bitRead(RDSPTY, 3));
  bitWrite(misc, 9, bitRead(RDSPTY, 4));
  Set_Property(0x2c03, misc);
}

void SI4713::RDS_COMP(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(misc, 14, 1);
  } else {
    bitWrite(misc, 14, 0);
  }
  Set_Property(0x2c03, misc);
}

void SI4713::RDS_ART(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(misc, 13, 1);
  } else {
    bitWrite(misc, 13, 0);
  }
  Set_Property(0x2c03, misc);
}

void SI4713::RDS_MS(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(misc, 12, 1);
  } else {
    bitWrite(misc, 12, 0);
  }
  Set_Property(0x2c03, misc);
}

void SI4713::RDS_TP(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(misc, 10, 1);
  } else {
    bitWrite(misc, 10, 0);
  }
  Set_Property(0x2c03, misc);
}

void SI4713::RDS_TA(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(misc, 4, 1);
  } else {
    bitWrite(misc, 4, 0);
  }
  Set_Property(0x2c03, misc);
}

void SI4713::RDS_AF(uint16_t AF)
{
  if (AF == 0) {
    Set_Property(0x2c06, 0xe0e0);
  } else {
    Set_Property(0x2c06, 0xdd95 + AF);
  }
}

void SI4713::RDS_MUSP(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(misc, 3, 1);
  } else {
    bitWrite(misc, 3, 0);
  }
  Set_Property(0x2c03, misc);
}

void SI4713::RDS_Enable(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(component, 2, 1);
  } else {
    bitWrite(component, 2, 0);
  }
  Set_Property(0x2100, component);
}

void SI4713::Audio_Comp_Threshold(int16_t threshold)
{
  Set_Property(0x2201, threshold);
}

void SI4713::Audio_Comp_Attack(uint16_t attack)
{
  Set_Property(0x2202, attack);
}

void SI4713::Audio_Comp_Release(uint16_t rel)
{
  Set_Property(0x2203, rel);
}

void SI4713::Audio_Comp_Gain(uint16_t gain)
{
  Set_Property(0x2204, gain);
}

void SI4713::Audio_Limiter_Release(uint16_t rel)
{
  Set_Property(0x2205, rel);
}

void SI4713::Audio_Limiter(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(acomp, 1, 1);
  } else {
    bitWrite(acomp, 1, 0);
  }
  Set_Property(0x2200, acomp);
}

void SI4713::Audio_AGC(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(acomp, 0, 1);
  } else {
    bitWrite(acomp, 0, 0);
  }
  Set_Property(0x2200, acomp);
}

void SI4713::MPX_Enable(bool ONOFF)
{
  if (ONOFF == true) {
    bitWrite(component, 0, 1);
    bitWrite(component, 1, 1);
  } else {
    bitWrite(component, 0, 0);
    bitWrite(component, 1, 0);
  }
  Set_Property(0x2100, component);
}

void SI4713::Audio_Mute(bool ONOFF)
{
  if (ONOFF == false) {
    Set_Property(0x2105, 0x0000);
  } else {
    Set_Property(0x2105, 0x0003);
  }
}

void SI4713::Audio_PreEmphasis(uint16_t emp)
{
  if (emp == 0) {
    Set_Property(0x2106, 0x0002);
  } else if (emp == 75) {
    Set_Property(0x2106, 0x0000);
  } else if (emp == 50) {
    Set_Property(0x2106, 0x0001);
  }
}

void SI4713::MPX_Freq(uint16_t freq)
{
  Set_Property(0x2107, freq);
}

void SI4713::MPX_Deviation(uint16_t dev)
{
  Set_Property(0x2102, dev);
}

void SI4713::RDS_Deviation(uint16_t dev)
{
  Set_Property(0x2103, dev);
}

void SI4713::Audio_Deviation(uint16_t dev)
{
  Set_Property(0x2101, dev);
}

void SI4713::GPO(bool GPO1, bool GPO2, bool GPO3)
{
  buf[0] = 0x81;
  if (GPO1 == true)
  {
    bitWrite(buf[1], 1, 1);
  } else {
    bitWrite(buf[1], 1, 0);
  }
  if (GPO2 == true)
  {
    bitWrite(buf[1], 2, 1);
  } else {
    bitWrite(buf[1], 2, 0);
  }
  if (GPO3 == true)
  {
    bitWrite(buf[1], 3, 1);
  } else {
    bitWrite(buf[1], 3, 0);
  }
  WriteBuffer(2);

}

void SI4713::RDS_PS(String PS, uint8_t number)
{
  char PSArray[9];
  for (uint8_t i = 0; i < 9; i++) {
    PSArray[i] = 0x20;
  }
  PS.toCharArray(PSArray, 9);
  for (uint8_t i = 0; i < 8; i++) {
    if (PSArray[i] == 0x00)
    {
      PSArray[i] = 0x20;
    }
  }
  buf[0] = 0x36;
  if (number > 0) {
    buf[1] = number * 2;
  } else {
    buf[1] = 0x00;
  }
  buf[2] = PSArray[0];
  buf[3] = PSArray[1];
  buf[4] = PSArray[2];
  buf[5] = PSArray[3];
  WriteBuffer(6);
  buf[0] = 0x36;
  if (number > 0) {
    buf[1] = (number * 2) + 1;
  } else {
    buf[1] = 0x01;
  }
  buf[2] = PSArray[4];
  buf[3] = PSArray[5];
  buf[4] = PSArray[6];
  buf[5] = PSArray[7];
  WriteBuffer(6);
}

void SI4713::RDS_RT(String RT) //Old functıon. I use new TX.RDS_2A_PAGING
{
  char RTArray[32];
  for (uint8_t i = 0; i < 32; i++) {
    RTArray[i] = 0x20;
  }

  RT.toCharArray(RTArray, 32);
  for (uint8_t i = 0; i < 32; i++) {
    if (RTArray[i] == 0x00)
    {
      RTArray[i] = 0x20;
    }
  }
  uint8_t counter = 0;
  for (uint8_t i = 0; i < 8; i++) {
    buf[0] = 0x35;
    if (i == 0) {
      buf[1] = 0x84; //source  value = 0x06 cirlcle buffer clear and start
    } else {
      buf[1] = 0x84; //Source  value = 0x04 cirlcle buffer
    }
    buf[2] = 0x20;
    buf[3] = i;
    buf[4] = RTArray[0 + counter];
    buf[5] = RTArray[1 + counter];
    buf[6] = RTArray[2 + counter];
    buf[7] = RTArray[3 + counter];
    WriteBuffer(8);
    counter += 4;
  }
}

void SI4713::Init(uint8_t RST, uint16_t clk, uint8_t address)
{
  addr = address;           // Copy I2C address
  pinMode(RST, OUTPUT);     // Send RST
  digitalWrite(RST, HIGH);
  delay(100); //source 50
  digitalWrite(RST, LOW);
  delay(100); //source 50
  digitalWrite(RST, HIGH);
  Wire.begin();
  Wire.beginTransmission(addr);
  Wire.write(0x01);
  Wire.write(0x12);
  Wire.write(0x50);
  Wire.endTransmission();
  buf[0] = 0x80;
  buf[1] = 0x0e;
  WriteBuffer(2);
  Set_Property(0x0201, clk);
  Set_Property(0x2300, 0x0007); // Enable ASQ Interrupts
  Set_Property(0x2C07, 0x0036); // Property 0x2C07. TX_RDS_FIFO_SIZE 0=FIFO Disabled, 4, 7, 10–54 
  
}

void SI4713::ASQ(bool &overmod, int8_t &inlevel)
{
  buf[0] = 0x34;
  buf[1] = 0x00;
  WriteBuffer(2);
  uint8_t resp[5];
  uint8_t len = 5;
  Wire.requestFrom(0x63, 5);
  if (Wire.available() == len) {
    for (uint16_t i = 0; i < len; i++) {
      resp[i] = Wire.read();
    }
    overmod = bitRead(resp[1], 2);
    inlevel = resp[4];
    buf[0] = 0x34;
    buf[1] = 0x01;
    WriteBuffer(2);
    return overmod, inlevel;
  }
}

void SI4713::Rev(uint8_t &pn, uint8_t &chiprev)
{
  buf[0] = 0x10;
  WriteBuffer(1);
  uint8_t resp[9];
  uint8_t len = 9;
  Wire.requestFrom(0x63, 9);
  if (Wire.available() == len) {
    for (uint16_t i = 0; i < len; i++) {
      resp[i] = Wire.read();
    }
    pn = resp[1];
    chiprev = resp[8];
  }
  return pn, chiprev;
}

// ------------------  Paging LAB  ---------------------------------
void SI4713::RDS_PS_MIX(uint16_t PS_MIX) //TX_RDS_PS_MIX 0=Only send RDS PS if RDS Group Buffer is empty; 3=defaul 50% time
{
 Set_Property(0x2c02, PS_MIX);
}
//=======================================================================================================

void SI4713::RDS_1A_PIN(uint16_t rds_pi, byte Bo, byte TP, byte PTY, byte rpc, uint16_t slc, uint16_t pinc, byte Monitor )
// Innput: PI, Bo, TP, PTY, RPC, SLC, PINC, Monitor
// Monitor: 1-Output full info, 0-Output only "T"
{
type_1A Sync; // create 1A group union for send 
Block block_A, block_B, block_C, block_D; // create 2 bytes blocks for send

Sync.refined.pinc = pinc; //  
Sync.refined.slc = slc; //  
Sync.refined.rpc = rpc; //  
Sync.refined.pty = PTY; //set PTY as Jazz channel; Does not matter
Sync.refined.TP = TP;  //set TP; Does not matter
Sync.refined.Bo = Bo; // Must be 0 = Version A 
Sync.refined.type = 1; //Must be 1
Sync.refined.pid = rds_pi; // PI Programm identification

// Fill RDS Group
block_A.raw = Sync.raw[3];
block_B.raw = Sync.raw[2]; 
block_C.raw = Sync.raw[1]; 
block_D.raw = Sync.raw[0]; 

RDS_SEND_BUFFER (block_A, block_B, block_C, block_D, "1A", Monitor); 

    if (Monitor) //Output log
    {Serial.println("RPC=" + Int2STR(rpc,2) + " ECC=" + Int2HEX(block_C.refined.LowByte,2) + " PINC=" + Int2HEX(pinc,4));    }
    else 
    {
      //Serial.print(".");
    }

}
//=====================================================================================================================================

void SI4713::RDS_7A_PAGING (uint16_t rds_pid, byte Bo, byte TP, byte PTY, byte ABflag, byte Type, uint32_t Address, String M_Text, byte Monitor)
// Input: PI, Bo, TP, PTY, Text A/B flag, Type, Message
// Monitor: 1-Output full info, 0-Output only "T"
{

type_7A Message; // create 7A group union for send Radio Text
Block block_A, block_B, block_C, block_D; // create 2 bytes blocks for send

// fill Radio Text Static fields 
Message.refined.ABflag = ABflag; //see notes  
Message.refined.pty = PTY; //set PTY as Jazz channel; Does not matter
Message.refined.TP = TP;  //set TP; Does not matter
Message.refined.Bo = Bo; // Must be 0 = Version A 
Message.refined.type = 7; //Must be 7
Message.refined.pid = rds_pid; // PI Programm identification

// Calculate groups qty
int RDSCounter = 0; // groups inside 24 symbols packet
int psacCounter = 0; //
bool AddressFlag = 1; // Set Flag to 1 for send addres group
String Text = M_Text; // source text
if (Text.length() == 0) {Text=" ";} //check for zero

switch (Type) { //prepare counters and input string 

     case TONE: //Tone Message
          Text = "89"; // any data
          RDSCounter = 1;
          psacCounter = 0; //Radio Paging Code
          break;

     case DIG10: //10 digits Numeric Message
          while ((Text.length() % 10) != 0) //fill up to 10 symbols
                {
                  Text = Text + ":"; // 
                }
          RDSCounter = 2;
          psacCounter = 2; //psac offset
          break;

     case DIG18: //18 digits Numeric Message
          while ((Text.length() % 18) != 0) //fill up to 18 symbols
                {
                  Text = Text + ":"; // 
                }
          RDSCounter = 3;
          psacCounter = 4; //psac offset
          break;

      case ALPHA: //Alpha Message
          //Serial.print(Text.length());
          while (((Text.length()) % 4) != 0) //fill up to 4 symbols block
                {
                  Text = Text + " "; //  
                }
          //Serial.print(Text.length());       
          RDSCounter = (Text.length()) / 4;
          psacCounter = 8; //psac offset
          break;
      }
// End of prepare counters

//---------------------------------------- Start Main Cycle
for (int i=0; i<RDSCounter; i++)
{
type_7A_adress tmp_CD; //union for adress c and d blocks
String tmp_Adress = Int2STR(Address,6); // 

if (Type != ALPHA) // non alpha messages
{
// Create and send address group
if ((psacCounter == 0) || (psacCounter == 2) || (psacCounter == 4)) // check for address group
{

// Create address
tmp_CD.refined.a_1 = int(tmp_Adress.charAt(0));
tmp_CD.refined.a_2 = int(tmp_Adress.charAt(1));
tmp_CD.refined.a_3 = int(tmp_Adress.charAt(2));
tmp_CD.refined.a_4 = int(tmp_Adress.charAt(3));
tmp_CD.refined.a_5 = int(tmp_Adress.charAt(4));
tmp_CD.refined.a_6 = int(tmp_Adress.charAt(5));

//Create data 
tmp_CD.refined.d_1 = int(Text.charAt(0));
tmp_CD.refined.d_2 = int(Text.charAt(1));

//upload address and data to Message Structure
Message.refined.address = tmp_CD.raw[1];
Message.refined.data = tmp_CD.raw[0];

//Define start counter for Paging Segment Address Code 
Message.refined.psac = psacCounter ; 

// Fill RDS Group
block_A.raw = Message.raw[3];
block_B.raw = Message.raw[2]; 
block_C.raw = Message.raw[1]; 
block_D.raw = Message.raw[0]; 

RDS_SEND_BUFFER (block_A, block_B, block_C, block_D, "7A", Monitor); 
if (Monitor) {Serial.println();}
i++; //next group
psacCounter++; //next group
if (Type == TONE) {break;} // interrupt for TONE type message
} //End of send address group non aplha 

// Prepare and send data groups
// fill data
tmp_CD.refined.a_1 = int(Text.charAt((i-1)*8+2));
tmp_CD.refined.a_2 = int(Text.charAt((i-1)*8+3));
tmp_CD.refined.a_3 = int(Text.charAt((i-1)*8+4));
tmp_CD.refined.a_4 = int(Text.charAt((i-1)*8+5));
tmp_CD.refined.a_5 = int(Text.charAt((i-1)*8+6));
tmp_CD.refined.a_6 = int(Text.charAt((i-1)*8+7));
tmp_CD.refined.d_1 = int(Text.charAt((i-1)*8+8));
tmp_CD.refined.d_2 = int(Text.charAt((i-1)*8+9));

//upload address and data to Message Structure
Message.refined.address = tmp_CD.raw[1];
Message.refined.data = tmp_CD.raw[0];

//Define start counter for Paging Segment Address Code 
Message.refined.psac = psacCounter ; 

// Fill RDS Group
block_A.raw = Message.raw[3];
block_B.raw = Message.raw[2]; 
block_C.raw = Message.raw[1]; 
block_D.raw = Message.raw[0]; 

RDS_SEND_BUFFER (block_A, block_B, block_C, block_D, "7A", Monitor); 
if (Monitor) {Serial.println();}
psacCounter++; //next group
} //End of NON ALPHA Message

if (Type == ALPHA) // alpha messages
{
// Create and send address group 
if (AddressFlag == 1) // check for address group
{
AddressFlag = 0; //reset addres group flag

// Create address
tmp_CD.refined.a_1 = int(tmp_Adress.charAt(0));
tmp_CD.refined.a_2 = int(tmp_Adress.charAt(1));
tmp_CD.refined.a_3 = int(tmp_Adress.charAt(2));
tmp_CD.refined.a_4 = int(tmp_Adress.charAt(3));
tmp_CD.refined.a_5 = int(tmp_Adress.charAt(4));
tmp_CD.refined.a_6 = int(tmp_Adress.charAt(5));

//Create data 
tmp_CD.refined.d_1 = 0x0; //any data
tmp_CD.refined.d_2 = 0x0; //any data

//upload address and data to Message Structure
Message.refined.address = tmp_CD.raw[1];
Message.refined.data = tmp_CD.raw[0];

//Define start counter for Paging Segment Address Code 
Message.refined.psac = psacCounter ; 

// Fill RDS Group
block_A.raw = Message.raw[3];
block_B.raw = Message.raw[2]; 
block_C.raw = Message.raw[1]; 
block_D.raw = Message.raw[0]; 

RDS_SEND_BUFFER (block_A, block_B, block_C, block_D, "7A", Monitor); 
if (Monitor) {Serial.println();}
psacCounter++; //next group
} //End of send address group non aplha   

// prepare data
type_7A_AplusD tmp_D;
tmp_D.raw = Text.charAt((i)*4);
tmp_CD.refined.a_1 = tmp_D.refined.data_lo; //symbol 1 in block
tmp_CD.refined.a_2 = tmp_D.refined.data_hi;

tmp_D.raw = Text.charAt((i)*4+1);
tmp_CD.refined.a_3 = tmp_D.refined.data_lo; //symbol 2 in block
tmp_CD.refined.a_4 = tmp_D.refined.data_hi;

tmp_D.raw = Text.charAt((i)*4+2);
tmp_CD.refined.a_5 = tmp_D.refined.data_lo; //symbol 3 in block
tmp_CD.refined.a_6 = tmp_D.refined.data_hi;

tmp_D.raw = Text.charAt((i)*4+3);
tmp_CD.refined.d_1 = tmp_D.refined.data_lo; //symbol 4 in block
tmp_CD.refined.d_2 = tmp_D.refined.data_hi;

//upload address and data to Message Structure
Message.refined.address = tmp_CD.raw[1];
Message.refined.data = tmp_CD.raw[0];

//Counter for Paging Segment Address Code 
if (i == (RDSCounter - 1)) //last packet
    {
       Message.refined.psac = 0xF; // Last packet
    } 
else 
    {
       Message.refined.psac = psacCounter ; // not last packet
    }  

// Fill RDS Group
block_A.raw = Message.raw[3];
block_B.raw = Message.raw[2]; 
block_C.raw = Message.raw[1]; 
block_D.raw = Message.raw[0]; 

RDS_SEND_BUFFER (block_A, block_B, block_C, block_D, "7A", Monitor); 
if (Monitor) {Serial.println();}

if (psacCounter == 0xE) //last group for 24 symbols group
   {
    psacCounter = 8; // reset psac for new 24 symbol group
   } 
else
   {psacCounter++;} //next group

} //End of ALPHA Message

} //End of main cycle 

    if (Monitor) //Output log
    {
      Serial.println("Sent 7A: " + Text);
    }

}
//======================================= END 7A================================================================


void SI4713::RDS_4A_TIME (uint16_t rds_pid, byte Bo, byte TP, byte PTY, uint16_t Year, byte Month, byte Day, byte Hour, byte Minute, byte O_Sign, byte O_Hour, byte O_Minute, byte Monitor) 
// Input: PI, Bo, TP, PTY, Year, Month, Day, Hour, Minute, Offset sign, Offset hour, Offset minute
// Monitor: 1-Output full info, 0-Output only "T"
{
long Current_MJD = ymd_to_mjd (Year, Month, Day); // Date as Modified Julian Date 60586=03/10/2024

//Start Calculate offset
int Offset = 0;
if (O_Hour <= 15)
{
  Offset = O_Hour * 2; 
}
if (O_Minute == 30)
{
  Offset = Offset + 1; 
}
//End Calculate offset

type_4A DateTime; // create 4A group union for send Date and Time
Block block_A, block_B, block_C, block_D; // create 2 bytes block for send

// fill DateTime Union 
DateTime.refined.offset = Offset; //0x1C=14:00; //
DateTime.refined.offset_sense = O_Sign; // Set offset sign bit
DateTime.refined.hour = Hour; // Set Time: hour
DateTime.refined.minute = Minute; // Set Time: minutes
DateTime.refined.mjd = Current_MJD; // Set Date
DateTime.refined.spr = 0; //Always 0; Does not matter, spare bits
DateTime.refined.pty = PTY; //set PTY as Scientist channel; Does not matter
DateTime.refined.TP = TP;  //set TP; Does not matter
DateTime.refined.Bo = Bo; // Must be 0 = Version A 
DateTime.refined.type = 4; //Must be 4
DateTime.refined.pid = rds_pid; // PI Programm identification

// Fill RDS Group
block_A.raw = DateTime.raw[3];
block_B.raw = DateTime.raw[2]; 
block_C.raw = DateTime.raw[1]; 
block_D.raw = DateTime.raw[0]; 

RDS_SEND_BUFFER (block_A, block_B, block_C, block_D, "4A", Monitor); 
    
    if (Monitor) //Output log
    {
      
      char Output [17]; //tmp string for Date Time Offset
      char O_sign_s[2] = {'+', NULL}; //1='-', 0='+'
      
      if (O_Sign == 1) //Check Offset Sign
      {
      O_sign_s[0] = '-'; //Set offset sign 
      }

      sprintf(Output, "%02d-%02d-%4d %02d:%02d %1s%02d:%02d", Day, Month, Year, Hour, Minute, O_sign_s, O_Hour, O_Minute);
      Serial.println(Output);
    
    }
}
//=======================================================================================================

// -----------------------------------  New functıon for senf 2A Group
void SI4713::RDS_2A_RT (uint16_t rds_pid, byte Bo, byte TP, byte PTY, byte ABflag, String RT, byte Monitor)
// Input: PI, Bo, TP, PTY, Text A/B flag, Radio Text
// Monitor: 1-Output full info, 0-Output only "T"
{
type_2A RadioText; // create 2A group union for send Radio Text
Block block_A, block_B, block_C, block_D; // create 2 bytes blocks for send

String Text = RT; // source text

while ((Text.length() % 4) != 0) //fill up to blocks with 4 symbols
    {
      Text = Text + " ";
    }

// fill Radio Text Static fields 
RadioText.refined.ABflag = 0; //Always 0; see comments in typedef type_2A_group  
RadioText.refined.pty = PTY; //set PTY as Jazz channel; Does not matter
RadioText.refined.TP = TP;  //set TP; Does not matter
RadioText.refined.Bo = Bo; // Must be 0 = Version A 
RadioText.refined.type = 2; //Must be 2
RadioText.refined.pid = rds_pid; // PI Programm identification

int RDSCounter = Text.length()/4; //Total packets for 2A groups 

for (int i=0; i<RDSCounter; i++)
{

RadioText.refined.counter = i; //Counter

Block tmp; //for Block C and D data filling
char Symbol = Text[i*4];

tmp.refined.HighByte = Symbol; // WORK !!!
Symbol = Text[i*4+1];
tmp.refined.LowByte = Symbol; 
RadioText.refined.text1 = tmp.raw; // 1 and 2 char 

Symbol = Text[i*4+2];
tmp.refined.HighByte = Symbol; // WORK !!!
Symbol = Text[i*4+3];
tmp.refined.LowByte = Symbol; 
RadioText.refined.text2 = tmp.raw; // 3 and 4 char 

// Fill RDS Group
block_A.raw = RadioText.raw[3];
block_B.raw = RadioText.raw[2]; 
block_C.raw = RadioText.raw[1]; 
block_D.raw = RadioText.raw[0]; 

RDS_SEND_BUFFER (block_A, block_B, block_C, block_D, "2A", Monitor); 

if (Monitor) //Output log
    {
      Serial.println(Text.substring(i*4,i*4+4));
    }
}
// End of cycle

    if (Monitor) //Output log
    {
      Serial.println("Sent 2A: " + Text);
    }
}
//=====================================================================================================================================

// --------------------------       Send RDS packet --------------------------------------------
void SI4713::RDS_SEND_BUFFER (Block A, Block B, Block C, Block D, String Group, byte Monitor) 
{
// Fill chip buffer for send RDS group
buf[0] = 0x35; //Create buffer TX_RDS_BUFF
buf[1] = 0x84; //Set FIFO and LDBUFF 0x84
buf[2] = B.refined.HighByte; //;
buf[3] = B.refined.LowByte; //;
buf[4] = C.refined.HighByte; //;
buf[5] = C.refined.LowByte; //;
buf[6] = D.refined.HighByte; //
buf[7] = D.refined.LowByte;  //

WriteBuffer(8);

    if (Monitor) //Output log
    {
      Serial.print("Tx " + Group + ": " + Int2HEX (A.raw, 4) + " " + Int2HEX (B.raw, 4) + " " + Int2HEX (C.raw, 4) + " " + Int2HEX (D.raw, 4) +" : ");
    }
}
//=======================================================================================================