#include <linux/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <math.h>

#define BUFSIZE 4096
#define SR_RATE 44100
#define NBUFS 10
#define WTABLESIZE 1000
#define FREQ 278.0
#define NOSCILS 1000

void makewave(short wtable[])
{
  int i;
  
  for(i = 0; i < WTABLESIZE; i++) {
    wtable[i] = sin((double) (2*M_PI * (float)i/WTABLESIZE)) * 32767.0;
  }
  return;
}

short oscil(float si, short wave[], float *phs)
{
  int i;
  
  i = *phs;
  *phs += si;
  if(*phs >= WTABLESIZE) *phs -= WTABLESIZE;
  return(wave[i]);
}

main () 
{
  int len,format,stereo,speed,cycle,frag_size;
  int audio_fd;
  int arg;
  short waveform[WTABLESIZE];
  short sampbuff[NBUFS][BUFSIZE];
  int i,j,k,bufno;
  int active;
  float si[NOSCILS],phase[NOSCILS];
  
  /* Open the audio port */
  if ((audio_fd = open("/dev/dsp", O_WRONLY, 0)) == -1) { 
    perror("/dev/dsp");
    exit(1);
  }

  /* Set the fragment size */
  arg = 0x777f000a;
  if (ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &arg)==-1) {
    perror("incorrect fragment size");
    exit(1);
  }

  /* Put the audio port in "sync" mode */
  ioctl(audio_fd, SNDCTL_DSP_SYNC, 0);

  /* Set the format for output samples */
  format = AFMT_S16_LE;
  if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format)==-1) { 
    perror("SNDCTL_DSP_SETFMT");
    exit(1);
  }

  /* Set stereo / mono (1/0) */
  stereo = 0;
  if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &stereo)==-1) { 
    perror("SNDCTL_DSP_STEREO");
    exit(1);
  }

  /* Set sample rate */
  speed = SR_RATE;
  if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed)==-1)
    { /* Fatal error */
      perror("SNDCTL_DSP_SPEED");
      exit(1);
    }

  /* Get the size of the audio buffer */
  /* Note that the driver computes this optimally */
  if (ioctl(audio_fd, SNDCTL_DSP_GETBLKSIZE, &frag_size) == -1) {
    exit(1);
  }
  printf("Audio buffer:  %d\n",frag_size);

  /* make a wavetable for the oscillator */
  makewave(waveform);
  
  /* load buffers and write them to the audio port */
  
  for (i = 0; i < NOSCILS; i++) {
    si[i] = (FREQ + (i*5)) * WTABLESIZE/SR_RATE;
    phase[i] = 0.0;
  }
  
  bufno = 0;
  active = 1;
  for (i = 0; i < 99999; i++) {

    /* fill a buffer with sound */
    for (k = 0; k < active; k++) {

      if (k == 0) {
        for (j = 0; j < BUFSIZE; j++) {
          sampbuff[bufno][j] = oscil(si[k],waveform,&phase[k]) * 1.0/active;
        }
      }
      else {
        for (j = 0; j < BUFSIZE; j++) {
          sampbuff[bufno][j] += oscil(si[k],waveform,&phase[k]) * 1.0/active;
        }
      }

    }

    if ((len = write(audio_fd, sampbuff[bufno], 2*BUFSIZE)) == -1) {
      perror("audio write");
      exit(1);
    }
    
    /* shift to the next buffer */
    if (++bufno > 3) {
      if (++active > NOSCILS)
        active -= 1;
      printf("%d oscillators, daddy-oh!\n",active);
      bufno = 0;
    }
  }
  
  /* Close the audio port */
  close(audio_fd);
}




