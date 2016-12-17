// Pohybové èidlo HC-SR501

// nastavení èísla vstupního pinu
const int cidloPin = 2;

void setup() {
  // komunikace pøes sériovou linku rychlostí 9600 baud
  Serial.begin(115200);  
  // inicializace digitálního vstupu
  pinMode(cidloPin, INPUT);
  // nastavení pøerušení na pin 2 (int0) 
  // pøi rostoucí hranì (logO->log1) se vykoná program prerus 
  attachInterrupt(0, detekce, RISING);
}

void loop() {
  // pro ukázku se každou vteøinu vytiskne
  // zpráva o poètu vteøin od zapnutí Arduina
  Serial.print("Cas od zapnuti: ");
  Serial.print(millis()/1000);
  Serial.println(" vterin.");
  delay(1000);
}

void detekce() {
  // pokud je aktivován digitální vstup,
  // vypiš informaci po sériové lince
  Serial.println("Detekce pohybu pomoci HC-SR501!");
}