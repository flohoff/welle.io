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
	private:
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

		friend std::ostream& operator<<(std::ostream& out, const DABDataFrame &frame) {
			return out << "Length " << frame.buffer.size() << std::endl
				<< Hexdump((const void *) frame.buffer.data(), frame.buffer.size());
		}
};

std::ostream& operator<<(std::ostream& out, const DABDataFrame &frame);

#endif
