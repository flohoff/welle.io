#include <vector>	// std::vector
#include <cstdint>	// uint8_t
#include <memory>	// shared_ptr
#include <list>		// std::list
			//
#include "DABPkt.h"
#include "DABPktConsumer.h"

/*
 * EN 300 401 - 5.3.5.2 - FEC for MSC packet Mod
 * Address: this 10-bit field shall take the binary value "1111111110" (1 022).
 */
#define pktaddress(x)   (((x[0]) & 0x3) << 8 | (x[1]))
/*
 * EN 300 401 - 5.3.5.2 - Packet header Counter b13 .. b10
 */
#define pktcounter(x)   ((x[0] >> 2) & 0xf)

/*
 * EN 300 401 - 5.3.5.2 - Packet header First/Last b11 b10
 */
#define pktfirstlast(x) ((x[0] >> 2) & 0x3)

/*
 * EN 300 401 - 5.3.5.2 - Packet header First/Last b11 b10
 */
#define pktusefullen(x) ((x[2]) & 0x7f)


class ReedSolomon : DABPktConsumer {
	private:
		std::vector<bool>	framereceived;
		std::vector<uint8_t>	buffer;
		std::vector<uint8_t>	processbuffer;

		int corr_pos[10];

		void	*rs_handle;

		std::vector<uint8_t>	fecbuffer;

		int	pktcount;
		std::list<std::shared_ptr<DABPkt>>	pkts;

		DABPktConsumer	&consumer;

		unsigned int	columns;
		unsigned int	rows;
		unsigned int	feccolumns;
		unsigned int	framelength;
		unsigned int	frames;
		unsigned int	pad;

		int	pktvalid;

	private:
		bool pkts_process_fec(void );
	public:
		ReedSolomon(DABPktConsumer &consumer, unsigned int columns, unsigned int rows,
				unsigned int feccolumns, unsigned int framelength,
				unsigned int frames, unsigned int pad);
		~ReedSolomon();
		void input(std::shared_ptr<DABPkt> pkt);
		void pkts_clear(void );
		std::list<std::shared_ptr<DABPkt>> pkt_list(void );
};
