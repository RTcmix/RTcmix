/* FM with a random index guide function -- different for each channel.
   These fed into resonators.
*/
rtsetparams(44100, 2)
load("FMINST")
load("JDELAY")

totdur = 20

/* ------------------------------------------------------------------- fm --- */
amp = 17000
car = 200
mod = car * (7/5)

imin = 0
imax = 15

/* random index guide function
   Note that FMINST interpolates between array values, so random numbers
   will never produce an on/off juxtaposition at slow speeds ... the
   random changes will be smoothed over.
*/
speed = 30         /* in Hz */
distribution = 1
seed = 1331
makegen(3,20, speed * totdur, distribution, seed)

makegen(2, 10, 1000, 1)

bus_config("FMINST", "aux 0 out")
FMINST(st=0, totdur, amp, car, mod, imin, imax)

bus_config("FMINST", "aux 1 out")
speed = 26
seed = 9240
makegen(3,20, speed * totdur, distribution, seed)  /* different for 2nd chan */
FMINST(st=0, totdur, amp, car, mod, imin, imax)

/* ------------------------------------------------------------ resonator --- */
bus_config("JDELAY", "aux 0-1 in", "out 0-1")
pitch = 5.02
regen = 0.985
wetdry = 0.18
cutoff = 0
ringdur = 8.0

setline(0,0, 1,1, 19,1, 20,0)

deltime = 1 / cpspch(pitch)
JDELAY(st=0, insk=0, totdur, amp=.5, deltime, regen, ringdur, cutoff, wetdry,
       inchan=0, pctleft=1)

deltime = 1 / cpspch(pitch + .002)
JDELAY(st=0, insk=0, totdur, amp=.5, deltime, regen, ringdur, cutoff, wetdry,
       inchan=1, pctleft=0)



/* john gibson, 17-june-00 */
