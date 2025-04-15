#ifndef DABPKTDATADEDUPE_H
#define DABPKTDATADEDUPE_H

#include <list>
#include "DABPktConsumer.h"

/*
 *
 * We want to deduplicate the stream of packets. As we have a ReedSolomon
 * FEC in front we receive all packets twice.
 *
 * We receive them directly, and again (the same packet object) when
 * FEC has been performed.
 *
 * In the good case only the fec_handled is set to true, but the packet hasnt changed.
 * In the bit error case the CRC/continuity may have been broken before, and is fixed
 * after FEC has run. FEC runs every 2256 bytes (See EN 300 401) so we might end up
 * ~100 packets late.
 *
 * In case we are running fine we simply pass on the packet
 *
 */

// #define DEBUG_DEDUPE 1

class DABPktDataDeDupe : public DABPktConsumer {
	private:
		DABPktConsumer	&consumer;
		bool		fec_enabled=false;
		uint8_t		continuity_last;
		int		seq_last;
		bool		plugged=false;

		std::list<std::shared_ptr<DABPkt>> pkts;
	public:
		~DABPktDataDeDupe() {};
		DABPktDataDeDupe(DABPktConsumer &_consumer) : consumer(_consumer) {};

		uint8_t continuity_expected(void ) {
			return (continuity_last+1)&0x3;
		}

		void queue_plugged(std::shared_ptr<DABPkt> pkt) {
			pkts.push_back(pkt);
		}

		void consumer_send(std::shared_ptr<DABPkt> pkt) {

#ifdef DEBUG_DEDUPE
			std::cout << (int) pkt->seq() << " "
				<< "Forwarding packet " << (int) pkt->seq()
				<< " CRC " << (pkt->crc_correct() ? "Valid" : "Invalid")
				<< " Continuity " << (int) pkt->continuity() << "/" << (int) continuity_expected()
				<< " FEC Handled " << (pkt->fec_handled() ? "Yes" : "No")
				<< std::endl;
#endif
			continuity_last=pkt->continuity();
			seq_last=pkt->seq();
			consumer.input(pkt);

			return;
		}

		void input(std::shared_ptr<DABPkt> pkt) {
			if (plugged) {
				if (pkt->seq() != seq_last)
					return;

#ifdef DEBUG_DEDUPE
				/* We store the sequence number of the pkt we fail */
				std::cout << (int) pkt->seq() << " "
					<< "Unplugging "
					<< " CRC " << (pkt->crc_correct() ? "Valid" : "Invalid")
					<< " Continuity " << (int) pkt->continuity() << "/" << (int) continuity_expected()
					<< " FEC Handled " << (pkt->fec_handled() ? "Yes" : "No")
					<< std::endl;
#endif
				consumer_send(pkt);
				plugged=false;
				return;
			}

			if (pkt->seq() <= seq_last) {
				/* We store the sequence number of the pkt we fail */
#ifdef DEBUG_DEDUPE
				std::cout << (int) pkt->seq() << " "
					<< "Dropping packet (seq) "
					<< " CRC " << (pkt->crc_correct() ? "Valid" : "Invalid")
					<< " Continuity " << (int) pkt->continuity() << "/" << (int) continuity_expected()
					<< " FEC Handled " << (pkt->fec_handled() ? "Yes" : "No")
					<< std::endl;
#endif
				return;
			}

			if (pkt->fec_handled() ||
					(pkt->crc_correct() && pkt->continuity() == continuity_expected())) {
				consumer_send(pkt);
				return;
			}
#ifdef DEBUG_DEDUPE
			/* We store the sequence number of the pkt we fail */
			std::cout << (int) pkt->seq() << " "
				<< "Plugging " 
				<< " CRC " << (pkt->crc_correct() ? "Valid" : "Invalid")
				<< " Continuity " << (int) pkt->continuity() << "/" << (int) continuity_expected()
				<< " FEC Handled " << (pkt->fec_handled() ? "Yes" : "No")
				<< std::endl;
#endif
			seq_last=pkt->seq();
			plugged=true;

			return;
		};
};

#endif
