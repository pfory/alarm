// Pohybov� �idlo HC-SR501

// nastaven� ��sla vstupn�ho pinu
const int cidloPin = D1;

void setup() {
  // komunikace p�es s�riovou linku rychlost� 9600 baud
  Serial.begin(115200);  
  // inicializace digit�ln�ho vstupu
  pinMode(cidloPin, INPUT_PULLUP);
  // nastaven� p�eru�en� na pin 2 (int0) 
  // p�i rostouc� hran� (logO->log1) se vykon� program prerus 
  attachInterrupt(digitalPinToInterrupt(cidloPin), detekce, RISING);
}

void loop() {
  // pro uk�zku se ka�dou vte�inu vytiskne
  // zpr�va o po�tu vte�in od zapnut� Arduina
  // Serial.print("Cas od zapnuti: ");
  // Serial.print(millis()/1000);
  // Serial.println(" vterin.");
  //Serial.println(digitalRead(cidloPin));
  //delay(100);
}

void detekce() {
  // pokud je aktivov�n digit�ln� vstup,
  // vypi� informaci po s�riov� lince
  Serial.println("Detekce pohybu pomoci HC-SR501!");
}