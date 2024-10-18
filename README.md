/* Paging LAB RDS Encoder V1.01

 * Version List:
 * - V1 - Start version
 * - V1.01 - Fix bug with long text messages more than 24 symbols; fix bug with Tone Only Messsges *
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
 * - Aplphanimeric - up to 80 symbols
 * - Data and Time - Fixed. You can upgrade encdoder with any RTC module if you want. F.e. https://github.com/PaulStoffregen/DS1307RTC
 * - Monitor: turn ON for monitoring RDS packet sending
 * - Test message: ON/OFF; periodicaly send Numeric 10 digits format message
 * - Warnings: there is no validation of input data values, please enter the data correctly
 * - IMPORTANT: serial port baudrate = 57600; Terminal settings: No line Ending
 * - National/International Mode: Work only in National mode, you need read Country code from EEPROM or try all countries from 1 to F
 * - Pager`s Adrress: The pager address can be found on the back cover. If it is missing, you will have to read it from EEPROM I2C 24C02.
 *
 * FaceBook group about paging: https://www.facebook.com/groups/116072934759380                    
 */

