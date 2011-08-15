#include <stdio.h>
#include <stddef.h>
#include "aacencoder_wrapper.h"

enum stream_format {
  RAW_STREAM = 0,
  ADTS_STREAM = 1,
};

#define DEFAULT_TNS 0

unsigned int aac_enc_open(unsigned long sampleRate,
		  unsigned int numChannels,
		  unsigned long *inputSamples,
		  unsigned long *maxOutputBytes)
{
	faacEncHandle hEncoder = faacEncOpen(sampleRate, numChannels, inputSamples, maxOutputBytes);
	if (NULL == hEncoder)
		return 0;

	faacEncConfigurationPtr expectedEncFormat;

	expectedEncFormat = faacEncGetCurrentConfiguration(hEncoder);

	expectedEncFormat->aacObjectType = LOW;
	expectedEncFormat->mpegVersion = MPEG2;
	expectedEncFormat->useTns = DEFAULT_TNS;
	expectedEncFormat->shortctl = SHORTCTL_NORMAL; //SHORTCTL_NORMAL, SHORTCTL_NOSHORT, SHORTCTL_NOLONG
	expectedEncFormat->allowMidside = 1;

	expectedEncFormat->useLfe = 0;
	expectedEncFormat->quantqual = 0;

	expectedEncFormat->bandWidth = 0; // default, otherwise half of sampling rate
	expectedEncFormat->bitRate = 64000; // 64kbps

	expectedEncFormat->outputFormat = ADTS_STREAM; //RAW_STREAM, ADTS_STREAM
	expectedEncFormat->inputFormat = FAAC_INPUT_FLOAT;

    if (!faacEncSetConfiguration(hEncoder, expectedEncFormat)) {
        fprintf(stderr, "Unsupported output format!\n");
        return 0;
    }

    fprintf(stderr, "Average bitrate: %d kbps\n",
        (expectedEncFormat->bitRate + 500)/1000*numChannels);
    fprintf(stderr, "Quantization quality: %ld\n", expectedEncFormat->quantqual);
    fprintf(stderr, "Bandwidth: %d Hz\n", expectedEncFormat->bandWidth);
    fprintf(stderr, "Object type: ");
    switch(expectedEncFormat->aacObjectType)
	{
	  case LOW:
		  fprintf(stderr, "Low Complexity");
		  break;
	  case MAIN:
		  fprintf(stderr, "Main");
		  break;
	  case LTP:
		  fprintf(stderr, "LTP");
		  break;
	}
    fprintf(stderr, "(MPEG-%d)", (expectedEncFormat->mpegVersion == MPEG4) ? 4 : 2);
    if (expectedEncFormat->useTns)
       fprintf(stderr, " + TNS");
    if (expectedEncFormat->allowMidside)
      fprintf(stderr, " + M/S");
    fprintf(stderr, "\n");

    return (unsigned int)hEncoder;
}

void aac_enc_close(faacEncHandle hEncoder)
{
	faacEncClose(hEncoder);
}

int aac_enc_encode(faacEncHandle hEncoder, int32_t * inputBuffer, unsigned int samplesInput,
		 unsigned char *outputBuffer,
		 unsigned int bufferSize)
{
	return faacEncEncode(hEncoder,
	                (int32_t *)inputBuffer,
	                samplesInput,
	                outputBuffer,
	                bufferSize);
}

