print_on(1);
load("SCRUB");
set_option("audio_on");
rtsetparams(44100, 2, 2048);
rtinput("/home/dscott/sounds/Track11-EnglishSpokenMP.wav");
transp = -1;
skip = 10;
dur = 5;
// Play backwards from time 10 for 5 seconds
SCRUB(0,skip,dur,1,transp, 16,40);

