print_on(1);
load("SCRUB");
set_option("audio_on");
rtsetparams(44100, 2, 2048);
rtinput("../../../snd/input.wav");
transp = 1;
dur = DUR(0);
skip = 0;
// Play forward from time 0
SCRUB(0,skip,dur,1,transp, 16,40);

