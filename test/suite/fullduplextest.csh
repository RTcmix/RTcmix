#!/bin/csh
#
# This script tests full duplex mode using a variety of settings.  Depending on
# your platform and HW, varous tests will fail and succeed
# (i.e., OSS under MACOSX).
#

echo "Testing default input and output HW"
echo

echo "Testing full duplex mono 3 seconds"
CMIX 44100 1 1024 3 < hw_2_hw.sco

echo "Testing full duplex stereo 3 seconds"
CMIX 44100 2 1024 3 < hw_2_hw.sco

echo "Testing full duplex mono, RTBUFSAMPS 16384"
CMIX 44100 1 16384 3 < hw_2_hw.sco

echo "Testing full duplex mono, RTBUFSAMPS 128"
CMIX 44100 1 128 3 < hw_2_hw.sco

echo "Testing full duplex mono, SR 11025"
CMIX 11025 1 1024 3 < hw_2_hw.sco

echo "Testing OSS input and output HW"
echo

echo "Testing full duplex mono 3 seconds"
CMIX 44100 1 1024 3 "device=/dev/dsp" < hw_2_hw.sco

echo "Testing full duplex stereo 3 seconds"
CMIX 44100 2 1024 3 "device=/dev/dsp" < hw_2_hw.sco

echo "Testing full duplex mono, RTBUFSAMPS 16384"
CMIX 44100 1 16384 3 "device=/dev/dsp" < hw_2_hw.sco

echo "Testing full duplex mono, RTBUFSAMPS 128"
CMIX 44100 1 128 3 "device=/dev/dsp" < hw_2_hw.sco

echo "Testing full duplex mono, SR 11025"
CMIX 11025 1 1024 3 "device=/dev/dsp" < hw_2_hw.sco


#
# Error Testing
#

echo "Error Testing"
echo

echo "Testing nonexistent OSS input device"
CMIX 44100 2 1024 4 "indevice=/dev/dsp88" < hw_2_hw.sco

echo "Testing nonexistent OSS output device"
CMIX 44100 2 1024 4 "outdevice=/dev/dsp99" < hw_2_hw.sco

echo "Testing unrecogized input device"
CMIX 44100 2 1024 4 "indevice=xxxgarbagexxx" < hw_2_hw.sco

echo "Testing unrecogized output device"
CMIX 44100 2 1024 4 "outdevice=xxxgarbagexxx" < hw_2_hw.sco

