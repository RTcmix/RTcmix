rtsetparams(44100, 1)
load("FMINST")
makegen(1, 24, 1000, 0, 0, 0.1,1, 4,1, 7,0)
makegen(2, 10, 1000, 1)
makegen(3, 24, 1000, 0,1, 7,0)
/* p0 = start; p1 = dur; p2 = pitch of carrier (hz or oct.pc); */
/* p3 = pitch of modulator (hz or oct.pc); p4 = fm index low point; */
/* p5 = fm index high point;  p6 = amp;  p7 = stereo spread (0-1) <optional> */
/* function slot 2 is oscillator waveform, slot 1 is the amp; */
/* slot 3 is index guide */

pitch = 100;
count = 0;
start = 0;
num = 9
denom = 8
subcount = 0;
incr = 1;

while (count < 52)
{
    FMINST(start, 3, 1000, pitch, pitch, 0, 0, 0.0)
    pitch = pitch * (num / denom);
    start = start + 0.2;
    count = count + 1;
    subcount = subcount + 1;
    if (subcount >= 5) {
	incr = incr * -1;
	subcount = 0;
    }
    num = num + incr;
    denom = denom + incr;
}
