#ifndef DABPKT_H
#define DABPKT_H

#include <vector>
#include <cstdint>

#include "backend/dab-constants.h"

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
		uint8_t			feccorrectedbytes=0;

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

		uint8_t	corrected(void ) {
			return feccorrectedbytes;
		}

		uint8_t corrected_increase(void ) {
			return feccorrectedbytes++;
		}

		/*
		 * EN 300 401 - 5.3.5.2 - FEC for MSC packet Mod
		 * Address: this 10-bit field shall take the binary value "1111111110" (1 022).
		 */
		bool is_fec(void ) {
			return (pktaddress(buffer.data()) == 0x3fe);
		};

		short fec_count(void ) {
			return pktcounter(buffer.data());
		};
};
#endif
