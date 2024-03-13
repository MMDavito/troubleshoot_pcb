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

const byte customChars[] = {
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11100110, // 9
  B11101110, // a
  B00111110, // b
  B10011100, // c
  B01111010, // d
  B10011110, // e
  B10001110  // f
};
const byte outputs[6]  =
{
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000100,
};

volatile byte outputValues[6] = {
  0b11000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000101,
};

volatile char outputChars[6];

volatile int counter = 0x0f;

void printByte(byte val) {
  for (int i = 7; i >= 0; i--)
  {
    bool b = bitRead(val, i);
    Serial.print(b);
  }
}

byte charToByte(char ch) {
  // Validate input character
  if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
    // Handle decimal or hexadecimal character
    int value;
    if (ch >= '0' && ch <= '9') {
      value = ch - '0';
    } else {
      value = ch - 'A' + 10; // Convert hex char to numerical value (A-F: 10-15)
    }
    return value; // Shift left by 4 bits
    //return value << 4; // Shift left by 4 bits
  } else {
    // Return 0 for invalid characters
    return 0;
  }
}

void setOutputValues(byte value) {
  // Convert to decimal string manually
  String strValue = String(value);
  // Handle single-digit values
  if (strValue.length() == 1)
    strValue = "00" + strValue; // Prepend leading zeros
  else if (strValue.length() == 2)
    strValue = "0" + strValue; // Prepend leading zeros
  strValue.toCharArray(outputChars, sizeof(outputChars)); // Copy to char array
  for (int i = 0; i < 3; i++) {
    outputValues[i] = customChars[charToByte(outputChars[2 - i])];
  }

  // Convert to hexadecimal string manually
  byte nibble;
  for (int i = 0; i < 2; i++) {
    nibble = (value >> (4 * i)) & 0x0F;
    outputChars[4 - i] = (nibble < 10) ? '0' + nibble : 'A' + nibble - 10;
    /*Serial.print("NIBBLE IS: ");
      Serial.println(nibble);*/
    outputValues[4 - i] = customChars[nibble];
    //outputValues[4-i] = customChars[charToByte(outputChars[4 - i])];
  }
  outputChars[5] = char(value);
  outputValues[5] = value;
}


void setup() {
  Serial.begin(2000000);
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

void loop() {
  //Need to write 16bitWord
  if (counter == 255)
    counter = 0;
  else
    counter ++;

  setOutputValues(counter);

  bool isDebug = false;
  if (isDebug) {
    Serial.print("Counter: ");
    printByte(counter);
    Serial.println('\n');

    Serial.println("___________________");
    Serial.print("Counter is: ");
    Serial.println(counter);

    for (int i = 0; i < 6; i++)
    {
      Serial.print("Output: ");
      printByte(outputs[i]);
      Serial.println("");
      Serial.print("Char: ");
      Serial.print(i);
      Serial.print(", is: ");
      Serial.println(outputChars[i]);
    }
    Serial.println("___________________");
  }

  long start = millis();
  //while (millis() - start < 10) {
  while (millis() - start < 500) {
    for (int i = 0; i < 6; i ++) {
      unsigned long combined = 0; // clear it out
      byte temp = 0;

      temp = outputValues[i];


      digitalWrite(RCK_OUTPUT, LOW);
      digitalWrite(RCK_TRANS, LOW);

      SPI.transfer(outputs[i]);
      SPI.transfer(temp);

      digitalWrite(OUTPUT_ENABLE, HIGH);
      digitalWrite(RCK_OUTPUT, HIGH);
      digitalWrite(RCK_TRANS, HIGH);

      digitalWrite(OUTPUT_ENABLE, LOW);

      delayMicroseconds(100);
    }
  }
}
