//hey, this is not thread-safe!
#include <stdlib.h>
#include <stdio.h>


unsigned char b[4];
int pos=0;
int len;
#include "wtf.h"
int wtf(unsigned char x)
{
    if (!pos )
    {
        pos=1;
        if ((x&224)==192)
        {
            len=2;
            b[1]=x&31;
        }
        else if ((x&240)==224)
        {
            b[1]=x&15;
            len=3;
        }
        else if ((x&248)==240)
        {
            b[1]=x&3;
            len=4;
        }
        else
        {
            pos=0;//o-0
            len=0;
        }
    }
    else
    {
        pos++;
//	printf("pos %i\n", pos);
        if ((x&192)==128)
            b[pos]=(x&63);
        else pos=0;//printf("hmm\n");
    }
    if (pos==len)
    {
        if (len==2)
        {
            b[pos]=b[pos]|((b[pos-1]&3)<<6);
            b[pos-1]=((b[pos-1]&31) >> 2);
            etf=b[pos]+b[pos-1]*256;
            pos=0;
            return 1;

        }
        else if (len==3)
        {
            b[pos]=b[pos]|((b[pos-1]&3)<<6);
            b[pos-1]=((b[pos-1]&60) >> 2) | ((b[pos-2]&15) << 4);
            etf=b[pos]+b[pos-1]*256;
            pos=0;
            return 1;

        }
        else if (len==4)
        {
            return 1;
        }
        pos=0;
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
		printf("%i\n",etf);
	    if (t=='A') break;
	}
	}


}




*/
