print_on(1);
load("SCRUB");
set_option("audio_on");
rtsetparams(44100, 2, 2048);
rtinput("../../../snd/input.wav");
transp = -0.5;
skip = DUR(0);
dur = DUR(0) * 2;
// Play reversed at half-speed from end
SCRUB(0,skip,dur,1,transp, 16,40);

