//Monitoring and debug
#define OFF 0
#define ON 1

#define RESET_TX_PIN 13 //RST pin: 11 (use -1 when using external supervisor)

#define TONE  0 //Tone message
#define DIG10 1 //10 digits numeric message
#define DIG18 2 //18 digits numeric message
#define ALPHA 3 //alpha message

//Timers
#define G_4A_PERIOD 60000 //default 1 minute = 60000 *DONT CHANGE IT*
#define G_1A_PERIOD 1000 //default 1 sec = 1000 *DONT CHANGE IT* 

// Menu
#define SHOW_STATUS          11   // Show Status Command
#define SET_MONITOR          12   // Set Monitor ON/OFF
#define SET_TEST_MESSAGE     13   // Set Test Messge ON/OFF

#define SET_FRQ         21   // Set frequency command

#define SET_COUNTRY     31   // Set country code command
#define SET_7A_ADDRESS  32   // Sep pager`s Address 

#define SEND_7A_TONE    71   // Send Message
#define SEND_7A_NUM_10  72
#define SEND_7A_NUM_18  73
#define SEND_7A_ALPHA   74

// Configuration 
typedef struct
    {
      //Monitor and Test message
      byte cfg_Monitor = OFF; // Debug Monitor ON/OFF
      byte cfg_Test_Message = ON; // Send Test Message ON/OFF
      long cfg_Test_Message_Period = 20000; //repeat test message in milliseconds 20 sec = 20000 Test message send in Numeric 10 digits format = milliseconds from system started
     

      //Tx Settings
      uint16_t cfg_Frequency = 9080;    //frequency 90.8 MHz
      
      //4A Settings
      int cfg_Year   = 2024;    // Year
      int cfg_Month  = 11;      // Month
      int cfg_Day    = 16;      // Day 
      int cfg_Hour   = 13;      // Hour  
      int cfg_Minute = 30;      // Minute 
      type_Offset cfg_Offset;          //yxxxz   y(offset_sign) -> 0 = '+'; 1 = '-'; xxx = from 0(00 hour) to 15(15 hour); z = 0=00 min, 1=30 min    
      
      //0A Settings
      type_pi cfg_pi;                  // Block A see PI structure above
      uint8_t  cfg_Bo  = 0;            // 0=A Version (for paging MUST be 0); 1=B Version 
      uint8_t  cfg_TP  = 0;            // 0=Non Traffic Proggram (default); 1=Non Traffic Proggram (does NOT affect for paging)
      uint8_t  cfg_PTY = 8;            // Programm Type 8 = Jazz (does NOT affect for paging)
            
      //0A Settings - Radio Name
      String cfg_Radio_Name1 = "Paging  "; // Radioname Slot_1, Max len=8
      String cfg_Radio_Name2 = "Lab     "; // Radioname Slot_2, 
      String cfg_Radio_Name3 = "RDS     "; // Radioname Slot_3, 
      String cfg_Radio_Name4 = "Encoder "; // Radioname Slot_4, 
      uint8_t cfg_0A_slots  = 4          ; // Number of slots Messages in carousel(4), (min 1, max 12). In this version I use only 4 Slots
      uint8_t cfg_0A_speed  = 1          ; // and carousel speed (min 1 sec, max sec);

      //2A settings
      String cfg_2A_Text = "Goog Luck!";   // Radio Text 
      uint8_t cfg_2A_Period  = 5;          // Duration for send 2A group (Radio Text); 0=Turn off send 2A group
      byte cfg_2A_ADflag = 0;              //Text A/B flag. I always use 0                     

      //1A Settings
      byte cfg_1A_Rpc = 6; // Radio Paging Codes (see protocol description bits: xxxyy xxx-group 001=0-99 groups yy-battery saving 
      uint16_t cfg_1A_Slc = 0x00E4; // Slow Labeling Code must be 00Ex x=0-4 (see Extended Country Code, Annex D, E4=Ukraine) (does NOT affect for paging in National mode)
      uint16_t cfg_1A_Pinc = 0x1234; // Programm Item Number (does NOT affect for paging)

      //7A Settings
      byte cfg_7A_ADflag = 0;             //Text A/B flag. If flag changined = new message; if not changed = repeat
      uint32_t cfg_7A_Address = 100466;   //Pager address ggnnnn gg-group nnnn-number in group //my pagers alpha text = 100466 //finder 100703
      String cfg_7A_Message ="";          // Current message 
                  
    }Config;

