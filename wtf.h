
#ifndef etfh
#define etfh
typedef struct
{
    int etff;
    unsigned char b[4];
    int pos;
    int len;
} wtfdecoder;

int wtf(unsigned char x, wtfdecoder *w);

#endif
