The little apps here employ an older method for 'imbedding' (or is it
'embedding'?) RTcmix inside an application environment.  Although this
technique still works, it isn't being maintained much anymore.  A better
approach is to build a dynamic or static library (see the "BUILDTYPE=OSXEMBEDDED"
option in site.conf) and use the entry-points for setting up and running RTcmix 
within a calling environment.  These entry points are declared in include/RTcmix_API.h.
The rtcmix~.c source file for the Max/MSP [rtcmix~] shows how to do this.

The downside to the shared/static lib is that they rely on an external
audio engine within the calling environment to convert samples.  This
isn't too difficult to do, though, and it also gives the calling
environment access to the sound sample buffers used by RTcmix. However,
the older 'imbedded' programs here handle their own audio with the RTcmix
audio engine.

Brad Garton
July, 2013

