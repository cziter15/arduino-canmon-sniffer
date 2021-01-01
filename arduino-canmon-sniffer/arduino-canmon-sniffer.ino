/*-----------------------------------------------------------------*/
//	Copyright (c) Krzysztof Strehlau
//	
//	This code has been written to sniff and emulate CAN packets
//	by using CAN Monitor-qt project avaiable at github:
//	https://github.com/tixiv/CAN-Monitor-qt
//
//	It emulates only basic packets of CanAdapterChina packets and
//	it does not support changing any CAN parameters on demand.
//
//	CAN bus speed can be changed below - see CAN BUS SETUP section.
//
//	Keep in mind that some ESP32 devkits do not support 2000000
//	serial speed. CP21XX based ones can be reconfigured to support
//	faster rates by using CP21xxCustomizationUtility from S. Labs.
//	
//	Requirements:
//	[Arduino-CAN]	https://github.com/sandeepmistry/arduino-CAN
/*-----------------------------------------------------------------*/
#include <CAN.h>

/*------------------------- CAN BUS SETUP -------------------------*/
#define CAN_TX_PIN		4				// << TRANSCEIVER TX
#define CAN_RX_PIN		5				// << TRANSCEIVER RX
#define CAN_BUS_SPEED	125E3			// << CAN BUS SPEED
/*------------------------- DO NOT MODIFY -------------------------*/
#define SERIAL_SPEED	2000000			// << CONST CANMON SPED
/*-----------------------------------------------------------------*/

/*--------------------- Checksum utility class --------------------*/
class utils
{
	private:
		static uint8_t calcChecksum(uint8_t(&packet)[20])
		{
			uint8_t checksumm = 0;
			for (int i = 2; i < 19; i++)
			{
				checksumm += (uint8_t)packet[i];
			}

			return checksumm;
		}
	public:
		static bool verifyChecksum(uint8_t(&packet)[20])
		{
			return packet[19] == calcChecksum(packet);
		}

		static void updateChecksum(uint8_t(&packet)[20])
		{
			packet[19] == calcChecksum(packet);
		}
};
/*-----------------------------------------------------------------*/
//	Arduino setup function.
//	Start Serial and CAN interfaces.
/*-----------------------------------------------------------------*/
void setup()
{
	Serial.begin(SERIAL_SPEED);
	CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
	CAN.begin(CAN_BUS_SPEED);
}

//	Arduino loop function.
//	Handle CAN and Serial communication.
/*-----------------------------------------------------------------*/
void loop() 
{
	if (CAN.parsePacket())
	{
		uint8_t packet[20] = {
			0xAA, 0x55,										// << magic
			0x01, 											// << type (CAN = 1)
			(uint8_t)(CAN.packetExtended() ? 0x02 : 0x01),	// << ext
			(uint8_t)(CAN.packetRtr() ? 0x02 : 0x01),		// << rtr
			(uint8_t)((CAN.packetId() >> 0) & 0xff),		// << read ID byte 1
			(uint8_t)((CAN.packetId() >> 8) & 0xff),		// << read ID byte 2
			(uint8_t)((CAN.packetId() >> 16) & 0xf),		// << read ID byte 3
			(uint8_t)((CAN.packetId() >> 24) & 0xf),		// << read ID byte 4,
			(uint8_t)CAN.packetDlc()						// << CAN DLC / size
															// ... and the rest 10 bytes here
		};

		CAN.readBytes(&packet[10], CAN.packetDlc());
		utils::updateChecksum(packet);
		Serial.write(packet, sizeof(packet));
	}

	if (Serial.available() > 2)								// << skip "C\r"
	{
		uint8_t packet[20];
		Serial.readBytes(packet, sizeof(packet));

		if (
			packet[0] == 0xAA && packet[1] == 0x55 &&		// << magic check here (check above)
			packet[2] == 0x01)								// << type (CAN = 1)
		{
			if (utils::verifyChecksum(packet))				// << verify packet checksum
			{
				uint8_t DLC = packet[9];					// << read DLC (data length)

				int id = ((uint8_t)packet[5]) << 0;			// << assemble ID byte 1
				id |= ((uint8_t)packet[6]) << 8;			// << assemble ID byte 2
				id |= ((uint8_t)packet[7]) << 16;			// << assemble ID byte 3
				id |= ((uint8_t)packet[8]) << 24;			// << assemble ID byte 4

				CAN.beginPacket(id, DLC, false);			// << assemble ID
				CAN.write(&packet[10], DLC);				// << write data
				CAN.endPacket();							// << end packet
			}
		}
	}
}