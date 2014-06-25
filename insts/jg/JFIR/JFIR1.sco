rtsetparams(44100, 2)
load("JFIR")

writeit = 0
if (writeit) {
   set_option("audio_off", "clobber_on")
   rtoutput("foo.snd", "sun", "float")
}
rtinput("/tmp/1stmove.snd")

outskip = 0.0
inskip = 0.0
dur = DUR()
amp = 4.0
order = 400
inchan = 0

nyq = 44100 / 2
makegen(2, 24, 5000,
0,0, 100,0, 200,1, 700,1, 1000,0, 1500,0, 1600,.8, 2200,.8, 4000,0, nyq,0)

setline(0,0, 1,1, 7,1, 9,0)

JFIR(outskip, inskip, dur, amp, order, inchan)

