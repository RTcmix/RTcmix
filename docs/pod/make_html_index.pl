#$title0 = $ARGV[0];

$inst_fname = "inst_html.tmp";
$cmd_fname = "cmd_html.tmp";
$script_fname = "script_html.tmp";
$api_fname = "api_html.tmp";

print qq{<html>\n<body bgcolor="white">\n\n};

print_section($inst_fname, "Instruments");
print_section($cmd_fname, "Command-line programs");
print_section($script_fname, "Script commands");
print_section($api_fname, "Programmer API");

print qq{\n</body>\n</html>\n};

sub print_section () {
   my($fname, $heading) = @_;
   my (@list, $word);

   open FILE, $fname or die "Can't open $fname ($!)";
   @list = split(/ /, <FILE>);

   # Remove duplicates and sort (from perlfaq4).
   undef %tmp;
   @tmp{@list} = ();
   @list = sort keys %tmp;

   print qq{$heading:<p><br>\n};

   print "<blockquote>\n";
   foreach $word (@list) {
      chomp $word;
      $word =~ s/(.*)\..*/$1/;
      print qq{<a href="$word.html">$word</a><br>\n};
   }
   print "</blockquote>\n\n";
}

