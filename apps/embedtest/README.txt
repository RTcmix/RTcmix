These test programs use PortAudio, usually in combination with
librtcmix_embedded. They exist mostly as simple ways to exercise
this library. They require a POSIX environment, so no Windows.

To compile these, install PortAudio in its default location (/usr/local/lib).
Get it here: http://portaudio.com/download.html.

If you don't want to use portaudio, do not enable it in the 
Makefile. Only scoreplayer will build in this case, and it will
operate without any audio, only showing the Minc job output.

Build RTcmix as follows.

1. Set BUILDTYPE to OSXEMBEDDED in rtcmix/site.conf.

2. ./configure; make && make install. This will place librtcmix_embedded
   where our Makefile will find it.

3. cd to the directory this README is in, and edit Makefile
   to say whether you have PortAudio, and if so, where it is.
   The default is to build without portaudio. If you have it,
   the default location is /usr/local/. (See the definitions
   for INCLUDES and LDFLAGS.)

4. Type "make".


Programs
--------
embedtest.c - simple example of writing scores to RTcmix
patest.c - play noise through PortAudio, without any RTcmix
scoreplayer.c - play multiple scores, with configurable overlap
scoretrigger.c - play scores interactively from the computer keyboard

-John Gibson, 8/1/17

