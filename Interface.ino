const unsigned int pinSC = 25;
const unsigned int pinSI = 26;
const unsigned int pinSD = 27;

void setup() {
  pmc_set_writeprotect(false);

  pmc_enable_periph_clk(ID_TC0);

  NVIC_EnableIRQ(TC0_IRQn);
  NVIC_EnableIRQ(PIOD_IRQn);

  Serial.begin(250000);

  pinMode(pinSC, INPUT_PULLUP);
  pinMode(pinSI, INPUT_PULLUP);
  pinMode(pinSD, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pinSD), startRead, FALLING);
}

unsigned int readCountSD = 0;
unsigned int bitsSD = 0;

void startTimer0Baud3() {
  // Timer clock1 (42MHz)
  REG_TC0_CMR0 = TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_CPCTRG;

  // Enabel RC compare interrupt
  REG_TC0_IER0 = TC_IER_CPCS;

  //Set RC compare value
  REG_TC0_RC0 = 365; // round(42 MHz / 115 200 pbs)

  //Start timer
  REG_TC0_CCR0 = TC_CCR_SWTRG | TC_CCR_CLKEN;
}

void stopTimer0() {
  REG_TC0_CCR0 = TC_CCR_CLKDIS;
}

void TC0_Handler() {
   // Clear timer 0 interrupt status
  (void)REG_TC0_SR0;
  if (readCountSD >= 16) {
    stopTimer0();
    // Clear port D interrupt status
    (void)REG_PIOD_ISR;
    NVIC_EnableIRQ(PIOD_IRQn);
    Serial.println(bitsSD, HEX);
  } else {
    unsigned int bitSD = digitalRead(pinSD);
    bitsSD |= bitSD << readCountSD;
  }

  readCountSD++;
}

void startRead() {
  if (!digitalRead(pinSC)) {
    readCountSD = 0;
    bitsSD = 0;
    NVIC_DisableIRQ(PIOD_IRQn);
    startTimer0Baud3();
  }
}

void loop() {
}
