rtsetparams(44100, 2)
load("JCHOR")

rtinput("your_input.snd")

outskip = 0
outdur = 10
inskip = 0.02
indur = 0.20
maintain_dur = 1
transposition = 0.07
nvoices = 60
minamp = 0.1
maxamp = 1.0
minwait = 0.00
maxwait = 0.60
seed = 0.9371

setline(0,0, outdur/4,1, outdur/1.8,1, outdur,0)
makegen(2, 5, 1000, .01,20, 1,980, .01)

reset(2000)

JCHOR(outskip, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed)

outskip = 2
indur = 0.90
transposition = -0.01
maxamp = 0.5
maxwait = 1.0
seed = 0.2353
JCHOR(outskip, inskip, outdur, indur, maintain_dur, transposition, nvoices,
      minamp, maxamp, minwait, maxwait, seed)

