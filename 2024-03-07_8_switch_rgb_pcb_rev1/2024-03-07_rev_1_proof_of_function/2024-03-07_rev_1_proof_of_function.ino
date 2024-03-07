#include <SPI.h>
/*
  #define MOSI 11
  #define MISO 12 //74hc165
  #define SCK 13
*/

#define BUTTON 3 //5
#define OUTPUT_ENABLE 2 //4
#define RCK_INPUT 7 //13
#define RCK_OUTPUT 8 //14
#define RCK_TRANS 9 //15

const byte outputs[6]  =
{
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000100,
  //0x0f,
};

volatile byte outputValues[6] = {
  0b11000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000101,
};

volatile int counter = 0x0f;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  //SPI.setDataMode(SPI_MODE0); // default - don't need
  //SPI.setDataMode(SPI_MODE2); //https://docs.arduino.cc/learn/communication/spi/

  SPI.setBitOrder(LSBFIRST); // MSBFIRST is default - leave out in that case


  //SPI.setClockDivider(SPI_CLOCK_DIV128); // why so slow?  might as well use shiftout()
  // HC595 and TPIC6B595 work just fine at default speed (4 MHz)

  pinMode(BUTTON, INPUT);
  pinMode(OUTPUT_ENABLE, OUTPUT);
  pinMode(RCK_INPUT, OUTPUT);
  pinMode(RCK_OUTPUT, OUTPUT);
  pinMode(RCK_TRANS, OUTPUT);


  digitalWrite(OUTPUT_ENABLE, HIGH);
  //digitalWrite(RCK_INPUT, HIGH);//RCK_INPUT high IS MORE UNSTABLE.
  //Could test 100k pulldown if high never works, but that is a later issue.

  digitalWrite(RCK_INPUT, LOW);
  digitalWrite(RCK_OUTPUT, HIGH);
  digitalWrite(RCK_TRANS, HIGH);
}


void printByte(byte val) {
  for (int i = 7; i >= 0; i--)
  {
    bool b = bitRead(val, i);
    Serial.print(b);
  }
}

void loop() {
  //Need to write 16bitWord
  if (counter == 255)
    counter = 0;
  else
    counter ++;

  Serial.print("Counter: ");
  printByte(counter);
  Serial.println('\n');


  long start = millis();
  while (millis() - start < 500) {
    //while (millis() - start < 5000) {
    for (int i = 0; i < 6; i ++) {
      unsigned long combined = 0; // clear it out
      byte temp = 0;
      //If switch to MSB, switch order here.
      if (i == 5)
      {
        combined = (counter << 8) | (outputs[i]);
        //combined = (outputs[i] << 8) | (counter);
        temp = counter;
      }
      else
      {

        combined = (outputValues[i] << 8) | (outputs[i]);
        temp = outputValues[i];
      }

      digitalWrite(OUTPUT_ENABLE, HIGH);
      digitalWrite(RCK_OUTPUT, LOW);
      digitalWrite(RCK_TRANS, LOW);
      //delay(50);
      //delayMicroseconds(50);

      //digitalWrite(RCK_OUTPUT, LOW);
      //SPI.transfer(combined);


      SPI.transfer(outputs[i]);
      SPI.transfer(temp);


      //SPI.transfer(0x0F);
      //SPI.transfer(0XF1);
      //SPI.transfer(counter);


      digitalWrite(RCK_OUTPUT, HIGH);
      digitalWrite(RCK_TRANS, HIGH);
      digitalWrite(OUTPUT_ENABLE, LOW);

      delay(1);
    }
  }
}
