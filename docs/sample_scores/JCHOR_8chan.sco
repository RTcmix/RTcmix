/* This is for an 8-channel audio card, with speakers arranged as follows:
   2 front, 2 front side, 2 rear side, 2 rear.    -JGG
*/
rtsetparams(44100, 8, 4096)
load("JCHOR")

/* -------------------------------------------------------------------------- */
totdur = 24

/* Most any sound'll work. */
rtinput("/snd/Public_Sounds/vccm_old/hitgong.snd")
inskip = 0.1

minamp = 0.1;   maxamp = 1.0
minwait = 0.2;  maxwait = 0.6

masteramp = 1.2

makegen(2, 25, 2000, 1)            /* grain envelope (hanning window) */
setline(0,0, 1,1, 2,1, 3,0)        /* overall envelope */

/* -------------------------------------------------------------------------- */
start = 0
bus_config("JCHOR", "in 0",  "out 0", "out 3")   /* front L and rside R */

outdur = totdur
indur = 0.06
transposition = 4.00
nvoices = 4

JCHOR(start, inskip, outdur, indur, maintain_dur=1, transposition, nvoices,
      minamp * masteramp, maxamp * masteramp, minwait, maxwait, seed=.371)

/* -------------------------------------------------------------------------- */
start = start + 4
bus_config("JCHOR", "in 0",  "out 6", "out 1")   /* rside L and front R */

outdur = totdur - start
indur = 0.08
transposition = 1.02
nvoices = 3

JCHOR(start, inskip, outdur, indur, maintain_dur=1, transposition, nvoices,
      minamp * masteramp, maxamp * masteramp, minwait, maxwait, seed=.937)

/* -------------------------------------------------------------------------- */
start = start + 1
bus_config("JCHOR", "in 0",  "out 7", "out 4")   /* fside L and rear R */

outdur = totdur - start
indur = 0.04
transposition = 2.07
nvoices = 2

JCHOR(start, inskip, outdur, indur, maintain_dur=1, transposition, nvoices,
      minamp * masteramp, maxamp * masteramp, minwait, maxwait, seed=.743)

/* -------------------------------------------------------------------------- */
start = start + 1
bus_config("JCHOR", "in 0",  "out 5", "out 2")   /* rear L and fside R */

outdur = totdur - start
indur = 0.08
transposition = -0.01
nvoices = 2
maxamp = maxamp + .6

JCHOR(start, inskip, outdur, indur, maintain_dur=1, transposition, nvoices,
      minamp * masteramp, maxamp * masteramp, minwait, maxwait, seed=.581)

