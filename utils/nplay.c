/* netplay -- very simple program to play samples from an RTcmix process
   running elsewhere on the network.  This expects short samples, SR = 44.1k,
   2 channels at present; internal buffers are 8k samps.

   Start this program up first.  Then start the RTcmix instrument
   with the "-r <hostname>" flag set ("r" for "remote"), where <hostname>
   is the IP name of the machine running this program.  This will accept
   a connection from the remote machine on socket # 9999 and will start
   trying to grab samples, playing them through the default output device.

   Be sure to shut down the RTcmix process before terminating this one
   or you will have to wait for a minute or two for the network tables
   to flush before restarting this program.  There's probably a way to
   be sure this doesn't happen, but I'm old and tired.

   BGG
*/

/* added command line arg for socket to listen on - 6/22/00 MH
ported to Linux 12/10/00 MH
to MacOSX 5/02 MH optional byte-swapping command line argument
	end of file finishes correctly
Ico's listen loop 5/18/02
MH & IB combined OSX and Linux versions 1/11/04
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <byte_routines.h>
#include <sndlibsupport.h>
#include <audio_port.h>

#define BUFSIZE 8192

#if defined(LINUX) 
#define BUF_FRAMES          1024 * 1 
#endif /* LINUX */

#if defined(MACOSX) 
#define BUF_FRAMES          1024 * 2 
int play_audio = 1;
typedef float *BufPtr;
BufPtr audioin_buffer[1];
BufPtr out_buffer[1];
#endif /* MACOSX */

#define NUM_ZERO_BUFS       8

int sockno;

#define VERBOSE             1       /* if true, print buffer size */

/* command line args:  socketnumber [swap]
	(cannot indicate byte-swapping unless socket is specified for now) */

int main(int argc, char *argv[])
{
	int i;

	/* socket stuff */
	int s, ns;
	struct sockaddr_in sss;
	int err,amt,len;
	int child_id;
	int result;
        
#ifdef LINUX        
	int resultBuffer;  	 
	int ports[2]; 
#endif /* LINUX */
#ifdef MACOSX        
  	 AudioDeviceID out_port; // OSX
#endif /* MACOSX */

	short sampbuf[BUFSIZE],*sbuf;

	int buf_frames, fragments;
	long pvbuf[2],buflen;
	int srate;
	int NCHANS;
	char hdr[54];
	char *bufp;
	int swap = 0;  	// swap bytes

/* store socket num */
	if ( argc > 1 ) 
		sockno = atoi(*++argv);
	else 
		sockno = 9999;
	if ( argc > 2 ) { 
		fprintf(stderr,"swap bytes\n");
		swap = 1;
	} 
/* configure and open output audio port */
	srate = 44100;
	NCHANS = 2;

// open audio port	

	buf_frames = BUF_FRAMES;  	
   	fragments = 2; 

#ifdef LINUX        
	result = open_ports(0, 0, NULL, 2, 2, ports, VERBOSE,
                                      srate, fragments, &buf_frames);
#endif /* LINUX */

#ifdef MACOSX       
       result = open_macosx_ports(0, 2, NULL, &out_port, VERBOSE,
                                      (float)srate, fragments, &buf_frames);
#endif /* MACOSX */

   	if (result == -1) {
   		perror("open port");
      		exit(1);
     	}

/* create the socket for listening */
	if( (s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	/* set up the socket address */
	bzero(&sss, sizeof(sss));
	sss.sin_family = AF_INET;
	sss.sin_addr.s_addr = INADDR_ANY;
	sss.sin_port = htons(sockno);

	if( (err = bind(s, (struct sockaddr *)&sss, sizeof(sss))) < 0) {
		perror("bind");
		exit(1);
	}

	/* listen for connections and accept when received */
	bufp = (char *)&sampbuf;
	
	while (1 ) {

		listen(s, 1);

		len = sizeof(sss);
		ns = accept(s, (struct sockaddr *)&sss, &len);
		if(ns < 0) {
			perror("accept");
			exit(1);
		}
		printf("connection received.\n");
	
		// header
		amt += read(ns, hdr, 872);
		result = 1;

		while(result) {

			// zero out sampbuf
			for( i = 0 ; i < BUFSIZE; i++ )
				sampbuf[i] = 0;

			/* read a buffer from the socket */ 
	
			result = read(ns, bufp, BUFSIZE);

			amt += result;

#ifdef MACOSX		
 			// swap the bytes 
			if ( swap ) {
 				for ( i = 0; i < BUFSIZE; i++ ) {
  					sampbuf[i] = reverse_int2(&sampbuf[i]);
				}
			}
 			// and write to audio port
			macosx_cmixplay_audio_write(sampbuf);
#endif /* MACOSX */ 				
#ifdef LINUX
			if ( result < BUFSIZE )
				resultBuffer = result;
			else
				resultBuffer = BUFSIZE;
				
			if ( resultBuffer != 0 )
			{
				for ( i = 0; i < resultBuffer/2; i++ ) {
					if ( swap )
					{
						sampbuf[i] = reverse_int2(&sampbuf[i]);
					}
					if (!(write(ports[0], &sampbuf[i], 2))) {
						fprintf(stderr, "ERROR: Bad write to audio convertor...Terminating stream.\n");
						return -1;
					}
				}
			}
		
#endif /* LINUX */ 				
		}

		printf("connection closed.\n");
		/* write zeroes to avoid end clicks */

		for (i = 0; i < BUFSIZE; i++)
			sampbuf[i] = 0;
#ifdef MACOSX
		macosx_cmixplay_audio_write(sampbuf);
#endif /* MACOSX */
#ifdef LINUX
                for (i = 0; i < BUFSIZE; i++) {
                        write(ports[0], &sampbuf[0], 2);
                }
#endif /* LINUX */ 				

	}
	return 0;

}


