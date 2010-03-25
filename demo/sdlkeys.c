#include "../rote.h"
#include "SDL/SDL_keysym.h"
#include <stdio.h>

void sdlkeys(RoteTerm *t, char key, int unicode, int mod)
{
					    if ( (key >= SDLK_F1) && (key <= SDLK_F15) )
					    {
						char *k;
						if(asprintf(&k ,"kf%i", key-SDLK_F1+1)!=-1)
						{
						    rote_vt_terminfo(t, k);
						    free(k);
						}
					    }
					    else
					    if ( (key == SDLK_SPACE) )
						rote_vt_keypress(t,32);
					    else
					    #define magic(x) rote_vt_terminfo(t, x )
					    if ( (key == SDLK_BACKSPACE) )
						magic( "kbs");
					    else
					    if ( (key == SDLK_ESCAPE) )
						rote_vt_keypress(t,27);
					    else
					    if ( (key == SDLK_LEFT) )
						magic("kcub1");
					    else
					    if ( (key == SDLK_RIGHT) )
						magic( "kcuf1");
					    else
					    if ( (key == SDLK_UP) )
						magic( "kcuu1");
					    else
					    if ( (key == SDLK_DOWN) )
						magic( "kcud1");
					    else
					    if ( (key == SDLK_END) )
						magic( "kend");
					    else
					    if ( (key == SDLK_HOME) )
						magic("khome");
					    else
					    if ( (key == SDLK_DELETE) )
						magic( "kdch1");
					    else
					    if ( (key == SDLK_PAGEDOWN) )
						magic( "knp");
					    else
					    if ( (key == SDLK_INSERT) )
						magic( "kich1");
					    else
					    if ( (key == SDLK_PAGEUP) )
						magic ( "kpp" );
					    else
					    if ( (key == SDLK_RETURN) )
						rote_vt_keypress(t,13);
					    else
					    if( unicode )//&& ( (event.key.keysym.unicode & 0xFF80) == 0 ) )
					    {
					    	if(mod&KMOD_ALT)
						{
						    char c[2];
						    c[0]=27;
						    c[1]=key;
						    rote_vt_write(t,c,2);
						}
						else
						{
						    printf("%i\n",     unicode);
						    rote_vt_keypress(t,unicode);
						}
					    }
}