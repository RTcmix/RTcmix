rtsetparams(44100, 2)
load("ELL")

rtinput("/snd/Public_Sounds/vccm_old/conga.snd")
inchan = 0
inskip = 0
dur = DUR()

ripple = 20
atten = 90
ringdur = .2

setline(0,0, .01,1, dur/2,1, dur,0)

srand(9)

for (start = 0; start < 15; start = start + .12) {
   pbcut = 400 + (rand() * 200)
   sbcut = 900 + (rand() * 200)
   ellset(pbcut, sbcut, 0, ripple, atten)
   amp = .5
   pctleft = random()
   st = start + (rand() * .01)
   ELL(st, inskip, dur, amp, ringdur, inchan, pctleft)

   pbcut = 900 + (rand() * 200)
   sbcut = 400 + (rand() * 200)
   ellset(pbcut, sbcut, 0, ripple, atten)
   amp = .23
   pctleft = random()
   st = start + (rand() * .01)
   ELL(st, inskip, dur, amp, ringdur, inchan, pctleft)
}


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
     p6  stereo percent to left channel [optional]

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
