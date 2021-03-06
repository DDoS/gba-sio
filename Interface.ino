const unsigned int pinSC = 25;
const unsigned int pinSI = 26;
const unsigned int pinSD = 27;

void setup() {
    pmc_set_writeprotect(false);

    pmc_enable_periph_clk(ID_TC0);

    NVIC_EnableIRQ(TC0_IRQn);
    NVIC_EnableIRQ(PIOD_IRQn);

    SerialUSB.begin(38400);
    while(!SerialUSB);

    pinMode(pinSC, INPUT_PULLUP);
    pinMode(pinSI, INPUT_PULLUP);
    pinMode(pinSD, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(pinSD), startRead, FALLING);
    attachInterrupt(digitalPinToInterrupt(pinSI), startWrite, FALLING);
}

volatile unsigned int dataChild = 0xFFFF;

unsigned int dataIn = 0;
unsigned int dataOut = 0;

unsigned int bitCount = 0;

bool writing = false;

void startTimerBaud3() {
    // Timer clock1 (42MHz)
    REG_TC0_CMR0 = TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_CPCTRG;

    // Enabel RC compare interrupt
    REG_TC0_IER0 = TC_IER_CPCS;

    //Set RC compare value
    REG_TC0_RC0 = 365; // round(42 MHz / 115 200 pbs)

    //Start timer
    REG_TC0_CCR0 = TC_CCR_SWTRG | TC_CCR_CLKEN;
}

void stopTimerBaud3() {
    REG_TC0_CCR0 = TC_CCR_CLKDIS;
}

void TC0_Handler() {
     // Clear timer 0 interrupt status
    (void)REG_TC0_SR0;

    if (writing) {
        if (bitCount < 1) {
            // Start bit
            pinMode(pinSD, OUTPUT);
            digitalWrite(pinSD, LOW);
            bitCount++;
        } else if (bitCount < 17) {
            unsigned int bitSD = (dataOut >> (bitCount - 1)) & 1;
            digitalWrite(pinSD, bitSD ? HIGH : LOW);
            bitCount++;
        } else {
            pinMode(pinSD, INPUT_PULLUP);
            // Switching to OUTPUT mode removed the interrupt
            attachInterrupt(digitalPinToInterrupt(pinSD), startRead, FALLING);

            stopTimerBaud3();
            // Clear port D interrupt status
            (void)REG_PIOD_ISR;
            NVIC_EnableIRQ(PIOD_IRQn);

            constexpr int dataSize = 4;
            byte data[dataSize] = {
                (byte)dataIn, (byte)(dataIn >> 8),
                (byte)dataOut, (byte)(dataOut >> 8)
            };
            SerialUSB.write(data, dataSize);
            //SerialUSB.println(dataIn, HEX);
            //SerialUSB.print(dataIn, HEX); SerialUSB.print(' '); SerialUSB.println(dataOut, HEX);
        }
    } else {
        if (bitCount >= 16) {
            stopTimerBaud3();
            // Clear port D interrupt status
            (void)REG_PIOD_ISR;
            NVIC_EnableIRQ(PIOD_IRQn);
        } else {
            unsigned int bitSD = digitalRead(pinSD);
            dataIn |= (bitSD & 1) << bitCount;
            bitCount++;
        }
    }
}

void startRead() {
    if (!digitalRead(pinSC)) {
        bitCount = 0;
        dataIn = 0;
        writing = false;
        NVIC_DisableIRQ(PIOD_IRQn);
        startTimerBaud3();
    }
}

void startWrite() {
    if (!digitalRead(pinSC)) {
        dataOut = dataChild;
        dataChild = 0xFFFF;
        bitCount = 0;
        writing = true;
        NVIC_DisableIRQ(PIOD_IRQn);
        startTimerBaud3();
    }
}

void loop() {
    if (SerialUSB.available() >= 2) {
        unsigned int data = SerialUSB.read() & 0xFF;
        data |= (SerialUSB.read() & 0xFF) << 8;

        dataChild = data;
    }
}
