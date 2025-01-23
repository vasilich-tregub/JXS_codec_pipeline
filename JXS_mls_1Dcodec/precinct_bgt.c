#include "precinct_bgt.h"

#define ALIGN_TO_BITS(value, nbits) ((value + nbits - 1) & (~(nbits - 1)))

#define GCLI_METHOD_NBITS 2
#define RA_BUDGET_INVALID 0x8000000

const int bands_count = 1;

void precinct_get_budget(//precinct_t* precinct,
	precinct_budget_table_t* pbt,
	int* gtli_table_gcli,
	int* gtli_table_data,
	//const uint8_t Rl,
	//int* res_gcli_sb_methods,
	precinct_budget_info_t* out)
{
	int gtli_gcli, gtli_data;
	int gcli_method;
	int subpkt = 0;
	int raw_budget;
	const int raw_method = 0;// method_get_idx(ALPHABET_RAW_4BITS, PRED_NONE, RUN_NONE); // = 0;
	const bool use_long_precinct_headers = false; // precinct_use_long_headers(precinct); // = false;

	memset(out, 0, sizeof(precinct_budget_info_t));

	out->prec_header_size = ALIGN_TO_BITS(PREC_HDR_PREC_SIZE + PREC_HDR_QUANTIZATION_SIZE + PREC_HDR_REFINEMENT_SIZE + bands_count/*_of(precinct)*/ * GCLI_METHOD_NBITS, PREC_HDR_ALIGNMENT); // = 48;
	out->precinct_bits = out->prec_header_size; // = 48;

	const int position_count = 2; // line_count_of(precinct); // = 2;
	for (int position = 0; position < position_count; position++)
	{
		const int lvl = position; // precinct_band_index_of(precinct, position); // (pos0) = 0; (pos1) = 1;
		/*if (!precinct_is_line_present(precinct, lvl, precinct_ypos_of(precinct, position))) // (pos0) skip; (pos1) skip;
		{
			continue;
		}*/

		subpkt = 0; // precinct_subpkt_of(precinct, position); // (pos0) = 0; (pos1) = 0;
		out->pkt_header_size[subpkt] = ALIGN_TO_BITS(
			use_long_precinct_headers ?
			(PKT_HDR_DATA_SIZE_LONG + PKT_HDR_GCLI_SIZE_LONG + PKT_HDR_SIGN_SIZE_LONG + 1) :
			(PKT_HDR_DATA_SIZE_SHORT + PKT_HDR_GCLI_SIZE_SHORT + PKT_HDR_SIGN_SIZE_SHORT + 1),
			PKT_HDR_ALIGNMENT); // (pos0) {40, 0, ... , 0}; (pos1) {40, 0, ... , 0};
		// gtli_table_gcli and gtli_table_data are zero with our image and configuration but, even for MLS, it may depend of lvl_gains and lvl_priorities,
		// see compute_gtli_tables(*q, *r, bands_count_of(), lvl_gains, lvl_priorities, gtli_table_data, gtli_table_gcli, &empty) call (line 261 rate_control.c)
		gtli_gcli = gtli_table_gcli[lvl]; // (pos0) = 0; (pos1) = 0;
		gtli_data = gtli_table_data[lvl]; // (pos0) = 0; (pos1) = 0;
		gcli_method = 2; // res_gcli_sb_methods[lvl]; // (pos0) = 2; (pos1) = 2;

		out->subpkt_size_sigf[subpkt] += pbt->sigf_budget_table[gtli_gcli]; //pbt_get_sigf_bgt_of(pbt, gcli_method, position)[gtli_gcli]; // (pos0) {0, 0, ... , 0}; (pos1) {0, 0, ... , 0};
		out->subpkt_size_gcli[subpkt] += pbt->gcli_budget_table[gtli_gcli]; //_get_gcli_bgt_of(pbt, gcli_method, position)[gtli_gcli]; // (pos0) {74, 0, ... , 0}; (pos1) {93, 0, ... , 0};
		out->subpkt_size_data[subpkt] += pbt->data_budget_table[gtli_gcli]; //_get_data_bgt_of(pbt, position)[gtli_data]; // Fs setting is handled by data_budget // (pos0) {296, 0, ... , 0}; (pos1) {364, 0, ... , 0};
		out->subpkt_size_sign[subpkt] += pbt->sign_budget_table[gtli_gcli]; //pbt_get_sign_bgt_of(pbt, position)[gtli_data]; // will be always 0 if Fs==0 // (pos0) {0, 0, ... , 0}; (pos1) {0, 0, ... , 0};

		raw_budget = pbt->gcli_budget_table[gtli_gcli]; // _get_gcli_bgt_of(pbt, raw_method, position)[gtli_gcli]; // (pos0) = 40; (pos1) 40;
		if (raw_budget == RA_BUDGET_INVALID)
		{
			out->subpkt_size_gcli_raw[subpkt] = RA_BUDGET_INVALID;
		}
		else if (out->subpkt_size_gcli_raw[subpkt] != RA_BUDGET_INVALID)
		{
			out->subpkt_size_gcli_raw[subpkt] += raw_budget; // (pos0) = {40, 0 ...}; (pos1) {80, 0, ... , 0};
		}
	}
	//for (subpkt = 0; subpkt < precinct_nb_subpkts_of(precinct); subpkt++)
	{
		out->subpkt_size_sigf[subpkt] = ALIGN_TO_BITS(out->subpkt_size_sigf[subpkt], 8); // 0
		out->subpkt_size_gcli[subpkt] = ALIGN_TO_BITS(out->subpkt_size_gcli[subpkt], 8); // 96
		out->subpkt_size_data[subpkt] = ALIGN_TO_BITS(out->subpkt_size_data[subpkt], 8); // 368
		out->subpkt_size_sign[subpkt] = ALIGN_TO_BITS(out->subpkt_size_sign[subpkt], 8); // 80
		out->subpkt_size_gcli_raw[subpkt] = ALIGN_TO_BITS(out->subpkt_size_gcli_raw[subpkt], 8);
	}

	//detect_gcli_raw_fallback(precinct, Rl, out); // adjust subpkt_size_sigf (if (Rl == 0), ..._sigf = 0;), subpkt_size_gcli (=subpkt_size_gcli_raw /*80*/);, and subpkt_uses_raw_fallback (=1);

	//for (subpkt = 0; subpkt < precinct_nb_subpkts_of(precinct); subpkt++)
	{
		out->precinct_bits += out->pkt_header_size[subpkt]; // = {48 + 40} /*88*/, add subpkt_header side (==pkt_header_size) to pkt_header_size VERIFY IF subpkt_header side == pkt_header_size
		out->precinct_bits += out->subpkt_size_sigf[subpkt]; // 88
		out->precinct_bits += out->subpkt_size_gcli[subpkt]; // +subpkt_size_gcli(=80) 168
		out->precinct_bits += out->subpkt_size_data[subpkt]; // 168+368 = 536
		out->precinct_bits += out->subpkt_size_sign[subpkt]; // 536
	}

#ifdef DEBUG_PREC_BUDGET
	fprintf(stderr, "Precinct Budget: precinct_bits=%8d (prec_hdr=%d):", out->precinct_bits, out->prec_header_size);
	for (subpkt = 0; subpkt < precinct_nb_subpkts_of(precinct); subpkt++)
		fprintf(stderr, " pkt%d (hdr=%d gcli=%d sigf=%d data=%d sign=%d)", subpkt, out->pkt_header_size[subpkt], out->subpkt_size_gcli[subpkt], out->subpkt_size_sigf[subpkt], out->subpkt_size_data[subpkt], out->subpkt_size_sign[subpkt]);
	fprintf(stderr, "\n");
#endif
}

precinct_budget_table_t* pbt_open(int position_count, int method_count)
{
	precinct_budget_table_t* pbt = malloc(sizeof(precinct_budget_table_t));
	if (pbt)
	{
		pbt->position_count = position_count;
		pbt->method_count = method_count;

		pbt->sigf_budget_table = (uint32_t*) malloc((MAX_GCLI + 1) * sizeof(uint32_t));// multi_buf_create_and_allocate(pbt->method_count * pbt->position_count, (MAX_GCLI + 1), sizeof(uint32_t));
		pbt->gcli_budget_table = (uint32_t*) malloc((MAX_GCLI + 1) * sizeof(uint32_t));// multi_buf_create_and_allocate(pbt->method_count * pbt->position_count, (MAX_GCLI + 1), sizeof(uint32_t));
		pbt->data_budget_table = (uint32_t*) malloc((MAX_GCLI + 1) * sizeof(uint32_t));// multi_buf_create_and_allocate(pbt->position_count, (MAX_GCLI + 1), sizeof(uint32_t));
		pbt->sign_budget_table = (uint32_t*) malloc((MAX_GCLI + 1) * sizeof(uint32_t));// multi_buf_create_and_allocate(pbt->position_count, (MAX_GCLI + 1), sizeof(uint32_t));

		//assert(pbt->sigf_budget_table);
		//assert(pbt->gcli_budget_table);
		//assert(pbt->data_budget_table);
		//assert(pbt->sign_budget_table);
	}
	return pbt;
}
