// This score includes two files, the second of which which contains an error.
// This is to test that the line number for the reported error
// is taken from the included file.

include 10line.include
printf("You should see an error listed for line 5\n");
include error_line5.include

system("sleep 1");
