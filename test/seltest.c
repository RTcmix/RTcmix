#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#ifdef _AIX
#include <sys/select.h>
#endif

int main(int argc, char *argv[])
{
        int fd;

        int tmp;

        char buf[128*1024]={0};

        int have_data = 0;

        int n, l;

int frag = 0x00020008;  /* 32 fragments of 2^8=256 bytes */
int frag_size;



        fd_set reads, writes;

        close(0);

        if ((fd=open("/dev/dsp", O_RDWR, 0))==-1)
        {
                perror("/dev/dsp open");
                exit(-1);
        }

	tmp = 0;
        ioctl(fd, SNDCTL_DSP_SETDUPLEX, &tmp);

        ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &frag);

        tmp = 44100;
        ioctl(fd, SNDCTL_DSP_SPEED, &tmp); /* And #channels & #bits if required */

        ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &frag_size);

        if (argc > 1)
        {
                n=atoi(argv[1]);
                n = ((n/frag_size+1)) * frag_size;
                if (n>sizeof(buf))
                   n=sizeof(buf);
                write(fd, buf, n);
        }

        while (1)
        {
                struct timeval time;

                FD_ZERO(&reads);
                FD_ZERO(&writes);
                
                if (have_data)
                   FD_SET(fd, &writes);
                else
                   FD_SET(fd, &reads);

                time.tv_sec = 1;
                time.tv_usec=0;
                if (select(fd+1, &reads, &writes, NULL, &time)==-1)
                {
                        perror("select");
                        exit(-1);
                }

                if (FD_ISSET(fd, &reads))
                {
                   struct audio_buf_info info;

                   if (ioctl(fd, SNDCTL_DSP_GETISPACE, &info)==-1)
                        {
                                perror("select");
                                exit(-1);
                        }

                   n = info.bytes;

printf("read  ");fflush(stdout);
                   l=read(fd, buf, n);
                   if (l>0)
                      have_data = 1;
                }

                if (FD_ISSET(fd, &writes))
                {

                   int i;

                   struct audio_buf_info info;

                   if (ioctl(fd, SNDCTL_DSP_GETOSPACE, &info)==-1)
                        {
                                perror("select");
                                exit(-1);
                        }

                   n = info.bytes;

                   /* printf("Write %d\n", l); */
printf("\rwrite %d %d ", info.fragstotal, info.fragsize);fflush(stdout);
                   write(fd, buf, l);
                   /* printf("OK"); */
                   have_data = 0;
                }
        }       

        exit(0);
}
