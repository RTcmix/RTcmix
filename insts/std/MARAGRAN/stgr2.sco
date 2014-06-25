/* STGRANR: sampling stochastic granular instrument
*
*  p0=start_time
*  p1=input start time
*  p2=duration
*  p3=amplitude
*  p4=rate, p5-8 ratevar
*  p9-12 duration
*  p13-16 location
*  p17-20 transposition
*  p21 granlayers
*  p22 seed
*  function slot 1 is waveform, 
*   slot 2 is amp envelope 
*   3 is grain env
*/
set_option("FULL_DUPLEX_ON")
rtsetparams(44100, 2)
load("STGRANR")
rtinput("AUDIO","MIC",2)

makegen(1, 7, 1000, 1, 950, 1, 50, 0)
makegen(2, 25, 1000, 1)
start = 0.0
    /* p0start, p1inputstt, p2dur, p3amp */
STGRANR(start, 0, 13, 5000, 
/* grain rate, ratevar values (must be positive, 
	% until next grain possible displacement): */
.1, 0.0, 0.1, 0.2, 1.0,
/* duration values: */
.1,.1,.1,2, 
/* location values: */
0.0,0.5,1.0,10.0, 
/* pitch values: */
0.0,0.00,0.07,2,
/* granlyrs, seed */
1,1)
