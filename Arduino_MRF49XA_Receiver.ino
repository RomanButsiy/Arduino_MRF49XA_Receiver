/*
    Розробив Roman
    Канал на YouTube: https://goo.gl/x8FL2o
    Відео з проектом: https://youtu.be/3V6ITlyMthA
    Спроба подружити Arduino з MRF49XA. 
    За основу було взято схему піротехнічного пульта на 31 команду з цієї статі: https://goo.gl/GNG4hi
    2018 Roman
*/

// gssdp-discover -i wlo1 --timeout=3

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
#define    DATA                  8
#define    BytesNumber           6
#define    PERIOD                170
#define    P_ERROR               30
#define    FP_MIN                (PERIOD - P_ERROR)
#define    FP_MAX                (PERIOD + P_ERROR)
#define    SP_MIN                (2 * PERIOD - P_ERROR)
#define    SP_MAX                (2 * PERIOD + P_ERROR)
//--------------------------------------------------------------------
//  Default data:
//--------------------------------------------------------------------
#define    R_ID                  0xFF
#define    D_BYTE                0
#define    N_BYTE                (~(1 << D_BYTE))

byte DataBits[2 * BytesNumber];

unsigned char OVF_counter = 0, duty, timer_0 = 0, timer_1 = 0;
unsigned long t0, t1, f;
volatile unsigned int falling, rising_0 = 0, rising_1, Si = 0, counter, j = 0;
volatile boolean checkIn = true, Syn = false, flagBytes = false;

ISR(TIMER1_OVF_vect)
{
  OVF_counter++;
}

ISR(TIMER1_CAPT_vect)
{
  if (checkIn)
  {
    falling = ICR1;
    timer_0 = OVF_counter;
    TCCR1B |= (1 << ICES1); // Встановлюємо переривання по наростаючому фронті імпульсу
    OVF_counter = 0;
    checkIn = false;
  }
  else
  {
    timer_1 = OVF_counter;
    rising_1 = ICR1;
    t0 = 4 * ((unsigned long)falling - (unsigned long)rising_0 + ((unsigned long)timer_0 * 65536));
    t1 = 4 * ((unsigned long)rising_1 - (unsigned long)falling + ((unsigned long)timer_1 * 65536));
    if (Syn && !flagBytes)
    {
      if (t0 > FP_MIN && t0 < FP_MAX)
      {
        DataBits[counter / 8] &= ~(1 << j);
      }
      else
      {
        if (t0 > SP_MIN && t0 < SP_MAX)
        {
           DataBits[counter / 8] |= (1 << j);
        }
        else 
        {
         Syn = false;
        }
      }
      j++;
      if( j >= 8)
      {
        j = 0;
      }
      counter++;
      if (counter >= (2 * 8 * BytesNumber))
      {
        flagBytes = true;
        Syn = false;
      }
    }
    else
    {
      Syn = false;
    }
    if (t0 > FP_MIN && t0 < FP_MAX && !Syn)
    {
      Si++;
      if (t1 > 1000 && Si > 3)
      {
        j = 0;
        counter = 0;
        Syn = true;
        Si = 0;
      }
    }
    else
    {
      Si = 0;
    }
    rising_0 = rising_1;
    TCCR1B &= ~(1 << ICES1); // Встановлюємо переривання по спадающему фронту імпульсу
    OVF_counter = 0;
    checkIn = true;
  }
}
  

void setup()
{
  pinMode(DATA, INPUT);
  digitalWrite(SS, HIGH);
  Serial.begin(9600);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);  
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode (SPI_MODE0);
  TestTransmission();
  StartReceiver();
  SPI.end();
  Serial.println("Receiver is staeted!!!");
  Serial.end();
  for(uint8_t i = 0; i < 8; pinMode(i++, OUTPUT));
  PORTD = 0xFF;
  TCCR1A = 0;
  TCCR1B |= (1 << ICNC1) | (1 << CS11);
  TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);
  TCCR1B &= ~(1 << ICES1);
  sei();
}



void loop() 
{
 if(CompareData() && TestData() && flagBytes)
 {
  PORTD = ~DataBits[2 * D_BYTE + 1];
  //SerialPrint();
  flagBytes = false;
 }
 else
 {
  flagBytes = false;
 }
delay(1000);
}

boolean TestData()
{
  if (DataBits[2 * BytesNumber - 1] != R_ID)
  {
    return false;
  }
  for(uint8_t i = 0; i < BytesNumber - 1; i++)
    {
      if ((DataBits[2 * i + 1] * (N_BYTE & (1 << i))))
      {
        return false;
      }
    }
  return true;
}

boolean CompareData()
{
  for(uint8_t i = 0; i < BytesNumber; i++)
  {
    if (ReverseByte(DataBits[2 * i]) != DataBits[2 * i + 1])
    {
      return false;
    }
  }
  return true;
}

byte ReverseByte(byte Byte)
{
  Byte = (Byte & 0x55) << 1 | (Byte & 0xAA) >> 1;
  Byte = (Byte & 0x33) << 2 | (Byte & 0xCC) >> 2;
  Byte = (Byte & 0x0F) << 4 | (Byte & 0xF0) >> 4;
  return ~Byte;
}



void SerialPrint()
{
    Serial.println("Bytes transfer:");
    Serial.print("A = ");
    Serial.println(DataBits[1], HEX);
    Serial.print("B = ");
    Serial.println(DataBits[3], HEX);
    Serial.print("C = ");
    Serial.println(DataBits[5], HEX);
    Serial.print("D = ");
    Serial.println(DataBits[7], HEX);
    Serial.print("E = ");
    Serial.println(DataBits[9], HEX);
    Serial.print("ID = ");
    Serial.println(DataBits[11], HEX);
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
