
#include <stdexcept>
#include <iostream>
#include <string.h>
#include "reedsolomon.h"
#include "backend/tools.h"


extern "C" {
#include <fec.h>
}

#ifdef RSDEBUG
static void dump_hex(const char *prefix, uint8_t *buf, int size, int cols) {
	int		i;
	unsigned char	ch;
	char		sascii[cols+1];
	char		linebuffer[cols*4+1];

	sascii[cols]=0x0;

	for(i=0;i<size;i++) {
		ch=buf[i];
		if (i%cols == 0) {
			sprintf(linebuffer, "%04x ", i);
		}
		sprintf(&linebuffer[(i%cols)*3], "%02x ", ch);
		if (ch >= ' ' && ch <= '}')
			sascii[i%cols]=ch;
		else
			sascii[i%cols]='.';

		if (i%cols == (cols-1))
			printf("%s %s  %s\n", prefix, linebuffer, sascii);
	}

	/* i++ after loop */
	if (i%cols != 0) {
		for(;i%cols != 0;i++) {
			sprintf(&linebuffer[(i%cols)*3], "   ");
			sascii[i%cols]=' ';
		}

		printf("%s %s  %s\n", prefix, linebuffer, sascii);
	}
}
#endif

ReedSolomon::ReedSolomon(DABPktConsumer &consumer, unsigned int columns, unsigned int rows,
			unsigned int feccolumns, unsigned int framelength,
			unsigned int frames, unsigned int pad) :
	consumer(consumer), columns(columns),rows(rows),feccolumns(feccolumns),framelength(framelength),frames(frames),pad(pad) {

	fecbuffer.resize(feccolumns*rows);
	buffer.resize(rows*columns);
	processbuffer.resize(rows*(columns+feccolumns));

	/*
	 * Symbol size 8 bit
	 * Poly 0x11d
	 * 16 bytes per row RS / FEC bytes
	 * 0 padding (We do it before we decode)
	 */
	rs_handle = init_rs_char(8, 0x11d, 0, 1, 16, 0);

	if(!rs_handle)
		throw std::runtime_error("RSDecoder: error while init_rs_char");

	/* Start at the beginning */
	pktvalid=0;
}

ReedSolomon::~ReedSolomon() {
	free_rs_char(rs_handle);
}

std::list<std::shared_ptr<DABPkt>> ReedSolomon::pkt_list(void ) {
	return pkts;
}

bool ReedSolomon::pkts_process_fec(void ) {
	uint8_t		rstable[rows][columns+feccolumns];
	int		dptr=0;		/* Data ptr */
	int		fecpkts=0;
	int		pktbytes=0;
	int		pktcount=0;

#define FEC_PKT_HDR_LENGTH	2
#define FEC_PKT_BYTES		22

	memset(rstable, 0, rows*(columns+feccolumns));

	for (auto &pkt : pkts) {
		uint8_t	*pbuf=pkt->data();

		if (pkt->is_fec()) {
			/*
			 * FEC packets (should be 9 in our buffer)
			 * Must be interleaved into columns from column 239 on
			 */
			int poff=pkt->fec_count()*FEC_PKT_BYTES;
			for(int i=0;i<FEC_PKT_BYTES;i++)
				rstable[(poff+i) % rows][columns + (poff+i) / rows]=pbuf[FEC_PKT_HDR_LENGTH+i];

			fecpkts++;
		} else {
			/* Overflowing buffer? */
			if (dptr+pkt->size() > rows*columns) {
				pkts_clear();
				return false;
			}

			/* Data packet - interleave into columns */
			for(size_t i=0;i<pkt->size();i++) {
				rstable[dptr % rows][pad + dptr / rows]=pbuf[i];
				dptr++;
			}
			pktbytes+=pkt->size();
			pktcount++;
		}
	}

#define FEC_EXACT_BYTES	2256

	if (pktbytes != FEC_EXACT_BYTES) {
		std::cerr << "Unable to run FEC - did not receive all packets" << std::endl;
		return false;
	}

#if 1 // #ifdef RSDEBUG || 1
	std::cout << "FEC pkts " << pktcount
		<< " bytes " << pktbytes
		<< " FEC packets " << fecpkts << std::endl;
#endif

	for(unsigned int r=0;r<rows;r++) {
		int corr_count=decode_rs_char(rs_handle, rstable[r], corr_pos, 0);
		/*
		 * We need to copy back to packet buffers in case of corrected bytes
		 * As we copyied them interleaved into the rows we need to walk through
		 * again and if it matches to the corrected position copy back the byte.
		 *
		 */

		if (corr_count < 0) {
			std::cerr << "Uncorrectable errors in FEC" << std::endl;
		}

		// FIXME - Mark packets which may contain uncorrectable errors
		for(int i=0;i<corr_count;i++) {
			dptr=0;
			unsigned int cpos=corr_pos[i];

			for (auto &pkt : pkts) {
				uint8_t	*pbuf=pkt->data();

				/* Data packet - interleave into columns */
				for(size_t j=0;j<pkt->size();j++) {
					if ((dptr % rows == r) && ((pad + dptr / rows) == cpos)) {
						pbuf[j]=rstable[dptr % rows][pad + dptr / rows];
						pkt->fec_bytes_inc();
					}
					dptr++;
				}
			}
		}
	}

	return true;
}

void ReedSolomon::pkts_clear(void ) {
	pkts.clear();
	pktcount=0;
}

void ReedSolomon::input(std::shared_ptr<DABPkt> pkt) {
	pktcount++;
	pkts.push_back(pkt);

	/* Immediatly push pakets further */
	consumer.input(pkt);

	/* We need to issue FEC if we have all 9 FEC frames (0-8) */
	if (pkt->is_fec() && pkt->fec_count() == 8) {
#ifdef RSDEBUG
		std::cout << "Got last fec packet" << std::endl;
#endif
		/* After successful FEC push packets to consumer */
		// FIXME Regardless of the return code - push packets to
		// allow the assumption that WITH FEC all packets appear twice
		if (pkts_process_fec()) {
			auto pktlist=pkts;
			for (auto pkt : pktlist) {
				pkt->fec_handled_set(true);
				consumer.input(pkt);
			}
		}

		pkts_clear();

		return;
	}

	/* Just a safety measure - possibly no FEC frames so we overflow memory */
	if (pktcount > 200) {
#ifdef RSDEBUG
		std::cerr << "Zapping packet list - noone consumed?" << std::endl;
#endif
		pkts_clear();
	}

	return;
}
