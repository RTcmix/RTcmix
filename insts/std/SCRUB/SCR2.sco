print_on(1);
load("SCRUB");
set_option("audio_on");
rtsetparams(44100, 2, 2048);
rtinput("../../../snd/input.wav");
transp = -1;
skip = DUR(0);
dur = skip;
// Play backwards from end to beginning
SCRUB(0,skip,dur,1,transp, 16,40);

