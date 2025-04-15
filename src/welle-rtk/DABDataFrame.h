#ifndef DABDATAFRAME_H
#define DABDATAFRAME_H

#include <vector>
#include <cstdint>
#include <iostream>

#include "hexdump.hpp"
#include "backend/dab-constants.h"
#include "backend/tools.h"

#include <memory>
#include "DABPkt.h"

class DABDataFrame {
	protected:
		std::vector<uint8_t>	buffer;
	public:
		DABDataFrame(void )  {
		};

		void append(const std::shared_ptr<DABPkt> pkt) {
			size_t len=pkt->data_len();
			const std::vector<uint8_t> &a=pkt->data_vector();

			auto begin=std::begin(a);
			std::advance(begin, 3);

			auto end=std::begin(a);
			std::advance(end, 3+len);

			std::copy(begin, end, std::back_inserter(buffer));
		}

		std::size_t size(void ) {
			return buffer.size();
		}

		uint8_t *data(void ) {
			return buffer.data();
		}

		uint16_t dg_crc(void ) {
			uint8_t *dptr=buffer.data()+buffer.size()-2;
			return dptr[0]<<8|dptr[1];
		}

		bool dg_crc_correct(void ) {
			uint16_t ccrc=CalcCRC::CalcCRC_CRC16_CCITT.Calc((const uint8_t *) buffer.data(), (size_t) buffer.size()-2);
			return (dg_crc() == ccrc);
		}

#define DG_EXTENSION_FLAG	0x80
#define DG_CRC_FLAG		0x40
#define DG_SEGMENT_FLAG		0x20
#define DG_USERACCESS_FLAG	0x10

		bool dg_has_crc(void ) {
			/* EN 300 401 V2.1.1 5.3.3.0 */
			return (buffer.data()[0] & DG_CRC_FLAG) != 0;
		}

		bool dg_has_segment(void ) {
			/* EN 300 401 V2.1.1 5.3.3.0 */
			return (buffer.data()[0] & DG_SEGMENT_FLAG) != 0;
		}

		bool dg_has_useraccess(void ) {
			/* EN 300 401 V2.1.1 5.3.3.0 */
			return (buffer.data()[0] & DG_USERACCESS_FLAG) != 0;
		}

		bool dg_has_extension(void ) {
			/* EN 300 401 V2.1.1 5.3.3.0 */
			return (buffer.data()[0] & DG_EXTENSION_FLAG) != 0;
		}

		uint8_t dg_continuity(void ) {
			/* EN 400 401 V2.1.1 5.3.3.1 */
			return (buffer.data()[1] & 0xf0) >> 4;
		}

		uint8_t dg_repetition(void ) {
			/* EN 400 401 V2.1.1 5.3.3.1 */
			return (buffer.data()[1] & 0xf);
		}

		friend std::ostream& operator<<(std::ostream& out, const DABDataFrame &frame) {
			return out << "Length " << frame.buffer.size() << std::endl
				<< Hexdump((const void *) frame.buffer.data(), frame.buffer.size());
		}
};

std::ostream& operator<<(std::ostream& out, const DABDataFrame &frame);

#endif
