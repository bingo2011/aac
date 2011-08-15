#ifndef _AACENCODER_WRAPPER_H
#define _AACENCODER_WRAPPER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "faac.h"

typedef enum
{
    AAC_48_KBPS,
    AAC_64_KBPS,
    AAC_BITRATE_MAX = AAC_64_KBPS
} E_AAC_BITRATE;

/* encoder interface function */
unsigned int aac_enc_open(unsigned long sampleRate,
		  unsigned int numChannels,
		  unsigned long *inputSamples,
		  unsigned long *maxOutputBytes);

void aac_enc_close(faacEncHandle hEncoder);
int aac_enc_encode(faacEncHandle hEncoder, int32_t * inputBuffer, unsigned int samplesInput,
		 unsigned char *outputBuffer,
		 unsigned int bufferSize);

#if defined(__cplusplus)
}
#endif

#endif
