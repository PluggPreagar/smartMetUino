byte inByte = 0;         // incoming serial byte

byte PLACEHOLDER = 0xFE;
byte TYPE = 0xFD;
byte SKIPP_BETWEEN = 0xFC;
//byte pattern[] = {0x77 ,0x07 ,0x01 ,0x00 ,PLACEHOLDER ,0x08 ,PLACEHOLDER ,0xFF ,0x64 ,0x1C ,0x21 ,0x04 ,0x72 ,0x62 ,0x01 ,0x65 ,0x03 ,0x35 ,0xBF ,0x79 ,0x62 ,0x1E ,0x52 ,0xFF ,0x65};
byte pattern[] = {0x77 ,0x07 ,0x01 ,0x00 ,PLACEHOLDER ,0x08 ,PLACEHOLDER ,0xFF , SKIPP_BETWEEN ,0xFF ,TYPE};
byte pattern_len = sizeof(pattern)/ sizeof(byte);

byte row_len[2] = {};
byte _match[2] = {};
byte digit_idx[2] = {};
#define DIGIT_MAX_VALUE_COUNT 6
#define DIGIT_VALUE_SIZE DIGIT_MAX_VALUE_COUNT+1
unsigned long digit[2][ DIGIT_VALUE_SIZE ] = {};


#if 0

#define LOGw(s) Serial.write(s)
#define LOG_(s) Serial.print(s)
#define LOG(s)  Serial.println(s)

#else

#define LOGw(s) ;
#define LOG_(s) ;
#define LOG(s)  ;


#endif


#include <SoftwareSerial.h>
#define rxPin_0 5
#define txPin_0 6
SoftwareSerial mySerial_0 =  SoftwareSerial(rxPin_0, txPin_0);
#define rxPin_1 7
#define txPin_1 8
SoftwareSerial mySerial_1 =  SoftwareSerial(rxPin_1, txPin_1);



void setup() {
  // LOG / Command - Line
  Serial.begin(9600);
  while (!Serial) {;} // wait for serial port to connect. Needed for native USB port only

  // Line 0
  pinMode(rxPin_0, INPUT);
  pinMode(txPin_0, OUTPUT);
  mySerial_0.begin(9600);


  // Line 1
  pinMode(rxPin_1, INPUT);
  pinMode(txPin_1, OUTPUT);
  mySerial_1.begin(9600);


  Serial.print( mySerial_0.isListening() ? "0" : "_" );Serial.print( mySerial_1.isListening() ? "1" : "_" );Serial.println( " is on");


  // 
  LOG("ready to mirror your entries!");
  LOG_("sizeof(pattern) = ");LOG(pattern_len);
  LOG("wait...");

  //memset(digit_idx, 0, sizeof(digit_idx)); // need to force clear 
}

void writeSML(){
  for(int srcId = 0 ; srcId < 2; srcId++) {
    for(int idx = 0 ; idx < DIGIT_MAX_VALUE_COUNT ; idx++) {
      Serial.print(digit[srcId][idx]);Serial.print(";");
    }
  }
  Serial.println("");
  memset(digit, 0, sizeof(digit));
  memset(_match, 0, sizeof(_match));
  memset(digit_idx, 0, sizeof(digit_idx));
}

void readSML(byte inByte, byte srcId){
  byte match = _match[srcId];
  // 77 07 01 00   01 08 00   FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65   04 35 50 BC   01  --> 1.8.0;70602940 --> 70602940
  // SKIPP:
  // 77 07 01 00   60 32 01   01 01 01 01 01 04 45 46 52 01                                                  --> 96.50.1
  // 77 07 01 00   60 01 00   FF 01 01 01 01 0B 0A 01 45 46 52 20 03 B2 A6 18 01                             --> 96.1.0
  // LOGw(inByte);// use write for bytes ...    rather than Serial.print(inByte);
  if ( pattern_len <= match) {
    if ( match < row_len[srcId] ) {
      digit[srcId][digit_idx[srcId]] = digit[srcId][digit_idx[srcId]] * 256 + inByte;
      match++;
    } else {
      // Serial.println(digit[srcId][digit_idx[srcId]]);
      digit_idx[srcId]++;
      if (DIGIT_VALUE_SIZE <= digit_idx[srcId]) digit_idx[srcId] = 0; // need  signal for "SML-Message complete"
      match = 0;
    }    
  } else if (inByte == pattern[match])  { // char(FF) != byte(255)  !!!
    match++;
  } else if (SKIPP_BETWEEN == pattern[match])  { 
    if (inByte == pattern[match+1]) {
      match += 2;
    }
  } else if (PLACEHOLDER == pattern[match] )  { 
    LOG_(inByte);  LOG_( 0xFF == pattern[match+1] ? ";" : ".");
    match++;
  } else if (TYPE == pattern[match] )  { // value tells about values type -- https://www.msxfaq.de/sonst/bastelbude/smartmeter_d0_sml_protokoll.htm
    // 52,53,55,59 => Integer 8/16/32/64          56 => Integer         62,63,65,69 => Unsigned Int 8/16/32/64
    row_len[srcId] = pattern_len + (0x62 == inByte ? 1 : 0x65 == inByte ? 4 : 1 );
    digit[srcId][digit_idx[srcId]] = 0; // reset .. just in case ... 
    match++;
  } else {
    match = 0;
  } // match
  _match[srcId] = match;
}


/* Test-Data

1B 1B 1B 1B 01 01 01 01 76 05 00 35 9B C5 62 00 62 00 72 63 01 01 76 01 07 FF FF FF FF FF FF 05 00 11 DE 98 0B 0A 01 45 46 52 20 03 B2 A6 18 72 62 01 65 03 46 E1 DD 01 63 AB 64 00 76 05 00 35 9B C6 62 00 62 00 72 63 07 01 77 07 FF FF FF FF FF FF 0B 0A 01 45 46 52 20 03 B2 A6 18 07 01 00 62 0A FF FF 72 62 01 65 03 46 E1 DD 79 77 07 01 00 60 32 01 01 01 01 01 01 04 45 46 52 01 77 07 01 00 60 01 00 FF 01 01 01 01 0B 0A 01 45 46 52 20 03 B2 A6 18 01 77 07 01 00 01 08 00 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 04 35 50 BC 01 77 07 01 00 01 08 01 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 02 BF 84 7A 01 77 07 01 00 01 08 02 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 01 75 CC 42 01 77 07 01 00 02 08 00 FF 01 72 62 01 65 03 46 E1 DD 62 1E 52 FF 62 10 01 77 07 01 00 02 08 01 FF 01 72 62 01 65 03 46 E1 DD 62 1E 52 FF 62 0E 01 77 07 01 00 02 08 02 FF 01 72 62 01 65 03 46 E1 DD 62 1E 52 FF 62 02 01 77 07 01 00 10 07 00 FF 01 01 62 1B 52 00 52 06 01 01 01 63 5E B8 00 76 05 00 35 9B C7 62 00 62 00 72 63 02 01 71 01 63 90 2D 00 00 1B 1B 1B 1B 1A 01 84 30 

*/

void loop() {

  // for tests ...    read from USB-Serial and clone #0 to #1
  //
  /*
  if (Serial.available() > 0) {
    byte inByte = Serial.read();
    readSML(inByte, 0);
    readSML(inByte, 1);
    if (digit_idx[0] >= DIGIT_MAX_VALUE_COUNT) {
      writeSML();
    }    
  }
  */




  if (mySerial_0.available() > 0) {
    readSML(mySerial_0.read(), 0);
    if (digit_idx[0] >= DIGIT_MAX_VALUE_COUNT) {
      writeSML();
      mySerial_1.listen();
    }
  }

  if (mySerial_1.available() > 0) {
    readSML(mySerial_1.read(), 1);
    if (digit_idx[1] >= DIGIT_MAX_VALUE_COUNT) {
      writeSML();
      mySerial_0.listen();
    }
  }



/*
    + Aktive Switch between both ...
    - erst serial-0.avail(), dann serial-1.avail() => NOTHING   (=|= es ging mal .... warum auch immmer ...)
    - erst serial-1.avail(), dann serial-0.avail() => NOTHING <= wenn beide exakt gleich sind kann nur 1 lesen
        - see "E:\OLD_D\_progs\Arduino\hardware\arduino\avr\libraries\SoftwareSerial\src\SoftwareSerial.cpp"
        - according listen() -  there is only one listening a time (cant run parallel as it actively listen the whole time ...  tunedDelay ... )
    - erst serial-0.avail(), dann serial-1.avail() =>  70602940;46105722;24497218;16;14;2;0;0;0;0;0;0; <= nur serial_0 gelesen ....
    - verbunden serial-Rx -> serial_0-Rx -> serial_1-Rx
*/  


}
