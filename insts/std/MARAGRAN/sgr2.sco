/* SGRANR: a simple stochastic gran instrument
*
*  p0=start_time
*  p1=duration
*  p2=amplitude
*  p3=rate, p4-7=ratevar
*  p8-11=duration
*  p12-15=location
*  p16-19=freq
*  p20 granlayers (not in use)
*  p21 seed
*  function slot 1 is amp env
*   slot 2 is waveform 
*   3 is grain env
*/

rtsetparams(44100, 2)
load("SGRANR")

makegen(1, 7, 1000, 1, 950, 1, 50, 0)
makegen(2, 10, 1000, 1)
makegen(3, 7, 1000, 0, 500, 1, 500, 0)
start = 0.0
SGRANR(start, 3, 5000, 
/* grain rate, ratevar values (must be positive, 
	% until next grain possible displacement): */
.1, 0.1, 0.1, 0.1, 1.0,
/* duration values: */
.1,.1,.1,2, 
/* location values: */
0.0,.5,1.0,1.0, 
/* pitch values: */
800,1000,9700,2,
/* granlyrs, seed */
1,1)


