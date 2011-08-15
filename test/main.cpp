#include "getopt.h"
#include "input.h"
#include "../../audioFilter/inc/fir.h"
#include "../inc/aacencoder_wrapper.h"

#include <string.h>
#include <stdlib.h>
#include <faac.h>

// RTP payloads,
enum
{
    RTP_PT_PCMU = 0,
    RTP_PT_PCMA = 8,
    RTP_PT_G722 = 9,
    RTP_PT_G723 = 4,
	RTP_PT_G723_1 = 4,
    RTP_PT_MPEG = 14,
    RTP_PT_G728 = 15,
    RTP_PT_G729 = 18,
    RTP_PT_WMA = 19,
    RTP_PT_AAC = 96,
    RTP_PT_CODEC_SIREN22_MONO_32 = 106,
    RTP_PT_CODEC_SIREN22_MONO_48 = 107,
    RTP_PT_CODEC_SIREN22_MONO_64 = 108,
    RTP_PT_CODEC_SIREN22_STEREO_64 = 109,
    RTP_PT_CODEC_SIREN22_STEREO_96 = 110,
    RTP_PT_CODEC_SIREN22_STEREO_128 = 111,
    RTP_PT_CODEC_G719_MONO_32 = 112,
    RTP_PT_CODEC_G719_MONO_48 = 113,
    RTP_PT_CODEC_G719_MONO_64 = 114,
    RTP_PT_CODEC_G719_MONO_96 = 115,
    RTP_PT_CODEC_G719_MONO_112 = 116,
    RTP_PT_CODEC_G719_MONO_128 = 117,
    RTP_PT_CODEC_G719_STEREO_64 = 118,
    RTP_PT_CODEC_G719_STEREO_96 = 119,
    RTP_PT_CODEC_G719_STEREO_128 = 120,
    RTP_PT_CODEC_G719_STEREO_192 = 121,
    RTP_PT_CODEC_G719_STEREO_224 = 122,
    RTP_PT_CODEC_G719_STEREO_256 = 123,
    RTP_PT_PCM48 = 125
/*
    RTP_PT_CODEC_SIREN14_24 = 97,
    RTP_PT_CODEC_SIREN14_32 = 98,
    RTP_PT_CODEC_SIREN14_48 = 99,
    RTP_PT_CODEC_G722_1C_24 = 100,
    RTP_PT_CODEC_G722_1C_32 = 101,
    RTP_PT_CODEC_G722_1C_48 = 102,
    RTP_PT_CODEC_SIREN7_16 = 103,
    RTP_PT_CODEC_G722_1_24 = 104,
    RTP_PT_CODEC_G722_1_32 = 105
*/
};


class EncoderInterface
{
public:
	EncoderInterface(int inLen=0,int outLen=0){m_inputLen = inLen; m_outputLen=outLen;}
	virtual ~EncoderInterface(){}
	int GetInputLen(){return m_inputLen;}
	int GetOutputLen(){return m_outputLen;}
	void SetMode(unsigned char mode){m_mode = mode;}
	virtual void OpenEncoderContext(){printf("Encoder Interface base Open, for why? hehe\n");}
	virtual void CloseEncoderContext(){printf("Encoder Interface base Close, for why? hehe\n");}
public:
	virtual void EncFunc(short * pincode,	unsigned char *	poutcode, int *	inLen, int * outLen)=0;
protected:
	int m_inputLen;
	int m_outputLen;
protected:
	unsigned int	m_hEncoderContext;		// Encoder context handle 0
	unsigned char m_mode;
};

// encode a frame, 960 samples, output 160 bytes
class AACEncoder : public EncoderInterface
{
public:
	AACEncoder():EncoderInterface(){}
	~AACEncoder(){}
	void OpenEncoderContext()
	{
		unsigned long sampleRate = 48000;  // 48k
		unsigned int numChannels = 1;      // mono
		unsigned long samplesInput, maxBytesOutput;

		m_hEncoderContext = aac_enc_open(sampleRate, numChannels, &samplesInput, &maxBytesOutput);
		if ( 0 == m_hEncoderContext)
		{
			fprintf(stderr, "Failed to open AAC Encoder\n");
		}
		m_inputLen = samplesInput;
		m_outputLen = maxBytesOutput;

	}
	void CloseEncoderContext(){aac_enc_close((faacEncHandle)m_hEncoderContext);}
	void EncFunc(short* pincode, unsigned char* poutcode, int*	pInputLen, int* pOutputLen)
	{
		int32_t * pcmbuf = (int32_t*)pincode;
		int samplesRead = *pInputLen;
		unsigned char* bitbuf = poutcode;
		unsigned long maxBytesOutput = m_outputLen;
		*pOutputLen = aac_enc_encode((faacEncHandle)m_hEncoderContext,
						pcmbuf,
						samplesRead,
						bitbuf,
						maxBytesOutput);
	}
};


void DoEnc(int format,FILE *fpin,FILE *fpout,int isFilterUsed, pcmfile_t *infile);
void RetrieveRawDataFromWAV(pcmfile_t* infile, FILE *outfile);

int main(int argc, char*argv[])
{
    pcmfile_t *infile = NULL;

    char *audioFileName = NULL;
    char *aacFileName = NULL;
    char *aacFileExt = NULL;
    int aacFileNameGiven = 0;

    int rawChans = 1; // disabled by default
    int rawBits = 16;
    int rawRate = 48000;
    int rawEndian = 0;

	/* begin process command line */
    int opt;
	while((opt = getopt(argc, argv, "or:")) != -1)
	{
		switch (opt) {
		case 'o':
        {
            int l = strlen(optarg);
			aacFileName = (char*)malloc(l+1);
			memcpy(aacFileName, optarg, l);
			aacFileName[l] = '\0';
			aacFileNameGiven = 1;
        }
    	break;
        case 'r':
            rawChans = atoi(optarg); // enable raw input
            break;
		default:
			fprintf(stderr, "Usage: %s [options] [-o outfile] infiles ...\n",
					argv[0]);
			exit(EXIT_FAILURE);
		}
	}

    if ((argc - optind) < 1) {
		fprintf(stderr, "Usage: %s [options] [-o outfile] infiles ...\n",
				argv[0]);
		exit(EXIT_FAILURE);
    }

    while (argc - optind > 0)
	{
		audioFileName = argv[optind++];

		/* generate the output file name, if necessary */
	    if (!aacFileNameGiven) {
	        char *t = strrchr(audioFileName, '.');
	        int l = t ? strlen(audioFileName) - strlen(t) : strlen(audioFileName);
	        aacFileExt = ".aac";
	        aacFileName = (char*)malloc(l+1+4);
	        memcpy(aacFileName, audioFileName, l);
	        memcpy(aacFileName + l, aacFileExt, 4);
	        aacFileName[l+4] = '\0';
	        aacFileExt = strrchr(aacFileName, '.');
	    }
	    else
	        aacFileExt = strrchr(aacFileName, '.');
	}

    /* open the audio input file */
    if (rawChans == 0) // use raw input
    {
    	infile = wav_open_read(audioFileName, 1);
		if (infile)
		{
			infile->bigendian = rawEndian;
			infile->channels = rawChans;
			infile->samplebytes = rawBits / 8;
			infile->samplerate = rawRate;
//			infile->samples /= (infile->channels * infile->samplebytes);
		}
    }
    else // header input
        infile = wav_open_read(audioFileName, 0);

    fprintf(stderr, "The input audio file %s: bigendian=%d, channels=%d, samplebytes=%d, samplerate=%d, samples=%d, isfloat=%d\n",
    		audioFileName,
    		infile->bigendian,
    		infile->channels,
    		infile->samplebytes,
    		infile->samplerate,
    		infile->samples,
    		infile->isfloat);

    if (infile == NULL)
    {
        fprintf(stderr, "Couldn't open input file %s\n", audioFileName);
        return 1;
    }

    FILE *outfile = NULL;
    outfile = fopen(aacFileName, "wb");
    if (!outfile)
	{
		fprintf(stderr, "Couldn't create output file %s\n", aacFileName);
		return 1;
	}

    // Get PCM file from wav
//    RetrieveRawDataFromWAV(infile, outfile);

    DoEnc(RTP_PT_AAC, infile->f, outfile, 0, infile);

    wav_close(infile);

    if (aacFileNameGiven) free(aacFileName);

	return 0;
}


void DoEnc(int format,FILE *fpin,FILE *fpout,int isFilterUsed, pcmfile_t *infile)
{
	unsigned int pFilter_48;

	EncoderInterface* m_pCurEncoderInterface=NULL;
	AACEncoder *m_pAacEncoder = NULL;

	int isMono=1;
    unsigned short encodedBytes=0;
    unsigned short decodedWords=0;
    short bitrateMode=0;
    int NeededSampleRateKHz=0;
	short clockValue=0;		//8 /16

	pFilter_48 = OpenFIR48KFilter();

    switch(format)
    {
	case RTP_PT_AAC:
		m_pAacEncoder = new AACEncoder;
		m_pCurEncoderInterface = m_pAacEncoder;
		m_pAacEncoder->SetMode(AAC_64_KBPS);
//		m_pAacEncoder->SetLenParams();
		encodedBytes = 160;
		decodedWords = 960;	//sample numbers
		break;
	default:
	    printf("enc:wrong format=%d\n",format);
	    exit(1);
	    break;
    }

    if(m_pCurEncoderInterface)
        m_pCurEncoderInterface->OpenEncoderContext();

	printf("Encoding is beginning....isMono=%d\n",isMono);
	int inSize = decodedWords;
	int outSize = encodedBytes;
	unsigned short m_seqno=0;
	unsigned long timeSpan=0;
	unsigned long count=0;

	unsigned long samplesInput = m_pCurEncoderInterface->GetInputLen();
	unsigned long maxBytesOutput = m_pCurEncoderInterface->GetOutputLen();

	float* pcmbuf = (float *)malloc(samplesInput*sizeof(float));
    unsigned char* bitbuf = (unsigned char*)malloc(maxBytesOutput*sizeof(unsigned char));


    int samplesRead = 0;
    while(1)
    {
    	int bytesWritten;
    	samplesRead = wav_read_float32(infile, pcmbuf, samplesInput, NULL);
//    	samplesRead = fread(pcmbuf, sizeof(float), samplesInput, fpin);
        /* all done, bail out */
        if (!samplesRead)
            break ;

        m_pCurEncoderInterface->EncFunc((short *)pcmbuf, bitbuf,  &samplesRead, &bytesWritten);
		fwrite(bitbuf, 1, bytesWritten, fpout);
    }

	printf("Encoding is end....count=%ld\n",count);

	m_pCurEncoderInterface->CloseEncoderContext();
	if(m_pAacEncoder)
		delete m_pAacEncoder;

	CloseFIR48KFilter(pFilter_48);

    fclose(fpout);

    if (pcmbuf) free(pcmbuf);
    if (bitbuf) free(bitbuf);

}


void RetrieveRawDataFromWAV(pcmfile_t* infile, FILE *outfile)
{
    /* open the encoder library */
    faacEncHandle hEncoder;
    unsigned long samplesInput, maxBytesOutput, totalBytesWritten=0;

    hEncoder = faacEncOpen(infile->samplerate, infile->channels,
        &samplesInput, &maxBytesOutput);

    unsigned char *pcmbuf;
    unsigned char *bitbuf;

    pcmbuf = (unsigned char*)malloc(samplesInput*sizeof(unsigned char)*infile->samplebytes);
    bitbuf = (unsigned char*)malloc(maxBytesOutput*sizeof(unsigned char));


    int samplesRead = 0;

    while(1)
    {
        samplesRead = fread(pcmbuf, infile->samplebytes, samplesInput, infile->f);

        /* all done, bail out */
        if (!samplesRead)
            break ;

        fwrite((unsigned char*)pcmbuf, infile->samplebytes, samplesRead, outfile);

    }

    fclose(outfile);
}
