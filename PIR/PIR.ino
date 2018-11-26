// Pohybové èidlo HC-SR501

// nastavení èísla vstupního pinu
const int cidloPin = D1;

void setup() {
  // komunikace pøes sériovou linku rychlostí 9600 baud
  Serial.begin(115200);  
  // inicializace digitálního vstupu
  pinMode(cidloPin, INPUT_PULLUP);
  // nastavení pøerušení na pin 2 (int0) 
  // pøi rostoucí hranì (logO->log1) se vykoná program prerus 
  attachInterrupt(digitalPinToInterrupt(cidloPin), detekce, RISING);
}

void loop() {
  // pro ukázku se každou vteøinu vytiskne
  // zpráva o poètu vteøin od zapnutí Arduina
  // Serial.print("Cas od zapnuti: ");
  // Serial.print(millis()/1000);
  // Serial.println(" vterin.");
  //Serial.println(digitalRead(cidloPin));
  //delay(100);
}

void detekce() {
  // pokud je aktivován digitální vstup,
  // vypiš informaci po sériové lince
  Serial.println("Detekce pohybu pomoci HC-SR501!");
}