rtsetparams(44100, 2)
load("SPECTACLE")

/* ftp://presto.music.virginia.edu/pub/rtcmix/snd/ah.snd */
rtinput("/Users/johgibso/snd/ah.snd")

inchan = 0
inskip = .2
indur = 2
ringdur = 15            /* play after indur elapses, while delay lines flush */
amp = 12
wetdry = 1              /* 100% wet */

fftlen = 1024           /* yielding 512 frequency bands */
winlen = fftlen * 2     /* the standard window length is twice FFT size */
overlap = 2             /* 2 hops per fftlen (4 per window) */
wintype = 0             /* use Hamming window */

/* input envelope (covering <indur>) */
makegen(1, 18, 1000, 0,0, 1,1, 19,1, 20,0)

/* output envelope (covering <indur> + <ringdur>) */
makegen(2, 4, 1000, 0,1,0, 1,1,-1, 2,0)

/* EQ curve: -90 dB at 0 Hz, ramping up to 0 dB at 300 Hz, etc. */
makegen(3, 18, 1000, 0,-90, 100,-90, 300,0, 8000,0, 22050,-90)

/* fixed delay times between .4 and 3, randomly spread across spectrum */
min = .4; max = 3
seed = 1
makegen(4, 20, 1000, 0, seed, min, max)

/* constant feedback of 80% for all freq. bands */
fb = .8
makegen(5, 18, 1000, 0,fb, 1,fb)

/* do it for the left chan! */
start = 0
SPECTACLE(start, inskip, indur, amp, ringdur, fftlen, winlen, wintype, overlap,
   wetdry, inchan, pctleft=1)

/* rotate delay table to make right channel sound different */
rotategen(4, 500)

/* do it for the right chan! */
SPECTACLE(start, inskip, indur, amp, ringdur, fftlen, winlen, wintype, overlap,
   wetdry, inchan, pctleft=0)

