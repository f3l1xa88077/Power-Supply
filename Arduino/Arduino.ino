// -------- Pin configuration --------

int vsense = A1;

void setup() {

  // -------- PIN I/O --------

  pinMode(vsense, INPUT);

  // -------- Serial --------

  Serial.begin(9600);

}

void loop() {
  
  // -------- VOLTAGE SENSING --------

  int tmp = analogRead(vsense);
  Serial.println(tmp);

}
