These test programs use PortAudio, usually in combination with
librtcmix_embedded. These exist mostly as simple ways to exercise
this library. These require a POSIX environment, so no Windows.

To compile these, install PortAudio in its default location (/usr/local/lib).
Get it here: http://portaudio.com/download.html.

Build RTcmix as follows.

1. Set BUILDTYPE to OSXEMBEDDED in rtcmix/site.conf.

2. ./configure; make && make install. This will place librtcmix_embedded
where our Makefile will find it.

3. cd to the directory this README is in, and make.


Programs
--------
embedtest.c - simple example of writing scores to RTcmix
patest.c - play noise through PortAudio, without any RTcmix
scoreplayer.c - play multiple scores, with configurable overlap
scoretrigger.c - play scores interactively from the computer keyboard

-John Gibson, 8/1/17

