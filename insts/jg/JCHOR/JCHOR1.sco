rtsetparams(44100, 2)
load("JCHOR")

rtinput("your_input.snd")

outskip = 0
outdur = 10
inskip = 0.20
indur = 0.20
maintain_dur = 1
transposition = 0.07
nvoices = 60
minamp = 0.01
maxamp = 1.0
minwait = 0.00
maxwait = 0.60
seed = 0.9371

makegen(2, 25, 1000, 1)                /* last arg: 1=hanning, 2=hamming */

JCHOR(outskip, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed)

