# Print html to stdout for a web page that provides an index of all the
# man pages, with brief descriptions grabbed from the man page.

# These tmp files are created and deleted by Makefile.
$inst_fname = "inst_html.tmp";
$script_fname = "script_html.tmp";
$cmd_fname = "cmd_html.tmp";
$api_fname = "api_html.tmp";

sub print_section($$);

################################################################################

$title = "RTcmix Documentation";

print "<html>\n<head>\n<title>";
print "$title</title>\n";
print qq{<link rel="stylesheet" href="style.css" type="text/css">\n};
print "</head>\n\n";

print qq{<h1>$title</h1>\n\n};

print_section($inst_fname, "Instruments");
print_section($script_fname, "Script commands");
print_section($cmd_fname, "Command-line programs");
#print_section($api_fname, "Programmer API");

print qq{\n</body>\n</html>\n};

sub print_section ($$) {
   my($fname, $heading) = @_;
   my (@list, $item);

   open FILE, $fname or die "Can't open $fname ($!)";
   @list = split(/ /, <FILE>);

   # Remove duplicates and sort (from perlfaq4).
   undef %tmp;
   @tmp{@list} = ();
   @list = sort keys %tmp;

   print qq{<p><h2 class="index">$heading</h2>\n};

   print "<blockquote>\n";
   foreach $item (@list) {
      chomp $item;
      $item =~ s/(.*)\..*/$1/;
      @lines = `grep -w $item $item.pod`;
      $blurb = $lines[0];
      chomp $blurb;
      $blurb =~ s/.*- (.*)/$1/;
      print qq{<a href="$item.html">$item</a> - $blurb<br>\n};
   }
   print "</blockquote>\n\n";
}

