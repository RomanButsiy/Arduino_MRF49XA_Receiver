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
#define    BytesNumber           6
#define    PERIOD                170
#define    P_ERROR               30
#define    FP_MIN                PERIOD - P_ERROR
#define    FP_MAX                PERIOD + P_ERROR
#define    SP_MIN                2 * PERIOD - P_ERROR
#define    SP_MAX                2 * PERIOD + P_ERROR
//--------------------------------------------------------------------
//  Default data:
//--------------------------------------------------------------------
byte DataBits[2 * BytesNumber];

void setup()
{
  pinMode(DATA, INPUT);
  digitalWrite(SS, HIGH);
  Serial.begin(9600);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);  
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode (SPI_MODE0);
  StartReceiver();
  SPI.end();
}

void loop() 
{
  if (digitalRead(DATA) == HIGH)
  {
    if (GetSynchronize())
    {
      if (GetData())
      {
        if (CompareData())
        {
          
        }
      }
    }
  }
}

boolean CompareData()
{
  for(uint8_t i = 0; i < BytesNumber; i++)
  {
    if (DataBits[2 * i] != ~DataBits[2 * i + 1])
    {
      return false;
    }
  }
  return true;
}

boolean GetData()
{
  uint8_t DataBit = digitalPinToBitMask(DATA);
  volatile uint8_t *DataPort = portInputRegister(digitalPinToPort(DATA));
  unsigned long FP_MIN_Clock = microsecondsToClockCycles(FP_MIN)/16;
  unsigned long FP_MAX_Clock = microsecondsToClockCycles(FP_MAX)/16;
  unsigned long SP_MIN_Clock = microsecondsToClockCycles(SP_MIN)/16;
  unsigned long SP_MAX_Clock = microsecondsToClockCycles(SP_MAX)/16;
  unsigned long F_width = 0;
  unsigned long S_width = 0;
  byte j = 0x01;
  for(uint8_t i = 0, l = 2 * 8 * BytesNumber; i < l; i++)
  {
    while ((*DataPort & DataBit) != 0);
    while ((*DataPort & DataBit) == 0)
    {
      if (++F_width == SP_MAX_Clock)
      {
        return false;
      }
    }
    while ((*DataPort & DataBit) == DataBit)
    {
      if (++S_width == SP_MAX_Clock)
      {
        return false;
      }
    }
    if (F_width >= FP_MIN_Clock && F_width <= FP_MAX_Clock)
    {
      if (S_width >= SP_MIN_Clock)
      {
        DataBits[i / 8] &= ~j;
      }
      else
      {
        return false;
      }
    }
    else
    {
      if (F_width >= SP_MIN_Clock && S_width >= FP_MIN_Clock && S_width <= FP_MAX_Clock)
      {
        DataBits[i / 8] |= j; 
      }
      else
      {
        return false;
      }
    }
  F_width = 0;
  S_width = 0;  
  if (!(j << 1))
  {
    j = 0x01;
  }
  }
  return true;
}

boolean GetSynchronize()
{
  unsigned long duration;
  for(uint8_t i = 0; i < 12; i++)
  {
    duration = pulseIn(DATA, LOW, SP_MAX);
    if (duration == 0 && digitalRead(DATA) == HIGH)
    {
      duration = pulseIn(DATA, LOW, 2000);
      if (duration == 0)
      {
        return true;
      }
      else 
      {
        break;
      }
    }
    if (duration < FP_MIN || duration > FP_MAX)
    {
      break;
    }
  }
  delayMicroseconds(SP_MAX);
  return false;
}

void SerialPrint()
{
    Serial.println("Bytes transfer:");
    Serial.print("A = ");
    //Serial.println(DataABCDEiD[0], HEX);
    Serial.println(DataBits[0], HEX);
    Serial.print("B = ");
    //Serial.println(DataABCDEiD[1], HEX);
    Serial.println(DataBits[2], HEX);
    Serial.print("C = ");
    //Serial.println(DataABCDEiD[2], HEX);
    Serial.println(DataBits[4], HEX);
    Serial.print("D = ");
    //Serial.println(DataABCDEiD[3], HEX);
    Serial.println(DataBits[6], HEX);
    Serial.print("E = ");
    //Serial.println(DataABCDEiD[4], HEX);
    Serial.println(DataBits[8], HEX);
    Serial.print("ID = ");
    //Serial.println(DataABCDEiD[5], HEX);
    Serial.println(DataBits[10], HEX);
}




void StartReceiver()
{
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

void TestTransmission()
{
  delay(1000);
  for(uint8_t i = 0; i < 2; i++)
  {
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
