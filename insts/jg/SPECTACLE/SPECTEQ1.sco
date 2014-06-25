sr = 44100
nyquist = sr / 2
rtsetparams(sr, 2)
load("SPECTEQ")

rtinput("../../../snd/huhh.wav")

inchan = 0
inskip = 0
indur = DUR()
ringdur = 0
amp = 15
fftlen = 1024           /* yielding 512 frequency bands */
winlen = fftlen * 2     /* the standard window length is twice FFT size */
overlap = 2             /* 2 hops per fftlen (4 per window) */
wintype = 0             /* use Hamming window */

/* input envelope (covering <indur>) */
makegen(1, 18, 1000, 0,0, 1,1, 19,1, 20,0)

/* output envelope (covering <indur> + <ringdur>) */
copygen(2, 1)

/* EQ curve: -90 dB at 0 Hz, ramping up to -10 dB at 400 Hz, etc. */
makegen(3, 18, nyquist/10,
       0,   -90,
     300,   -90,
     400,   -10,
     800,   -20,
    1000,   -90,
    2000,   -90,
    5000,   0,
 nyquist,   -40)
//fplot(3, 5, "with lines")

/* do it for the left chan! */
start = 0
SPECTEQ(start, inskip, indur, amp, ringdur, fftlen, winlen, wintype, overlap,
   inchan, pctleft=1)

/* do it for the right chan! */
SPECTEQ(start, inskip, indur, amp, ringdur, fftlen, winlen, wintype, overlap,
   inchan, pctleft=0)

