$cmixdir = $ARGV[0];
$vers_h = $cmixdir . "/src/rtcmix/version.h";

open VERS_H, $vers_h or die "Can't open $vers_h ($!)";

while (<VERS_H>) {
   chomp;
   if (/RTCMIX_NAME/) {
      s/.*"(.*)"/$1/;
      $progname = $_;
   }
   elsif (/RTCMIX_VERSION/) {
      s/.*"(.*)"/$1/;
      $version = $_;
      break;
   }
}

print "$progname v$version\n";

