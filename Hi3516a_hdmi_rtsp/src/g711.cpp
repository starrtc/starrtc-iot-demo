#include "g711.h"

#define         SIGN_BIT        (0x80)      /* Sign bit for a A-law byte. */
#define         QUANT_MASK      (0xf)       /* Quantization field mask. */
#define         NSEGS           (8)         /* Number of A-law segments. */
#define         SEG_SHIFT       (4)         /* Left shift for segment number. */
#define         SEG_MASK        (0x70)      /* Segment field mask. */
#define         BIAS            (0x84)      /* Bias for linear code. */

int alaw2linear(unsigned char a_val)
{
        int t;
        int seg;

        a_val ^= 0x55;

        t = a_val & QUANT_MASK;
        seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
        if(seg) t= (t + t + 1 + 32) << (seg + 2);
        else    t= (t + t + 1     ) << 3;

        return (a_val & SIGN_BIT) ? t : -t;
}

void build_xlaw_table(byte *linear_to_xlaw,
                             int (*xlaw2linear)(unsigned char),
                             int mask)
{
    int i, j, v, v1, v2;

    j = 0;
    for(i=0;i<128;i++) {
        if (i != 127) {
            v1 = xlaw2linear(i ^ mask);
            v2 = xlaw2linear((i + 1) ^ mask);
            v = (v1 + v2 + 4) >> 3;
        } else {
            v = 8192;
        }
        for(;j<v;j++) {
            linear_to_xlaw[8192 + j] = (i ^ mask);
            if (j > 0)
                linear_to_xlaw[8192 - j] = (i ^ (mask ^ 0x80));
        }
    }
    linear_to_xlaw[0] = linear_to_xlaw[1];
}

static byte linear_to_alaw[16384];

static short decode_alaw_table[256];

void g711_init(){
	build_xlaw_table(linear_to_alaw, alaw2linear, 0xd5);

    for(int i=0;i<256;i++){
        decode_alaw_table[i] = alaw2linear(i);
	}
}

void g711_encode(byte * pcm, size_t pcm_size, byte * g711){
	short *samples = (short *)pcm;
	
    for(int n = pcm_size / 2; n > 0; n--) {
        int v = *samples++;
        *g711++ = linear_to_alaw[(v + 32768) >> 2];
    }
}

void g711_decode(byte * g711, size_t g711_size, byte * pcm){
	short *samples = (short *)pcm;
	
	for(int n = g711_size; n > 0; n--) {
		*samples++ = decode_alaw_table[*g711++];
	}
}
