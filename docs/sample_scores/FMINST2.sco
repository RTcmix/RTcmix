/* FMINST -- simple fm instrument
*
*  p0 = start time
*  p1 = duration
*  p2 = amp
*  p3 = pitch of carrier (hz or oct.pc)
*  p4 = pitch of modulator (hz or oct.pc)
*  p5 = fm index low point
*  p6 = fm index high point
*  p7 = stereo spread (0-1) <optional>
*  function slot 1 is oscillator waveform, slot 2 is the amp envelope,
*     slot 3 is index guide
*/

rtsetparams(44100, 2)
load("FMINST")
makegen(1, 10, 1000, 1)
makegen(2, 24, 1000, 0, 0, 3.5,1, 7,0)
makegen(3, 24, 1000, 0, 0, 5,1, 7, 0)
FMINST(0, 7, 10000, 8.00, 179, 0, 10, 0.1)
makegen(3, 24, 1000, 0,1, 7,0)
FMINST(3.5, 7, 10000, 8.07, 179, 0, 10, 0.9)
