#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include "Adafruit_HX711.h"
#include <Adafruit_SleepyDog.h>

// Define the pins for the HX711 communication
// For the weight sensor
const uint8_t DATA_PIN = 12; // Can use any pins!
const uint8_t CLOCK_PIN = 11; // Can use any pins!
Adafruit_HX711 hx711(DATA_PIN, CLOCK_PIN);

// Ultrasonic sensor
const int trigPin = 9;
const int echoPin = 10;
const int buzzPin = 5;
int32_t duration, distance;

float weight;

// Used for software SPI
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11

// Used for hardware & software SPI
#define LIS3DH_CS 10

// unsigned long acc_timer = millis();
unsigned long acc_timer;

const int taskDelay = 100;

// TTN Configuration
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0xD6, 0xB3, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0xE3, 0x07, 0xAD, 0xBF, 0x80, 0x58, 0xA6, 0x14, 0x89, 0x65, 0xE1, 0x3D, 0xC3, 0x48, 0xBE, 0x1D };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

// payload to send to TTN gateway
static uint8_t payload[5];
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 5;

// Pin mapping for Adafruit Feather M0 LoRa
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 6, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,              // LBT cal for the Adafruit Feather M0 LoRa, in dB
    .spi_freq = 8000000,
};

void setup() {
    acc_timer = millis();

    // Ultrasonic sensor
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(buzzPin, OUTPUT);
    delay(1000);
    while (! Serial);
    Serial.begin(115200);

    // wait for serial port to connect. Needed for native USB port only
    while (!Serial) {
    delay(10);
    }
    Serial.println("Adafruit HX711 Test!");
    // Initialize the HX711
    hx711.begin();
    // read and toss 3 values each
    Serial.println("Tareing....");
    for (uint8_t t=0; t<3; t++) {
    hx711.tareA(hx711.readChannelRaw(CHAN_A_GAIN_128));
    // hx711.tareA(hx711.readChannelRaw(CHAN_A_GAIN_128));
    // hx711.tareB(hx711.readChannelRaw(CHAN_B_GAIN_32));
    // hx711.tareB(hx711.readChannelRaw(CHAN_B_GAIN_32));
    }

    // LMIC init.
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
    // Disable link-check mode and ADR, because ADR tends to complicate testing.
    LMIC_setLinkCheckMode(0);
    // Set the data rate to Spreading Factor 7.  This is the fastest supported rate for 125 kHz channels, and it
    // minimizes air time and battery power. Set the transmission power to 14 dBi (25 mW).
    LMIC_setDrTxpow(DR_SF7,14);
    // in the US, with TTN, it saves join time if we start on subband 1 (channels 8-15). This will
    // get overridden after the join by parameters from the network. If working with other
    // networks or in other regions, this will need to be changed.
    //LMIC_selectSubBand(1);

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {
    // we call the LMIC's runloop processor. This will cause things to happen based on events and time. One
    // of the things that will happen is callbacks for transmission complete or received messages. We also
    // use this loop to queue periodic data transmissions.  You can put other things here in the `loop()` routine,
    // but beware that LoRaWAN timing is pretty tight, so if you do more than a few milliseconds of work, you
    // will want to call `os_runloop_once()` every so often, to keep the radio running.
    os_runloop_once();

}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("artKey: ");
              for (int i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                Serial.print(artKey[i], HEX);
              }
              Serial.println("");
              Serial.print("nwkKey: ");
              for (int i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      Serial.print(nwkKey[i], HEX);
              }
              Serial.println("");
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
      // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:            
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            digitalWrite(LED_BUILTIN, LOW);
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            digitalWrite(LED_BUILTIN, HIGH);
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } 
    else {
        // Read from Channel A with Gain 128, can also try CHAN_A_GAIN_64 or
        CHAN_B_GAIN_32;
        // since the read is blocking this will not be more than 10 or 80 SPS (L or H switch)
        int32_t weightA128 = hx711.readChannelBlocking(CHAN_A_GAIN_128);
        Serial.print("Channel A (Gain 128): ");
        Serial.println(weightA128);
        
        // Read from Channel A with Gain 128, can also try CHAN_A_GAIN_64 or
        // CHAN_B_GAIN_32;
        //   int32_t weightB32 = hx711.readChannelBlocking(CHAN_B_GAIN_32);
        //   Serial.print("Channel B (Gain 32): ");
        //   Serial.println(weightB32);
        if (weightA128 < - 217257){
          //plus ou moins 8.5 kg
          Serial.println("STOP ! TOO MUCH WEIGHT ! CRIMINAL");
          delay(1000);
          digitalWrite(buzzPin, HIGH);
          delay(1000);
          digitalWrite(buzzPin, LOW);
          delay(1000);
          digitalWrite(buzzPin, HIGH);
          delay(1000);
          digitalWrite(buzzPin, LOW);
          delay(1000);
          digitalWrite(buzzPin, HIGH);
          delay(1000);
          digitalWrite(buzzPin, LOW);

        }

        // byte weightLow = lowByte(weightA128);
        // byte weightHigh = highByte(weightA128);
        // payload[0] = weightLow;
        // payload[1] = weightHigh;

        payload[0] = weightA128 & 0xFF;         // Least significant byte
        payload[1] = (weightA128 >> 8) & 0xFF; // Second byte
        payload[2] = (weightA128 >> 16) & 0xFF; // Third byte
        payload[3] = (weightA128 >> 24) & 0xFF; // Most significant byte

        // Ultrasonic sensor send wave
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        duration = pulseIn(echoPin, HIGH);
        distance = (duration*.0343)/2;
        if (distance < 10) {
          Serial.print("Distance is too close !");
          delay(1000);
          digitalWrite(buzzPin, HIGH);
          delay(1000);
          digitalWrite(buzzPin, LOW);
          delay(1000);
          digitalWrite(buzzPin, HIGH);
          delay(1000);
          digitalWrite(buzzPin, LOW);
          delay(1000);
          digitalWrite(buzzPin, HIGH);
          delay(1000);
          digitalWrite(buzzPin, LOW);
        }
        else {
          Serial.print("Distance: ");
          Serial.println(distance);
          delay(100);

          byte fillLow = lowByte(distance);
          byte fillHigh = highByte(distance);
          payload[4] = fillLow;
          payload[5] = fillHigh;
        }
        

        // prepare upstream data transmission at the next possible time.
        // transmit on port 1 (the first parameter); you can use any value from 1 to 223 (others are reserved).
        // don't request an ack (the last parameter, if not zero, requests an ack from the network).
        // Remember, acks consume a lot of network resources; don't ask for an ack unless you really need it.
        LMIC_setTxData2(1, payload, 6, 0);
        
    }
    // Next TX is scheduled after TX_COMPLETE event.
}