print_on(1);
load("SCRUB");
set_option("audio_on");
rtsetparams(44100, 2, 2048);
rtinput("/home/dscott/sounds/Track11-EnglishSpokenMP.wav");
transp = -0.5;
dur = 20;
skip = 10;
// Play reversed at half-speed from time 10 for 20 seconds.
SCRUB(0,skip,dur,1,transp, 16,40);

