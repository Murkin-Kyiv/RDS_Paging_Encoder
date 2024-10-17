// http://www.leapsecond.com/tools/gdate.c
// Convert year/month/day calendar date to Modified Julian Day (MJD).
// - year is (4-digit calendar year), month is (1-12), day is (1-31)
// - valid for Gregorian dates from 17-Nov-1858
// - adapted (tvb) from sci.astro FAQ
//
static long ymd_to_mjd (int year, int month, int day)
{
    long Y = year, M = month, D = day;
    long mjd =
        367 * Y
        - 7 * (Y + (M + 9) / 12) / 4
        - 3 * ((Y + (M - 9) / 7) / 100 + 1) / 4
        + 275 * M / 9
        + D + 1721029 - 2400001;
    return mjd;
}
// =================================================================================================

// convert integer to HEX as string f.e. 0x15 -> "0F", with special length, f.e. Len = 4 "000F" 
String Int2HEX (long Value, int Len) 
// input integer data or adress, output Len symbols as hex
{
String Out = "";
  Out = String(Value,HEX); 
  Out.toUpperCase();

  for (int i=0; Out.length() < Len; i++)
  {
    Out = "0"+ Out;
  }
  
return Out;
}
// =============================================================================================

// convert integer to String and add '0' in start up to len, f.e. 13,4 = "0013"
String Int2STR (long Value, int Len) 
{
String Out = "";
  Out = String(Value); 
  
  for (int i=0; Out.length() < Len; i++)
  {
    Out = "0"+ Out;
  }
  
return Out;
}
