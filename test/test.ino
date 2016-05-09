#define SIM800 Serial1
#define GSMReset 5
String GSMsignal;
#define LED 13
#define SERIAL_TX_BUFFER_SIZE 300
#define SERIAL_RX_BUFFER_SIZE 300
String buffer;

void setup()
{

  Serial.begin(115200);
  SIM800.begin(115200);
  restartSIMHW();
  SIM800Init();  
	buffer = sendATcommandResponse("AT+CPBR=1", "+CPBR:", 3000,128);
  int start = buffer.indexOf(",")+1;
  int end = buffer.indexOf(",",start);
  String cislo = buffer.substring(start,end);
  Serial.println(start);
  Serial.println(end);
  Serial.println(buffer.substring(start,end));
  //cislo = "ATD"+cislo;
	sendATcommandResponse(cislo.c_str(), "OK", 2000);
  
  
}

void loop() {
  delay(5000);
  Serial.println("cyklus");
  //checkSMS();
	//Serial.println(sendATcommandResponse("ATD724232639;", "OK", 2000));
  
}

void restartSIMHW()
{
	//HW restart
	digitalWrite(GSMReset,LOW);
	delay(400);
	digitalWrite(GSMReset,HIGH);
	delay(400);
}

void SIM800Init()
{
	while (Serial.available() > 0) Serial.read();
	Serial.println F("****************************************");
	while (sendATcommand("AT","OK",2000) == 0);
	Serial.println F("SIM800 modul dostupny!");
	//otestuj registraci do site
	while ( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) ||
	sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 );
	while (Serial.available() > 0) Serial.read();
	Serial.println F("SIM800 registrovan do site!");
  /*
	//nastaveni parametru pro obdrzeni casu ze site
	while( sendATcommand("AT+CLTS=1", "OK", 500) == 0 );
	while( sendATcommand("AT+CENG=3", "OK", 500) == 0 );
	while (Serial.available() > 0) Serial.read();
	Serial.print("Casova znacka:");
	Serial.println(timeStamp());
  */
	//CSQ, pozice 4
	GSMsignal = sendATcommandResponse("AT+CSQ", "+CSQ:", 1000, 3);
	
	Serial.println("GSM signal quality:"+GSMsignal);
	if (GSMsignal.length()==3) GSMsignal = GSMsignal.substring(1);
	String providerName = sendATcommandResponse("AT+COPS?","+COPS:",1000,20);
	uint8_t tempIndex = providerName.indexOf('"');
	providerName = providerName.substring(tempIndex+1);
	tempIndex = providerName.indexOf('"');
	providerName = providerName.substring(0,tempIndex);
	Serial.println("GSM provider:"+providerName);
	sendATcommand("AT+CMGF=1", "OK", 500);
	//zakaz indikace SMS
	sendATcommand("AT+CNMI=0,0", "OK",500);
	sendATcommand("AT+CLIP=1", "OK", 500);
	//smaz vsechny SMSky
	/*sendATcommand("AT+CMGD=1,4", "OK", 2000);
	sendATcommand("AT+CMGD=1", "OK", 2000);
	Serial.println F("Stare SMS smazany!");
  */
	sendATcommand("ATE0", "OK", 2000);
	Serial.println F("Zakladni parametry nastaveny!");
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
	SIM800.println(ATcommand);    // Send the AT command
	//WDT_Restart(WDT);

	x = 0;
	previous = millis();

	// this loop waits for the answer
	do
	{
		//WDT_Restart(WDT);
		if (SIM800.available() != 0) {
			// if there are data in the UART input buffer, reads it and checks for the asnwer
			response[x] = SIM800.read();
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
	SIM800.println(ATcommand);    // Send the AT command


	x = 0;
	previous = millis();
	//WDT_Restart(WDT);
	// this loop waits for the answer
	do
	{
		//WDT_Restart(WDT);
		if (SIM800.available() != 0) {
			// if there are data in the UART input buffer, reads it and checks for the asnwer
			response[x] = SIM800.read();
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
	for (x = 0; x < buf; x++) response[x] = SIM800.read();
	//clrSIMbuffer();
	for (x = 0; x < buf; x++) rest += char(response[x]);
	delay(100);
	return rest;

}


//Receive SMS
void checkSMS()
{
	char g;
	String gcmd;
	String number;
	uint8_t gindexOd;
	uint8_t gindexDo;
	clrSIMbuffer();
	SIM800.println("AT+CMGR=1");
	delay(100);
	while(SIM800.available()>0) {
    g=SIM800.read();
    gcmd+=g;
    if (g=='\n') {
      if (gcmd.indexOf("+CMGR")>0) {
				Serial.println(gcmd);
				Serial.println("Nova SMSka!!!!");
				gindexOd = gcmd.indexOf(',');
				gindexDo = gcmd.indexOf(',',gindexOd+1);
				number = gcmd.substring(gindexOd+2,gindexDo-1);
				Serial.println("Od:" + number);
        gcmd="";
				while (SIM800.available()>0) {
          //precti obsah SMS
					g=SIM800.read();
					gcmd += g;
        }
        Serial.println("Obsah SMS:"+gcmd);
   			SMSparser(gcmd);
				gcmd = "";
  			sendATcommand("AT+CMGD=1", "OK", 1000);
        sendATcommand("AT+CMGD=1,4", "OK", 1000);
				Serial.println("SMS smazana z pameti.");
      }
    }
  }
 return;
 /*
				while (SIM800.available()>0)
				{
					g=SIM800.read();
					gcmd += g;
					if (g=='\n')
					{
						gcmd = gcmd.substring(0,gcmd.length()-2);
						Serial.println("Obsah SMS:" + gcmd);
						sendSMS(number,"Vykonano!");
						gcmd = "";
						sendATcommand("AT+CMGD=1", "OK", 1000);
						sendATcommand("AT+CMGD=1,4", "OK", 1000);
						Serial.println("SMS smazana z pameti.");
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
	Serial.println("SMS pro:"+number);
	Serial.println("Obsah:"+sms);
	delay(500);
	clrSIMbuffer();
	//number = "+420"+number;
	SIM800.println ("AT+CMGS=\""+number+"\"");
	delay(200);
	//toSerial();
	SIM800.println (sms);        // message to send
	delay(100);
	SIM800.write ((char) 26);	//CTRL+Z
	delay(100);
	SIM800.println();
	delay(100);
	sendATcommand("AT+CMGD=1", "OK", 2000);
	sendATcommand("AT+CMGD=1,4", "OK", 2000);
	delay(500);
	clrSIMbuffer();
	Serial.println("SMS odeslana!");
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
	Serial.println("Neznamy prikaz!");

}

String timeStamp()
{
	String ts="";
	ts = sendATcommandResponse("AT+CCLK?", "+CCLK: \"", 1000, 20);
	//13/11/04,15:23:19+04
	//return ("20"+ts.substring(0,2)+ts.substring(3,5)+ts.substring(6,8)+ts.substring(9,11)+ts.substring(12,14)+ts.substring(15,17));
	return ts;
}
void clrSIMbuffer()
{
	while(SIM800.available()>0)
	{
		delay(1);
		SIM800.read();
	}
}