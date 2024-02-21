//Shift register tpic595 vs hc595
//https://www.instructables.com/How-Shift-Registers-Work-74HC595/

/*
  //Power/ TPIC6C595:
  //Pin connected to ST_CP of TPIC6C595
  int latchPin = 8;
  //Pin connected to SH_CP of TPIC6C595
  int clockPin = 9;
  ////Pin connected to DS of TPIC6C595
  int dataPin = 10;
*/

#define OUTPUT_ENABLE 2 //4
#define LATCH_DRAIN 8 //14
#define LATCH_TRANS 9 //15
#define LATCH_INPUT 7 // 13

#define CLOCK 6 //12
#define DATA_OUT 4 // 6
#define DATA_IN 5 // 11

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



void customShiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, byte val)
{
  digitalWrite(clockPin, LOW);
  int i;

  for (i = 0; i < 8; i++)  {
    if (bitOrder == LSBFIRST)
      digitalWrite(dataPin, !!(val & (1 << i)));
    else
      digitalWrite(dataPin, !!(val & (1 << (7 - i))));
    digitalWrite(clockPin, HIGH);

    digitalWrite(clockPin, LOW);
    digitalWrite(dataPin, LOW);
    delayMicroseconds(1);
  }
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

void readSerial() {

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
}


void setup() {
  Serial.begin(9600);

  pinMode(OUTPUT_ENABLE, OUTPUT);
  pinMode(LATCH_DRAIN, OUTPUT);
  pinMode(LATCH_TRANS, OUTPUT);
  pinMode(LATCH_INPUT, OUTPUT);
  pinMode(DATA_OUT, OUTPUT);

  digitalWrite(OUTPUT_ENABLE, HIGH);
  digitalWrite(LATCH_DRAIN, HIGH);
  digitalWrite(LATCH_TRANS, HIGH);
  digitalWrite(LATCH_INPUT, HIGH);

  digitalWrite(CLOCK, HIGH);
}

void loop() {
  if (counter == 250)
    counter = 0;
  else
    counter += 1;
  long start = millis();

  while (millis() - start < 250) {
    //ground latchPin and hold low for as long as you are transmitting

    for (int i = 0; i < 6; i ++) {
      digitalWrite(LATCH_DRAIN, LOW);
      customShiftOut(DATA_OUT, CLOCK, LSBFIRST, counter);

      //Disable old output before shifting new display value:
      digitalWrite(OUTPUT_ENABLE, HIGH);
      digitalWrite(LATCH_DRAIN, HIGH);

      digitalWrite(LATCH_TRANS, LOW);
      customShiftOut(DATA_OUT, CLOCK, LSBFIRST, outputs[i]);
      
      //Shift the address and enable the output.
      digitalWrite(LATCH_TRANS, HIGH);
      digitalWrite(OUTPUT_ENABLE, LOW);
      digitalWrite(CLOCK, HIGH);
      digitalWrite(DATA_OUT, HIGH);
      delayMicroseconds(5);
      //delay(5);
    }
  }
}
