rtsetparams(44100, 2)
load("VOCODESYNTH")

rtinput("my_stereo_sound.aif")
inskip = 0.0
dur = DUR()
amp = 1.0
numbands = 22
lowcf = 300
interval = 0.025
cartransp = 0.00
bw = 0.06
winlen = 0.0001
smooth = 0.98
thresh = 0.0001
atktime = 0.001
reltime = 0.01
hipassmod = 0.0
hipasscf = 2000

makegen(2, 10, 10000, 1)

scale1 = .5
scale2 = 1
makegen(3, 4, 100, 0,scale1,1, 1,scale2)

setline(0,0, .1,1, dur-.1,1, dur,0)

spacemult = cpspch(interval) / cpspch(0.0)

VOCODESYNTH(0, inskip, dur, amp, numbands, lowcf, spacemult, cartransp, bw,
   winlen, smooth, thresh, atktime, reltime, hipassmod, hipasscf, 0, 1)
VOCODESYNTH(0, inskip, dur, amp, numbands, lowcf, spacemult, cartransp, bw,
   winlen, smooth, thresh, atktime, reltime, hipassmod, hipasscf, 1, 0)

