/*
LICENSE INFORMATION:
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License (LGPL) as published by the Free Software Foundation.

Please refer to the COPYING file for more information.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

Copyright (c) 2004 Bruno T. C. de Oliveira
*/


#include "rote.h"
#include "curses.h"
#include "term.h"
#include <stdlib.h>
#include <string.h>

RoteTerm *rit;


void rote_vt_keypress(RoteTerm *rt, int i)
{
    char c=(char)i;
    if(rt)
    {

    rote_vt_write(rt, &c, 1); /* not special, just write it */
    }
//    else printf("no rt\n");
}


int _rtput(int i)
{
    char c=(char)i;
    rote_vt_write(rit,&c,1);
    return 1;
}


void rote_vt_terminfo(RoteTerm *rt, const char *c)
{
    if(rt)
    {
    rit=rt;
    char *cc=strdup(c);
    char *u=tigetstr(cc);
    free(cc);
    if (u)
    {
	//if (u==(char *)-1)
//	    printf("bubak\n");
	//else
	{
	//    printf("%s\n",u);
	    tputs(u,1,_rtput);
	}
    }
    }
//    else printf("no rt\n");
}




