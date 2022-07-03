byte inByte = 0;         // incoming serial byte
byte match=0;
byte pattern[] = {0x77 ,0x07 ,0x01 ,0x00 ,0x01 ,0x08 ,0x00 ,0xFF ,0x64 ,0x1C ,0x21 ,0x04 ,0x72 ,0x62 ,0x01 ,0x65 ,0x03 ,0x35 ,0xBF ,0x79 ,0x62 ,0x1E ,0x52 ,0xFF ,0x65};
byte pattern_len = sizeof(pattern)/ sizeof(byte);
byte digit_len = 4;
unsigned long digit = 0;
byte row_len = pattern_len + digit_len;

#define LOGw(s) Serial.write(s)
#define LOG_(s) Serial.print(s)
#define LOG(s)  Serial.println(s)

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
    } else if (inByte == pattern[match++] )  { // char(FF) != byte(255)  !!!
      LOG_("= ");
    } else {
      LOG_("<");LOG_(inByte);LOG_("<->");LOG_(pattern[match-1]);LOG("> ");
      match = 0;
    } // match
  } // serial.avail
  
}
