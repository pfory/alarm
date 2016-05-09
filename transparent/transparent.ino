#define SIM900 Serial1
#define GSMReset 5
char c,s;

void setup()
{

  Serial.begin(115200);
  SIM900.begin(9600);
  digitalWrite(GSMReset,LOW);
  delay(200);
  digitalWrite(GSMReset,HIGH);
  delay(200);
  

}

void loop()
{
	if(Serial.available()>0)
	{
		c = Serial.read();
		SIM900.print(c);
	}
	if (SIM900.available()>0)
	{
		s = SIM900.read();
		Serial.print(s);
	}
  
}