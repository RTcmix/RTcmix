print_on(1);
load("SCRUB");
set_option("audio_on");
rtsetparams(44100, 2, 2048);
rtinput("../../../snd/input.wav");
transp = maketable("line", 8192, 0, 1, 1, -1);
dur = DUR(0) * 2;
skip = 0;
// Play file, moving from normal speed to reverse normal
SCRUB(0,skip,dur,1,transp, 16,40);

