#include <stdlib.h>
#include <stdio.h>


#include "wtf.h"
int wtf(unsigned char x, wtfdecoder *w)
{
    if (!w->pos )
    {
        w->pos=1;
        if ((x&224)==192)
        {
            w->len=2;
            w->b[1]=x&31;
        }
        else if ((x&240)==224)
        {
            w->b[1]=x&15;
            w->len=3;
        }
        else if ((x&248)==240)
        {
            w->b[1]=x&3;
            w->len=4;
        }
        else
        {
            w->pos=0;//o-0
            w->len=0;
        }
    }
    else
    {
        w->pos++;
//	printf("pos %i\n", pos);
        if ((x&192)==128)
            w->b[w->pos]=(x&63);
        else w->pos=0;//printf("hmm\n");
    }
    if (w->pos==w->len)
    {
        if (w->len==2)
        {
            w->b[w->pos]=w->b[w->pos]|((w->b[w->pos-1]&3)<<6);
            w->b[w->pos-1]=((w->b[w->pos-1]&31) >> 2);
            w->etff=w->b[w->pos]+w->b[w->pos-1]*256;
            w->pos=0;
            return 1;

        }
        else if (w->len==3)
        {
            w->b[w->pos]=w->b[w->pos]|((w->b[w->pos-1]&3)<<6);
            w->b[w->pos-1]=((w->b[w->pos-1]&60) >> 2) | ((w->b[w->pos-2]&15) << 4);
            w->etff=w->b[w->pos]+w->b[w->pos-1]*256;
            w->pos=0;
            return 1;

        }
        else if (w->len==4)
        {
    	    w->pos=0;
            return 1;
        }
        w->pos=0;
    }
    if(w->pos>4)
    {
	w->pos=0;
	w->len=0;
    }
    return 0;
}

/*
int main(int argc, char *argv[])
{

	FILE *g;
	g= fopen("./e", "r");
	if(g)
	{
	unsigned char t;
	while (t=fgetc(g))
	{
	    printf("%i ",t);
	    if(wtf(t))
		printf("%i\n",etff);
	    if (t=='A') break;
	}
	}


}




*/
