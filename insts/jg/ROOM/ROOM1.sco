rtsetparams(44100, 2)
load("ROOM")

rtinput("/tmp/1stmove.snd")
if (0) {
   set_option("clobber_on", "audio_off")
   rtoutput("ROOM1.snd", "sun", "float")
}

x = 300
y = 200
xsrc = .9
ysrc = .1
xlwall = .1
ylwall = .9
xrwall = .1
yrwall = .9
absorb = 2
seed = .1730

roomset(x, y, xsrc, ysrc, xlwall, ylwall, xrwall, yrwall, absorb, seed)

outskip = 0
inskip = 13
dur = 9.0
amp = 3.0
inchan = 0

setline(0,0, 1,1, 9,1, 10,0)
ROOM(outskip, inskip, dur, amp, inchan)

