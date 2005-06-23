/* JCHOR - random-wait chorus instrument based on Paul Lansky's chor
           and Doug Scott's trans code

   p0  = output start time
   p1  = input start time
   p2  = output duration
   p3  = input duration (not input end time)
   p4  = maintain input duration, regardless of transposition (1: yes, 0: no)
   p5  = transposition (8ve.pc)
   p6  = number of voices (minimum of 1)
   p7  = minimum grain amplitude
   p8  = maximum grain amplitude
   p9  = minimum grain wait (seconds)
   p10 = maximum grain wait (seconds)
   p11 = seed (0 - 1)
   p12 = input channel
   p13 = overall amplitude multiplier
   p14 = reference to grain envelope table

   p7 (min amp), p8 (max amp), p9 (min wait), p10 (max wait) and p13 (amp mult)
   can receive dynamic updates from a table or real-time control source.

   Output can be either mono or stereo. If it's stereo, the program randomly
   distributes the voices across the stereo field.

   Notes on p4 (maintain input duration):

      Because the transposition method doesn't try to maintain duration -- it
      works like the speed control on a tape deck -- you have an option about
      the way to handle the duration of the input read:

      - If p4 is 1, the grain length after transposition will be the same as
        that given by p3 (input duration). This means that the amount actually
        read from the input file will be shorter or longer than p3, depending
        on the transposition.

      - If p4 is 0, the segment of sound specified by p3 will be read, and the
        grain length will be shorter or longer than p3, depending on the
        transposition.

   John Gibson, 9/20/98, RT'd 6/24/99; rev for v4, 7/24/04
*/

rtsetparams(44100, 2)
load("JCHOR")

rtinput("../../snd/huhh.wav")
inchan = 0
inskip = 0.20

outdur = 10

indur = 0.60
maintain_dur = 1
transposition = 0.07
nvoices = 50
minamp = 0.01
maxamp = 1.0
minwait = 0.00
maxwait = 0.30
seed = 0.9371

amp = 0.8
env = maketable("line", 1000, 0,0, 1,1, outdur-1,1, outdur,0)

grainenv = maketable("window", 1000, "hanning")

JCHOR(0, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed, inchan, amp * env, grainenv)

