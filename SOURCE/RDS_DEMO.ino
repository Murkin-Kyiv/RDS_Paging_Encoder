/* Paging LAB RDS Encoder V1 
 * 
 * Protocol description: https://en.wikipedia.org/wiki/Radio_Data_System
 * Datasheet: https://web.archive.org/web/20161020015638/http://www.nrscstandards.org/SG/nrsc-4-B.pdf

 * Transmitter chip: SI4713 (f.e. Adafruit SI4713 PCB)
 * Source for chip library: https://github.com/PE5PVB/si4713 Library updated for send 1A, 4A and 7A groups
 * Datasheet for chip: https://www.skyworksinc.com/-/media/Skyworks/SL/documents/public/application-notes/AN332.pdf
 *  
 * Description: 
 * - Tone Only Messages
 * - 10 digits Numeric Messages - use ":" for space symbol
 * - 18 digits Numeric Messages - use ":" for space symbol
 * - Aplphanimeric - only 24 symbols (for messages longer than 24 characters there is a loss of synchronization, I haven't solution for fix this bug yet)
 * - Data and Time - Fixed. You can upgrade encdoder with any RTC module if you want. F.e. https://github.com/PaulStoffregen/DS1307RTC
 * - Monitor: turn ON for monitoring RDS packet sending
 * - Test message: ON/OFF; periodicaly send Numeric 10 digits format message
 * - Warnings: there is no validation of input data values, please enter the data correctly
 * - IMPORTANT: serial port baudrate = 57600; Terminal settings: No line Ending
 * - National/International Mode: Work only in National mode, you need read Country code from EEPROM or try all countries from 1 to F
 * - Pager`s Adrress: The pager address can be found on the back cover. If it is missing, you will have to read it from EEPROM I2C 24C02.
 *
 *FaceBook group about paging: https://www.facebook.com/groups/116072934759380                    
*/

#include "si4713.h" //transmitter library

bool overmod;
int8_t inlevel;

//Cycle Counters 
long G_4A_Counter = millis();
long G_1A_Counter = millis();
long G_7A_Counter = millis();
long G_2A_Counter = millis();


static Config Cfg_Base; //create  default config

//RTC
SI4713 TX;

void setup() {
  Cfg_Base.cfg_Offset.All = 0x04; // default UTC offset
  Cfg_Base.cfg_pi.All = 0x6277; // default PI, 6=Ukraine 0x6277
    
  Serial.begin(57600);
  Serial.setTimeout(50); //default 1000 ms
  
  TX.Init(RESET_TX_PIN, 32768, 0x63);   // RST pin (use -1 when using external supervisor), Crystal: 32.768kHz, I2C address: 0x63
 
  uint8_t pn;
  uint8_t chiprev;
 
  TX.Rev(pn, chiprev);                         // Receive Chipversion and revision Right: Detected chip: SI4113 Revision: 65
  Serial.print("TX CHIP: SI41" + String(pn));
  Serial.println(" Rev: " + String(chiprev));
  
  TX.Output(115, 4);                    // Output level: 115dBuV, antenna capacitor: 0.25pF * 4 = 1pF)
  TX.Freq(Cfg_Base.cfg_Frequency);      // Set Output frequency
  
  TX.MPX_Enable(1);                     // 1 = Enable MPX Stereocoder
  TX.MPX_Deviation(675);                // Set MPX pilot deviation to 6.75kHz
  TX.MPX_Freq(19000);                   // Set MPX pilot to 19000Hz (19kHz)

  TX.Audio_Deviation(7500);             // Set Audio deviation to 75.00kHz
  TX.Audio_Mute(0);                     // Unmute audio
  TX.Audio_PreEmphasis(50);             // Set Audio pre-emphasis to 50uS (can be switched to 75uS or 0 for no pre-emphasis)
  TX.Audio_Limiter(0);                  // Disable Audio limiter
  TX.Audio_AGC(1);                      // Enable Audio Dynamic Range Control
  TX.Audio_Comp_Threshold(-40);         // Set Audio Dynamic Range Threshold to -40dBFS (max = 0dBFS)
  TX.Audio_Comp_Attack(0);              // Set Audio Dynamic Range Attack time to 0.5mS (max value = 9 -> 5.0mS, stepsize is 0.5mS)
  TX.Audio_Comp_Release(4);             // Set Audio Dynamic Range Release time to 1S (see p43 of application notes for other values)
  TX.Audio_Comp_Gain(15);               // Set Audio Dynamic Range Gain to 15dB (max=20dB)
  TX.Audio_Limiter_Release(102);        // Set Audio Limiter Release time to 5.01mS (see p44 of application notes for other values)

  TX.RDS_PI(Cfg_Base.cfg_pi.All);       // Set RDS PI code
  TX.RDS_AF(Cfg_Base.cfg_Frequency);    // Set RDS AF to 90.8MHz (use 0 to disable AF)
  TX.RDS_PTY(10);                       // Set RDS PTY to Pop Music (see list of codes at https://www.electronics-notes.com/articles/audio-video/broadcast-audio/rds-radio-data-system-pty-codes.php)
  TX.RDS_Deviation(200);                // Set RDS deviation to 2.00kHz
  TX.RDS_COMP(0);                       // Set RDS Compressed code to not Compressed
  TX.RDS_ART(0);                        // Set RDS Artificial Head code to Not artificial head
  TX.RDS_MS(1);                         // Set RDS Mono/Stereo code to stereo
  TX.RDS_TP(1);                         // Enable RDS Traffic Program
  TX.RDS_TA(0);                         // Disable RDS Traffic Announcement
  TX.RDS_MUSP(1);                       // Set RDS Music/Speech selection to Music
  
  TX.RDS_PS_MIX(0);                     // default=3; Sets the ratio of RDS PS (group 0A) and circular buffer/FIFO groups.  
  Update_0A(); // Update radioname and parameters

  TX.RDS_Enable(1);                     // 1 = Enable RDS encoder, 0 = Disable RDS encoder
  TX.GPO(0,0,0);                        // Set GPO outputs 1,2 and 3 to low.
  
  ShowStatus();
}

// ----------------------------------------------- Loop ------------------------------------------
void loop() {

//--------------------------------------------------------------------------- TIMERS
if (((millis() - G_4A_Counter) > G_4A_PERIOD) || (G_4A_Counter ==0 )) // send 4A each minute
  {
  TX.RDS_4A_TIME (Cfg_Base.cfg_pi.All, Cfg_Base.cfg_Bo, Cfg_Base.cfg_TP, Cfg_Base.cfg_PTY,
                   Cfg_Base.cfg_Year, Cfg_Base.cfg_Month, Cfg_Base.cfg_Day, Cfg_Base.cfg_Hour, Cfg_Base.cfg_Minute, 
                   Cfg_Base.cfg_Offset.refined.offset_sign, Cfg_Base.cfg_Offset.refined.offset_hour, Cfg_Base.cfg_Offset.refined.offset_minute*30, Cfg_Base.cfg_Monitor);
  G_4A_Counter = millis();
  }

if ((millis() - G_1A_Counter) > G_1A_PERIOD) // send 1A Group each second
   {
    TX.RDS_1A_PIN (Cfg_Base.cfg_pi.All, Cfg_Base.cfg_Bo, Cfg_Base.cfg_TP, Cfg_Base.cfg_PTY, Cfg_Base.cfg_1A_Rpc, Cfg_Base.cfg_1A_Slc, Cfg_Base.cfg_1A_Pinc, Cfg_Base.cfg_Monitor);
    G_1A_Counter = millis();
   }

if (((millis() - G_7A_Counter) > Cfg_Base.cfg_Test_Message_Period) && (Cfg_Base.cfg_Test_Message == ON)) // Send test Message
   {
    ADflagInvert();
    Serial.println("Debug Messade>>" + Int2STR(G_7A_Counter, 10));
    TX.RDS_7A_PAGING (Cfg_Base.cfg_pi.All, Cfg_Base.cfg_Bo, Cfg_Base.cfg_TP, Cfg_Base.cfg_PTY, Cfg_Base.cfg_7A_ADflag, DIG10, Cfg_Base.cfg_7A_Address, Int2STR(G_7A_Counter, 10), Cfg_Base.cfg_Monitor);
    G_7A_Counter = millis();
   }
   
if (((millis() - G_2A_Counter) > Cfg_Base.cfg_2A_Period*1000)  && Cfg_Base.cfg_2A_Period !=0) // send Radiotext
   {
    TX.RDS_2A_RT (Cfg_Base.cfg_pi.All, Cfg_Base.cfg_Bo, Cfg_Base.cfg_TP, Cfg_Base.cfg_PTY, Cfg_Base.cfg_2A_ADflag, Cfg_Base.cfg_2A_Text, Cfg_Base.cfg_Monitor);
    G_2A_Counter = millis();
   }

//----------------------------------------End Timers

  if (Serial.available() > 0) {
    
    String Command = ""; //copmmand from terminal or Sprut
    Command = Serial.readString(); // read command
        
    switch (Command.toInt()) { //parce command

    case SET_MONITOR: //Monitor mode ON/OFF
        SetMonitor();     
        ShowStatus ();
        break;

    case SET_TEST_MESSAGE: //Test Message Mode ON/OFF
        SetTestMessage();     
        ShowStatus ();
        break;
    
    case SEND_7A_TONE: //Send Tone Message

        ADflagInvert(); //set new message flag                
        TX.RDS_7A_PAGING (Cfg_Base.cfg_pi.All, Cfg_Base.cfg_Bo, Cfg_Base.cfg_TP, Cfg_Base.cfg_PTY, Cfg_Base.cfg_7A_ADflag, TONE, Cfg_Base.cfg_7A_Address, "AA", Cfg_Base.cfg_Monitor);
        break;

    case SEND_7A_NUM_10: //Send 10 Digits Numeric Message

        ADflagInvert(); //set new message flag                
        TX.RDS_7A_PAGING (Cfg_Base.cfg_pi.All, Cfg_Base.cfg_Bo, Cfg_Base.cfg_TP, Cfg_Base.cfg_PTY, Cfg_Base.cfg_7A_ADflag, DIG10, Cfg_Base.cfg_7A_Address, SetMessage ("Numeric Message [max=10]>"), Cfg_Base.cfg_Monitor);
        break;

    case SEND_7A_NUM_18: //Send 10 Digits Numeric Message

        ADflagInvert(); //set new message flag                
        TX.RDS_7A_PAGING (Cfg_Base.cfg_pi.All, Cfg_Base.cfg_Bo, Cfg_Base.cfg_TP, Cfg_Base.cfg_PTY, Cfg_Base.cfg_7A_ADflag, DIG18, Cfg_Base.cfg_7A_Address, SetMessage ("Numeric Message [max=18]>"), Cfg_Base.cfg_Monitor);
        break;

    case SEND_7A_ALPHA: //Send debug RDS packet
        
        ADflagInvert(); //set new message flag                
        TX.RDS_7A_PAGING (Cfg_Base.cfg_pi.All, Cfg_Base.cfg_Bo, Cfg_Base.cfg_TP, Cfg_Base.cfg_PTY, Cfg_Base.cfg_7A_ADflag, ALPHA, Cfg_Base.cfg_7A_Address, SetMessage ("Text Message [max=24]>"), Cfg_Base.cfg_Monitor);
        break;
    
     case SHOW_STATUS: //Send debug RDS packet
        ShowStatus ();
        break;

     case SET_FRQ: //Set frequency
        SetFRQ();     
        ShowStatus ();
        break;

      case SET_COUNTRY: //Set Country Code
        SetCountry();     
        ShowStatus ();
        break;
 
      case SET_7A_ADDRESS: //Set pager address
        Set7AAddress();     
        ShowStatus ();
        break;

      default:
      Serial.println("Command error");
      break;
    }
    
  }

}// ================================== End Loop

// FUNCTIONS

// --------------------------------- Show Status ------------------------
void ShowStatus()
{
    
  Serial.println("---------< Paging LAB * RDS Encoder V1 >----------");
  Serial.println("[xx]Radio Name: " + Cfg_Base.cfg_Radio_Name1 + "|" + Cfg_Base.cfg_Radio_Name2 + "|" + Cfg_Base.cfg_Radio_Name3 + "|" + Cfg_Base.cfg_Radio_Name4);
  Serial.println("[xx]Radio Text: " + Cfg_Base.cfg_2A_Text );
  
  //TIME and Frequency
  String tmp_Offset_str = "+"; //offset output string
  if (Cfg_Base.cfg_Offset.refined.offset_sign == 1) {tmp_Offset_str = "-";} //parce offset sign
  tmp_Offset_str = tmp_Offset_str + Int2STR(Cfg_Base.cfg_Offset.refined.offset_hour,2); //parce offset hour
  if (Cfg_Base.cfg_Offset.refined.offset_minute == 1) //parce offset minute
  {tmp_Offset_str = tmp_Offset_str + ":30";} 
  else
  {tmp_Offset_str = tmp_Offset_str + ":00";} 
  String tmp_FRQ = Int2STR(Cfg_Base.cfg_Frequency,5); //**** FREQUENCY *****
  Serial.println("[xx]UTC:" + Int2STR(Cfg_Base.cfg_Day,2) + "-" + Int2STR(Cfg_Base.cfg_Month,2) + "-" + Int2STR(Cfg_Base.cfg_Year,4) + " " +  Int2STR(Cfg_Base.cfg_Hour,2) + ":" + Int2STR(Cfg_Base.cfg_Minute,2) + " " + tmp_Offset_str + " [21]FRQ:" + tmp_FRQ.substring(0,3) + "." + tmp_FRQ.substring(3,4) + "MHz");

  // Help and monitoring  
  String tmp_Monitor = "OFF";
  if (Cfg_Base.cfg_Monitor==ON) {tmp_Monitor = "ON";}
  String tmp_Message = "OFF";
  if (Cfg_Base.cfg_Test_Message==ON) {tmp_Message = "ON";}
  Serial.println("[11]Help [12]Monitor:" + tmp_Monitor + " [13]Test Message:" + tmp_Message);
  // Country and address  
  Serial.println("[31]Country:"+ Int2HEX(Cfg_Base.cfg_pi.refined.pi_country,1) +" PI:" + Int2HEX(Cfg_Base.cfg_pi.All,4) + " [32]Address:"+ Int2STR(Cfg_Base.cfg_7A_Address,6));
  Serial.println("[71]Tone [72]Num_10 [73]Num_18 [74]Alphanumeric");
  Serial.println("--------------------------------------------------");
  
}
// =============================================================================================

// --------- Set Frequency Menu ------------------
void SetFRQ()
{
String Date_Input = "";
byte flag = 0;

Serial.print ("Frequency [xxx.x]>" );
while (flag == 0)
{
if (Serial.available()) // waiting input data
  {
    Date_Input = Serial.readString();
    flag = 1;

    uint16_t tmp_FRQ = Date_Input.substring(0,3).toInt() * 100 + Date_Input.substring(4,6).toInt()*10; 
    
    if (tmp_FRQ<=7600 || tmp_FRQ>=10800)
    {
      Serial.println("FRQ: Error");
    }
    else
    {
      Cfg_Base.cfg_Frequency = tmp_FRQ; //save config
      TX.Freq(Cfg_Base.cfg_Frequency); //set base frq
      TX.RDS_AF(Cfg_Base.cfg_Frequency);  //set RDS frq
    }
  }
}
}
//=====================================================================

//----------------------------- Set Country CODE  -----------------------
void SetCountry()
{  
String Input = "";
byte flag = 0;

Serial.print ("Country [xx]>" );
while (flag == 0)
{
if (Serial.available()) // waiting input data
  {
    Input = Serial.readString(); 
    flag = 1;
    
    if (Input.toInt() != 0) //validate country code
    {
      Cfg_Base.cfg_pi.refined.pi_country = Input.toInt(); //save to config
      TX.RDS_PI(Cfg_Base.cfg_pi.All); //update PI      
    }
    Serial.println(Input);
  }
}
}
//=================================================================================

//------------------------------- Update radioname and parameters for 0A Group ------------------------------------------------
void Update_0A() 
{ 
  TX.RDS_PS(Cfg_Base.cfg_Radio_Name1, 0);               // Set PS Message (max 8 characters) and position number in carousel
  TX.RDS_PS(Cfg_Base.cfg_Radio_Name2, 1);               // Set PS Message (max 8 characters) and position number in carousel
  TX.RDS_PS(Cfg_Base.cfg_Radio_Name3, 2);               // Set PS Message (max 8 characters) and position number in carousel
  TX.RDS_PS(Cfg_Base.cfg_Radio_Name4, 3);               // Set PS Message (max 8 characters) and position number in carousel
  TX.RDS_PSCOUNT(Cfg_Base.cfg_0A_slots, Cfg_Base.cfg_0A_speed);                // Number of PS Messages in carousel(4), (min 1, max 12),  and carousel speed (10) (min 1, max 255);
}
//=================================================================================

// -----------------------------  Set Pager Address -------------------------------
void Set7AAddress()

{  
String Input = "";
byte flag = 0;

Serial.print ("Address [xxxxxx]>" );
while (flag == 0)
 {
 if (Serial.available()) // waiting input data
  {
    Input = Serial.readString(); 
    flag = 1;
    
    Cfg_Base.cfg_7A_Address = Input.toInt();
    Serial.println(Int2STR(Cfg_Base.cfg_7A_Address,6));
  }
 }
}
//=================================================================================

// -------------------------- A/B Invertion for new message -----------------------
void ADflagInvert()
{
  if (Cfg_Base.cfg_7A_ADflag == 0) //Set A/B flag as new message
     {Cfg_Base.cfg_7A_ADflag = 1;}
  else
     {Cfg_Base.cfg_7A_ADflag = 0;}
}        
//=================================================================================

// -------------------------- Enter New Message -----------------------------------
String SetMessage(String Hint)
{
String Date_Input = "Error";
byte flag = 0;

Serial.print (Hint + ">" );
  while (flag == 0)
  {
    if (Serial.available()) // waiting input data
      {
        Date_Input = Serial.readString();
        flag = 1;
        Serial.println (Date_Input);
        return Date_Input;   
      } 
  }
}
//=================================================================================

//------------------------- Set Monitor Mode --------------------------------------
void SetMonitor()
{  
String Input = "";
byte flag = 0;

Serial.print ("Monitor [0/1]>" );
while (flag == 0)
  {
    if (Serial.available()) // waiting input data
      {
        Input = Serial.readString(); 
        flag = 1;
    
        if (Input.toInt() == 0) //Turn OFF
          {
            Cfg_Base.cfg_Monitor = OFF; //save to config
          }
          else 
          {
            Cfg_Base.cfg_Monitor = ON; //save to config
          }
        Serial.println(Input);
      }
  }
}
//=================================================================================

//------------------------- Set Test Message Mode --------------------------------------
void SetTestMessage()
{  
String Input = "";
byte flag = 0;

Serial.print ("Test Message [0/1]>" );
while (flag == 0)
 {
  if (Serial.available()) // waiting input data
   {
    Input = Serial.readString(); 
    flag = 1;
    
    if (Input.toInt() == OFF) //Turn OFF
    {
      Cfg_Base.cfg_Test_Message = OFF; //save to config
    }
    else 
    {
      Cfg_Base.cfg_Test_Message = ON; //save to config
    }
    Serial.println(Input);
   } 
 }
}
//=================================================================================
