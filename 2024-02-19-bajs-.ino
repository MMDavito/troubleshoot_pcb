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

//STANDARD 74HC595:
byte dataToTransfer = 0b100000001;//== Qa on MSBFIRST, Qh on LSBFIRST
byte oldData = dataToTransfer;
//byte dataToTransfer = 0b11111111;//== Qa on MSBFIRST, Qh on LSBFIRST
//byte dataToTransfer = 0xFF;//== Qa on MSBFIRST, Qh on LSBFIRST

//POWER HC/TPIC6C595:

//byte dataToTransfer = 0b11111111;
//byte dataToTransfer = 0b00000001;//== drain0 on MSBFIRST, drain7 on LSBFIRST

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
  }


  char charArray[3]; // Allocate space for 3 characters including null terminator

  // Copy the string to the character array (including null terminator)
  temp.toCharArray(charArray, sizeof(charArray));

  Serial.print("C magic: (god bless gemini and google): ");
  byte hexRep = hexCharToByte(charArray[0], charArray[1]);
  Serial.println(hexRep);
  dataToTransfer = hexRep;
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
}

void loop() {
  //ground latchPin and hold low for as long as you are transmitting
  if (dataToTransfer != oldData)
  {
    Serial.print("Old data:");
    Serial.println(oldData);

    Serial.print("New data:");
    Serial.println(dataToTransfer);
    oldData = dataToTransfer;
  }
  digitalWrite(LATCH_DRAIN, LOW);
  /*
  digitalWrite(DATA_OUT, HIGH);
  digitalWrite(CLOCK, HIGH);
  */
  //delay(10);
  shiftOut(0xFF, CLOCK, LSBFIRST, dataToTransfer);
  //digitalWrite(LATCH_DRAIN, HIGH);
  /*
  digitalWrite(DATA_OUT, HIGH);
  digitalWrite(CLOCK, HIGH);
  */
  //delay(10);
  //digitalWrite(LATCH_DRAIN, LOW);
    
  shiftOut(DATA_OUT, CLOCK, LSBFIRST, dataToTransfer);

  //return the latch pin high to signal chip that it
  //no longer needs to listen for information
  
  //delay(10);
  digitalWrite(LATCH_DRAIN, HIGH);
  
  if (Serial.available()) {
    Serial.println("Will read serial");
    readSerial();
  }
  delay(5);
}
