#ifndef DABDATAFRAMECONSUMER_H
#define DABDATAFRAMECONSUMER_H

#include <memory>
#include "DABDataFrame.h"

class DABDataFrameConsumer {
	public:
		virtual ~DABDataFrameConsumer() {};
		virtual void input(std::shared_ptr<DABDataFrame> frame) = 0;
};

#endif
