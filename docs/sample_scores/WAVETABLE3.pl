# This is a translation of sample_scos_1.0/WAVETABLE3.sco into Perl.   -JGG

use RT;

rtsetparams(44100, 2);
load("WAVETABLE");
print_off();
$maxamp = 4000;
$amp = maketable("line", "nonorm", 5000, 0,0, .005,$maxamp, 1,0);
$tabsize = 5000;
$wavet = maketable("wave", $tabsize, 1, 0.3, 0.2);
control_rate(44100);

srand(0);

for ($start = 0; $start < 7; $start += 0.14) {
   $freq = (random() * 200) + 35;
   for ($i = 0; $i < 3; $i += 1) {
      WAVETABLE($start, 0.4, $amp, $freq, 0, $wavet);
      WAVETABLE($start + random()*0.1, 0.4, $amp,
				$freq + (random()*7), 1, $wavet);
      if ($start > 3.5) {
         $wavet = maketable("wave", $tabsize, 1, random(), random(), random(),
							random(), random(), random(), random(), random(),
							random(), random(), random());
      }
      $freq += 125;
   }
}
