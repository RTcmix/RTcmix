/* ELL - elliptical filter

   First call ellset() to set up the filter.

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
       p3  ripple (in db)  [try 0.2]
       p4  attenuation at stopband (in db)  [try 90 for a steep filter]

   Then call ELL() to perform the filtration.

     p0  output start time
     p1  input start time
     p2  duration (not input end time)
     p3  amplitude multiplier
     p4  ring-down duration
     p5  input channel [optional]
     p6  stereo spread [optional]

   Assumes function table 1 holds amplitude envelope. (Or you can just use
   setline.) If this function table is empty, uses flat envelope.


   NOTES:

   <ripple> controls the amount of ringing in the filter. A small ripple
   designs the filter to minimize ringing, whereas a large ripple (c. 20db)
   causes the filter to ring very noticeably. A large ripple can sound
   good with a tight bandpass, producing a fairly clear pitch.

   The filter design program (invoked by ellset) sometimes can't fulfill
   the design criteria -- a particular combination of cutoff freqs, ripple
   and attenuation. If this happens, the program will die with a "Filter
   design failed!" message, instead of running the job. If you ask for a
   very steep cutoff and very little ripple, you may see this.
*/
rtsetparams(44100, 2)
load("ELL")

rtinput("your.snd")

outskip = 0
inskip = 0
endin = DUR()
dur = endin - inskip
amp = 1.0
ringdur = 0.1

p0 = 1600
p1 = 9000
p2 = 0
ripple = 0.1
attenuation = 90.0

ellset(p0, p1, p2, ripple, attenuation)
ELL(outskip, inskip, dur, amp, ringdur)

