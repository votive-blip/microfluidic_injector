const int MEDIA_PIN_1 = 4;
const int BASE_PIN_1 = 3;

void setup() {

  pinMode(MEDIA_PIN_1, OUTPUT);
  pinMode(BASE_PIN_1, OUTPUT);
}

void loop() {

  // washout(time in minutes);
  washout(1);






  digitalWrite(MEDIA_PIN_1, LOW);
  digitalWrite(BASE_PIN_1, LOW);
  exit(0);  // exit main loop
}

int washout(float duration) {
  for (int i = 0; i <= duration * 60; i++) {
    digitalWrite(BASE_PIN_1, HIGH);
    digitalWrite(MEDIA_PIN_1, LOW);
    delay(500);
    digitalWrite(BASE_PIN_1, LOW);
    digitalWrite(MEDIA_PIN_1, HIGH);
    delay(500);
  }
}
