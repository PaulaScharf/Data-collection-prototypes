// #include <lmic.h>
// #include <hal/hal.h>
// #include <SPI.h>
// char* slave_name = "slv1";

// #define TX_INTERVAL 2000

// #define VCC_ENABLE 41

// // Pin mapping
// const lmic_pinmap lmic_pins = {
//   .nss = 34,
//   .rxtx = LMIC_UNUSED_PIN,
//   .rst = LMIC_UNUSED_PIN,
//   .dio = {33, 33, LMIC_UNUSED_PIN},
// };


// // These callbacks are only used in over-the-air activation, so they are
// // left empty here (we cannot leave them out completely unless
// // DISABLE_JOIN is set in arduino-lmoc/project_config/lmic_project_config.h,
// // otherwise the linker will complain).
// void os_getArtEui (u1_t* buf) { }
// void os_getDevEui (u1_t* buf) { }
// void os_getDevKey (u1_t* buf) { }

// void onEvent (ev_t ev) {
// }

// osjob_t txjob;
// osjob_t timeoutjob;

// // Transmit the given string and call the given function afterwards
// void tx(const char *str, osjobcb_t func) {
//   os_radio(RADIO_RST); // Stop RX first
//   delay(1); // Wait a bit, without this os_radio below asserts, apparently because the state hasn't changed yet
//   LMIC.dataLen = 0;
//   while (*str)
//     LMIC.frame[LMIC.dataLen++] = *str++;
//   LMIC.osjob.func = func;
//   os_radio(RADIO_TX);
// }

// // Enable rx mode and call func when a packet is received
// void rx(osjobcb_t func) {
//   LMIC.osjob.func = func;
//   LMIC.rxtime = os_getTime(); // RX _now_
//   // Enable "continuous" RX (e.g. without a timeout, still stops after
//   // receiving a packet)
//   os_radio(RADIO_RXON);
// }

// void setup_lora() {
//   // LMIC initialize runtime env
//   pinMode(VCC_ENABLE, OUTPUT);
//   digitalWrite(VCC_ENABLE, LOW);
//   delay(1000);

//   // initialize runtime env
//   os_init();
//   // Use a frequency in the g3 which allows 10% duty cycling.
//   LMIC.freq = 869525000;
//   // Use a medium spread factor. This can be increased up to SF12 for
//   // better range, but then, the interval should be (significantly)
//   // raised to comply with duty cycle limits as well.
//   LMIC.datarate = DR_SF9;
//   // Maximum TX power
//   LMIC.txpow = 27;


//   // disable RX IQ inversion
//   LMIC.noRXIQinversion = true;

//   // This sets CR 4/5, BW125 (except for EU/AS923 DR_SF7B, which uses BW250)
//   LMIC.rps = updr2rps(LMIC.datarate);

//   Serial.print("Frequency: "); Serial.print(LMIC.freq / 1000000);
//             Serial.print("."); Serial.print((LMIC.freq / 100000) % 10);
//             Serial.print("MHz");
//   Serial.print("  LMIC.datarate: "); Serial.print(LMIC.datarate);
//   Serial.print("  LMIC.txpow: "); Serial.println(LMIC.txpow);

//   // This sets CR 4/5, BW125 (except for DR_SF7B, which uses BW250)
//   LMIC.rps = updr2rps(LMIC.datarate);

//   // disable RX IQ inversion
//   LMIC.noRXIQinversion = true;
// }
