#include <stdio.h>
#include <CoreAudio/CoreAudio.h>

const Boolean isOutput = 0;
const Boolean isInput = 1;


const char *errToString(OSStatus err)
{
   const char *errstring;
   switch (err) {
   case kAudioHardwareNoError:
      errstring = "No error.";
      break;
   case kAudioHardwareNotRunningError:
      errstring = "Hardware not running.";
      break;
   case kAudioHardwareUnspecifiedError:
      errstring = "Unspecified error.";
      break;
   case kAudioHardwareUnknownPropertyError:
      errstring = "Unknown hardware property.";
      break;
   case kAudioDeviceUnsupportedFormatError:
      errstring = "Unsupported audio format.";
      break;
   case kAudioHardwareBadPropertySizeError:
      errstring = "Bad hardware propery size.";
      break;
   case kAudioHardwareIllegalOperationError:
      errstring = "Illegal operation.";
      break;
   default:
      errstring = "Unknown error.";
   }
   return errstring;
}


UInt32
GetDefaultDeviceID(Boolean isInput)
{
   UInt32 devID;

   UInt32 size = sizeof(devID);
   OSStatus err = AudioHardwareGetProperty(
                  kAudioHardwarePropertyDefaultOutputDevice,
                  &size,
                  &devID);
   if (err != kAudioHardwareNoError || devID == kAudioDeviceUnknown) {
      fprintf(stderr, "Can't find default %s device: %s\n",
            isInput ? "input" : "output", errToString(err));
      return 0;
   }

   return devID;
}


UInt32
PrintBufferList(UInt32 devID, Boolean isInput)
{
   UInt32 size = 0;
   Boolean writeable = 0;

   printf("\nBuffer List --------------------------------------------------\n");

   // Get size of AudioBufferList, in bytes.
   OSStatus err = AudioDeviceGetPropertyInfo(devID,
                           0, isInput,
                           kAudioDevicePropertyStreamConfiguration,
                           &size,
                           &writeable);
   if (err != kAudioHardwareNoError) {
      fprintf(stderr, "Can't get %s device (%d) property info: %s\n",
            isInput ? "input" : "output", devID, errToString(err));
      return 0;
   }

//printf("stream config prop size: %d, writeable: %d\n", size, writeable);

   // Fill list with description of buffers.
   AudioBufferList *list = new AudioBufferList [size / sizeof(AudioBufferList)];
   err = AudioDeviceGetProperty(devID,
                           0, isInput,
                           kAudioDevicePropertyStreamConfiguration,
                           &size,
                           list);
   if (err != kAudioHardwareNoError) {
      fprintf(stderr, "Can't get %s device (%d) configuration: %s\n",
            isInput ? "input" : "output", devID, errToString(err));
      return 0;
   }

   printf("%s device (%d) has %d buffer%s:\n",
            isInput ? "Input" : "Output", devID, list->mNumberBuffers,
            list->mNumberBuffers > 1 ? "s" : "");

   // Print number of channels in each buffer.
   UInt32 totalChans = 0;
   for (UInt32 i = 0; i < list->mNumberBuffers; i++) {
      UInt32 numChans = list->mBuffers[i].mNumberChannels;
      printf("   Buffer %d has %d channels\n", i, numChans);
      totalChans += numChans;
   }
   printf("   Total: %d channels\n", totalChans);

   delete list;

   return totalChans;
}


int
PrintStreamDescription(UInt32 devID, UInt32 chan, Boolean isInput)
{
   UInt32 size = sizeof(AudioStreamBasicDescription);
   AudioStreamBasicDescription strDesc;

   strDesc.mSampleRate = 0;
   strDesc.mFormatID = 0;
   strDesc.mFormatFlags = 0;
   strDesc.mBytesPerPacket = 0;
   strDesc.mFramesPerPacket = 0;
   strDesc.mBytesPerFrame = 0;
   strDesc.mChannelsPerFrame = 0;
   strDesc.mBitsPerChannel = 0;
   strDesc.mReserved = 0;

   OSStatus err = AudioDeviceGetProperty(devID,
                           chan, isInput,
                           kAudioDevicePropertyStreamFormatMatch,
                           &size,
                           &strDesc);
   if (err != kAudioHardwareNoError) {
      fprintf(stderr, "Can't get %s device stream format for chan %d: %s\n",
            isInput? "input" : "output", chan, errToString(err));
      return -1;
   }

   printf("The %s stream containing channel %d has %d channels per frame.\n",
            isInput? "input" : "output", chan, strDesc.mChannelsPerFrame);

   return 0;
}


int
main(int argc, char *argv[])
{
   UInt32 devID = GetDefaultDeviceID(isOutput);
   if (devID == 0)
      return -1;
   UInt32 totChans = PrintBufferList(devID, isOutput);
   if (totChans == 0)
      return -1;
   printf("\nStream Description -------------------------------------------\n");
   for (UInt32 n = 0; n <= totChans; n++)
      PrintStreamDescription(devID, n, isOutput);

   devID = GetDefaultDeviceID(isInput);
   if (devID == 0)
      return -1;
   totChans = PrintBufferList(devID, isInput);
   if (totChans == 0)
      return -1;
   printf("\nStream Description -------------------------------------------\n");
   for (UInt32 n = 0; n <= totChans; n++)
      PrintStreamDescription(devID, n, isInput);

   return 0;
}

