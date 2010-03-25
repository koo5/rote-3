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
#include "errno.h"
#include "inject_csi.h"
#include "rote.h"
#include "roteprivate.h"
#include <stdlib.h>
#define gboolean int
#define G_BEGIN_DECLS
#define G_END_DECLS
#include "signal.h"
#include <pty.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "sys/ioctl.h"
#define ROTE_VT_UPDATE_ITERATIONS 55
#define true 1
#define false 0
#include <termios.h>
#include "term.h"

#ifdef WINDOWS
struct timeval {
  time_t      tv_sec;
  suseconds_t tv_usec;
};
#endif

void tt_winsize(RoteTerm *rt,int fd, int xx, int yy)
{
    struct winsize  ws;
//    printf ("winsize\n");
    if (fd < 0)
	return;

    if((xx==rt->cols)&&(yy==rt->rows))
	return;
    if(xx<1)xx=1;
    if(yy<1)yy=1;
	
    ws.ws_col = (unsigned short)xx;
    ws.ws_row = (unsigned short)yy;
#ifndef __CYGWIN32__
    ws.ws_xpixel = ws.ws_ypixel = 0;
#endif

    ioctl(fd, TIOCGWINSZ, &ws);
//    printf ("ioctlin from %i %i %i %i \n",ws.ws_col, ws.ws_row, ws.ws_xpixel, ws.ws_ypixel);
    ws.ws_col = (unsigned short)xx;
    ws.ws_row = (unsigned short)yy;
    ioctl(fd, TIOCSWINSZ, &ws);
//    printf ("ioctled to %i %i %i %i \n",ws.ws_col, ws.ws_row, ws.ws_xpixel, ws.ws_ypixel);
//    if ((ws.ws_col==xx)&&(ws.ws_row==yy))

	int or=rt->rows;
	int oc=rt->cols;
	int i;
	rt->cols=xx;
	rt->rows=yy;

//        printf ("W\n");    
	for (i = yy; i < or; i++)
	{//	printf(";;%i;;\n", i);
    		free(rt->cells[i]);
    	}
	if(or != yy)
	{
		rt->line_dirty = (bool*) realloc(rt->line_dirty, sizeof(bool) * rt->rows);
		rt->cells = (RoteCell**) realloc(rt->cells, sizeof(RoteCell*) * rt->rows);
	}
	for (i = or; i < yy; i++)
	{
		rt->line_dirty[i] = 1;
    		rt->cells[i] = (RoteCell*) malloc(sizeof(RoteCell) * rt->cols);    
    	}

    if(oc != xx)
    {
//        printf ("X\n");    
	for (i = 0; i < yy; i++)
	{
    	    rt->cells[i] = (RoteCell*) realloc(rt->cells[i], sizeof(RoteCell) * rt->cols);
//    	    printf("resized row %i at address %i\n", i, rt->cells[i]);
    	}
//    	printf("QUACK!\n");
    }
    rt->scrollbottom+=yy;
    if(rt->scrollbottom<0)rt->scrollbottom=0;
    if (rt->ccol>=rt->cols)
	rt->ccol=rt->cols-1;
    if (rt->crow>=rt->rows)
	rt->crow=rt->rows-1;
    if (rt->scrolltop>=rt->rows)
	rt->scrolltop=rt->rows-1;
    if (rt->scrollbottom>=rt->rows)
	rt->scrollbottom=rt->rows-1;
//    if(rt->childpid>1)
//    {
//	int pgrp;
//	if(ioctl(fd,TIOCGPGRP,&pgrp)!=-1)
//	kill(-pgrp, SIGWINCH);//rt->childpid
//    }
//i dunno how to make this work
}

void rote_vt_resize(RoteTerm *rt,int rows, int cols)
{
//    printf("resize %i\n",rt->pd);
    tt_winsize(rt, rt->pd->pty,cols,rows);
}

RoteTerm *rote_vt_create(int rows, int cols) {
   RoteTerm *rt;
   int i, j;
   setupterm("linux", 1, (int *)0);

   if (rows <= 0 || cols <= 0) return NULL;

   if (! (rt = (RoteTerm*) malloc(sizeof(RoteTerm))) ) return NULL;
   memset(rt, 0, sizeof(RoteTerm));

   /* record dimensions */
   rt->rows = rows;
   rt->cols = cols;

   rt->log=0;
   rt->logl=0;
   rt->logstart=0;
    rt->cursorhidden=0;
   /* create the cell matrix */
   rt->cells = (RoteCell**) malloc(sizeof(RoteCell*) * rt->rows);
   for (i = 0; i < rt->rows; i++) {
      /* create row */
      rt->cells[i] = (RoteCell*) malloc(sizeof(RoteCell) * rt->cols);

      /* fill row with spaces */
      for (j = 0; j < rt->cols; j++) {
         rt->cells[i][j].ch = 0x20;    /* a space */
         rt->cells[i][j].attr = 0x70;  /* white text, black background */
      }
   }
   
   /* allocate dirtiness array */
   rt->line_dirty = (bool*) malloc(sizeof(bool) * rt->rows);

   /* initialization of other public fields */
   rt->crow = rt->ccol = 0;
   rt->curattr = 0x70;  /* white text over black background */

   /* allocate private data */
   rt->pd = (RoteTermPrivate*) malloc(sizeof(RoteTermPrivate));
   memset(rt->pd, 0, sizeof(RoteTermPrivate));
   rt->pd->pty = -1;  /* no pty for now */

   /* initial scrolling area is the whole window */
   rt->scrolltop = 0;
   rt->scrollbottom = rt->rows - 1;
   rt->docellmouse=0;
//   #ifdef DEBUG
//   fprintf(stderr, "Created a %d x %d terminal.\n", rt->rows, rt->cols);
//   #endif
   
   return rt;
}

void rote_vt_destroy(RoteTerm *rt) {
   int i;
   if (!rt) return;

   free(rt->pd);
   free(rt->line_dirty);
   for (i = 0; i < rt->rows; i++) free(rt->cells[i]);
   free(rt->cells);
   if(rt->log)
   {
    for (i = 0; i < rt->logl; i++) if(rt->log[i])free(rt->log[i]);
    free(rt->log);
   }
   free(rt);
}
/*
static void default_cur_set_attr(WINDOW *win, unsigned char attr) {
   int cp = ROTE_ATTR_BG(attr) * 8 + 7 - ROTE_ATTR_FG(attr);
   if (!cp) wattrset(win, A_NORMAL);
   else     wattrset(win, COLOR_PAIR(cp));

   if (ROTE_ATTR_BOLD(attr))     wattron(win, A_BOLD);
   if (ROTE_ATTR_BLINK(attr))    wattron(win, A_BLINK);
}

static inline unsigned char ensure_printable(unsigned char ch) 
                                        { return ch >= 32 ? ch : 32; }

void rote_vt_draw(RoteTerm *rt, WINDOW *win, int srow, int scol, 
                                void (*cur_set_attr)(WINDOW*,unsigned char)) {

   int i, j;
   rote_vt_update(rt);
   
   if (!cur_set_attr) cur_set_attr = default_cur_set_attr;
   for (i = 0; i < rt->rows; i++) {
      wmove(win, srow + i, scol);
      for (j = 0; j < rt->cols; j++) {
         (*cur_set_attr)(win, rt->cells[i][j].attr);
         waddch(win, ensure_printable(rt->cells[i][j].ch));
      }
   }

   wmove(win, srow + rt->crow, scol + rt->ccol);
}
*/
pid_t rote_vt_forkpty(RoteTerm *rt, const char *command) {
   struct winsize ws;
   pid_t childpid;
   
   ws.ws_row = rt->rows;
   ws.ws_col = rt->cols;
   ws.ws_xpixel = ws.ws_ypixel = 0;

   childpid = forkpty(&rt->pd->pty, NULL, NULL, &ws);
   if (childpid < 0) return -1;

   if (childpid == 0) {
      /* we are the child, running under the slave side of the pty. */

      /* Cajole application into using linux-console-compatible escape
       * sequences (which is what we are prepared to interpret) */
      setenv("TERM", "linux", 1);

      /* Now we will exec /bin/sh -c command. */
      execl("/bin/sh", "/bin/sh", "-c", command, NULL);

//      fprintf(stderr, "\nexecl() failed.\nCommand: '%s'\n", command);
      exit(127);  /* error exec'ing */
   }

   /* if we got here we are the parent process */
   rt->childpid = childpid;
   return childpid;
}

void rote_vt_forsake_child(RoteTerm *rt) {
   if (rt->pd->pty >= 0) close(rt->pd->pty);
   rt->pd->pty = -1;
   rt->childpid = 0;
}



void rote_vt_update(RoteTerm *rt) {
   fd_set ifs;
   struct    timeval tvzero;
   char buf[512];
   int bytesread;
   int n = ROTE_VT_UPDATE_ITERATIONS;
   if (rt->pd->pty < 0) return;  /* nothing to pump */

   while (n--) { /* iterate at most ROtE_VT_UPDATE_ITERATIONS times.
                  * As Phil Endecott pointed out, if we don't restrict this,
                  * a program that floods the terminal with output
                  * could cause this loop to iterate forever, never
                  * being able to catch up. So we'll rely on the client
                  * calling rote_vt_update often, as the documentation
                  * recommends :-) */

      /* check if pty has something to say */
      FD_ZERO(&ifs); FD_SET(rt->pd->pty, &ifs);
      tvzero.tv_sec = 0; tvzero.tv_usec = 0;
      if (select(rt->pd->pty + 1, &ifs, NULL, NULL, &tvzero) <= 0)
         return; /* nothing to read, or select() failed */

      /* read what we can. This is guaranteed not to block, since
       * select() told us there was something to read. */
      bytesread = read(rt->pd->pty, buf, 512);
      if (bytesread <= 0) return;   

      /* inject the data into the terminal */
      rote_vt_inject(rt, buf, bytesread);
   }
}
		    //?//
void rote_vt_update_thready(char * buf, int bs, int * br, RoteTerm *rt) {
   fd_set ifs;
   struct timeval tvzero;

   int bytesread;
//   int n = ROTE_VT_UPDATE_ITERATIONS;
   if (rt->pd->pty < 0) return;  /* nothing to pump */

      /* check if pty has something to say */
      FD_ZERO(&ifs); FD_SET(rt->pd->pty, &ifs);
      tvzero.tv_sec = 0; tvzero.tv_usec = 0;
      int res;
      if ((res = select(rt->pd->pty + 1, &ifs, NULL, NULL, NULL) <= 0))
       return; /* nothing to read, or select() failed */
      /* read what we can. This is guaranteed not to block, since
       * select() told us there was something to read. */
      bytesread = read(rt->pd->pty, buf, 512);
      if((bytesread < 0) && (errno != EAGAIN))
      {
             errno=0;
    	    rote_vt_forsake_child(rt);
//    	    printf("dfsdfdfsHAHA!/n");
      }
      if (bytesread <= 0) return;
      *br=bytesread;
}


void rote_vt_write(RoteTerm *rt, const char *data, int len) {
   if (rt->pd->pty < 0) {
      /* no pty, so just inject the data plain and simple */
      rote_vt_inject(rt, data, len);
      return;
   }

   /* write data to pty. Keep calling write() until we have written
    * everything. */
   while (len > 0) {
      int byteswritten = write(rt->pd->pty, data, len);
      if (byteswritten < 0) {
         /* very ugly way to inform the error. Improvements welcome! */
         static char errormsg[] = "\n(ROTE: pty write() error)\n";
         rote_vt_inject(rt, errormsg, strlen(errormsg));
         return;
      }

      data += byteswritten;
      len  -= byteswritten;
   }
}

void rote_vt_install_handler(RoteTerm *rt, rote_es_handler_t handler) {
   rt->pd->handler = handler;
}
/*g++
void *rote_vt_take_snapshot(RoteTerm *rt) {
   int i;
   int bytes_per_row = sizeof(RoteCell) * rt->cols;
   void *buf = malloc(bytes_per_row * rt->rows);
   void *ptr = buf;

   for (i = 0; i < rt->rows; i++, ptr += bytes_per_row)
      memcpy(ptr, rt->cells[i], bytes_per_row);

   return buf;
}

void rote_vt_restore_snapshot(RoteTerm *rt, void *snapbuf) {
   int i;
   int bytes_per_row = sizeof(RoteCell) * rt->cols;

   for (i = 0; i < rt->rows; i++, snapbuf += bytes_per_row) {
      rt->line_dirty[i] = true;
      memcpy(rt->cells[i], snapbuf, bytes_per_row);
   }
}
*/
int rote_vt_get_pty_fd(RoteTerm *rt) {
   return rt->pd->pty;
}


void rote_vt_mousez(RoteTerm*vt,char s, int x, int y)
{
    if (vt->docellmouse)
    {
	printf("docellmouse\n");
	if(x<0)return;
	if(y<0)return;
	if(x>vt->cols)return;
	if(y>vt->rows)return;
	
        char xy[6];
        xy[0]=27;
        xy[1]='[';
        xy[2]='M';
        xy[3]=s;
        xy[4]=33+x;
        xy[5]=33+y;
        rote_vt_write(vt, xy, 6);//thx void
    }
}

void rote_vt_mousedown(RoteTerm *vt, int x, int y)
{
    rote_vt_mousez(vt,' ',x,y);
}
void rote_vt_mouseup(RoteTerm *vt, int x, int y)
{
    rote_vt_mousez(vt,'#',x,y);
}
void rote_vt_mousemove(RoteTerm *vt, int x, int y)
{
    rote_vt_mousez(vt,'@',x,y);
}


char * rotoclipin(int sel)
{
    char *buf;
    RoteTerm *t;
    t = rote_vt_create(10,10);
    if(sel)
        rote_vt_forkpty(t, "xclip -o -selection clipboard");
    else
        rote_vt_forkpty(t, "xclip -o");
    int br=0;
    buf=malloc(512);
    rote_vt_update_thready(buf, 6, &br,t);
    //?
    buf[br]=0;
    rote_vt_forsake_child(t);
    rote_vt_destroy(t);
    if(br>0)return buf;
    else { free(buf);return     0;}
}

void rotoclipout(char * x, RoteTerm *t, int selection)
{
    if(selection ==3)
	rote_vt_forkpty(t, "xclip -i -selection clipboard");
    else if(selection ==2)
	rote_vt_forkpty(t, "xclip -i -selection secondary");
    else if(selection ==1)
	rote_vt_forkpty(t, "xclip -i");
    rote_vt_write(t,x,strlen(x));
    rote_vt_write(t,"\4\4",strlen("\4\4"));
    rote_vt_update(t);
}



