load("stgran")

system("rm -f stgran2.wav")
system("sfcreate -t wav -f stgran2.wav")
output("stgran2.wav")

setline(0,0, 1,1, 7,1, 8,0)
makegen(2,18,1000, 0,0, 1,1)
makegen(3,18,1000, 0,0, 1,1)
makegen(4,18,1000, 0,0, 1,1)
makegen(5,18,1000, 0,0, 1,1)
makegen(6,18,1000, 0,0, 1,1)
makegen(7,18,1000, 0,0, 1,1)
makegen(8,25,4000, 1)                 /* grain envelope */

input("/snd/Public_Sounds/egg.snd")
inchan = 0
inskip = 0

outdur = 60
seed = 3

stgran(start=0, inskip, outdur, indur=0, amp=1,
/* grain rates (seconds per grain): */
.001, .002,                 /* input file rate: start, end */
.01, .008,                  /* output file rate: start, end */

0, 0, 0, 1,                 /* variation in start input rate [ignored now] */
0, 0, 0, 1,                 /* variation in end input rate [ignored now] */
.2, .2, .2, 1,              /* variation in start output rate */
.2, .2, .2, 1,              /* variation in end output rate */

/* grain duration: */
.1, .3, .5, 1,              /* start grain dur */
.1, .3, .5, .5,             /* end grain dur */

/* grain transposition (oct.pc): */
-.10, 0, .05, 1,            /* start transposition */
-1,   0, .07, 1,            /* end transposition */

/* grain amp: */
.3, .5, 1, 1,               /* start amp */
.3, .5, 1, 1,               /* end amp */

/* grain stereo location: */
0, .5, 1, 1,                /* start stereo location */
0, .5, 1, 1,                /* end stereo location */

seed, inchan)               /* random seed */

system("rescale -r -P 30000 stgran2.wav")

