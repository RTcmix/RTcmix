# Transform our extension .pm.in file into a .pm file by inserting the
# functions we want to export in the appropriate place.  These functions
# reside in the "funclist" file.   -JGG, 30-Jul-00

$extname = $ARGV[0];               # name of extension passed in as arg
$funclist = "./funclist";
$pm_in_file = "$extname/$extname.pm.in";
$output = "$extname/$extname.pm";
$insert_after = "our \@EXPORT = qw(\n";

open FUNCLIST, "< $funclist" || die "Can't open $funclist ($!)";
open PM_IN, "< $pm_in_file" || die "Can't open $pm_in_file ($!)";
open OUTPUT, "> $output" || die "Can't open $output ($!)";

$found = 0;
while (<PM_IN>) {
   if (!$found and ($_ eq $insert_after)) {
      print OUTPUT;
      while (<FUNCLIST>) {
         print OUTPUT;
      }
      close FUNCLIST;
      $found = 1;
   }
   else {
      print OUTPUT;
   }
}

close PM_IN;

