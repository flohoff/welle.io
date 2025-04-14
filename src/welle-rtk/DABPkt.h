#ifndef DABPKT_H
#define DABPKT_H

#include <vector>
#include <cstdint>
#include <iostream>

#include "hexdump.hpp"
#include "backend/dab-constants.h"
#include "backend/tools.h"


/*
 * EN 300 401 - 5.3.5.2 - FEC for MSC packet Mod
 * Address: this 10-bit field shall take the binary value "1111111110" (1 022).
 */
#define pktaddress(x)   (((x[0]) & 0x3) << 8 | (x[1]))

/*
 * EN 300 401 - 5.3.5.2 - Packet header Counter b13 .. b10
 */
#define pktcounter(x)   ((x[0] >> 2) & 0xf)


class DABPkt {
	private:
		std::vector<uint8_t>	buffer;
		bool			fechandled=false;
		uint8_t			fecbytes=0;

	public:
		DABPkt(const std::vector<uint8_t> &bits) {
			const uint8_t	*bitbuffer=bits.data();

			buffer.resize(bits.size() / 8);

			for (std::size_t i=0;i<bits.size()/8;i++) {
				uint8_t k=0;
				for (int j = 0; j < 8; j ++) {
					k=k<<1|(bitbuffer[8 * i + j] & 01);
				}
				buffer[i]=k;
			}
		};

		std::size_t size(void ) {
			return buffer.size();
		}

		uint8_t *data(void ) {
			return buffer.data();
		}

		void fec_handled_set(bool state) {
			fechandled=state;
		}

		bool fec_handled(void ) {
			return fechandled;
		}

		uint8_t	fec_bytes(void ) {
			return fecbytes;
		}

		uint8_t fec_bytes_inc(void ) {
			return fecbytes++;
		}

		bool crc_correct(void ) {
			return (crc_calc() == crc());
		}

		uint16_t crc_calc(void ) {
			uint16_t crc=CalcCRC::CalcCRC_CRC16_CCITT.Calc((const uint8_t *) buffer.data(), (size_t) buffer.size()-2);
			return crc;
		}

		uint16_t crc(void ) {
			uint8_t	*dptr=buffer.data()+buffer.size()-2;

			return dptr[0]<<8|dptr[1];
		}

		/*
		 * EN 300 401 - 5.3.5.2 - FEC for MSC packet Mod
		 * Address: this 10-bit field shall take the binary value "1111111110" (1 022).
		 */
		bool is_fec(void ) {
			return (pktaddress(buffer.data()) == 0x3fe);
		};

		uint16_t address(void ) {
			return pktaddress(buffer.data());
		}

		bool is_empty(void ) {
			return (pktaddress(buffer.data()) == 0);
		}

		short fec_count(void ) {
			return pktcounter(buffer.data());
		};


		friend std::ostream& operator<<(std::ostream& out, const DABPkt &pkt) {
			return out << "Length " << pkt.buffer.size() << std::endl
				<< Hexdump((const void *) pkt.buffer.data(), pkt.buffer.size());
		}
};

std::ostream& operator<<(std::ostream& out, const DABPkt &pkt);

#endif
