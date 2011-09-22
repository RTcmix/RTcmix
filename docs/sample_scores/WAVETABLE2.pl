# This is a translation of sample_scores/WAVETABLE2.sco into Perl.   -JGG
# Note the call to control_rate instead of reset.  We do that so as not
# to conflict with Perl's reset function.

use RT;

print_off();	# gets rid of all the screen output

rtsetparams(44100,2);
load("WAVETABLE");

$amp = 10000;
$env = maketable("line", 1000, 0,0, 0.01,1, 0.1,0.2, 0.4,0);
$wavet = maketable("wave", 3000, 1, 0.3, 0.2);

control_rate(20000);
$start = 0.0;
$freq = 149.0;

for ($i = 0; $i < 100; $i++) {
   WAVETABLE($start, 0.4, mul($env, $amp), $freq, 0, $wavet);
   WAVETABLE($start + random()*0.07, 0.4, mul($env, $amp),
		$freq + random()*2, 1, $wavet);
   $start += 0.1;
   $wavet = maketable("wave", 3000, 1, random(), random(), random(), random(),
                                                random(), random());
}

