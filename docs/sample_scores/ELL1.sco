rtsetparams(44100, 2)
load("STEREO")
load("ELL")

rtinput("/snd/Public_Sounds/vccm_old/ah.snd")
inchan = 0
inskip = 0
dur = DUR()

/* --------------------------------------------------------------- STEREO --- */
amp = 8
STEREO(start=0, inskip, dur, amp, pctleft=.5)

/* --------------------------------------------------------- ELL low-pass --- */
pbcut = 400
sbcut = 800
ripple = .1
atten = 90

ellset(pbcut, sbcut, 0, ripple, atten)

amp = 12
ringdur = .1

ELL(start=dur+1, inskip, dur, amp, ringdur, inchan, pctleft=.5)

/* --------------------------------------------------------- ELL low-pass --- */
pbcut = 800
sbcut = 400
ripple = .1
atten = 90

ellset(pbcut, sbcut, 0, ripple, atten)

amp = 15
ringdur = .1

ELL(start=start+dur+1, inskip, dur, amp, ringdur, inchan, pctleft=.5)


/* -------------------------------------------------------------------------- */
/* First call ellset() to set up the filter.

     for lowpass filter:
       p0  passband cutoff (in cps) - this will be lower than the stopband
       p1  stopband cutoff (in cps)
       p2  set this to zero

     for hipass filter:
       p0  passband cutoff (in cps) - this will be higher than the stopband
       p1  stopband cutoff (in cps)
       p2  set this to zero

     for bandpass filter:
       p0  lower passband cutoff (in cps)
       p1  higher passband cutoff (in cps)
       p2  stopband cutoff, either higher or lower (in cps)
           (higher seems more reliable)

     for all three types:
       p3  ripple (in db)  [try 0.1]
       p4  attenuation at stopband (in db)  [try 90 for a steep filter]

   Then call ELL() to perform the filtration.

     p0  output start time
     p1  input start time
     p2  duration
     p3  amplitude multiplier
     p4  ring-down duration
     p5  input channel
     p6  stereo position (pctleft)
*/
