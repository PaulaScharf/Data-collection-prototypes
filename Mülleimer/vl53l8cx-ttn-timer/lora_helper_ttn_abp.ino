// /*******************************************************************************
//  * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
//  * Copyright (c) 2018 Terry Moore, MCCI
//  *
//  * Permission is hereby granted, free of charge, to anyone
//  * obtaining a copy of this document and accompanying files,
//  * to do whatever they want with them without any restriction,
//  * including, but not limited to, copying, modification and redistribution.
//  * NO WARRANTY OF ANY KIND IS PROVIDED.
//  *
//  * This example sends a valid LoRaWAN packet with payload "Hello,
//  * world!", using frequency and encryption settings matching those of
//  * the The Things Network.
//  *
//  * This uses OTAA (Over-the-air activation), where where a DevEUI and
//  * application key is configured, which are used in an over-the-air
//  * activation procedure where a DevAddr and session keys are
//  * assigned/generated for use with all further communication.
//  *
//  * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
//  * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
//  * violated by this sketch when left running for longer)!

//  * To use this sketch, first register your application and device with
//  * the things network, to set or generate an AppEUI, DevEUI and AppKey.
//  * Multiple devices can use the same AppEUI, but each device has its own
//  * DevEUI and AppKey.
//  *
//  * Do not forget to define the radio type correctly in
//  * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
//  *
//  *******************************************************************************/
// #include <lmic.h>
// #include <hal/hal.h>
// #include <SPI.h>

// #include <CayenneLPP.h> // http://librarymanager/All#CayenneLPP

// CayenneLPP lpp(51);

// #define VCC_ENABLE 41

// // LoRaWAN NwkSKey, network session key
// // This should be in big-endian (aka msb).
// static const PROGMEM u1_t NWKSKEY[16] = { 0x07, 0x47, 0xB9, 0xD6, 0xE7, 0x3A, 0x76, 0xA6, 0x92, 0x0A, 0x14, 0xAD, 0x87, 0x47, 0x96, 0x4E };

// // LoRaWAN AppSKey, application session key
// // This should also be in big-endian (aka msb).
// static const u1_t PROGMEM APPSKEY[16] = { 0x36, 0x2D, 0xFB, 0xD6, 0x4F, 0x30, 0xCF, 0xE5, 0xD3, 0xD4, 0xBC, 0xA6, 0xDB, 0x1C, 0x0F, 0x68 };

// // LoRaWAN end-device address (DevAddr)
// // See http://thethingsnetwork.org/wiki/AddressSpace
// // The library converts the address to network byte order as needed, so this should be in big-endian (aka msb) too.
// static const u4_t DEVADDR = 0x260BED30 ; // <-- Change this address for every node!

// // These callbacks are only used in over-the-air activation, so they are
// // left empty here (we cannot leave them out completely unless
// // DISABLE_JOIN is set in arduino-lmic/project_config/lmic_project_config.h,
// // otherwise the linker will complain).
// void os_getArtEui (u1_t* buf) { }
// void os_getDevEui (u1_t* buf) { }
// void os_getDevKey (u1_t* buf) { }

// // Schedule TX every this many seconds (might become longer due to duty
// // cycle limitations).
// const unsigned TX_INTERVAL = 60;

// // Pin mapping
// const lmic_pinmap lmic_pins = {
//   .nss = 34,
//   .rxtx = LMIC_UNUSED_PIN,
//   .rst = LMIC_UNUSED_PIN,
//   .dio = {33, 33, LMIC_UNUSED_PIN},
// };
// void printHex2(unsigned v) {
//     v &= 0xff;
//     if (v < 16)
//         Serial.print('0');
//     Serial.print(v, HEX);
// }

// void do_send(osjob_t* j, float data){
//     // Check if there is not a current TX/RX job running
//     if (LMIC.opmode & OP_TXRXPEND) {
//         Serial.println(F("OP_TXRXPEND, not sending"));
//     } else {
//         lpp.reset();
//         lpp.addAnalogInput(1, (float)data);

//         // Prepare upstream data transmission at the next possible time.
//         LMIC_setTxData2(1, lpp.getBuffer(), lpp.getSize(), 0);
//         Serial.println(F("Packet queued"));
//     }
//     // Next TX is scheduled after TX_COMPLETE event.
// }

// void setup_lora() {
//   // LMIC initialize runtime env
//   pinMode(VCC_ENABLE, OUTPUT);
//   digitalWrite(VCC_ENABLE, LOW);
//   delay(1000);

//   // LMIC init
//   os_init();
//   // Reset the MAC state. Session and pending data transfers will be discarded.
//   LMIC_reset();
//   // On AVR, these values are stored in flash and only copied to RAM
//   // once. Copy them to a temporary buffer here, LMIC_setSession will
//   // copy them into a buffer of its own again.
//   uint8_t appskey[sizeof(APPSKEY)];
//   uint8_t nwkskey[sizeof(NWKSKEY)];
//   memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
//   memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
//   LMIC_setSession (0x13, DEVADDR, nwkskey, appskey);

//   // Set up the channels used by the Things Network, which corresponds
//   // to the defaults of most gateways. Without this, only three base
//   // channels from the LoRaWAN specification are used, which certainly
//   // works, so it is good for debugging, but can overload those
//   // frequencies, so be sure to configure the full frequency range of
//   // your network here (unless your network autoconfigures them).
//   // Setting up channels should happen after LMIC_setSession, as that
//   // configures the minimal channel set. The LMIC doesn't let you change
//   // the three basic settings, but we show them here.
//   // LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//   // LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
//   // LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//   // LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//   // LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//   // LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//   // LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//   // LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
//   // LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

//   // Disable link check validation
//   LMIC_setLinkCheckMode(0);

//   // TTN uses SF9 for its RX2 window.
//   LMIC.dn2Dr = DR_SF9;

//   // Set data rate and transmit power for uplink
//   LMIC_setDrTxpow(DR_SF7,14);
// }
