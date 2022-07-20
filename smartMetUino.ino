
#if 1
#if 1

#define LOGw(s) ;
#define LOGHEX(s) ;
#define LOG_(s) ;
#define LOG__(s) Serial.print(s)
#define LOG(s)  Serial.println(s)

#else

// may interfere with serial.read ....
#define LOGw(s) Serial.write(s)
#define LOGHEX(s) Serial.print(s, HEX)
#define LOG_(s) Serial.print(s)
#define LOG__(s) Serial.print(s)
#define LOG(s)  Serial.println(s)

#endif

#else

#define LOGw(s) ;
#define LOG_(s) ;
#define LOG__(s) ;
#define LOG(s)  ;
#define LOGHEX(s);


#endif


#include <SoftwareSerial.h>

#if 0
#define rxPin_0 5
#define txPin_0 6
SoftwareSerial mySerial_0 =  SoftwareSerial(rxPin_0, txPin_0);
#endif

#define rxPin_1 7
#define txPin_1 8
SoftwareSerial mySerial_1 =  SoftwareSerial(rxPin_1, txPin_1);

#define ledPin_Board 13
#define ledPin_0  4
#define ledPin_1  3


byte inByte = 0;         // incoming serial byte

byte PLACEHOLDER   = 0xFE;
byte SKIPP_BETWEEN = 0xFC;
byte TYPE          = 0xFD;
//byte pattern[] = {0x77 ,0x07 ,0x01 ,0x00 ,PLACEHOLDER ,0x08 ,PLACEHOLDER ,0xFF ,0x64 ,0x1C ,0x21 ,0x04 ,0x72 ,0x62 ,0x01 ,0x65 ,0x03 ,0x35 ,0xBF ,0x79 ,0x62 ,0x1E ,0x52 ,0xFF ,0x65};
byte _pattern[]         = {0x77 ,0x07 ,0x01 ,0x00 ,PLACEHOLDER ,0x08 ,PLACEHOLDER ,0xFF , SKIPP_BETWEEN ,0xFF ,TYPE};
byte _pattern_len = sizeof(_pattern)/ sizeof(byte);
byte _pattern_runtime[] = {0x00 ,0x62 ,0x0A ,0xFF ,0xFF        ,0x72 ,0x62        ,0x01 , TYPE}; // kludge ensure same  length ...
byte _pattern_runtime_len = sizeof(_pattern_runtime)/ sizeof(byte);



byte _row_len[2] = {};
byte _match[2] = {};
byte _digit_idx[2] = {};

#define DIGIT_MAX_VALUE_COUNT 7
#define DIGIT_VALUE_SIZE DIGIT_MAX_VALUE_COUNT+1
unsigned long digit[2][ DIGIT_VALUE_SIZE ] = {};
byte          ledPin[2] = { ledPin_0, ledPin_1  };


// max delay (after last serial*.available) in milli-sec to reset serial-data as "incomplete"
#define SERIAL_TIMER_DISABLE  1
#define SERIAL_TIMER_CLEAR    0
#define SERIAL_TIMER_DELAY  100
unsigned long timer[2] = {};


#include <SPI.h>
#include <SD.h>
File dataFile;


void setup() {
  // LOG / Command - Line
  Serial.begin(9600);
  while (!Serial) {;} // wait for serial port to connect. Needed for native USB port only

  digitalWrite( ledPin_Board, HIGH );

  // Line 0
  //pinMode(rxPin_0, INPUT);
  //pinMode(txPin_0, OUTPUT);
  //mySerial_0.begin(9600);


  // Line 1
  pinMode(rxPin_1, INPUT);
  pinMode(txPin_1, OUTPUT);
  mySerial_1.begin(9600);


  // LED
  pinMode(ledPin_0  , OUTPUT);
  pinMode(ledPin_1  , OUTPUT);

  //Serial.print( mySerial_0.isListening() ? "0" : "_" );
  Serial.print( mySerial_1.isListening() ? "1" : "_" );Serial.println( " is on");

  digitalWrite( ledPin_0, HIGH );
  digitalWrite( ledPin_1, HIGH );

  LOG("Initializing SD card...");
  if (!SD.begin(10)) 
  {
      Serial.println("initialization 10 failed!");
      while (1);
  }  
  digitalWrite( ledPin_0, LOW );

  if (!SD.begin(9)) 
  {
      Serial.println("initialization 9 failed!");
      while (1);
  }
  digitalWrite( ledPin_1, LOW );

  // 
  LOG("ready to mirror your entries!");

  digitalWrite( ledPin_Board, LOW );
  //memset(_digit_idx, 0, sizeof(_digit_idx)); // need to force clear 
}



bool writeSD(byte sdPin){
  bool written = false;
  if (!SD.begin(sdPin)) 
  {
      Serial.println(String(String(sdPin  ) + "initialization failed!"));
      // while (1); // just skipp
  } else {
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    //
    // String fileName = String(String( digit[0][0] % 100000000 ) + ".txt");
    String fileName = String(String(( digit[0][0] % 100000000 ) / 1000 ) + ".txt");
    if (10 == sdPin && 0 == digit[0][0] % 10) {
          Serial.print( fileName );
          Serial.print(" ");Serial.print(digit[0][1]);
          Serial.print(" ");Serial.print(digit[0][6]);
          Serial.print(" ");Serial.print(digit[1][1]);
          Serial.print(" ");Serial.println(digit[1][6]);
    };
    dataFile = SD.open( fileName, FILE_WRITE);
    if (dataFile)  // if the file opened okay, write to it:
    {
        LOG_("Writing to ...");
        for(int srcId = 0 ; srcId < 2; srcId++) {
          for(int idx = 0 ; idx < DIGIT_MAX_VALUE_COUNT ; idx++) {
            dataFile.print(digit[srcId][idx]);dataFile.print(";");
          }
        }
        dataFile.println("");
        dataFile.close();
        LOG("done.");
        written = true;
    } 
    else 
    {
        Serial.print("error opening file: ");Serial.println(fileName);// if the file didn't open, print an error:
    }

  }
  return written;
}


void writeSML(){
  writeSD(9);
  if (!writeSD(10)) {
    bool toggle = HIGH;
    digitalWrite( ledPin_Board, toggle );
    while(!writeSD(10)) {
      delay(500);
      digitalWrite( ledPin[0] , toggle = !toggle); 
      digitalWrite( ledPin[1] , !toggle); 
    }
  }
  /*
  for(int srcId = 0 ; srcId < 2; srcId++) {
    for(int idx = 0 ; idx < DIGIT_MAX_VALUE_COUNT ; idx++) {
      Serial.print(digit[srcId][idx]);Serial.print(";");
    }
  }
  Serial.println("");
  */
  memset(_match, 0, sizeof(_match));
  memset(digit, 0, sizeof(digit));
  memset(_digit_idx, 0, sizeof(_digit_idx));
}

void resetSML(){
  memset(_match, 0, sizeof(_match));
  memset(digit, 0, sizeof(digit));
  memset(_digit_idx, 0, sizeof(_digit_idx));
}

void resetSML(byte srcId){
  _match[ srcId ] = 0;
  for(byte i=0; i < DIGIT_MAX_VALUE_COUNT; i++) digit[ srcId ][ i ] = 0;
  _digit_idx[ srcId ] = 0;
}


void checkTimer( byte srcId) {
  if ( SERIAL_TIMER_DISABLE == timer[ srcId ]) {
  } else if ( SERIAL_TIMER_CLEAR == timer[ srcId ]) {
    timer[ srcId ] = millis() + SERIAL_TIMER_DELAY ;
  } else if ( timer[ srcId ] <  millis() ) {
    if ( _digit_idx[ srcId ] > 0 || _match[ srcId ] > 0 ) { // reset data --- havn't seen data for a while
        resetSML( srcId );
        digitalWrite(  ledPin[ srcId ], HIGH) ; // signal distress - will be reset by usual led-switching 
        digitalWrite(  ledPin_Board , HIGH) ; // signal distress - will be reset by usual led-switching 
    } 
    timer[ srcId  ] = SERIAL_TIMER_DISABLE ;
  }
}

void readSML(byte inByte, byte srcId) {
  readSML(inByte, srcId,  0 == _digit_idx[srcId] ? _pattern_runtime : _pattern ,  0 == _digit_idx[srcId] ? _pattern_runtime_len : _pattern_len); 
  //readSML(inByte, srcId,  _pattern ,  _pattern_len); 
}

void readSML(byte inByte, byte srcId, byte pattern[], byte pattern_len){ 
  byte match = _match[srcId];
  timer[ srcId ] = SERIAL_TIMER_CLEAR ;
  // 77 07 01 00   01 08 00   FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65   04 35 50 BC   01  --> 1.8.0;70602940 --> 70602940
  // SKIPP:
  // 77 07 01 00   60 32 01   01 01 01 01 01 04 45 46 52 01                                                  --> 96.50.1
  // 77 07 01 00   60 01 00   FF 01 01 01 01 0B 0A 01 45 46 52 20 03 B2 A6 18 01                             --> 96.1.0
  // LOGw(inByte);// use write for bytes ...    rather than Serial.print(inByte);

  if (0!=match) LOGHEX(inByte);   
  if ( pattern_len <= match) {
    if ( _digit_idx[srcId] >= DIGIT_VALUE_SIZE ) {
        // _digit_idx[srcId]=0 ; // just to be safe !!!
    } else if ( match < _row_len[srcId] ) {
      if (0 == _digit_idx[srcId] /*&& 0 == digit[srcId][_digit_idx[srcId]]*/) {
        digitalWrite( ledPin[srcId] , HIGH);
      } 
      digit[srcId][_digit_idx[srcId]] = digit[srcId][_digit_idx[srcId]] * 256 + inByte;
      LOG__("+");
      match++;
    } else {
      // Serial.println(digit[srcId][_digit_idx[srcId]]);
      digitalWrite( ledPin[srcId] , LOW); 
      _digit_idx[srcId]++;
      LOG__("=");LOG_(" ");
      match = 0;
    }    
  } else if (inByte == pattern[match])  { // char(FF) != byte(255)  !!!
    if( 0 == match) {LOG_("  >");LOGHEX (inByte);};LOG_(".");
    match++;
  } else if (SKIPP_BETWEEN == pattern[match])  { 
    if (inByte == pattern[match+1]) {
      match += 2;
    }
    LOG__("*");
  } else if (PLACEHOLDER == pattern[match] )  { 
    LOG_(inByte);  LOG_( 0xFF == pattern[match+1] ? ";" : ".");
    match++;
  } else if (TYPE == pattern[match] )  { // value tells about values type -- https://www.msxfaq.de/sonst/bastelbude/smartmeter_d0_sml_protokoll.htm
    // 52,53,55,59 => Integer 8/16/32/64          56 => Integer         62,63,65,69 => Unsigned Int 8/16/32/64
    _row_len[srcId] = pattern_len + (0x62 == inByte ? 1 : 0x65 == inByte ? 4 : 1 );
    if (_digit_idx[srcId] < DIGIT_VALUE_SIZE) digit[srcId][_digit_idx[srcId]] = 0; // reset .. just in case ... 
    LOG_("T");
    match++;
  } else {
    if (0!=match) {LOG__("<  ");}   
    match = 0;
  } // match
  _match[srcId] = match;
  if (0!=match) {LOG_(" ");}   
  
}



/* Test-Data

1B 1B 1B 1B 01 01 01 01 76 05 00 35 9B C5 62 00 62 00 72 63 01 01 76 01 07 FF FF FF FF FF FF 05 00 11 DE 98 0B 0A 01 45 46 52 20 03 B2 A6 18 72 62 01 65 03 46 E1 DD 01 63 AB 64 00 76 05 00 35 9B C6 62 00 62 00 72 63 07 01 77 07 FF FF FF FF FF FF 0B 0A 01 45 46 52 20 03 B2 A6 18 07 01 00 62 0A FF FF 72 62 01 65 03 46 E1 DD 79 77 07 01 00 60 32 01 01 01 01 01 01 04 45 46 52 01 77 07 01 00 60 01 00 FF 01 01 01 01 0B 0A 01 45 46 52 20 03 B2 A6 18 01 77 07 01 00 01 08 00 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 04 35 50 BC 01 77 07 01 00 01 08 01 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 02 BF 84 7A 01 77 07 01 00 01 08 02 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 01 75 CC 42 01 77 07 01 00 02 08 00 FF 01 72 62 01 65 03 46 E1 DD 62 1E 52 FF 62 10 01 77 07 01 00 02 08 01 FF 01 72 62 01 65 03 46 E1 DD 62 1E 52 FF 62 0E 01 77 07 01 00 02 08 02 FF 01 72 62 01 65 03 46 E1 DD 62 1E 52 FF 62 02 01 77 07 01 00 10 07 00 FF 01 01 62 1B 52 00 52 06 01 01 01 63 5E B8 00 76 05 00 35 9B C7 62 00 62 00 72 63 02 01 71 01 63 90 2D 00 00 1B 1B 1B 1B 1A 01 84 30 

-- Time 
18 07 01 00 62 0A FF FF 72 62 01 65 03 46 E1 DD 79 77 07 01

*/


void loop() {


  if (Serial.available() > 0) {
    byte inByte = Serial.read();
    readSML(inByte, 0);
    if (_digit_idx[0] > DIGIT_MAX_VALUE_COUNT) {
      Serial.print("H");Serial.print(_digit_idx[0]);Serial.print(_digit_idx[1]);
    }    
  } else checkTimer( 0) ;


  if (mySerial_1.available() > 0) {
    readSML(mySerial_1.read(), 1);
    if (_digit_idx[1] > DIGIT_MAX_VALUE_COUNT) {
      Serial.print("S");Serial.print(_digit_idx[0]);Serial.print(_digit_idx[1]);
    }
  } else checkTimer( 1) ;


  if ((   (0 == _digit_idx[0] && _digit_idx[1] > 0) || _digit_idx[0] >= DIGIT_MAX_VALUE_COUNT) 
      && ((0 == _digit_idx[1] && _digit_idx[0] > 0) || _digit_idx[1] >= DIGIT_MAX_VALUE_COUNT))  {
    //serials_reset_time  = SERIAL_RESET_DISABLE; // all good - no need to check ...
    // correct times ...
  //  digit[0][0] += 1657884311 - 54976989;
  //  digit[1][0] += 1657884311 - 54976989;
    //
    digitalWrite(ledPin_Board , HIGH);
    Serial.println("");
    writeSML();
    digitalWrite(ledPin_Board , LOW);
    // resetSML();
  }




/*

1B 1B 1B 1B 01 01 01 01 76 05 00 35 9B C5 62 00 62 00 72 63 01 01 76 01 07 FF FF FF FF FF FF 05 00 11 DE 98 0B 0A 01 45 46 52 20 03 B2 A6 18 72 62 01 65 03 46 E1 DD 01 63 AB 64 00 76 05 00 35 9B C6 62 00 62 00 72 63 07 01 77 07 FF FF FF FF FF FF 0B 0A 01 45 46 52 20 03 B2 A6 18 07 01 00 62 0A FF FF 72 62 01 65 03 46 E1 DD 79 77 07 01 00 60 32 01 01 01 01 01 01 04 45 46 52 01 77 07 01 00 60 01 00 FF 01 01 01 01 0B 0A 01 45 46 52 20 03 B2 A6 18 01 77 07 01 00 01 08 00 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 04 35 50 BC 01 77 07 01 00 01 08 01 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 02 BF 84 7A 01 77 07 01 00 01 08 02 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 01 75 CC 42 01 77 07 01 00 02 08 00 FF 01 72 62 01 65 03 46 E1 DD 62 1E 52 FF 62 10 01 77 07 01 00 02 08 01 FF 01 72 62 01 65 03 46 E1 DD 62 1E 52 FF 62 0E 01 77 07 01 00 02 08 02 FF 01 72 62 01 65 03 46 E1 DD 62 1E 52 FF 62 02 01 77 07 01 00 10 07 00 FF 01 01 62 1B 52 00 52 06 01 01 01 63 5E B8 00 76 05 00 35 9B C7 62 00 62 00 72 63 02 01 71 01 63 90 2D 00 00 1B 1B 1B 1B 1A 01 84 30 

    
incomplete...
1B 1B 1B 1B 01 01 01 01 76 05 00 35 9B C5 62 00 62 00 72 63 01 01 76 01 07 FF FF FF FF FF FF 05 00 11 DE 98 0B 0A 01 45 46 52 20 03 B2 A6 18 72 62 01 65 03 46 E1 DD 01 63 AB 64 00 76 05 00 35 9B C6 62 00 62 00 72 63 07 01 77 07 FF FF FF FF FF FF 0B 0A 01 45 46 52 20 03 B2 A6 18 07 01 00 62 0A FF FF 72 62 01 65 03 46 E1 DD 79 77 07 01 00 60 32 01 01 01 01 01 01 04 45 46 52 01 77 07 01 00 60 01 00 FF 01 01 01 01 0B 0A 01 45 46 52 20 03 B2 A6 18 01 77 07 01 00 01 08 00 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 04 35 50 BC 01 77 07 01 00 01 08 01 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF 65 02 BF 84 7A 01 77 07 01 00 01 08 02 FF 64 1C 21 04 72 62 01 65 03 46 E1 DD 62 1E 52 FF


    = works
    + add timer to clear incomplete data ... / generic timer
    - both LED stay on, wont get off
    + LED per channel, LED on when getting data / off if read
    ==> SW+HW-Serial can read in parallel !!!!
    ! make sure waiting HW-AND-SW-SERIAL to feed both with data ...
    ! write SML erases both -> wait for both to finish ...
    + just resetSML and do not print ... -> only <HW> Tag recieved, but no <SW>
    = works - <SW><\r><\n>70602940;46105722;24497218;16;14;2;70602940;46105722;24497218;16;14;2;<\r><\n>   <-- but <HW>-Tag is missing .... and hterm
    + parallel HW-Serial and SW-Serial
    - works like sharme .. but may reduce sampling rate 
    + Aktive Switch between both ...
    - erst serial-0.avail(), dann serial-1.avail() => NOTHING   (=|= es ging mal .... warum auch immmer ...)
    - erst serial-1.avail(), dann serial-0.avail() => NOTHING <= wenn beide exakt gleich sind kann nur 1 lesen
        - see "E:\OLD_D\_progs\Arduino\hardware\arduino\avr\libraries\SoftwareSerial\src\SoftwareSerial.cpp"
        - according listen() -  there is only one listening a time (cant run parallel as it actively listen the whole time ...  tunedDelay ... )
    - erst serial-0.avail(), dann serial-1.avail() =>  70602940;46105722;24497218;16;14;2;0;0;0;0;0;0; <= nur serial_0 gelesen ....
    - verbunden serial-Rx -> serial_0-Rx -> serial_1-Rx
*/  


}
