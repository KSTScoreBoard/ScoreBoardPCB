// use twelite mwx c++ template library
#include <TWELITE>
#include <NWK_SIMPLE>
#include "ST7032.h"
#include <stdio.h>

/*** Config part */
// application ID
const uint32_t APP_ID = 0x1234abcd;

// channel
const uint8_t CHANNEL = 13;


/*** function prototype */
void vTransmit(uint16_t score);

/*** application defs */
// packet message
const int MSG_LEN = 4;
const char MSG_PING[] = "PING";
const char MSG_PONG[] = "PONG";

ST7032 lcd;

/*** setup procedure (run once at cold boot) */
void setup() {
	lcd.begin(8,2);
	lcd.setContrast(25);
	lcd.write('O');
	lcd.write('K');
	/*** SETUP section */
	pinMode(12,INPUT_PULLUP);
	pinMode(13,INPUT_PULLUP);
	Buttons.setup(5);
	Buttons.begin((1UL << 12) | (1UL << 13),5,10);

	Buttons.setup(5); // init button manager with 5 history table.

	// the twelite main class
	the_twelite
		<< TWENET::appid(APP_ID)    // set application ID (identify network group)
		<< TWENET::channel(CHANNEL) // set channel (pysical channel)
		<< TWENET::rx_when_idle();  // open receive circuit (if not set, it can't listen packts from others)

	// Register Network
	auto&& nwksmpl = the_twelite.network.use<NWK_SIMPLE>();
	nwksmpl << NWK_SIMPLE::logical_id(0xFE) // set Logical ID. (0xFE means a child device with no ID)
	        << NWK_SIMPLE::repeat_max(3);   // can repeat a packet up to three times. (being kind of a router)


	the_twelite.begin(); // start twelite!

	/*** INIT message */
	Serial << "--- PingPong sample (press 't' to transmit) ---" << mwx::crlf;
}

/*** loop procedure (called every event) */
int _score = 0;
void loop() {
	if (Buttons.available()) {
        uint32_t bm, cm;
        Buttons.read(bm, cm);

        if ((bm >> 12 & 1) == LOW){		
			delay(1);
			/*
			Wire.beginTransmission(120);
			Wire.write(0xF0);
			Wire.write(0xF1);
			Wire.endTransmission();
			*/

		Serial << "A" << mwx::crlf;
			uint8_t data[8];
			int len = Wire.requestFrom(1, 2);
			data[0] = Wire.read();
			data[1] = Wire.read();
			delay(10);
			
			char str[32];   
			sprintf(str,"B1 %d\n%x %x",len,data[0],data[1]);
			lcd.print(str);
			

		}
		if ((bm >> 13 & 1) == LOW){
			char str[32];   
			sprintf(str,"B2");
			lcd.print(str);
		}
    }
}

/**
 * @brief	Transmits the given message.
 *
 *          The packet data structure is:
 *            uint8_t[4] : the message "PING" or "PONG"
 *            uint16_t   : A1=ADC0 adc value [0..1023 (max=2470mV)]
 *            uint16_t   : Vcc [mV]
 *            uint32_t   : system timer count in ms.
 *
 * @param msg	the message with length of MSG_LEN.
 * @param addr	destination address
 *
 */
void vTransmit(uint16_t score) {
	Serial << "vTransmit()" << mwx::crlf;

	if (auto&& pkt = the_twelite.network.use<NWK_SIMPLE>().prepare_tx_packet()) {
		// set tx packet behavior
		pkt << tx_addr(0xFF)  // 0..0xFF (LID 0:parent, FE:child w/ no id, FF:LID broad cast), 0x8XXXXXXX (long address)
			<< tx_retry(0x3) // set retry (0x3 send four times in total)
			<< tx_packet_delay(100,200,20); // send packet w/ delay (send first packet with randomized delay from 100 to 200ms, repeat every 20ms)

		// prepare packet payload
		pack_bytes(pkt.get_payload() // set payload data objects.
		,score
		);
	
		// do transmit 
		pkt.transmit();
	}
}

/**
 * @brief receive packet callback
 * 
 * @param rx      received data object.
 * @param handled DO NOT CHANGE, in system use.
 */
void on_rx_packet(packet_rx& rx, bool_t &handled) {
	// rx >> Serial; // debugging (display longer packet information)

	uint8_t buff[2];

	// expand packet payload (shall match with sent packet data structure, see pack_bytes())
	expand_bytes(rx.get_payload().begin(), rx.get_payload().end()
			,buff
	);
	
	char str[32];
	sprintf(str,"%03d %03d\n%d",buff[0],buff[1],(buff[0]<<8)+buff[1]);
	lcd.print(str);
}

/* Copyright (C) 2019-2021 Mono Wireless Inc. All Rights Reserved. *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE     *
 * AGREEMENT).                                                     */