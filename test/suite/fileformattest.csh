#!/bin/csh
#
# This script generates output files in all supported sample formats
# and plays the results, then deletes the output files.  You should hear
# a series of 6 short sine tones separated by silence.

echo "Testing little-endian short integer output"
CMIX sinetone_short.wav short < filefmt.sco
echo "Playing result..."
cmixplay sinetone_short.wav
rm -f sinetone_short.wav
echo

echo "Testing big-endian short integer output"
CMIX sinetone_short.aiff short < filefmt.sco
echo "Playing result..."
cmixplay sinetone_short.aiff
rm -f sinetone_short.aiff
echo

echo "Testing little-endian floating point output"
CMIX sinetone_float.wav float < filefmt.sco
echo "Playing result..."
cmixplay sinetone_float.wav
rm -f sinetone_float.wav
echo

echo "Testing big-endian floating point output"
CMIX sinetone_float.aifc float < filefmt.sco
echo "Playing result..."
cmixplay sinetone_float.aifc
rm -f sinetone_float.aifc
echo

echo "Testing little-endian 24bit integer output"
CMIX sinetone_24.wav 24 < filefmt.sco
echo "Playing result..."
cmixplay sinetone_24.wav
rm -f sinetone_24.wav
echo

echo "Testing big-endian 24bit integer output"
CMIX sinetone_24.aiff 24 < filefmt.sco
echo "Playing result..."
cmixplay sinetone_24.aiff
rm -f sinetone_24.aiff
echo
