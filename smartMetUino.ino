byte inByte = 0;         // incoming serial byte
byte match=0;
byte PLACEHOLDER = 0xFE;
byte TYPE = 0xFD;
byte SKIPP_BETWEEN = 0xFC;
//byte pattern[] = {0x77 ,0x07 ,0x01 ,0x00 ,PLACEHOLDER ,0x08 ,PLACEHOLDER ,0xFF ,0x64 ,0x1C ,0x21 ,0x04 ,0x72 ,0x62 ,0x01 ,0x65 ,0x03 ,0x35 ,0xBF ,0x79 ,0x62 ,0x1E ,0x52 ,0xFF ,0x65};
byte pattern[] = {0x77 ,0x07 ,0x01 ,0x00 ,PLACEHOLDER ,PLACEHOLDER ,PLACEHOLDER ,0xFF , SKIPP_BETWEEN ,0xFF ,TYPE};
byte pattern_len = sizeof(pattern)/ sizeof(byte);
byte digit_len = 4;
unsigned long digit = 0;
byte row_len = pattern_len + digit_len;

#if 0

#define LOGw(s) Serial.write(s)
#define LOG_(s) Serial.print(s)
#define LOG(s)  Serial.println(s)

#else

#define LOGw(s) inByte
#define LOG_(s) inByte
#define LOG(s)  inByte


#endif

void setup() {
  Serial.begin(9600);
  while (!Serial) {;} // wait for serial port to connect. Needed for native USB port only

  // 
  LOG("ready to mirror your entries!");
  LOG_("sizeof(pattern) = ");LOG(pattern_len);
  LOG("wait...");
}

void loop() {

  if (Serial.available() > 0) {
    inByte = Serial.read();
    LOGw(inByte);// use write for bytes ...    rather than Serial.print(inByte);
    if ( pattern_len <= match) {
      if ( match++ < row_len ) {
        if ( match == pattern_len) LOG(" << matched ");
        digit = digit * 256 + inByte;
        LOG_("+"); // Serial.print("=>");Serial.println(digit);
      } else {
        LOG_("=>");Serial.println(digit);
        match = 0;
        digit = 0; 
      }
    } else if (SKIPP_BETWEEN == pattern[match] && (inByte != pattern[match+1] || 0==++match))  { // tricke negative sideeffect to incr-match and still keep searching condition ...
    } else if (inByte == pattern[match])  { // char(FF) != byte(255)  !!!
      LOG_("= ");
      match++;
    } else if (PLACEHOLDER == pattern[match] )  { 
      Serial.print(inByte);  Serial.print( 0xFF == pattern[match+1] ? ";" : ".");
      match++;
    } else if (TYPE == pattern[match] )  { // value tells about values type 
      if ( 0x62 == inByte || 0x65 == inByte ) row_len = pattern_len + (0x62 == inByte ? 1 : 4 );
      match++;
    } else {
      LOG_("<");LOG_(inByte);LOG_("<->");LOG_(pattern[match]);LOG("> ");
      match = 0;
    } // match
  } // serial.avail
  
}
