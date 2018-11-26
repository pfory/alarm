#include <Ticker.h>


String GSMsignal;
#define LED 13
#define SERIAL_TX_BUFFER_SIZE 300
#define SERIAL_RX_BUFFER_SIZE 300
String buffer;

#define verbose
#ifdef verbose
 #define PORTSPEED 115200
 #define DEBUG_PRINT(x)         Serial.print (x)
 #define DEBUG_PRINTDEC(x)      Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)       Serial.println (x)
 #define DEBUG_PRINTF(x, y)     Serial.printf (x, y)
 #define SERIAL_BEGIN           Serial.begin(PORTSPEED)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINTF(x, y)
 #define SERIAL_BEGIN
#endif 

#include <SoftwareSerial.h>
 //SIM800 TX is connected to Arduino D8
#define SIM800_TX_PIN       D6
//SIM800 RX is connected to Arduino D7
#define SIM800_RX_PIN       D5
#define GSMReset            D1

//Create software serial object to communicate with SIM800
SoftwareSerial serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN);

//for LED status
#include <Ticker.h>
Ticker ticker;

void tick() {
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

void setup()
{
  SERIAL_BEGIN;
  DEBUG_PRINTLN("\n\rALARM v 0.1");
  serialSIM800.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);
  ticker.attach(1, tick);

  restartSIMHW();
  SIM800Init();  
	// buffer = sendATcommandResponse("AT+CPBR=1", "+CPBR:", 3000,128);
  // int start = buffer.indexOf(",")+1;
  // int end = buffer.indexOf(",",start);
  // String cislo = buffer.substring(start,end);
  // DEBUG_PRINTLN(start);
  // DEBUG_PRINTLN(end);
  // DEBUG_PRINTLN(buffer.substring(start,end));
  // //cislo = "ATD"+cislo;
	// sendATcommandResponse(cislo.c_str(), "OK", 2000);
  
  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, HIGH);  
}

void loop() {
  delay(5000);
  DEBUG_PRINTLN("cyklus");
  checkSMS();
	//DEBUG_PRINTLN(sendATcommandResponse("ATD724232639;", "OK", 2000));
  
}

void restartSIMHW()
{
	//HW restart
  DEBUG_PRINT("SIM800 restart...");
	digitalWrite(GSMReset,LOW);
	delay(400);
	digitalWrite(GSMReset,HIGH);
	delay(15000);
  DEBUG_PRINTLN("done");
}

void SIM800Init()
{
	while (Serial.available() > 0) Serial.read();
	DEBUG_PRINTLN("****************************************");
	while (sendATcommand("AT","OK",2000) == 0);
	DEBUG_PRINTLN("SIM800 modul dostupny!");
	//otestuj registraci do site
	while ( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) ||
	sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 );
	while (Serial.available() > 0) Serial.read();
	DEBUG_PRINTLN("SIM800 registrovan do site!");
  /*
	//nastaveni parametru pro obdrzeni casu ze site
	while( sendATcommand("AT+CLTS=1", "OK", 500) == 0 );
	while( sendATcommand("AT+CENG=3", "OK", 500) == 0 );
	while (Serial.available() > 0) Serial.read();
	DEBUG_PRINT("Casova znacka:");
	DEBUG_PRINTLN(timeStamp());
  */
	//CSQ, pozice 4
	GSMsignal = sendATcommandResponse("AT+CSQ", "+CSQ:", 1000, 3);
	
	DEBUG_PRINTLN("GSM signal quality:-"+GSMsignal+" dBm");
	if (GSMsignal.length()==3) GSMsignal = GSMsignal.substring(1);
	String providerName = sendATcommandResponse("AT+COPS?","+COPS:",1000,20);
	uint8_t tempIndex = providerName.indexOf('"');
	providerName = providerName.substring(tempIndex+1);
	tempIndex = providerName.indexOf('"');
	providerName = providerName.substring(0,tempIndex);
	DEBUG_PRINTLN("GSM provider:"+providerName);
	sendATcommand("AT+CMGF=1", "OK", 500);
	//zakaz indikace SMS
	sendATcommand("AT+CNMI=0,0", "OK",500);
	sendATcommand("AT+CLIP=1", "OK", 500);
	//smaz vsechny SMSky
	/*sendATcommand("AT+CMGD=1,4", "OK", 2000);
	sendATcommand("AT+CMGD=1", "OK", 2000);
	DEBUG_PRINTLN F("Stare SMS smazany!");
  */
	sendATcommand("ATE0", "OK", 2000);
	DEBUG_PRINTLN("Zakladni parametry nastaveny!");
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout)
{

	//WDT_Restart(WDT);
	uint8_t x = 0,  answer = 0;
	char response[100];
	unsigned long previous;

	memset(response, '\0', 100);    // Initialize the string

	delay(100);

	//clrSIMbuffer();
	serialSIM800.println(ATcommand);    // Send the AT command
	//WDT_Restart(WDT);

	x = 0;
	previous = millis();

	// this loop waits for the answer
	do
	{
		//WDT_Restart(WDT);
		if (serialSIM800.available() != 0) {
			// if there are data in the UART input buffer, reads it and checks for the asnwer
			response[x] = serialSIM800.read();
			x++;
			// check if the desired answer  is in the response of the module
			if (strstr(response, expected_answer) != NULL)
			{
				answer = 1;
			}
		}
		// Waits for the asnwer with time out
	}
	while ((answer == 0) && ((millis() - previous) < timeout));

	//clrSIMbuffer();
	return answer;
}


//AT prikaz s ocekavanou odpovedi a zbytkem
String sendATcommandResponse(char* ATcommand, char* expected_answer, unsigned int timeout, unsigned int buf)
{
	//WDT_Restart(WDT);
	uint8_t x = 0,  answer = 0;
	char response[150];
	unsigned long previous;
	String rest;

	memset(response, '\0', 100);    // Initialize the string

	delay(100);

	//clrSIMbuffer();
	serialSIM800.println(ATcommand);    // Send the AT command


	x = 0;
	previous = millis();
	//WDT_Restart(WDT);
	// this loop waits for the answer
	do
	{
		//WDT_Restart(WDT);
		if (serialSIM800.available() != 0) {
			// if there are data in the UART input buffer, reads it and checks for the asnwer
			response[x] = serialSIM800.read();
			x++;
			// check if the desired answer  is in the response of the module
			if (strstr(response, expected_answer) != NULL)
			{
				answer = 1;
			}
		}
		// Waits for the asnwer with time out
	}
	while ((answer == 0) && ((millis() - previous) < timeout));

	//p?e?ti zbytek - max 20 byte
	memset(response, '\0', buf);    // Initialize the string
	delay(100);
	for (x = 0; x < buf; x++) response[x] = serialSIM800.read();
	//clrSIMbuffer();
	for (x = 0; x < buf; x++) rest += char(response[x]);
	delay(100);
	return rest;

}


//Receive SMS
void checkSMS() {
  DEBUG_PRINTLN("Check new SMS...");
	char g;
	String gcmd;
	String number;
	uint8_t gindexOd;
	uint8_t gindexDo;
	clrSIMbuffer();
	serialSIM800.println("AT+CMGR=1");
	delay(1000);
	while(serialSIM800.available()>0) {
    g=serialSIM800.read();
    gcmd+=g;
    if (g=='\n') {
      if (gcmd.indexOf("+CMGR")>0) {
				DEBUG_PRINTLN(gcmd);
				DEBUG_PRINTLN("Nova SMSka!!!!");
				gindexOd = gcmd.indexOf(',');
				gindexDo = gcmd.indexOf(',',gindexOd+1);
				number = gcmd.substring(gindexOd+2,gindexDo-1);
				DEBUG_PRINTLN("Od:" + number);
        gcmd="";
				while (serialSIM800.available()>0) {
          //precti obsah SMS
					g=serialSIM800.read();
					gcmd += g;
        }
        DEBUG_PRINTLN("Obsah SMS:"+gcmd);
   			SMSparser(gcmd);
				gcmd = "";
  			sendATcommand("AT+CMGD=1", "OK", 1000);
        sendATcommand("AT+CMGD=1,4", "OK", 1000);
				DEBUG_PRINTLN("SMS smazana z pameti.");
      }
    }
  }
 return;
 /*
				while (serialSIM800.available()>0)
				{
					g=serialSIM800.read();
					gcmd += g;
					if (g=='\n')
					{
						gcmd = gcmd.substring(0,gcmd.length()-2);
						DEBUG_PRINTLN("Obsah SMS:" + gcmd);
						sendSMS(number,"Vykonano!");
						gcmd = "";
						sendATcommand("AT+CMGD=1", "OK", 1000);
						sendATcommand("AT+CMGD=1,4", "OK", 1000);
						DEBUG_PRINTLN("SMS smazana z pameti.");
						return;
					}
				}
				gcmd="";
				sendATcommand("AT+CMGD=1", "OK", 1000);
				sendATcommand("AT+CMGD=1,4", "OK", 1000);
				return;
			}
		}
	}*/
}

void sendSMS(String number, String sms)
{
	DEBUG_PRINTLN("SMS pro:"+number);
	DEBUG_PRINTLN("Obsah:"+sms);
	delay(500);
	clrSIMbuffer();
	//number = "+420"+number;
	serialSIM800.println ("AT+CMGS=\""+number+"\"");
	delay(200);
	//toSerial();
	serialSIM800.println (sms);        // message to send
	delay(100);
	serialSIM800.write ((char) 26);	//CTRL+Z
	delay(100);
	serialSIM800.println();
	delay(100);
	sendATcommand("AT+CMGD=1", "OK", 2000);
	sendATcommand("AT+CMGD=1,4", "OK", 2000);
	delay(500);
	clrSIMbuffer();
	DEBUG_PRINTLN("SMS odeslana!");
}

void SMSparser(String command)
{
	if (command.startsWith("LED:ON")) 	{
		digitalWrite(LED,HIGH);
		return;
	}
	if (command.startsWith("LED:OFF")) {
		digitalWrite(LED,LOW);
		return;
	}
	DEBUG_PRINTLN("Neznamy prikaz!");

}

String timeStamp()
{
	String ts="";
	ts = sendATcommandResponse("AT+CCLK?", "+CCLK: \"", 1000, 20);
	//13/11/04,15:23:19+04
	//return ("20"+ts.substring(0,2)+ts.substring(3,5)+ts.substring(6,8)+ts.substring(9,11)+ts.substring(12,14)+ts.substring(15,17));
	return ts;
}
void clrSIMbuffer() {
	while(serialSIM800.available()>0) {
		delay(1);
		serialSIM800.read();
	}
}