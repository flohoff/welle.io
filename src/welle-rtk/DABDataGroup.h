#ifndef DABDATAGROUP_H
#define DABDATAGROUP_H

#include "DABDataFrame.h"
#include "DABDataFrameConsumer.h"
#include <ios>
#include <fstream>

class DABDataGroup : public DABDataFrameConsumer {
	private:
	public:
		~DABDataGroup() {};

		void input(std::shared_ptr<DABDataFrame> frame) {
			if (frame->dg_has_segment()) {
				std::cerr << "DABDGDataFrame does not support segments" << std::endl;
				return;
			}

			if (frame->dg_has_useraccess()) {
				std::cerr << "DABDGDataFrame does not support useraccess" << std::endl;
				return;
			}

			if (frame->dg_has_crc()) {
				if (!frame->dg_crc_correct()) {
					std::cerr << "DABDGDataFrame CRC mismatch of frame" << std::endl;
					return;
				}
			}

			std::cout << "Continuity " << (int) frame->dg_continuity()
				<< " Repetition " << (int) frame->dg_repetition()
				<< std::endl;
			std::cout << *frame << std::endl;

			{
				uint8_t *dptr=frame->data();

				std::ofstream fout;
				fout.open("file.bin", std::ios_base::out | std::ios_base::binary | std::ios::app);
				fout.seekp(std::ios_base::end);
				fout.write((const char *) dptr+2, frame->size()-4);
				fout.close();
			}
		}
};

#endif
