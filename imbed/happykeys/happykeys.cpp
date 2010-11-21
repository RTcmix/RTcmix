/*  happykeys -- fun RTcmix interface

This is from the original RTsocket version README file:

 * happykeys -- a simple demo of an RTcmix interface
 * uses the STRUM instrument, fires off notes when keys are
 * pressed.  The ASCII keyboard is set up sort-of like a piano
 * keyboard, with "a" being the C below middle C, "w" being the C#
 * above that, "s" being the D... etc.
 *
 * This application uses the "curses" library to allow us to capture
 * key presses with no carriage returns
 * ESC will exit 

    demonstration of the RTcmix object -- BGG, 11/2002
*/

#define MAIN
#include <stdio.h>
#include <curses.h>
#include <fcntl.h>
#include <unistd.h>

#include <globals.h>
#include "RTcmix.h"

int
main(int argc, char *argv[])
{
	RTcmix *rrr;
	int term;
	int thechar;
	int i;
	double start, pval, spread;

	rrr = new RTcmix(44100.0, 2, 256);
	rrr->printOff();
	sleep(1); // give the thread time to initialized

	rrr->cmd("load", 1, "STRUM");


	/* set up the terminal for 'raw' character capturing */
	term = open("/dev/tty", O_RDWR);

	/* set up to grab the keys with no newline */
	initscr();
	cbreak();

	addstr("type ESC to quit\n");
	while(1) {
		thechar = getch();

		switch (thechar) {
			case 'a':
				pval = 7.00;
				spread = 1.00;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'w':
				pval = 7.01;
				spread = 0.99;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 's':
				pval = 7.02;
				spread = 0.87;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'e':
				pval = 7.03;
				spread = 0.87;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'd':
				pval = 7.04;
				spread = 0.78;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'f':
				pval = 7.05;
				spread = 0.69;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 't':
				pval = 7.06;
				spread = 0.69;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'g':
				pval = 7.07;
				spread = 0.56;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'y':
				pval = 7.08;
				spread = 0.56;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'h':
				pval = 7.09;
				spread = 0.49;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'u':
				pval = 7.10;
				spread = 0.39;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'j':
				pval = 7.09;
				pval = 7.11;
				spread = 0.39;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'k':
				pval = 8.00;
				spread = 0.32;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'o':
				pval = 8.01;
				spread = 0.32;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'l':
				pval = 8.02;
				spread = 0.24;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'p':
				pval = 8.03;
				spread = 0.24;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 59:
				pval = 8.04;
				spread = 0.15;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 39:
				pval = 8.05;
				spread = 0.08;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case ']':
				pval = 8.06;
				spread = 0.08;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 10:
				pval = 8.07;
				spread = 0.0;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 92:
				pval = 8.08;
				spread = 0.0;
				rrr->cmd("START", 8, 0.0, 1.0, pval, 1.0, 0.7, 7000.0, 1.0, spread);
				break;
			case 'x':
				pval = 7.00;
				start = 1.0;
				for (i = 0; i < 25; i = i+1) {
					spread = 49.0/(float)(i+1);
					rrr->cmd("START", 8, start, 0.05, pval, 1.0, 0.7, 1000.0, 2.0, spread);
					start = start + 0.035;
					pval = pval + 0.01;
					}
				break;
			case 27: /* ESC shuts it down */
				printf("\n\nshutting down...\n");
				fflush(stdout);
				rrr->close();
				endwin();
				return(1);
			default:
				break;
		}
	}
}
