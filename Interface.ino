const unsigned int pinSC = 2;
const unsigned int pinSI = 4;
const unsigned int pinSD = 3;

const unsigned long baud3 = 115200L;

void setup() {
  Serial.begin(baud3);
  pinMode(pinSC, INPUT_PULLUP);
  pinMode(pinSI, INPUT_PULLUP);
  pinMode(pinSD, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(pinSC), signalTransfer, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinSD), startRead, FALLING);
}

bool transfering = false;
/*bool reading = false;
unsigned int readCountSD = 0;
unsigned int bitsSD = 0;

void startTimer0Baud3() { 
  TCCR0A = 1 << WGM01; // CTC mode
  TCCR0B = 1 << CS00; // 1 prescaler 
  OCR0A = 138; // 16MHz / (baud3 * prescaler) - 1
  TCNT0 = 0; // clear counter value
  
  TIMSK0 |= 1 << OCIE0A;
}

void stopTimer0() {
  TIMSK0 &= ~(1 << OCIE0A);
}

ISR(TIMER0_COMPA_vect) {  
  if (readCountSD >= 16) {
    reading = false;
    stopTimer0();
    Serial.println(bitsSD, HEX);
  } else {
    unsigned int bitSD = (PIND >> pinSD) & 1;
    bitsSD |= bitSD << readCountSD;
  }
  
  readCountSD++;
}*/

void signalTransfer() {
  transfering = !((PIND >> pinSC) & 1);
}

#define NOP __asm__ __volatile__ ("nop\n\t")

void startRead() {
  if (transfering) {
    unsigned int test = 0;

    for (int i = 0; i < 16; i++) {
      NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
      NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
      NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;//NOP;NOP;NOP;NOP;
      
      unsigned int bitSD = (PIND >> pinSD) & 1;
      test |= bitSD << i;
    }

    Serial.println(test, HEX);
  }
  /*if (transfering && !reading) {
    reading = true;
    readCountSD = 0;
    bitsSD = 0;
    startTimer0Baud3(); 
  }*/
}

//3ede
void loop() {
}
