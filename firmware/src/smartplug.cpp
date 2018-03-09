/////////////////////////////////////////////////////////////////////////////
/** @file
ESP8266 Etekcity/Vesync smart plug hacking

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "smartplug.h"
#include <Arduino.h>

namespace {
    static volatile int pulsePin = SmartPlug::PIN_CF1;
    static volatile int pulseStart = 0;
}

SmartPlug* SmartPlug::instance_ = nullptr;

/////////////////////////////////////////////////////////////////////////////
SmartPlug::SmartPlug() {
    assert(!instance_);
    instance_ = this;
}
SmartPlug::~SmartPlug() {
    detachInterrupt(pulsePin);
    instance_ = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
/// smart plug
void SmartPlug::begin() {
    pinMode(PIN_SW1, INPUT);

    pinMode(PIN_CF,  INPUT);
    pinMode(PIN_CF1, INPUT);
    pulsePin = PIN_CF1;
    attachInterrupt(pulsePin, onRisingInterrupt_, RISING);

    pinMode(PIN_BLUE_LED, OUTPUT);
    pinMode(PIN_MOD_LED, OUTPUT);

    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);
}

/////////////////////////////////////////////////////////////////////////////
void SmartPlug::tick() {
    
}

void SmartPlug::setRelay(bool state) {
    relay_ = state;
    digitalWrite(PIN_RELAY, state);
}

/////////////////////////////////////////////////////////////////////////////
ICACHE_RAM_ATTR void SmartPlug::onRisingInterrupt_() {
    pulseStart = micros();
    attachInterrupt(pulsePin, onFallingInterrupt_, FALLING);
}
ICACHE_RAM_ATTR void SmartPlug::onFallingInterrupt_() {
    const auto pulseWidth = micros() - pulseStart;
    detachInterrupt(pulsePin);
    if (pulseWidth <= 0) return;

    const int PWM_PERIOD = 1000000; // 1 hz
//    if (pulse_width > PWM_PERIOD) return;

    // fcf  = v1 * v2 * 48 / v2ref * fosc / 128
    // fcf1 = v1 * 24 / vref * fosc / 512

    // https://tech.scargill.net/pow-th16-and-dual/
    // V1 is the differential voltage between measurement pins V1P - V1N (+/-43.75mV peak)
    // V2 is the differential voltage between measurement pins V2P and GND (+/-700mV peak)
    // fosc is 3.579 MHz -+/-15 %)
    // Vref is 2.43 V

    const auto FOSC = 3579000.0;
    const auto VREF = 2.43;

    const auto ONE_MICROS = 1000000; // 1 microsecond
    const auto freq = ONE_MICROS / (2.0 * pulseWidth);

    const auto V1_R = 0.002;
    const auto V2_R1 = 2 * 953000.0;
    const auto V2_R2 = 1000.0;
    const auto V2_RDIV = V2_R2 / (V2_R1 + V2_R2);

    if (PIN_CF1 == pulsePin) {
        pulsePin = PIN_CF;

        // Voltage RMS calculation formula
        // fcfu = (v2 * 2) / vref * fosc / 512
        const auto v2 = freq * 512 / FOSC * VREF / 2.0;
        const auto vin = v2 / V2_RDIV;
        instance_->measVoltage_ = vin;

        // Serial.print("CF pulseWidth: ");
        // Serial.print(pulseWidth);
        // Serial.print(" voltage: ");
        // Serial.println(vin);

    } else {
        pulsePin = PIN_CF1;

        // fcf = (v1 * v2 * 48) / vref^2 * fosc / 128
        const auto v1v2 = freq * 128 / FOSC * (VREF * VREF) / 48.0;
        const auto power = v1v2 / V2_RDIV / V1_R;
        instance_->measPower_ = power;

        // Serial.print("CF1 pulseWidth: ");
        // Serial.print(pulseWidth);
        // Serial.print(" v1v2: ");
        // Serial.print(v1v2);
        // Serial.print(" power: ");
        // Serial.println(power);
    }

    attachInterrupt(pulsePin, onRisingInterrupt_, RISING);
}
