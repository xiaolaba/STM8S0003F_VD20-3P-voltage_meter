/* 
 *  
 * patch the code, testing for V20D-3P volt meter with 3-digits of 7-seg LED
 * MCU STM8S003F3P6
 * PD1/PD2/PD3, LED cathode 3, 2, 1
 * PA1/PA2/PA3, LED anode a,b,c 
 * PC3/PC4/PC5/PC6/PC7, LED Anode s, e, f, g, dot(or h)
 * xiao_laba_cn@yahoo.com
 * 2020-05-10  
 * output "oHi" to 3 digit of 7-seg LED, blink once per second
*/

// Pin definitions
// MY3631A / CL3631A, 3 digit 7-seg LED red color
#define DIG1 PD1 // Digit 1 cathode
#define DIG2 PD2 // Digit 2 cathode
#define DIG3 PD3 // Digit 3 cathode

#define SEG_A PA1 // Segment a anode
#define SEG_B PA2 // Segment b anode
#define SEG_C PA3 // Segment c anode

#define SEG_D PC3 // Segment d anode
#define SEG_E PC4 // Segment e anode
#define SEG_F PC5 // Segment f anode
#define SEG_G PC6 // Segment g anode
#define SEG_DOT PC7 // Decimal point anode

// Segment patterns for 0-9, A-F, H, i, (bit order: a,b,c,d,e,f,g,dot)
const uint8_t segmentPatterns[16+3] = {
  0b11111100, // 0: a,b,c,d,e,f, index 0
  0b01100000, // 1: b,c
  0b11011010, // 2: a,b,d,e,g
  0b11110010, // 3: a,b,c,d,g
  0b01100110, // 4: b,c,f,g
  0b10110110, // 5: a,c,d,f,g
  0b10111110, // 6: a,c,d,e,f,g
  0b11100000, // 7: a,b,c
  0b11111110, // 8: a,b,c,d,e,f,g
  0b11110110, // 9: a,b,c,d,f,g
  0b11101110, // A: a,b,c,e,f,g
  0b00111110, // B: c,d,e,f,g
  0b10011100, // C: a,d,e,f
  0b01111010, // D: b,c,d,e,g
  0b10011110, // E: a,d,e,f,g
  0b10001110, // F: a,e,f,g
  
  0b00111010, // o: 00cde0g0, index 16
  0b01101110, // H: 0bc0efg0, index 17
  0b00100001, // i: 00c00001, index 18
   
};


// Test values: display "123" on digits 1, 2, 3
uint8_t digitValues[4] = {0, 1, 2, 3}; // Digit 1=1, Digit 2=2, Digit 3=3
// Test values: display "oHi." on digits 1, 2, 3
uint8_t hi[3] = {16, 17, 18}; // "oHi"


void setup() {
  // Set digit pins as outputs (cathodes)
  pinMode(DIG1, OUTPUT);
  pinMode(DIG2, OUTPUT);
  pinMode(DIG3, OUTPUT);
  // Set segment pins as outputs (anodes)
  pinMode(SEG_A, OUTPUT);
  pinMode(SEG_B, OUTPUT);
  pinMode(SEG_C, OUTPUT);
  pinMode(SEG_D, OUTPUT);
  pinMode(SEG_E, OUTPUT);
  pinMode(SEG_F, OUTPUT);
  pinMode(SEG_G, OUTPUT);
  pinMode(SEG_DOT, OUTPUT);
  // Initialize digits off
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, HIGH);
  digitalWrite(DIG3, HIGH);
  
  // Diagnostic: Test each segment (a-g, dot) on Digit 3
  digitalWrite(DIG3, LOW); // Enable Digit 3
  digitalWrite(SEG_A, HIGH); delay(300); digitalWrite(SEG_A, LOW);
  digitalWrite(SEG_B, HIGH); delay(300); digitalWrite(SEG_B, LOW);
  digitalWrite(SEG_C, HIGH); delay(300); digitalWrite(SEG_C, LOW);
  digitalWrite(SEG_D, HIGH); delay(300); digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, HIGH); delay(300); digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, HIGH); delay(300); digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, HIGH); delay(300); digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DOT, HIGH); delay(300); digitalWrite(SEG_DOT, LOW);
  digitalWrite(DIG1, HIGH); // Turn off Digit 1
}

void displayDigit(uint8_t digit, uint8_t value) {
  // Turn off all digits
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, HIGH);
  digitalWrite(DIG3, HIGH);
  // Turn off all segments
  digitalWrite(SEG_A, LOW);
  digitalWrite(SEG_B, LOW);
  digitalWrite(SEG_C, LOW);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DOT, LOW);
  
  // Set segment states based on pattern
  uint8_t pattern = segmentPatterns[value];
  digitalWrite(SEG_A, (pattern & (1 << 7)) ? HIGH : LOW);
  digitalWrite(SEG_B, (pattern & (1 << 6)) ? HIGH : LOW);
  digitalWrite(SEG_C, (pattern & (1 << 5)) ? HIGH : LOW);
  digitalWrite(SEG_D, (pattern & (1 << 4)) ? HIGH : LOW);
  digitalWrite(SEG_E, (pattern & (1 << 3)) ? HIGH : LOW);
  digitalWrite(SEG_F, (pattern & (1 << 2)) ? HIGH : LOW);
  digitalWrite(SEG_G, (pattern & (1 << 1)) ? HIGH : LOW);
  digitalWrite(SEG_DOT, (pattern & (1 << 0)) ? HIGH : LOW);
  
  // Enable the selected digit
  if (digit == 1) digitalWrite(DIG1, LOW);
  else if (digit == 2) digitalWrite(DIG2, LOW);
  else if (digit == 3) digitalWrite(DIG3, LOW);
}

void loop() {
  // Blink ON for 500ms
  for (uint16_t i = 0; i < 50; i++) { // Reduced iterations for slower multiplexing

/*
    //// my demo board PD1(SWIM) pulled up by 1K resistor always for STLINK-V2 programmer
    //// no test for digit3
    //displayDigit(1, digitValues[1]); // Digit 1 shows "1"
    //delay(5);   
    displayDigit(2, digitValues[2]); // Digit 2 shows "2"
    delay(5);
    displayDigit(3, digitValues[3]); // Digit 3 shows "3"
    delay(5);

*/

    displayDigit(1, hi[0]); // Digit 1 shows "o"
    delay(5);
    displayDigit(2, hi[1]); // Digit 2 shows "H"
    delay(5);
    displayDigit(3, hi[2]); // Digit 3 shows "i."
    delay(5);
    
  }
  
  // Blink OFF for 500ms
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, HIGH);
  digitalWrite(DIG3, HIGH);
  delay(200);





}
