rtsetparams(44100, 2, 4096);
rtinput("/home/dscott/sounds/fine1.wav");

load("SCRUB");

dur = DUR(0);
amp = 0.8;
pan = 0.5;

SCRUB(0, 0, dur*16, amp, -4, 16, 20, 0, pan);

