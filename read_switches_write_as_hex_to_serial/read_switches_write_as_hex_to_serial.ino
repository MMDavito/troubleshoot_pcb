//Will expand number of bits once I start soldering more ATMEGA328
byte switches [] = { 12, 11, 10, 9, 8, 7 , 6, 5}; //LSB => MSB
byte switchValues = 0b10000000;
byte lastWrittenToSerial = 0b00000000;

void readSwitches() {
  //Call this only if WR_EN is enabled
  byte temp = 0b00000000;
  for (unsigned i = 0; i < 8; i++) {
    bool isHigh = digitalRead(switches[i]) == HIGH;
    if (isHigh) {
      switch (i) {
        case 0:
          temp = temp | 0b00000001;
          break;
        case 1:
          temp = temp | 0b00000010;
          break;
        case 2:
          temp = temp | 0b00000100;
          break;
        case 3:
          temp = temp | 0b00001000;
          break;
        case 4:
          temp = temp | 0b00010000;
          break;
        case 5:
          temp = temp | 0b00100000;
          break;
        case 6:
          temp = temp | 0b01000000;
          break;
        case 7:
          temp = temp | 0b10000000;
          break;
      }
      delay(10);
    }
    /*
        //TEMP TROUBLESHOOTING:

        delay(1000);
        Serial.print("Switch: ");
        Serial.print(i);
        Serial.print(", was: ");
        Serial.println(isHigh);


    */
  }
  switchValues = temp;
}

void writeToSerial() {
  String temp = "";
  //Write from switches:
  if (lastWrittenToSerial == switchValues) return;
  temp = String(switchValues, HEX);
  if (temp.length() == 1)
    temp = '0' + temp;
  temp.toUpperCase();
  lastWrittenToSerial = switchValues;
  temp += "\n";
  Serial.print(temp);
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 8; i++)  {
    pinMode (switches [i], INPUT);
  }
  writeToSerial();
}

void loop() {
  readSwitches();
  writeToSerial();
  delay(200);
}
