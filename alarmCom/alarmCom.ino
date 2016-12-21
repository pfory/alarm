/**************************************************************
 *
 * This script tries to auto-detect the baud rate
 * and allows direct AT commands access
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *
 **************************************************************/

// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

#define LOOP0 9
#define LOOP1 10
#define LOOP2 11
#define LOOP3 12
#define LOOP4 13

bool isAlarmCall = false;
bool isArmed = true;
unsigned long alarmCallStart;
unsigned long alarmCallDelay=20000;

#include <TinyGsmClient.h>
TinyGsm modem(SerialAT);

#define PORT_SPEED 115200

void setup() {
  // Set console baud rate
  SerialMon.begin(PORT_SPEED);
  SerialMon.println("Alarm");
  pinMode(LOOP0, INPUT_PULLUP);
  delay(5000);
  modemSetup();
}


void loop() {
  if (digitalRead(LOOP0)==LOW) {
    if (!isAlarmCall && isArmed) {
      isAlarmCall=true;
      alarmCallStart = millis();
      SerialMon.println("ALARM, call 724 232 639....");
      SerialAT.write("ATD 724232639;\n");
    }
  }
  if (SerialAT.available()) {
    String incBytes = SerialAT.readStringUntil("\n");
    SerialMon.println(incBytes);
    if (incBytes.indexOf("+CLIP:")>0) {
      SerialMon.println("Incoming call..");
      if (incBytes.indexOf("724232639")>0) {
        SerialMon.println("Identify number as known..");
        SerialAT.write("ATA\n");
      } else {
        SerialAT.write("ATH\n");
      }
    }
  }
  
  if (isAlarmCall) {
    if (millis() - alarmCallStart >= alarmCallDelay) {
        SerialMon.println("Alarm, hang up line...");
        SerialAT.write("ATH\n");
        isAlarmCall=false;
    }
  }
  
  
  
  
  if (SerialMon.available()) {
    SerialAT.write(SerialMon.read());
  }
  //delay(100);
  //SerialMon.println(millis());
}

void modemSetup() {
    // Detect module baud rate
  uint32_t rate = 0;
  uint32_t rates[] = { 115200, 9600, 57600, 19200, 74400, 74880 };

  SerialMon.println("Autodetecting baud rate");
  for (unsigned i = 0; i < sizeof(rates)/sizeof(rates[0]); i++) {
    SerialMon.print(String("Trying baud rate ") + rates[i] + "... ");
    SerialAT.begin(rates[i]);
    delay(10);
    if (modem.autoBaud(2000)) {
      rate = rates[i];
      SerialMon.println(F("OK"));
      break;
    } else {
      SerialMon.println(F("fail"));
    }
  }

  if (!rate) {
    SerialMon.println(F("***********************************************************"));
    SerialMon.println(F(" Module does not respond!"));
    SerialMon.println(F("   Check your Serial wiring"));
    SerialMon.println(F("   Check the module is correctly powered and turned on"));
    SerialMon.println(F("***********************************************************"));
    delay(30000L);
    return;
  }

  // Access AT commands from Serial Monitor
  SerialMon.println(F("***********************************************************"));
  SerialMon.println(F(" You can now send AT commands"));
  SerialMon.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
  SerialMon.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
  SerialMon.println(F("***********************************************************"));
  
  SerialAT.write("AT+CMGF=1\n");
  delay(3000);
  if (SerialAT.available()) {
    String incBytes = SerialAT.readStringUntil("\n");
    SerialMon.println(incBytes);
  }
  SerialAT.write("AT+CNMI=0,0\n");
  delay(3000);
  if (SerialAT.available()) {
    String incBytes = SerialAT.readStringUntil("\n");
    SerialMon.println(incBytes);
  }
  SerialAT.write("AT+CREG?\n");
  delay(3000);
  if (SerialAT.available()) {
    String incBytes = SerialAT.readStringUntil("\n");
    SerialMon.println(incBytes);
  }
  SerialAT.write("AT+CSQ\n");
  delay(3000);
  if (SerialAT.available()) {
    String incBytes = SerialAT.readStringUntil("\n");
    SerialMon.println(incBytes);
  }
  SerialAT.write("AT+COPS?\n");
  delay(3000);
  if (SerialAT.available()) {
    String incBytes = SerialAT.readStringUntil("\n");
    SerialMon.println(incBytes);
  }
  SerialAT.write("AT+CLIP=1\n");
  delay(3000);
  if (SerialAT.available()) {
    String incBytes = SerialAT.readStringUntil("\n");
    SerialMon.println(incBytes);
  }
}