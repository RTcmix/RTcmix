#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <math.h>
#include <sys/stat.h>

#define BUFSIZE 64

int
main (int argc, char* argv[]) 
{
  int len,format,stereo,speed,cycle,frag_size,samps,srate,chans,magic,class;
  int Nspeed,Nmagic,Nchans,Nclass;
  int in_port, out_port;
  int arg,i,j,c1,c2;
  int ifp, ifp2;
  short sampbuff[BUFSIZE];
  short waveform[BUFSIZE];
  float time;
  int check;
  int rec_time;

  /* Check % of args */
  printf("======== RTCmix (full-duplex-test) LINUX ========\n");
  
  speed = 44100;
  chans = 2;

  /* Print out some stuff for folks */
  printf("rate: %d\n", speed);
  printf("chans: %d\n", chans);

  /* Open the output audio port */
  if ((out_port = (int)open("/dev/dsp", O_RDWR, 0)) == -1) { 
    perror("/dev/dsp");
    exit(1);
  }

  in_port = out_port;

  /* Set the fragment size */
  arg = 0x000a0006;

  if (ioctl(out_port, SNDCTL_DSP_SETFRAGMENT, &arg)==-1) {
    perror("FRAGMENT SIZE");
    exit(1);
  }

  /* Set the format for output samples */
  format = AFMT_S16_LE;

  if (ioctl(out_port, SNDCTL_DSP_SETFMT, &format)==-1) { 
    perror("SNDCTL_DSP_SETFMT");
    exit(1);
  }

  /* Set stereo / mono (1/0) */
  stereo = chans-1;

  if (ioctl(out_port, SNDCTL_DSP_STEREO, &stereo)==-1) { 
    perror("SNDCTL_DSP_STEREO");
    exit(1);
  }

  /* Set sample rate */

  if (ioctl(out_port, SNDCTL_DSP_SPEED, &speed)==-1)
    { /* Fatal error */
      perror("SNDCTL_DSP_SPEED");
      exit(1);
    }

  /* Get the size of the audio buffer */
  /* Note that the driver computes this optimally */

  if (ioctl(out_port, SNDCTL_DSP_GETBLKSIZE, &frag_size) == -1) {
    exit(1);
  }

  printf("audio_in buffer:  %d\n",frag_size);

  printf("audio_out buffer:  %d\n",frag_size);

  /* zero the buffer */
  for (i=0;i<BUFSIZE;i++) {
    sampbuff[i] = 0;
  }

  printf("attempting write...\n");
  if ((len = write(out_port, sampbuff, 2*BUFSIZE)) == -1) {
    perror("audio write");
    exit(1);
  }
  printf("write done.\n");

  printf("reading/writing in loop\n");
  while(1) {
    if ((len = read(in_port, sampbuff, 2*BUFSIZE)) == -1) {
      perror("audio read");
      exit(1);
    }
    
    if ((len = write(out_port, sampbuff, 2*BUFSIZE)) == -1) {
      perror("audio write");
      exit(1);
    }
	printf(".");
    
    /* zero the buffer again ... there might be leftover garbage at the last buffer */
    for (i=0;i<BUFSIZE;i++) {
      sampbuff[i] = 0;
    }
  }
  printf("\n");

  /* Close the audio port */
  close(in_port);
  close(out_port);

}




