/*
    Розробив Roman
    Канал на YouTube: https://goo.gl/x8FL2o
    Відео з проектом: 
    Спроба подружити Arduino з MRF49XA. 
    За основу було взято схему піротехнічного пульта на 31 команду з цієї статі: https://goo.gl/GNG4hi
    2018 Roman
*/

#include<SPI.h>

//--------------------------------------------------------------------
// MRF49XA SPI commands:
//--------------------------------------------------------------------
#define    CREG                  0x801F         
#define    FSREG                 0xAE10
#define    TCREG0                0x9120       
#define    TCREG1                0x98F0
#define    AFCREG0               0xC400       
#define    AFCREG1               0xC663    
#define    PMREG0                0x8239         
#define    PMREG1                0x8201
#define    PMREG2                0x82D9           
//--------------------------------------------------------------------
//  FSK/DATA/FSEL:
//--------------------------------------------------------------------
#define    DATA                  9
#define    PERIOD                170
//--------------------------------------------------------------------
//  Default data:
//--------------------------------------------------------------------
byte DataABCDEiD[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};



void setup() {
  digitalWrite(SS, HIGH);
  Serial.begin(9600);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);  
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode (SPI_MODE0);
}

void loop() {
 
}

void SerialPrint(){
    Serial.println("Bytes transfer:");
    Serial.print("A = ");
    Serial.println(DataABCDEiD[0], HEX);
    Serial.print("B = ");
    Serial.println(DataABCDEiD[1], HEX);
    Serial.print("C = ");
    Serial.println(DataABCDEiD[2], HEX);
    Serial.print("D = ");
    Serial.println(DataABCDEiD[3], HEX);
    Serial.print("E = ");
    Serial.println(DataABCDEiD[4], HEX);
    Serial.print("ID = ");
    Serial.println(DataABCDEiD[5], HEX);
  }




void StartReceiver(){
  digitalWrite(SS, LOW);
  SPI.transfer16(PMREG1);
  SPI.transfer16(CREG);
  SPI.transfer16(FSREG);
  SPI.transfer16(TCREG0);
  SPI.transfer16(AFCREG1);
  SPI.transfer16(AFCREG0);
  SPI.transfer16(PMREG2);
  digitalWrite(SS, HIGH);
  }

void TestTransmission(){
  delay(1000);
  for(uint8_t i = 0; i < 2; i++){
  digitalWrite(SS, LOW);
  SPI.transfer16(PMREG1);
  SPI.transfer16(CREG);
  SPI.transfer16(FSREG);
  SPI.transfer16(TCREG1);
  SPI.transfer16(AFCREG0);
  SPI.transfer16(PMREG0);
  digitalWrite(SS, HIGH);
  delay(1000);
  digitalWrite(SS, LOW);
  SPI.transfer16(PMREG1);
  digitalWrite(SS, HIGH);
  delay(1000);
  }
  }
