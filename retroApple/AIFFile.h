#ifndef AIFFILEincluded
#define AIFFILEincluded
class CAIFFile : public CMobileSampleContent
{
public:
	DYNOBJECT(CAIFFile)
	DYNDEFPROPERTY

	CAIFFile(void);
	virtual ~CAIFFile(void);
	virtual void Read(istream &ar);
	virtual void Write(ostream &ar);

	enum InfoIndex {	infoTitle,
						infoCopyright,
						infoArranged,
						infoComment,
						infoLast	};

	uint32_t nGetChannels(void) {return m_Header.wChannelCount;};
	uint32_t nGetBitsPerSample(void) {return (int)m_Header.wSampleSize;};
	uint32_t nGetSamplesPerSecond(void) {return m_Header.nSampleRate;};
	uint32_t nGetSamples(void) {return m_Header.nSampleFrames;};
	uint32_t nGetPlaytime();

protected:
	struct AIFFHEADER
	{
		char		form[4];            // 'F','O','R','M'                 
		uint32_t	size;				// size of CMX file from here on 
		char		aiff[4];            // 'A','I','F','F'

		//COMM
		uint16_t 		wChannelCount;				
		uint32_t		nSampleFrames;
		uint16_t 		wSampleSize;
		unsigned char	pcExtFloat_SampleRate[10];
		uint32_t		nSampleRate;

		//META
		char			pcName[256];
		char			pcCopyright[256];
		char			pcAnnotation[256];
		char			pcAuthor[256];

		//MARKERS
		uint16_t 		wNumMarkers;

		//INSTRUMENT
		char			cBaseNote;
		char			cDetune;
		char			cLowNote;
		char			cHighNote;
		char			cLowVelocity;
		char			cHighVelocity;
		uint16_t 		wGain;
		//WORD			wInstrumentPlaymode;

		//SAMPLE / SOUND
		uint32_t		nOffset;
		uint32_t		nBlockSize;
	};

	enum ChunkIDs {	cidCOMM, 
					cidINST,
					cidNAME,
					cidAUTH,
					cidCOPYRIGHT,
					cidANNO,
					cidCOMT,
					cidMARK,
					cidSSND,
					cidMIDI	};

	AIFFHEADER m_Header;

	uint32_t nExtAppleFloatToLong(uint8_t *buffer);
	uint8_t *pcExtLongToAppleFloat(uint8_t *buffer,uint32_t value);
	uint32_t nReadSubchunk(istream &ar, AIFFHEADER *pAiff);

	friend class CAIFProperty;
};
#endif
