/**************************************************************
 *
 *
 **************************************************************/
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <TinyGsmClient.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>
 
// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

#define LOOPALARM LOW
#define LOOP0 9
#define LOOP1 10
#define LOOP2 11
#define LOOP3 12
#define LOOP4 13

bool isAlarmCall = false;
bool isArmed = true;
unsigned long alarmCallStart;
unsigned long alarmCallDelay=20000;
uint32_t loopStatus = 0;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xCC};

#define AIO_SERVER      "178.77.238.20"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "datel"
#define AIO_KEY         "hanka12"

EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
unsigned int const sendTimeDelay=30000;
signed long lastSendTime = sendTimeDelay * -1;

Adafruit_MQTT_Publish _loopStatus = Adafruit_MQTT_Publish(&mqtt,  "/home/bedNew/alarm/loopStatus");

TinyGsm modem(SerialAT);

#define SERIALSPEED 115200

void setup() {
  // Set console baud rate
  SerialMon.begin(SERIALSPEED);
  SerialMon.println("Alarm");
  pinMode(LOOP0, INPUT_PULLUP);
  delay(5000);
  modemSetup();

  if (Ethernet.begin(mac) == 0) {
    SerialMon.println("Failed using DHCP");
  }
  SerialMon.print("\nIP:");
  SerialMon.println(Ethernet.localIP());
  SerialMon.print("Mask:");
  SerialMon.println(Ethernet.subnetMask());
  SerialMon.print("Gateway:");
  SerialMon.println(Ethernet.gatewayIP());
  SerialMon.print("DNS:");
  SerialMon.println(Ethernet.dnsServerIP());
  SerialMon.println();
}


void loop() {
  if (millis() - lastSendTime >= sendTimeDelay) {
    MQTT_connect();
    lastSendTime = millis();

    loopStatus = 0;
    loopStatus = digitalRead(LOOP0);
    loopStatus = 1 << digitalRead(LOOP1) | loopStatus;
    loopStatus = 2 << digitalRead(LOOP2) | loopStatus;
    loopStatus = 3 << digitalRead(LOOP3) | loopStatus;
    loopStatus = 4 << digitalRead(LOOP4) | loopStatus;
      
    SerialMon.println(loopStatus);
    
    if (! _loopStatus.publish(loopStatus)) {
      SerialMon.println(F("Failed"));
    } else {
      SerialMon.println(F("OK!"));
    }
  }
  
  if (digitalRead(LOOP0)==LOOPALARM) {
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
        sendCommand("ATA");
//        SerialAT.write("ATA\n");
      } else {
        sendCommand("ATH");
        // SerialAT.write("ATH\n");
      }
    }
  }
  
  if (isAlarmCall) {
    if (millis() - alarmCallStart >= alarmCallDelay) {
        SerialMon.println("Alarm, hang up line...");
        sendCommand("ATH");
        // SerialAT.write("ATH\n");
        isAlarmCall=false;
    }
  }
  
  if (SerialMon.available()) {
    SerialAT.write(SerialMon.read());
  }
  
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  SerialMon.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  SerialMon.println("MQTT Connected!");
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
  
  sendCommand("AT+CMGF=1");
  // SerialAT.write("AT+CMGF=1\n");
  // delay(3000);
  // if (SerialAT.available()) {
    // String incBytes = SerialAT.readStringUntil("\n");
    // SerialMon.println(incBytes);
  // }
  sendCommand("AT+CNMI=0,0");
  // SerialAT.write("AT+CNMI=0,0\n");
  // delay(3000);
  // if (SerialAT.available()) {
    // String incBytes = SerialAT.readStringUntil("\n");
    // SerialMon.println(incBytes);
  // }
  sendCommand("AT+CREG?");
  // SerialAT.write("AT+CREG?\n");
  // delay(3000);
  // if (SerialAT.available()) {
    // String incBytes = SerialAT.readStringUntil("\n");
    // SerialMon.println(incBytes);
  // }
  sendCommand("AT+CSQ");
  // SerialAT.write("AT+CSQ\n");
  // delay(3000);
  // if (SerialAT.available()) {
    // String incBytes = SerialAT.readStringUntil("\n");
    // SerialMon.println(incBytes);
  // }
  sendCommand("AT+COPS?");
  // SerialAT.write("AT+COPS?\n");
  // delay(3000);
  // if (SerialAT.available()) {
    // String incBytes = SerialAT.readStringUntil("\n");
    // SerialMon.println(incBytes);
  // }
  sendCommand("AT+CLIP=1");
  // SerialAT.write("AT+CLIP=1\n");
  // delay(3000);
  // if (SerialAT.available()) {
    // String incBytes = SerialAT.readStringUntil("\n");
    // SerialMon.println(incBytes);
  // }
}

void sendCommand(String command) {
  command += "\n";
  SerialAT.println(command);
  delay(3000);
  if (SerialAT.available()) {
    String incBytes = SerialAT.readStringUntil("\n");
    SerialMon.println(incBytes);
  }
}