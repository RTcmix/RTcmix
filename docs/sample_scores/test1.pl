use RT;

$pch = 8.00;
$freq = cpspch($pch);

print "$pch (opc) => $freq (Hz)\n";

$amp = 15000;
$db = dbamp($amp);

print "$amp (amp) => $db (dB)\n";

