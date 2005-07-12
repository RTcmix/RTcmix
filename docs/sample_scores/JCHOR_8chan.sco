// This is for an 8-channel audio card, with speakers arranged as follows:
// 2 front, 2 front side, 2 rear side, 2 rear.    -JGG

rtsetparams(44100, 8)
load("JCHOR")

// --------------------------------------------------------------------------
totdur = 24

// Most any sound'll work.
rtinput("../../snd/loocher.aiff")
inchan = 0
inskip = 0.2

minamp = 0.2
maxamp = 1.0

env = maketable("line", 1000, 0,0, 1,1, 2,1, 3,0)
masteramp = 1.0 * env

grainenv = maketable("window", 1000, "hanning")

minwait = 0.2
maxwait = 0.6

// --------------------------------------------------------------------------
start = 0
bus_config("JCHOR", "in 0",  "out 0", "out 3")   // front L and rside R

outdur = totdur
indur = 0.09
transposition = 4.00
nvoices = 4

JCHOR(start, inskip, outdur, indur, maintain_dur=1, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed=.371, inchan, masteramp, grainenv)

// --------------------------------------------------------------------------
start = start + 4
bus_config("JCHOR", "in 0",  "out 6", "out 1")   // rside L and front R

outdur = totdur - start
indur = 0.08
transposition = 1.02
nvoices = 3

JCHOR(start, inskip, outdur, indur, maintain_dur=1, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed=.937, inchan, masteramp, grainenv)

// --------------------------------------------------------------------------
start = start + 1
bus_config("JCHOR", "in 0",  "out 7", "out 4")   // fside L and rear R

outdur = totdur - start
indur = 0.04
transposition = 2.07
nvoices = 2

JCHOR(start, inskip, outdur, indur, maintain_dur=1, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed=.743, inchan, masteramp, grainenv)

// --------------------------------------------------------------------------
start = start + 1
bus_config("JCHOR", "in 0",  "out 5", "out 2")   // rear L and fside R

outdur = totdur - start
indur = 0.08
transposition = -0.01
nvoices = 2
maxamp += 0.6

JCHOR(start, inskip, outdur, indur, maintain_dur=1, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed=.581, inchan, masteramp, grainenv)

