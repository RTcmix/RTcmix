print_off()
rtsetparams(44100, 2)
load("STRUM")
load("JDELAY")

makegen(1, 24, 1000, 0,1,1,1)

bus_config("START", "aux 0-1 out")
bus_config("JDELAY", "aux 0-1 in", "out0-1")

totdur = 30

/* 1: resonate strum notes, 0: echo strum notes */
resonate = 1

/*----------------------------------------------------------------------------*/
setline(0,0, 1,1, 30,1, 50,0)
numpitches = makegen(2, 2, 100,
   6.119, 7.00, 7.02, 7.05, 7.07, 7.071, 7.08, 7.10, 8.00, 8.02, 8.07, 8.072)

srand(232)

notedur = 0.10
incr = .116

maxampdb = 92
minampdb = 65
ampdiff = maxampdb - minampdb

maxsquish = 6
minsquish = 1
squishdiff = maxsquish - minsquish

smearpct = .001
dursmearpct = .2

for (start = 0; start < totdur; start = start + incr) {
   pind = random() * numpitches
   pitch = sampfunc(2, pind)
   amp = ampdb(minampdb + (ampdiff * random()))
   squish = minsquish + (squishdiff * random())
   smear = start * smearpct * random()
   dursmear = notedur * dursmearpct * random()

   START(start + smear, notedur + dursmear, pitch, 1.0, 0.1, amp,
                                                          squish, random(),1)
}

/*----------------------------------------------------------------------------*/
amp = 2.0
if (resonate) {
   deltime = 1 / cpspch(6.00)
   regen = 0.99
   wetdry = 0.10
   cutoff = 2000
}
else {
   deltime = notedur * 2
   regen = 0.85
   wetdry = 0.23
   cutoff = 0
}
ringdur = 5.0

atk = 4; dcy = 4
setline(0,0, atk,1, totdur-dcy,1, totdur,0)

JDELAY(st=0, insk=0, totdur, amp, deltime, regen, ringdur, cutoff, wetdry,
       inchan=0, pctleft=1)
JDELAY(st=0.02, insk=0, totdur, amp, deltime, regen, ringdur, cutoff, wetdry,
       inchan=1, pctleft=0)



/* JGG, 28-may-00 */
