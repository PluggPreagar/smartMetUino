#include <SPI.h>
#include <SD.h>

File dataFile;

void writeSD(){
    Serial.println("initialization done.");
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    
    dataFile = SD.open("data.txt", FILE_WRITE);
    
    // if the file opened okay, write to it:
    if (dataFile) 
    {
        Serial.print("Writing to data.txt...");
        dataFile.println("This is a data file :)");
        dataFile.println("testing 1, 2, 3.");
        for (int i = 0; i < 20; i++) 
        {
            dataFile.println(i);
        }
        // close the file:
        dataFile.close();
        Serial.println("done.");
    } 
    else 
    {
        // if the file didn't open, print an error:
        Serial.println("error opening data.txt");
    }
}

void setup() 
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial); // wait for serial port to connect. Needed for native USB port only

    Serial.print("Initializing SD card...");
    if (!SD.begin(10)) 
    {
        Serial.println("initialization 10 failed!");
        while (1);
    }

    writeSD();

    if (!SD.begin(9)) 
    {
        Serial.println("initialization 9 failed!");
        while (1);
    }

    writeSD();

    
}
void loop() 
{
// nothing happens after setup
}
