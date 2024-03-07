#include <SPI.h>

/*If I put this to two, maybe I could actually scrap update of the OUTPUT_TRANS.
  But would probably need to hotwire serial...

  In same moment I realize, no I would have needed 1 shift reg per output?
  Afternoon 2024-02-22 David started becomeing delusional of overwork....
*/
#define OUTPUT_ENABLE 2 //4
#define DATA_OUT 4 // 6

#define CLOCK 6 //12
#define LATCH_DRAIN 8 //14

#define LATCH_TRANS 9 //15






volatile byte value = 0xF1;
volatile long lastDebounceTime = 0;
volatile byte counter = 0;
const byte outputs[6]  =
{
  0b10000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000100,
};
volatile char outputChars[6];
volatile byte outputValues[6] = {
  0b11000000,
  0b01000000,
  0b00100000,
  0b00010000,
  0b00001000,
  0b00000101,
};





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




void customShiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, byte val)
{
  digitalWrite(clockPin, LOW);
  delayMicroseconds(1);
  //delay(500);
  int i;

  for (i = 0; i < 8; i++)  {
    if (bitOrder == LSBFIRST)
      digitalWrite(dataPin, !!(val & (1 << i)));
    else
      digitalWrite(dataPin, !!(val & (1 << (7 - i))));
    //delayMicroseconds(20);
    digitalWrite(clockPin, HIGH);
    //delayMicroseconds(50);
    digitalWrite(clockPin, LOW);
    //digitalWrite(dataPin, LOW);
    delayMicroseconds(1);
    //delay(500);
  }
  //digitalWrite(clockPin, HIGH);
}

byte hexCharToByte(char hexChar1, char hexChar2) {
  // Check for valid characters
  if (!isHexDigit(hexChar1) || !isHexDigit(hexChar2)) {
    return 0; // Or throw an error if desired
  }

  // Convert hex characters to integer values (0-15)
  int nibble1 = nibbleToHex(hexChar1);
  int nibble2 = nibbleToHex(hexChar2);

  // Combine nibbles into a byte (shift left by 4 for nibble1)
  return (nibble1 << 4) | nibble2;
}

// Helper function to convert a hex character to its nibble value (0-15)
int nibbleToHex(char hexChar) {
  if (hexChar >= '0' && hexChar <= '9') {
    return hexChar - '0';
  } else if (hexChar >= 'A' && hexChar <= 'F') {
    return hexChar - 'A' + 10;
  } else if (hexChar >= 'a' && hexChar <= 'f') {
    return hexChar - 'a' + 10;
  } else {
    return 0; // Or throw an error if desired
  }
}

// Helper function to check if a character is a valid hex digit
bool isHexDigit(char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

byte readSerial() {

  byte b = 0;
  String temp = "";

  while (Serial.available()) {
    long startByte = micros();
    b = Serial.read();
    //if (b  - '0' == 218) break;
    if (b == '\0') break;
    temp += char(b);
    Serial.print("String: ");
    Serial.println(temp);
    delay(1);
  }


  char charArray[3]; // Allocate space for 3 characters including null terminator

  // Copy the string to the character array (including null terminator)
  temp.toCharArray(charArray, sizeof(charArray));

  Serial.print("C magic: (god bless gemini and google): ");
  byte hexRep = hexCharToByte(charArray[0], charArray[1]);
  Serial.println(hexRep);
  return hexRep;
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
    outputValues[i] = customChars[charToByte(outputChars[i])];
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
  Serial.begin(9600);

  pinMode(OUTPUT_ENABLE, OUTPUT);
  pinMode(LATCH_DRAIN, OUTPUT);
  pinMode(LATCH_TRANS, OUTPUT);

  digitalWrite(OUTPUT_ENABLE, LOW);
  digitalWrite(LATCH_DRAIN, HIGH);
  digitalWrite(LATCH_TRANS, HIGH);

  digitalWrite(CLOCK, HIGH);
}

void printByte(byte val) {
  for (int i = 7; i >= 0; i--)
  {
    bool b = bitRead(val, i);
    Serial.print(b);
  }
}
void loop() {
  long start = millis();
  if (counter == 255)
    counter = 0;
  else
    counter ++;
  while (millis() - start < 500) {
    for (int i = 0; i < 6; i ++) {
      digitalWrite(LATCH_DRAIN, LOW);
      //customShiftOut(DATA_OUT, CLOCK, LSBFIRST, outputValues[i]);
      if (i == 5)
        //customShiftOut(DATA_OUT, CLOCK, LSBFIRST, counter);
      shiftOut(DATA_OUT, CLOCK, LSBFIRST, counter);
      else
        shiftOut(DATA_OUT, CLOCK, LSBFIRST, outputValues[i]);
      digitalWrite(LATCH_DRAIN, HIGH);
      
      //digitalWrite(CLOCK, HIGH);
      //digitalWrite(DATA_OUT, HIGH);
      //delay(1);

      digitalWrite(LATCH_TRANS, LOW);

      shiftOut(DATA_OUT, CLOCK, LSBFIRST, outputs[i]);//Much more stable than alternative!
      //Shift the address and enable the output.
      digitalWrite(LATCH_TRANS, HIGH);
      digitalWrite(OUTPUT_ENABLE, LOW);
      //delayMicroseconds(10);
      //digitalWrite(CLOCK, HIGH);
      //digitalWrite(DATA_OUT, HIGH);

      delayMicroseconds(500);
      digitalWrite(OUTPUT_ENABLE, HIGH);
      //delayMicroseconds(500);
      //delay(1);
      //}
    }
  }
}
