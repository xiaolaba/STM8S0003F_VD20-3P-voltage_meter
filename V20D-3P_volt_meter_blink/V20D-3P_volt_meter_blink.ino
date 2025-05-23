/*
 * patch the code, testing for V20D-3P volt meter with 3-digits of 7-seg LED
 * MCU STM8S003F3P6
 * PD1/PD2/PD3, LED cathode
 * PA1/PA2/PA3, PC3/PC4/PC5/PC6/PC7, LED Anode
 * xiao_laba_cn@yahoo.com
 * 2020-05-10
 */

void setup() {
  // Set PA3 as output (anode, positive side)
  pinMode(PA3, OUTPUT);
  // Set PD3 as output (cathode, negative side)
  pinMode(PD3, OUTPUT);
}

void loop() {
  // Turn LED ON: PA3 HIGH, PD3 LOW
  digitalWrite(PA3, HIGH);
  digitalWrite(PD3, LOW);
  delay(1000); // Wait for 1 second
  
  // Turn LED OFF: PA3 LOW, PD3 HIGH
  digitalWrite(PA3, LOW);
  digitalWrite(PD3, HIGH);
  delay(1000); // Wait for 1 second
}
