#define photoresPin 36
#define lightPin 22

int lihgtInit;
int lightVal;

void setup() {
  pinMode(lightPin, OUTPUT);

  lihgtInit = analogRead(photoresPin);
}

void loop() {
  lightVal = analogRead(photoresPin);
  if(lightVal - lihgtInit < 50) {
    digitalWrite(lightPin, HIGH);
  }
  else {
    digitalWrite(lightPin, LOW);
  }
}
