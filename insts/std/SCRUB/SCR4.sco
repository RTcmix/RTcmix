print_on(1);
load("SCRUB");
set_option("audio_on");
rtsetparams(44100, 2, 2048);
rtinput("/home/dscott/sounds/Track11-EnglishSpokenMP.wav");
transp = maketable("line", 8192, 0, 1, 1, -1);
dur = 20;
skip = 0;
// Play 20 seconds, moving from normal speed to reverse normal
SCRUB(0,skip,dur,1,transp, 16,40);

