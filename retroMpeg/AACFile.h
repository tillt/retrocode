#ifndef AACFILEincluded
#define AACFILEincluded
/* MicroSoft channel definitions */
#define SPEAKER_FRONT_LEFT             0x1
#define SPEAKER_FRONT_RIGHT            0x2
#define SPEAKER_FRONT_CENTER           0x4
#define SPEAKER_LOW_FREQUENCY          0x8
#define SPEAKER_BACK_LEFT              0x10
#define SPEAKER_BACK_RIGHT             0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#define SPEAKER_BACK_CENTER            0x100
#define SPEAKER_SIDE_LEFT              0x200
#define SPEAKER_SIDE_RIGHT             0x400
#define SPEAKER_TOP_CENTER             0x800
#define SPEAKER_TOP_FRONT_LEFT         0x1000
#define SPEAKER_TOP_FRONT_CENTER       0x2000
#define SPEAKER_TOP_FRONT_RIGHT        0x4000
#define SPEAKER_TOP_BACK_LEFT          0x8000
#define SPEAKER_TOP_BACK_CENTER        0x10000
#define SPEAKER_TOP_BACK_RIGHT         0x20000
#define SPEAKER_RESERVED               0x80000000 

#define MAX_CHANNELS 6 /* make this higher to support files with
                          more channels */

//FAAD file buffering routines
typedef struct 
{
    long bytes_into_buffer;
    long bytes_consumed;
    long file_offset;
    unsigned char *buffer;
	unsigned char *buffer_end;
    int at_eof;
	istream *ar;
} aac_buffer;
 
typedef struct 
{
    int version;
    int channels;
    int sampling_rate;
	bool bVariableBitrate;
    int bitrate;
    int length;
    int object_type;
    int headertype;
} faadAACInfo;

class CAACFile : public CFAACBase
{
public:
	DYNOBJECT(CAACFile)
	DYNDEFPROPERTY

	CAACFile(void);
	virtual ~CAACFile(void);

	static tstring sGetHeaderName(int nHeader);

	bool bMagicHead(std::istream &ar,uint32_t nSize);
	virtual void Read(istream &ar);
	virtual void Write(ostream &out);

    faadAACInfo info;
	
protected:
	bool m_bADIF;
	uint32_t m_nTagSize;
	int ReadADIFHeader(istream &ar);
	int ReadADTSHeader(istream &ar,uint32_t **seek_table,int *seek_table_len,int tagsize, int no_seek_table);

	int nFillBuffer(aac_buffer *b);
	void AdvanceBuffer(aac_buffer *b, int bytes);

	void AdtsParse(aac_buffer *b, int *bitrate, float *length);

	int GetAACFormat(istream &ar,uint32_t **seek_table, int *seek_table_len,int no_seek_table);
};
#endif
