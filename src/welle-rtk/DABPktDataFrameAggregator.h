#ifndef DABPKTDATAFRAMEAGGREGATOR_H
#define DABPKTDATAFRAMEAGGREGATOR_H

#include "DABPktConsumer.h"

class DABPktDataFrameAggregator : public DABPktConsumer {
	private:
	public:
		~DABPktDataFrameAggregator() {};
		void input(std::shared_ptr<DABPkt> pkt) {
			std::cout << "Address " << pkt->address()
				<< " FEC handled " << (pkt->fec_handled() ? "Yes" : "No")
				<< " CRC " << (pkt->crc_correct() ? "Valid" : "Invalid")
				<< std::endl;
		}
};

#endif
