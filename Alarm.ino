#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <FS.h>          //this needs to be first
#include <Ticker.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include "DoubleResetDetector.h" // https://github.com/datacute/DoubleResetDetector
#include "Sender.h"


// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590

// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMon Serial

#define ota
#ifdef ota
#include <ArduinoOTA.h>
#define HOSTNAMEOTA   "alarm"
#endif

#define serverHTTP
#ifdef serverHTTP
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#endif

#define time
#ifdef time
#include <TimeLib.h>
#include <Timezone.h>
WiFiUDP EthernetUdp;
static const char     ntpServerName[]       = "tik.cesnet.cz";
//const int timeZone = 2;     // Central European Time
//Central European Time (Frankfurt, Paris)
TimeChangeRule        CEST                  = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule        CET                   = {"CET", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);
unsigned int          localPort             = 8888;  // local port to listen for UDP packets
time_t getNtpTime();
#endif

SenderClass sender;


#define verbose
#ifdef verbose
  #define DEBUG_PRINT(x)         Serial.print (x)
  #define DEBUG_PRINTDEC(x)      Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x)       Serial.println (x)
  #define DEBUG_PRINTF(x, y)     Serial.printf (x, y)
  #define PORTSPEED 115200
  #define SERIAL_BEGIN           Serial.begin(PORTSPEED) 
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, y)
#endif 

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(D6, D5); // RX, TX


//#define DUMP_AT_COMMANDS
#define TINY_GSM_DEBUG SerialMon

// Set phone numbers, if you want to test SMS and Calls
#define SMS_TARGET  "+420724232639"
#define CALL_TARGET "+420724232639"

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "internet.t-mobile.cz";
const char user[] = "";
const char pass[] = "";

int buffer;

const unsigned long   sendDelay             = 5000; //in ms
const unsigned long   sendStatDelay         = 60000;
const unsigned long   readSMSDelay          = 10000;


float versionSW                   = 0.1;
String versionSWString            = "Alarm v";
uint32_t heartBeat                = 0;

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

struct SMSmessage {
  String number;
  String message;
};

struct SMSList {
  int* ids;
  int count;
};

char                  mqtt_server[40]       = "192.168.1.56";
uint16_t              mqtt_port             = 1883;
char                  mqtt_username[40]     = "datel";
char                  mqtt_key[20]          = "hanka12";
char                  mqtt_base[60]         = "/home/Alarm";
char                  static_ip[16]         = "192.168.1.111";
char                  static_gw[16]         = "192.168.1.1";
char                  static_sn[16]         = "255.255.255.0";

char                  alarmPhone1[15]       = "+420724232639";
char                  alarmPhone2[15]       = "630210602";
char                  alarmPhone3[15]       = "";

char                  alarmSMS1[15]         = "+420724232639";
char                  alarmSMS2[15]         = "";
char                  alarmSMS3[15]         = "";

char                  alarmSMSText[100]     = "Alarm chata";

char                  alarmIncPhone1[15]    = "+420724232639";
char                  alarmIncPhone2[15]    = "";
char                  alarmIncPhone3[15]    = "";

char                  acCode1[6]            = "2245";
char                  acCode2[6]            = "";
char                  acCode3[6]            = "";
char                  acCode4[6]            = "";
char                  acCode5[6]            = "";
char                  acCode6[6]            = "";

byte                  arrLOOPStatus[10];
byte                  arrLOOPArmed[10];

byte                  alarmArmed            = false;

#define CFGFILE "/config.json"

ADC_MODE(ADC_VCC);

bool isDebugEnabled() {
#ifdef verbose
  return true;
#endif // verbose
  return false;
}

//for LED status
#include <Ticker.h>
Ticker ticker;

#include <timer.h>
auto timer = timer_create_default(); // create a timer with default settings
Timer<> default_timer; // save as above

#ifdef serverHTTP
void handleRoot() {
	char temp[600];
  // DEBUG_PRINT(year());
  // DEBUG_PRINT(month());
  // DEBUG_PRINT(day());
  // DEBUG_PRINT(hour());
  // DEBUG_PRINT(minute());
  // DEBUG_PRINT(second());
  printSystemTime();
  DEBUG_PRINTLN(" Client request");
  digitalWrite(BUILTIN_LED, LOW);
  
	snprintf ( temp, 400,
      "<html>\
        <head>\
          <meta charset='UTF-8'>\
        </head>\
        <body>\
        </body>\
      </html>"
      // year(), month(), day(), hour(), minute(), second(),
      // temperature<0 && temperature>-1 ? "-":"",
      // (int)temperature, 
      // abs((temperature - (int)temperature) * 100),
      // year(), month(), day(), hour(), minute(), second(),
      // (int)humidity,
      // year(), month(), day(), hour(), minute(), second(),
      // (int)pressure,
      // year(), month(), day(), hour(), minute(), second(),
      // dewPoint<0 && dewPoint>-1 ? "-":"",
      // (int)dewPoint, 
      // abs((dewPoint - (int)dewPoint) * 100)
	);
	server.send ( 200, "text/html", temp );
  digitalWrite(BUILTIN_LED, HIGH);
}
#endif

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}
  
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  DEBUG_PRINTLN("Should save config");
  shouldSaveConfig = true;
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  DEBUG_PRINTLN("Entered config mode");
  DEBUG_PRINTLN(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  DEBUG_PRINTLN(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}


/*-------------------------------------------------------------S E T U P----------------------------------------------------------*/
void setup() {
  // Set console baud rate
  SERIAL_BEGIN;
  DEBUG_PRINT(versionSWString);
  DEBUG_PRINTLN(versionSW);

  pinMode(BUILTIN_LED, OUTPUT);
  ticker.attach(1, tick);

  setupWifi();
  setupModem();
  
#ifdef serverHTTP
  server.on ( "/", handleRoot );
  server.begin();
  DEBUG_PRINTLN ( "HTTP server started!!" );
#endif

#ifdef time
  DEBUG_PRINTLN("Setup TIME");
  EthernetUdp.begin(localPort);
  DEBUG_PRINT("Local port: ");
  DEBUG_PRINTLN(EthernetUdp.localPort());
  DEBUG_PRINTLN("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  
  printSystemTime();
#endif

  setupOTA();
  
  //setup timers
  timer.every(readSMSDelay, readSMS);
  timer.every(sendDelay, sendDataHA);
  timer.every(sendStatDelay, sendStatisticHA);

  DEBUG_PRINTLN(" Ready");
 
  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, HIGH);

  prefSettings();
}


void setupOTA() {
#ifdef ota
  //OTA
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(HOSTNAMEOTA);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    // String type;
    // if (ArduinoOTA.getCommand() == U_FLASH)
      // type = "sketch";
    // else // U_SPIFFS
      // type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    //DEBUG_PRINTLN("Start updating " + type);
    DEBUG_PRINTLN("Start updating ");
  });
  ArduinoOTA.onEnd([]() {
   DEBUG_PRINTLN("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINTF("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PRINTF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
    else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
  });
  ArduinoOTA.begin();
#endif
}

void setupWifi() {
  DEBUG_PRINTLN("-------WIFI Setup---------");
  WiFi.printDiag(Serial);
    
  bool validConf = readConfig();
  if (!validConf) {
    DEBUG_PRINTLN(F("ERROR config corrupted"));
  }
  
  rst_info *_reset_info = ESP.getResetInfoPtr();
  uint8_t _reset_reason = _reset_info->reason;
  DEBUG_PRINT("Boot-Mode: ");
  DEBUG_PRINTLN(_reset_reason);
  heartBeat = _reset_reason;

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();
  
  //wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  IPAddress _ip,_gw,_sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);

  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  
  DEBUG_PRINTLN(_ip);
  DEBUG_PRINTLN(_gw);
  DEBUG_PRINTLN(_sn);

  //if (WiFi.SSID()!="") wifiManager.setConfigPortalTimeout(60); //If no access point name has been previously entered disable timeout.

  
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  
    //DEBUG_PRINTLN("Double reset detected. Config mode.");

  WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT server port", String(mqtt_port).c_str(), 5);
  WiFiManagerParameter custom_mqtt_uname("mqtt_uname", "MQTT username", mqtt_username, 40);
  WiFiManagerParameter custom_mqtt_key("mqtt_key", "MQTT password", mqtt_key, 20);
  WiFiManagerParameter custom_mqtt_base("mqtt_base", "MQTT topic end without /", mqtt_base, 60);

  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_uname);
  wifiManager.addParameter(&custom_mqtt_key);
  wifiManager.addParameter(&custom_mqtt_base);

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  
  wifiManager.setTimeout(30);
  wifiManager.setConnectTimeout(10); 
  wifiManager.setConfigPortalTimeout(60);
  //wifiManager.setBreakAfterConfig(true);
  
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Alarm", "password")) { 
    DEBUG_PRINTLN("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  } 
  
  validateInput(custom_mqtt_server.getValue(), mqtt_server);
  mqtt_port = String(custom_mqtt_port.getValue()).toInt();
  validateInput(custom_mqtt_uname.getValue(), mqtt_username);
  validateInput(custom_mqtt_key.getValue(), mqtt_key);
  validateInput(custom_mqtt_base.getValue(), mqtt_base);
  
  if (shouldSaveConfig) {
    saveConfig();
  }
  
  //if you get here you have connected to the WiFi
  DEBUG_PRINTLN("CONNECTED");
  DEBUG_PRINT("Local ip : ");
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINTLN(WiFi.subnetMask());
}

void setupModem() {
  DEBUG_PRINTLN("-------Modem Setup---------");

  // Set GSM module baud rate
  //TinyGsmAutoBaud(SerialAT);
  SerialAT.begin(19200);
  
    // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBG("Initializing modem...");
  if (!modem.init()) {
    delay(10000);
    return;
  }

  String modemInfo = modem.getModemInfo();
  DBG("Modem:", modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  DBG("Waiting for network...");
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected()) {
    DBG("Network connected");
  }

  // DBG("Connecting to", apn);
  // if (!modem.gprsConnect(apn, user, pass)) {
    // delay(10000);
    // return;
  // }

  // bool res = modem.isGprsConnected();
  // DBG("GPRS status:", res ? "connected" : "not connected");

  String ccid = modem.getSimCCID();
  DBG("CCID:", ccid);

  String imei = modem.getIMEI();
  DBG("IMEI:", imei);

  String cop = modem.getOperator();
  DBG("Operator:", cop);
  
  IPAddress local = modem.localIP();
  DBG("Local IP:", local);

  int csq = modem.getSignalQuality();
  DBG("Signal quality:", csq);

  // This is NOT supported on M590
  int battLevel = modem.getBattPercent();
  DBG("Battery lavel:", battLevel);

  // This is only supported on SIMxxx series
  float battVoltage = modem.getBattVoltage() / 1000.0F;
  DBG("Battery voltage:", battVoltage);

  // This is only supported on SIMxxx series
  String gsmLoc = modem.getGsmLocation();
  DBG("GSM location:", gsmLoc);

  // This is only supported on SIMxxx series
  String gsmTime = modem.getGSMDateTime(DATE_TIME);
  DBG("GSM Time:", gsmTime);
  String gsmDate = modem.getGSMDateTime(DATE_DATE);
  DBG("GSM Date:", gsmDate);

  // String ussd_balance = modem.sendUSSD("*111#");
  // DBG("Balance (USSD):", ussd_balance);

  // String ussd_phone_num = modem.sendUSSD("*161#");
  // DBG("Phone number (USSD):", ussd_phone_num);
}


/*-------------------------------------------------------------L O O P----------------------------------------------------------*/
void loop() {
	if(SerialMon.available()>0) {
		buffer = SerialMon.read();
    if (buffer==97) {
      callPhone();
    }
    if (buffer==104) {
      hangUpPhone();
    }
	}

  modem.maintain();
  
  timer.tick(); // tick the timer
#ifdef serverHTTP
  server.handleClient();
#endif

#ifdef ota
  ArduinoOTA.handle();
#endif
  
  // printSystemTime();
  // Serial.println();
  // delay(1000);
}

void sendSMS() {
    // res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
  // DBG("SMS:", res ? "OK" : "fail");

  // This is only supported on SIMxxx series
  bool res = modem.sendSMS_UTF16(SMS_TARGET, u"Привіііт!", 9);
  DBG("UTF16 SMS:", res ? "OK" : "fail");
}

void hangUpPhone() {
  DBG("Hanging up:", CALL_TARGET);
  bool res = modem.callHangup();
  DBG("Hang up:", res ? "OK" : "fail");
}

void callPhone() {
  DBG("Calling:", CALL_TARGET);

  // This is NOT supported on M590
  bool res = modem.callNumber(CALL_TARGET);
  DBG("Call:", res ? "OK" : "fail");

  if (res) {
    delay(1000L);

    // Play DTMF A, duration 1000ms
    modem.dtmfSend('A', 1000);

    // Play DTMF 0..4, default duration (100ms)
    for (char tone='0'; tone<='4'; tone++) {
      modem.dtmfSend(tone);
    }
  }
}


bool readConfig() {
  DEBUG_PRINT(F("Mounting FS..."));

  if (SPIFFS.begin()) {
    DEBUG_PRINTLN(F(" mounted!"));
    if (SPIFFS.exists(CFGFILE)) {
      // file exists, reading and loading
      DEBUG_PRINTLN(F("Reading config file"));
      File configFile = SPIFFS.open(CFGFILE, "r");
      if (configFile) {
        DEBUG_PRINTLN(F("Opened config file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());

        if (json.success()) {
          DEBUG_PRINTLN(F("Parsed json"));
          
          if (json.containsKey("MQTT_server")) {
            strcpy(mqtt_server, json["MQTT_server"]);
            DEBUG_PRINT(F("MQTT server: "));
            DEBUG_PRINTLN(mqtt_server);
          }
          if (json.containsKey("MQTT_port")) {
            mqtt_port = json["MQTT_port"];
            DEBUG_PRINT(F("MQTT port: "));
            DEBUG_PRINTLN(mqtt_port);
          }
          if (json.containsKey("MQTT_uname")) {
            strcpy(mqtt_username, json["MQTT_uname"]);
            DEBUG_PRINT(F("MQTT username: "));
            DEBUG_PRINTLN(mqtt_username);
          }
          if (json.containsKey("MQTT_pwd")) {
            strcpy(mqtt_key, json["MQTT_pwd"]);
            DEBUG_PRINT(F("MQTT password: "));
            DEBUG_PRINTLN(mqtt_key);
          }
          if (json.containsKey("MQTT_base")) {
            strcpy(mqtt_base, json["MQTT_base"]);
            DEBUG_PRINT(F("MQTT base: "));
            DEBUG_PRINTLN(mqtt_base);
          }
          // if (json.containsKey("SSID")) {
            // my_ssid = (const char *)json["SSID"];
          // }
          // if (json.containsKey("PSK")) {
            // my_psk = (const char *)json["PSK"];
          // }
          
          if(json["ip"]) {
            DEBUG_PRINTLN("setting custom ip from config");
            strcpy(static_ip, json["ip"]);
            strcpy(static_gw, json["gateway"]);
            strcpy(static_sn, json["subnet"]);
            DEBUG_PRINTLN(static_ip);
          } else {
            DEBUG_PRINTLN("no custom ip in config");
          }

          
          DEBUG_PRINTLN(F("Parsed config:"));
          if (isDebugEnabled) {
            json.printTo(Serial);
            DEBUG_PRINTLN();
          }
          return true;
        }
        else {
          DEBUG_PRINTLN(F("ERROR: failed to load json config"));
          return false;
        }
      }
      DEBUG_PRINTLN(F("ERROR: unable to open config file"));
    } else {
      DEBUG_PRINTLN(F("ERROR: config file not exist"));
    }
  } else {
    DEBUG_PRINTLN(F(" ERROR: failed to mount FS!"));
  }
  return false;
}

bool saveConfig() {
  DEBUG_PRINTLN(F("Saving config..."));

  // if SPIFFS is not usable
  if (!SPIFFS.begin() || !SPIFFS.exists(CFGFILE) ||
      !SPIFFS.open(CFGFILE, "w"))
  {
    DEBUG_PRINTLN(F("Need to format SPIFFS: "));
    SPIFFS.end();
    SPIFFS.begin();
    DEBUG_PRINTLN(SPIFFS.format());
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();

  json["MQTT_server"] = mqtt_server;
  json["MQTT_port"]   = mqtt_port;
  json["MQTT_uname"]  = mqtt_username;
  json["MQTT_pwd"]    = mqtt_key;
  json["MQTT_base"]   = mqtt_base;
  
  json["ip"] = WiFi.localIP().toString();
  json["gateway"] = WiFi.gatewayIP().toString();
  json["subnet"] = WiFi.subnetMask().toString();

  // Store current Wifi credentials
  // json["SSID"]        = WiFi.SSID();
  // json["PSK"]         = WiFi.psk();

  File configFile = SPIFFS.open(CFGFILE, "w+");
  if (!configFile) {
    DEBUG_PRINTLN(F("Failed to open config file for writing"));
    SPIFFS.end();
    return false;
  } else {
    if (isDebugEnabled) {
      json.printTo(Serial);
    }
    json.printTo(configFile);
    configFile.close();
    SPIFFS.end();
    DEBUG_PRINTLN(F("\nSaved successfully"));
    return true;
  }
}

void validateInput(const char *input, char *output)
{
  String tmp = input;
  tmp.trim();
  tmp.replace(' ', '_');
  tmp.toCharArray(output, tmp.length() + 1);
}

#ifdef time
/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  //IPAddress ntpServerIP; // NTP server's ip address
  IPAddress ntpServerIP = IPAddress(195, 113, 144, 201);

  while (EthernetUdp.parsePacket() > 0) ; // discard any previously received packets
  DEBUG_PRINTLN("Transmit NTP Request");
  // get a random server from the pool
  //WiFi.hostByName(ntpServerName, ntpServerIP);
  DEBUG_PRINT(ntpServerName);
  DEBUG_PRINT(": ");
  DEBUG_PRINTLN(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = EthernetUdp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      DEBUG_PRINTLN("Receive NTP Response");
      EthernetUdp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      DEBUG_PRINT("Seconds since Jan 1 1900 = " );
      DEBUG_PRINTLN(secsSince1900);

      // now convert NTP time into everyday time:
      DEBUG_PRINT("Unix time = ");
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;
      // print Unix time:
      DEBUG_PRINTLN(epoch);
	  
      TimeChangeRule *tcr;
      time_t utc;
      utc = epoch;
      
      return CE.toLocal(utc, &tcr);
      //return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  DEBUG_PRINTLN("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  EthernetUdp.beginPacket(address, 123); //NTP requests are to port 123
  EthernetUdp.write(packetBuffer, NTP_PACKET_SIZE);
  EthernetUdp.endPacket();
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding
  // colon and leading 0
  DEBUG_PRINT(":");
  if(digits < 10)
    DEBUG_PRINT('0');
  DEBUG_PRINT(digits);
}

#endif

void printSystemTime(){
#ifdef time
  DEBUG_PRINT(day());
  DEBUG_PRINT(".");
  DEBUG_PRINT(month());
  DEBUG_PRINT(".");
  DEBUG_PRINT(year());
  DEBUG_PRINT(" ");
  DEBUG_PRINT(hour());
  printDigits(minute());
  printDigits(second());
#endif
}

bool readSMS(void *) {
  readSMSPos(1);
  return true;
}

SMSmessage readSMSPos(uint8_t i) {
  DEBUG_PRINTLN("Reading SMS");
  char message[300];
  char number[50];
  char date[50];
  char type[10];

  String head;
  String body;

  SMSmessage sms = (const struct SMSmessage) {
    "", ""
  };
  modem.sendAT(GF("+CMGR="), i);

  if (modem.waitResponse(10000L, GF(GSM_NL "+CMGR:"))) {
    head = modem.stream.readStringUntil('\n');
    body = modem.stream.readStringUntil('\n');
    DEBUG_PRINTLN(head);
    DEBUG_PRINTLN(body);

    int counter = 0;
    int var = 0;

    for(int i=0; i< head.length(); i++) {
      // There is a comma after comma or Quotes sign after comma
      if(head.charAt(i) == ',') {
            // Skip , and " sign. I know what I'm doing!
        i++; i++;
        while(head.charAt(i) != ',') {
          number[counter] = head.charAt(i);
          // I know what I'm doing!
          counter++;
          i++;
        }
        number[counter-1] = '\0';
      }
    }

    sms.number  = String(number);
    DEBUG_PRINTLN(sms.number);
    body.toCharArray(message, body.length());
    sms.message = message;
    DEBUG_PRINTLN(sms.message);
  }
  return sms;
}

void prefSettings() {
  modem.sendAT(GF("+CPMS=ME,ME,ME"));
  modem.waitResponse();

  modem.sendAT(GF("+CMGF=1")); //text mode
  modem.waitResponse();

  modem.sendAT(GF("+CNMI=1,0"));
  modem.waitResponse();

  modem.sendAT(GF("+CSCS=\"GSM\""));
  modem.waitResponse();
}

bool sendStatisticHA(void *) {
  digitalWrite(BUILTIN_LED, LOW);
  printSystemTime();
  DEBUG_PRINTLN(F(" - I am sending statistic to HA"));

  sender.add("VersionSW", versionSW);
  sender.add("Napeti",  ESP.getVcc());
  sender.add("HeartBeat", heartBeat++);
  sender.add("RSSI", WiFi.RSSI());
  DEBUG_PRINTLN(F("Calling MQTT"));
  
  sender.subscribe(mqtt_base, "");
  
  sender.sendMQTT(mqtt_server, mqtt_port, mqtt_username, mqtt_key, mqtt_base);
  digitalWrite(BUILTIN_LED, HIGH);
  return true;
}

bool sendDataHA(void *) {
  digitalWrite(BUILTIN_LED, LOW);
  printSystemTime();
  DEBUG_PRINTLN(F(" - I am sending data to HA"));
  
  sender.add("Status", alarmArmed);
  
  for (byte i=0; i<10; i++) {
    sender.add(String("LOOP") + String(i), (uint8_t)random(0,2));
    sender.add(String("LOOP") + String(i) + String("Armed/status"), arrLOOPStatus[i]);
  }
  DEBUG_PRINTLN(F("Calling MQTT"));
  
  sender.sendMQTT(mqtt_server, mqtt_port, mqtt_username, mqtt_key, mqtt_base);
  digitalWrite(BUILTIN_LED, HIGH);
  return true;
}
