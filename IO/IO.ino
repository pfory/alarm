/*
--------------------------------------------------------------------------------------------------------------------------
ALARM - home alarm unit
Petr Fory pfory@seznam.cz
GIT - https://github.com/pfory/Alarm
*/

/*Pinout Arduino Pro Mini
D0 - Rx
D1 - Tx
D2 - LOOP0
D3 - LOOP1
D4 - LOOP2
D5 - LOOP3
D6 - LOOP4
D7 - LOOP5
D8 - LOOP6
D9 - LOOP7
D10 - LOOP8
D11 - LOOP9
D12 - LOOP10
D13 - LOOP11
A0 - LOOP12
A1 - LOOP13
A2 - LOOP14
A3 - LOOP15
+5V
GND
*/

//smycky
#define LOOP0                   2
#define LOOP1                   3
#define LOOP2                   4
#define LOOP3                   5
#define LOOP4                   6
#define LOOP5                   7
#define LOOP6                   8
#define LOOP7                   9
#define LOOP8                   10
#define LOOP9                   11
#define LOOP10                  12
#define LOOP11                  13
#define LOOP12                  A0
#define LOOP13                  A1
#define LOOP14                  A2
#define LOOP15                  A3
#define LOOP16                  A4
#define LOOP17                  A5
#define LOOP18                  A6
#define LOOP19                  A7

#define SEND_DELAY              10000  //prodleva mezi poslanim dat v ms
#define PORTSPEED 9600


unsigned long lastSend          = 0;  //ms posledniho poslani dat


uint16_t status                 = 0;


void setup(void) {
  Serial.begin(PORTSPEED);
  pinMode(LOOP0, INPUT_PULLUP);
  pinMode(LOOP1, INPUT_PULLUP);
  pinMode(LOOP2, INPUT_PULLUP);
  pinMode(LOOP3, INPUT_PULLUP);
  pinMode(LOOP4, INPUT_PULLUP);
  pinMode(LOOP5, INPUT_PULLUP);
  pinMode(LOOP6, INPUT_PULLUP);
  pinMode(LOOP7, INPUT_PULLUP);
  pinMode(LOOP8, INPUT_PULLUP);
  pinMode(LOOP9, INPUT_PULLUP);
  pinMode(LOOP10, INPUT_PULLUP);
  pinMode(LOOP11, INPUT_PULLUP);
  pinMode(LOOP12, INPUT_PULLUP);
  pinMode(LOOP13, INPUT_PULLUP);
  pinMode(LOOP14, INPUT_PULLUP);
  pinMode(LOOP15, INPUT_PULLUP);
  pinMode(LOOP16, INPUT_PULLUP);
  pinMode(LOOP17, INPUT_PULLUP);
  pinMode(LOOP18, INPUT_PULLUP);
  pinMode(LOOP19, INPUT_PULLUP);
}

void loop(void) {
  if (digitalRead(LOOP0) != bitRead(status,0)) {
    bitWrite(status,0,digitalRead(LOOP0));
  }
  if (digitalRead(LOOP1) != bitRead(status,1)) {
    bitWrite(status,1,digitalRead(LOOP1));
  }
  if (digitalRead(LOOP2) != bitRead(status,2)) {
    bitWrite(status,2,digitalRead(LOOP2));
  }
  if (digitalRead(LOOP3) != bitRead(status,3)) {
    bitWrite(status,3,digitalRead(LOOP3));
  }
  if (digitalRead(LOOP4) != bitRead(status,4)) {
    bitWrite(status,4,digitalRead(LOOP4));
  }
  if (digitalRead(LOOP5) != bitRead(status,5)) {
    bitWrite(status,5,digitalRead(LOOP5));
  }
  if (digitalRead(LOOP6) != bitRead(status,6)) {
    bitWrite(status,6,digitalRead(LOOP6));
  }
  if (digitalRead(LOOP7) != bitRead(status,7)) {
    bitWrite(status,7,digitalRead(LOOP7));
  }
  if (digitalRead(LOOP8) != bitRead(status,8)) {
    bitWrite(status,8,digitalRead(LOOP8));
  }
  if (digitalRead(LOOP9) != bitRead(status,9)) {
    bitWrite(status,9,digitalRead(LOOP9));
  }
  if (digitalRead(LOOP10) != bitRead(status,10)) {
    bitWrite(status,10,digitalRead(LOOP10));
  }
  if (digitalRead(LOOP11) != bitRead(status,11)) {
    bitWrite(status,11,digitalRead(LOOP11));
  }
  if (digitalRead(LOOP12) != bitRead(status,12)) {
    bitWrite(status,12,digitalRead(LOOP12));
  }
  if (digitalRead(LOOP13) != bitRead(status,13)) {
    bitWrite(status,13,digitalRead(LOOP13));
  }
  if (digitalRead(LOOP14) != bitRead(status,14)) {
    bitWrite(status,14,digitalRead(LOOP14));
  }
  if (digitalRead(LOOP15) != bitRead(status,15)) {
    bitWrite(status,15,digitalRead(LOOP15));
  }
  if (digitalRead(LOOP16) != bitRead(status,16)) {
    bitWrite(status,16,digitalRead(LOOP16));
  }
  if (digitalRead(LOOP17) != bitRead(status,17)) {
    bitWrite(status,17,digitalRead(LOOP17));
  }
  if (digitalRead(LOOP18) != bitRead(status,18)) {
    bitWrite(status,18,digitalRead(LOOP18));
  }
  if (digitalRead(LOOP19) != bitRead(status,19)) {
    bitWrite(status,19,digitalRead(LOOP19));
  }
  
  if (millis() - lastSend >= SEND_DELAY) {
    Serial.print(status);
    Serial.print("*");
    Serial.flush();
    lastSend = millis();
  }

}
