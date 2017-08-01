These test programs use PortAudio, usually in combination with
librtcmix_embedded. These exist mostly as simple ways to exercise
this library. These should compile on Windows, but I haven't tried
it or done whatever is needed to set this up.

To compile these, install PortAudio in its default location (/usr/local/lib).
Configure RTcmix as follows.

1. Set BUILDTYPE to OSXEMBEDDED in rtcmix/site.conf.

2. ./configure; make && make install. This will place librtcmix_embedded
where our Makefile will find it.

3. cd to the directory this README is in, and make.


-John Gibson, 8/1/17

