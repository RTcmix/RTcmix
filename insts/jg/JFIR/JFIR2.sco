rtsetparams(44100, 2)
load("JFIR")

writeit = 0
if (writeit) {
   set_option("audio_off", "clobber_on")
   rtoutput("foo.snd", "sun", "float")
}
rtinput("/tmp/1stmove.snd")

reset(4000)
setline_size(12000)
setline(0,0, 1,1, 29,1, 30,0)

inchan = 0
amp = 2.0
inskip = 13
totaldur = 5.5
dur = .05

nyq = 44100 / 2

numentries = 5
makegen(3, 2, numentries, 0)
500 2000 1000 3000 1200

half_bandwidth_percent = .50

order = 300

for (st = 0; st < totaldur - dur; st = st + dur) {
   n = (st / totaldur) * (numentries - 1)
   cf = sampfunci(3, n)
   low = cf - (cf * half_bandwidth_percent)
   high = cf + (cf * half_bandwidth_percent)
   makegen(2, 24, 5000, 0,0, low,0, cf,1, high,0, nyq,0)
   JFIR(st, inskip, dur, amp, order, inchan)
   inskip = inskip + dur
}


