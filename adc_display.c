#define FREQUENCY    80
//
#include "ESP8266WiFi.h"
extern "C" {
#include "user_interface.h"
}

#include "Wire.h"
int PCF8591=0x48; // I2C bus address
byte ana0, ana1, ana2, ana3;    // 0=Poti, 1=CdS, 2=None, 3=Thermistor

int segcnt = 0;
int value = 0;
const int DIO = 14;      // data
const int sck = 12;      // clock
const int RCK = 4;       // latch
static int powersof10[] = {10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
byte segbuf[8];

byte col[] = {
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000
};

byte seg[] = {
    0b11000000,  // 0
    0b11111001,  // 1
    0b10100100,  // 2
    0b10110000,  // 3
    0b10011001,  // 4
    0b10010010,  // 5
    0b10000010,  // 6
    0b11111000,  // 7
    0b10000000,  // 8
    0b10010000   // 9  
};     

void LED_irq(void) {
    Wire.beginTransmission(PCF8591); // launch PCF8591
    Wire.write(0x04);       // ADC0 control byte
    Wire.endTransmission();
    Wire.requestFrom(PCF8591, 5);
    ana0=Wire.read();      // garbage data
    ana0=Wire.read();
    ana1=Wire.read();
    ana2=Wire.read();
    ana3=Wire.read();
    value = ((ana0 / powersof10[segcnt]) % 10);
    digitalWrite(RCK, LOW);
    shiftOut(DIO, sck, MSBFIRST, col[segcnt]);
    shiftOut(DIO, sck, MSBFIRST, seg[value]);
    digitalWrite(RCK, HIGH);
    segcnt ++;
    segcnt &= 7;
}

void setup() {
  WiFi.forceSleepBegin();                  // disable ESP8266 wifi to save energy
delay(1);                                // shutdown break
system_update_cpu_freq(FREQUENCY);
    pinMode(RCK, OUTPUT);
    pinMode(sck, OUTPUT);
    pinMode(DIO, OUTPUT);


    Wire.pins(0,2);
    Wire.begin(0,2);

    for (int i=0; i<8; i++)
        segbuf[i] = seg[i];

    timer1_isr_init();
    timer1_attachInterrupt(LED_irq);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
    timer1_write((clockCyclesPerMicrosecond() / 16) * 2000);
    
}

void loop() {
wdt_reset();
    delay(1) ;
}
