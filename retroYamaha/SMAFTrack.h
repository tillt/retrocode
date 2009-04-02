#ifndef SMAFTRACKincluded
#define SMAFTRACKincluded
class CSMAFSample;
class CSMAFEvent;

#define MIDI_SYSXGID_SMAF	0x79
#define MIDI_SYSXPID_MA3	6
#define MIDI_SYSXPID_MA5	7
#define MIDI_SYSXPID_MA7	8

class CSMAFTrack : public CSMAFDecoder
{
public:
	class ZeroEvent
	{
	public:
		ZeroEvent(int nSize,const TCHAR *szDesc,void *pPara=NULL) : nEventSize(nSize),szDescription(szDesc),pParameter(pPara){};
		//ZeroEvent(int nSize,const TCHAR *szDesc,unsigned char cPara) : nEventSize(nSize),szDescription(szDesc),cParameter(cPara),pParameter(NULL){};
		~ZeroEvent(){};

		int nEventSize;
		const TCHAR *szDescription;
		
		void *pParameter;
		unsigned char *cParameter;
	};


	CSMAFTrack(int nChannelOffset=0);
	~CSMAFTrack();

	static int nDecodeVariableQuantity (unsigned char **pcBuffer,unsigned char *pcLimit,int *n);
	static int nEncodeVariableQuantity (unsigned int nValue,unsigned char *pcBuffer); 
	static int nDecodeVariableLength(unsigned char **pcBuffer,unsigned char *pcLimit,int *n);

	uint32_t nGetEventCount(){return (uint32_t)m_Events.size();};
	CSMAFEvent *pGetEvent(int iEvent){return m_Events[iEvent];};

	uint32_t nGetPlaytime();

	void CheckBankChanges(void);

	void Decode(unsigned char **pcBuffer,unsigned int nSize);
	uint32_t nGetType(void){return m_nType;};
	uint32_t nGetFormat(void){return m_nFormat;};

	tstring sParseSysEx(char *pcEvent,int nSize,bool bMTSU);
	static int nRenderSysEx(char *pcDest,int nDeviceId,const char *pcMsg,unsigned char nSize);
	static int nRenderMainVolumeSysEx(char *pcDest, int nDeviceId, unsigned char cVol);
	static int nRenderSetupResetSysEx(	char *pcDest,	int nDeviceId);
	static int nRenderSetupChnReserveSysEx(	char *pcDest,	int nDeviceId, unsigned char cChannels);
	
	uint32_t nGetSampleCount(){return (uint32_t)m_Samples.size();};
	bool bUsesHumanVoice(){return m_bUsesHumanVoice;};
	bool bUsesSynthesizer(){return m_bUsesSynthesizer;};
	CSMAFSample *pGetSample(int iIndex);
	uint32_t nGetMaxSamplePlaytime();

	uint32_t nGetHighestLocatedDeviceId(void) { return m_nHighestLocatedDeviceId; };

	enum TrackTypes {	typeUndefined,
						typeHumanVoice,
						typeSequence,
						typeSample,
						typeInstrument,
						typeSetup};

	enum TrackFormats {	formatHandyphone,
						formatMobileCompressed,
						formatMobileUncompressed,
						formatMA7	};

	enum TrackChunkIDs {	tcidMsp,
							tcidMspI,
							tcidMtsu,
							tcidMthv,
							tcidMtsp,
							tcidMSTR,
							tcidMtsq	};

	enum HumanVoiceTrackChunkIDs {	hvidMhvs,
									hvidMhsc	};


protected:
	void DecodeMtsq_handy(unsigned char *pcBuffer,unsigned int nSize);
	void DecodeMtsq_mobile(unsigned char *pcBuffer,unsigned int nSize);
	void DecodeMtsp(unsigned char *pcBuffer,unsigned int nSize);
	void DecodeMtsu_handy(unsigned char *pcBuffer,unsigned int nSize);
	void DecodeMtsu_mobile(unsigned char *pcBuffer,unsigned int nSize);
	void DecodeMthv(unsigned char *pcBuffer,unsigned int nSize);

	vector<CSMAFSample *> m_Samples;
	vector<CSMAFEvent *> m_Events;

	uint32_t m_nFormat;
	uint32_t m_nType;
	uint32_t m_nResolution;
	bool m_bUsesHumanVoice;
	bool m_bUsesSynthesizer;
	uint32_t m_nChannelOffset;
	uint32_t m_nHighestLocatedDeviceId;
};
#endif
