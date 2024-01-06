#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ex_zd.h"
#include "trans.h"
#include "press.h"
#include "streamvbyte/include/streamvbyte.h"

#define ASSERT(statement, ret) \
if (!(statement)) { \
    fprintf(stderr, "line %d: assertion `%s' failed\n", __LINE__, #statement); \
    return ret; \
}

static int ex_press(const uint16_t *in, uint32_t nin, uint8_t **out_ptr,
		    uint64_t *cap_out_ptr, size_t *offset_ptr, uint64_t *nout)
{
	uint32_t nex;
	uint32_t *ex;
	uint8_t *ex_press;
	uint32_t *ex_pos;
	uint32_t *ex_pos_delta;
	uint8_t *ex_pos_press;
	uint64_t nr_press_tmp;
	uint32_t nex_pos_press;
	uint32_t nex_press;
	uint32_t i;
	uint32_t j;

	uint64_t cap_out = *cap_out_ptr;
	uint8_t *out = *out_ptr;
	uint64_t offset = *offset_ptr;

	nex = 0;
	size_t ex_pos_buff_s = UINT16_MAX;
	ex_pos = (uint32_t *) malloc(ex_pos_buff_s * sizeof *ex_pos);
	if (!ex_pos) {
		/*SLOW5_MALLOC_ERROR();
		slow5_errno = SLOW5_ERR_MEM;*/
		return -1;
	}

	ex = (uint32_t *) malloc(ex_pos_buff_s * sizeof *ex);
	if (!ex) {
		free(ex_pos);
		/*SLOW5_MALLOC_ERROR();
		slow5_errno = SLOW5_ERR_MEM;*/
		return -1;
	}

	for (i = 0; i < nin; i++) {
		if (in[i] > UINT8_MAX) {
			ex_pos[nex] = i;
			ex[nex] = in[i] - UINT8_MAX - 1;
			nex++;
			if (nex == 0){
				/*SLOW5_ERROR("ex-zd failed: too many exceptions %d",nex);
				slow5_errno = SLOW5_ERR_PRESS;*/
				free(ex_pos);
				free(ex);
				return -1;
			} else if (nex == ex_pos_buff_s) {
				ex_pos_buff_s *= 2;
				ex_pos = (uint32_t *) realloc(ex_pos, ex_pos_buff_s * sizeof *ex_pos);
				if (!ex_pos) {
					//SLOW5_MALLOC_ERROR();
					free(ex);
					//slow5_errno = SLOW5_ERR_MEM;
					return -1;
				}
				ex = (uint32_t *) realloc(ex, ex_pos_buff_s * sizeof *ex);
				if (!ex) {
					//SLOW5_MALLOC_ERROR();
					free(ex_pos);
					//slow5_errno = SLOW5_ERR_MEM;
					return -1;
				}
			}
		}
	}

	/*
	if(nex > nin/10){
		SLOW5_WARNING("ex-zd: %d Exceptions out of %d samples. Compression may not be ideal.",nex,nin);
	}
	*/

	ASSERT(cap_out - offset >= sizeof nex, -1);
	(void) memcpy(out+offset, &nex, sizeof nex);
	offset += sizeof nex;

	if (nex > 1) {

		//exception positions

		ex_pos_delta = delta_increasing_u32(ex_pos, nex);
		if(!ex_pos_delta){
			free(ex_pos);
			free(ex);
			return -1;
		}

		nr_press_tmp = svb_bound(nex);
		ex_pos_press = (uint8_t *)malloc(nr_press_tmp);
		if(!ex_pos_press){
			free(ex_pos_delta);
			free(ex_pos);
			free(ex);
			/*SLOW5_MALLOC_ERROR();
			slow5_errno = SLOW5_ERR_MEM;*/
			return -1;
		}

		svb_press(ex_pos_delta, nex, ex_pos_press, &nr_press_tmp);
		free(ex_pos_delta);
		nex_pos_press = (uint32_t) nr_press_tmp;
		ASSERT(nex_pos_press > 0, -1);

		ASSERT(cap_out - offset >= sizeof nex_pos_press, -1);
		(void) memcpy(out + offset, &nex_pos_press, sizeof nex_pos_press);
		offset += sizeof nex_pos_press;

		ASSERT(cap_out - offset >= nex_pos_press, -1);
		(void) memcpy(out + offset, ex_pos_press, nex_pos_press);
		free(ex_pos_press);
		offset += nex_pos_press;

		//actual exceptions
		nr_press_tmp = svb_bound(nex);
		ex_press = (uint8_t *)malloc(nr_press_tmp);
		if(!ex_press){
		    free(ex_pos);
		    free(ex);
		    /*SLOW5_MALLOC_ERROR();
		    slow5_errno = SLOW5_ERR_MEM;*/
		    return -1;
		}

		svb_press(ex, nex, ex_press, &nr_press_tmp);
		nex_press = (uint32_t) nr_press_tmp;
		ASSERT(nex_press > 0, -1);

		ASSERT(cap_out - offset >= sizeof nex_press, -1);
		(void) memcpy(out + offset, &nex_press, sizeof nex_press);
		offset += sizeof nex_press;

		ASSERT(cap_out - offset >= nex_press, -1);
		(void) memcpy(out + offset, ex_press, nex_press);
		free(ex_press);
		offset += nex_press;

	} else if (nex == 1) {
		ASSERT(cap_out - offset >= nex * sizeof *ex_pos, -1);
		(void) memcpy(out + offset, ex_pos, nex * sizeof *ex_pos);
		offset += nex * sizeof *ex_pos;
		ASSERT(cap_out - offset >= nex * sizeof *ex, -1);
		(void) memcpy(out + offset, ex, nex * sizeof *ex);
		offset += nex * sizeof *ex;
	}
	free(ex);

	j = 0;
	for (i = 0; i < nin; i++) {
		if (j < nex && i == ex_pos[j]) {
			j++;
		} else {
			ASSERT(cap_out - offset >= 1, -1);
			(void) memcpy(out + offset, in + i, 1);
			offset++;
		}
	}

	free(ex_pos);

	*nout = offset-*offset_ptr;
	*offset_ptr = offset;
	return 0;
}

static int ex_depress(const uint8_t *in, uint64_t nin, uint16_t *out, uint64_t *nout)
{
	uint32_t nex;
	uint32_t *ex;
	uint32_t *ex_pos;
	uint8_t *ex_pos_press;
	uint8_t *ex_press;
	uint32_t nex_press;
	uint32_t nex_pos_press;
	uint32_t i;
	uint32_t j;
	uint64_t offset;

	(void) memcpy(&nex, in, sizeof nex);
	offset = sizeof nex;
	ex_pos = malloc(nex * sizeof *ex_pos);
	if(!ex_pos){
		/*SLOW5_MALLOC_ERROR();
		slow5_errno = SLOW5_ERR_MEM;*/
		return -1;
	}

	if (nex > 0) {
		ex = malloc(nex * sizeof *ex);
		if(!ex){
			free(ex_pos);
			/*SLOW5_MALLOC_ERROR();
			slow5_errno = SLOW5_ERR_MEM;*/
			return -1;
		}

		if (nex > 1) {

			(void) memcpy(&nex_pos_press, in + offset, sizeof nex_pos_press);
			offset += sizeof nex_pos_press;

			ex_pos_press = malloc(nex_pos_press);
			if(!ex_pos_press){
				free(ex_pos);
				free(ex);
				/*SLOW5_MALLOC_ERROR();
				slow5_errno = SLOW5_ERR_MEM;*/
				return -1;
			}

			(void) memcpy(ex_pos_press, in + offset, nex_pos_press);
			offset += nex_pos_press;

			int ret = streamvbyte_decode(ex_pos_press, ex_pos, nex);
			if (ret !=nex_pos_press){
				/*SLOW5_ERROR("Expected streamvbyte_decode to read '%d' bytes, instead read '%d' bytes.",
				        nex_pos_press, ret);
				slow5_errno = SLOW5_ERR_PRESS;*/
				free(ex_pos_press);
				free(ex_pos);
				free(ex);
				return -1;
			}

			free(ex_pos_press);
			undelta_inplace_increasing_u32(ex_pos, nex);

			(void) memcpy(&nex_press, in + offset, sizeof nex_press);
			offset += sizeof nex_press;

			ex_press = malloc(nex_press);
			if(!ex_press){
				free(ex_pos);
				free(ex);
				/*SLOW5_MALLOC_ERROR();
				slow5_errno = SLOW5_ERR_MEM;*/
				return -1;
			}

			(void) memcpy(ex_press, in + offset, nex_press);
			offset += nex_press;


			ret = streamvbyte_decode(ex_press, ex, nex);
			if (ret != nex_press){
				/*SLOW5_ERROR("Expected streamvbyte_decode to read '%d' bytes, instead read '%d' bytes.",
				        nex_press, ret);
				slow5_errno = SLOW5_ERR_PRESS;*/
				free(ex_press);
				free(ex_pos);
				free(ex);
				return -1;
			}
			free(ex_press);
		} else if (nex == 1) {
			(void) memcpy(ex_pos, in + offset, nex * sizeof *ex_pos);
			offset += nex * sizeof *ex_pos;
			(void) memcpy(ex, in + offset, nex * sizeof *ex);
			offset += nex * sizeof *ex;
		}

		for (i = 0; i < nex; i++) {
			out[ex_pos[i]] = ex[i] + UINT8_MAX + 1;
		}
		free(ex);
	}

	i = 0;
	j = 0;
	while (offset < nin || j < nex) {
		if (j < nex && i == ex_pos[j]) {
			j++;
		} else {
			out[i] = in[offset];
			offset++;
		}

		i++;
	}

	free(ex_pos);
	*nout = i;

	return 0;
}

static int ex_zd_press_16(const int16_t *in, uint32_t nin, uint8_t **out_ptr,
			  uint64_t *cap_out_ptr, size_t *offset_ptr, uint64_t *nout)
{

	uint8_t *out = *out_ptr;
	uint64_t cap_out = *cap_out_ptr;
	size_t offset = *offset_ptr;

	uint16_t *in_zd;
	uint64_t nout_tmp;

	in_zd = zigdelta_16_u16(in, nin);
	if(!in_zd){
		return -1;
	}

	size_t sz = sizeof *in_zd;
	ASSERT(cap_out - offset >= sz, -1);
	memcpy(out + offset, in_zd, sz);
	offset += sz;

	nout_tmp = cap_out - offset;
	int ret = ex_press(in_zd + 1, nin - 1, &out, &cap_out, &offset, &nout_tmp);
	if(ret<0){
		free(in_zd);
		return -1;
	}

	*nout = nout_tmp + sz;
	*offset_ptr = offset;
	free(in_zd);
	return 0;
}

static int ex_zd_depress_16(const uint8_t *in, uint64_t nin, int16_t *out,
			    uint64_t *nout)
{
	uint16_t *out_zd;
	uint64_t nout_tmp;

	out_zd = (uint16_t *) malloc(nin * sizeof *out_zd);
	if(!out_zd){
		/*SLOW5_MALLOC_ERROR();
		slow5_errno = SLOW5_ERR_MEM;*/
		return -1;
	}
	(void) memcpy(out_zd, in, sizeof *out_zd);

	nout_tmp = nin - 1;
	int ret = ex_depress(in + sizeof *out_zd, nin - sizeof *out_zd,
			     out_zd + 1, &nout_tmp);
	if(ret<0){
		free(out_zd);
		return -1;
	}
	*nout = nout_tmp + 1;

	unzigdelta_u16_16(out_zd, *nout, out);
	free(out_zd);

	return 0;
}

static inline uint8_t find_qts_of_sample(int16_t s, uint8_t max){
    uint8_t q = max;
    while(q){
        uint16_t mask = (1 << q) - 1;
        if(!(s & mask)) {
            break;
        }
        q--;
    }
    return q;
}

static inline uint8_t find_qts(const int16_t *s, uint64_t n, uint8_t max){
    uint8_t q = max;
    for(uint64_t i=0; i<n; i++){
        q = find_qts_of_sample(s[i], q);
        //fprintf(stderr,"Possible q=%d\n", q);
        if(!q) {
            //fprintf(stderr,"q foudn to %d st element %d\n", q,i);
            break;
        }
    }
    return q;
}

static inline int16_t *do_qts(const int16_t *s, uint64_t n, uint8_t q){
    int16_t *out = (int16_t *) malloc(n * sizeof *out);
    if(!out){
        /*SLOW5_MALLOC_ERROR();
        slow5_errno = SLOW5_ERR_MEM;*/
        return NULL;
    }
    for(uint64_t i=0; i<n; i++){
        out[i] = s[i] >> q;
    }
    return out;
}

uint8_t findo_qts(const int16_t *s, uint64_t n, uint8_t max, int16_t **qs) {
	uint8_t q;

	q = find_qts(s, n, max);
	if (q)
		*qs = do_qts(s, n, q);
	else
		*qs = (int16_t *) s;
	return q;
}

void do_rev_qts_inplace(int16_t *s, uint64_t n, uint8_t q){
    for(uint64_t i=0; i<n; i++){
        s[i] = s[i] << q;
    }
}

void do_rev_qts_inplace_u16(uint16_t *s, uint64_t n, uint8_t q){
    for(uint64_t i=0; i<n; i++){
        s[i] = s[i] << q;
    }
}

void do_rev_qts_inplace_32(int32_t *s, uint64_t n, uint8_t q){
    for(uint64_t i=0; i<n; i++){
        s[i] = s[i] << q;
    }
}

void do_rev_qts_inplace_u32(uint32_t *s, uint64_t n, uint8_t q){
    for(uint64_t i=0; i<n; i++){
        s[i] = s[i] << q;
    }
}

uint8_t *ptr_compress_ex_zd_v0(const int16_t *ptr, size_t count, size_t *n)
{
	uint64_t nin = count / sizeof *ptr;
	const int16_t *in = ptr;

	uint8_t exzd_ver = 0;
	uint8_t q;

	uint64_t cap_out_vb = count + 1024; //heuristic
	size_t offset = 0;
	uint8_t *out_vb = (uint8_t *) malloc(cap_out_vb);
	if (!out_vb) {
		/*SLOW5_MALLOC_ERROR();
		slow5_errno = SLOW5_ERR_MEM;*/
		return NULL;
	}

	size_t sz = sizeof exzd_ver;
	ASSERT(cap_out_vb - offset >= sz, NULL);
	memcpy(out_vb, &exzd_ver, sz);
	offset += sz;

	sz = sizeof nin;
	ASSERT(cap_out_vb - offset >= sz, NULL);
	memcpy(out_vb+offset, &nin, sz);
	offset += sz;

	q = find_qts(in, nin, MAX_QTS_SEARCH);
	int16_t *q_in = NULL;
	if(q){
		q_in = do_qts(in, nin, q);
		in = q_in;
	}
	sz = sizeof q;
	ASSERT(cap_out_vb - offset >= sz, NULL);
	memcpy(out_vb+offset, &q, sz);
	offset += sz;
	//fprintf(stderr,"here q=%d\n", q);

	uint64_t nout_vb = 0;
	int ret = ex_zd_press_16(in, nin, &out_vb, &cap_out_vb, &offset, &nout_vb);
	if(ret < 0){
		free(out_vb);
		free(q_in);
		return NULL;
	}
	free(q_in);

	ASSERT(cap_out_vb >= offset, NULL);
	ASSERT(offset == nout_vb + sizeof nin + sizeof q + sizeof exzd_ver, NULL); //nout_vb is redundant, can be removed

	*n = offset;

	return out_vb;
}

static int16_t *ptr_depress_ex_zd_v0(const uint8_t *ptr, size_t count, size_t *n){

    uint64_t nout;
    uint64_t offset = 0;
    size_t sz = sizeof nout;
    memcpy(&nout, ptr+offset, sz);
    offset += sz;

    int16_t *out = (int16_t *) malloc(nout*sizeof *out);
    if(!out){
        /*SLOW5_MALLOC_ERROR();
        slow5_errno = SLOW5_ERR_MEM;*/
        return NULL;
    }

    uint8_t q = 0;
    sz = sizeof q;
    memcpy(&q, ptr+offset, sz);
    offset += sz;

    ASSERT(q <= 5, NULL);

    int ret = ex_zd_depress_16(ptr+offset, count-offset, out, &nout);
    if(ret <0 ){
        free(out);
        return NULL;
    }

    if(q){
        do_rev_qts_inplace(out, nout, q);
    }

    *n = nout * sizeof *out;
    return out;
}

int16_t *ptr_depress_ex_zd(const uint8_t *ptr, size_t count, size_t *n){

    uint64_t offset = 0;
    uint8_t exzd_ver = 0;
    int16_t *out;

    ASSERT(count >= sizeof exzd_ver, NULL);

    size_t sz = sizeof exzd_ver;
    memcpy(&exzd_ver, ptr, sz);
    offset += sz;
    if(exzd_ver == 0){
        out = ptr_depress_ex_zd_v0(ptr+offset, count-sz, n);
    } else {
        /*SLOW5_ERROR("Unsupported exzd version %d. Try a new version of slow5lib/slow5tools", exzd_ver);
        slow5_errno = SLOW5_ERR_PRESS;*/
        return NULL;
    }

    return out;
}
