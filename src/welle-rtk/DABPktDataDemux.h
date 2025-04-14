#ifndef DABPKTDATADEMUX_H
#define DABPKTDATADEMUX_H

#include <map>
#include "DABPktConsumer.h"

class DABPktDataDemux : public DABPktConsumer {
	private:
		std::map<uint16_t,DABPktConsumer &> sinks;
	public:
		~DABPktDataDemux() {};
		void input(std::shared_ptr<DABPkt> pkt) {
			if (pkt->is_empty())
				return;

			uint16_t addr=pkt->address();

			try {
				DABPktConsumer &consumer=sinks.at(addr);
				consumer.input(pkt);
			} catch (const std::out_of_range& ex) {
			};
		};

		void demux_add(uint16_t address, DABPktConsumer &consumer) {
			sinks.emplace(address, consumer);
		};

		void demux_remove(uint16_t address) {
			sinks.erase(address);
		};
};

#endif
