use RT;

print "\nWelcome to the Perl front end to RTcmix!\n\n";

rtsetparams(44100, 1);
load("WAVETABLE");

setline(0,0, 1,1, 4,1, 5,0);
makegen(2, 10, 1000, 1, 0.3, 0.2);

$start = 0;
$dur = 0.7;
$amp = 15000;
$freq = 7.00;

for ($i = 0; $i < 5; $i++) {
   WAVETABLE($start, $dur, $amp, $freq);
   $start += 0.5;
   $amp -= 2000;
   $freq += 0.07;
}

