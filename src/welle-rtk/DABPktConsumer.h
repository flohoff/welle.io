#ifndef DABPKTCONSUMER_H
#define DABPKTCONSUMER_H

#include <memory>
#include "DABPkt.h"

class DABPktConsumer {
	public:
		virtual ~DABPktConsumer() {};
		virtual void input(std::shared_ptr<DABPkt> pkt) = 0;
};

#endif
