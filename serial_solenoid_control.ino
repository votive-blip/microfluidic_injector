const int left = 3; // baseline 
const int right = 4; // thing to increase

void setup() {

  Serial.begin(9600);
  delay(200);
  Serial.println("!!!!!!!!!!!!!!!!1");

  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);


  digitalWrite(left, LOW);
  digitalWrite(right, LOW);
}

void loop() {


  String Serial_input = Serial.readString();
  if (Serial_input == "left on") {
    Serial.println("left on");
    digitalWrite(left, HIGH);
  }
  if (Serial_input == "left off") {
    Serial.println("left off");
    digitalWrite(left, LOW);
  }

  if (Serial_input == "right on") {
    Serial.println("right on");
    digitalWrite(right, HIGH);
  }
  if (Serial_input == "right off") {
    Serial.println("right off");
    digitalWrite(right, LOW);
  }
}
