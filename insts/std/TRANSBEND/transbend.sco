rtsetparams(44100, 2);
rtinput("/snd/sources/toot01.aifc");

load("TRANSBEND");

dur = DUR(0);
amp = 0.8;
pan = 0.5;

/* amplitude curve */
setline(0,0, 1, 1, 90, 1, 100, 0);

/* transpose from 4 semitones up to 8 down - stored in gen slot 2 */
makegen(-2, 18, 512, 1,.4, 512,-.8);

TRANSBEND(0, 0, dur, amp, 2, 0, pan);

