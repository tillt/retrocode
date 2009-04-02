#ifndef PACKER_HEADER
#define PACKER_HEADER
#ifdef WIN32
#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif
#else
#undef DllExport
#define DllExport 
#endif

class Packer 
{
public:
    DllExport Packer(std::ostream &stream, bool little);
    DllExport bool write(const char *fmt, void *source);
	DllExport bool write(uint32_t nValue) { return write("l",&nValue); };
	DllExport bool write(uint16_t wValue) { return write("s",&wValue); };
	//DllExport bool write(float fValue) { return write("f",&fValue); };

private:
    std::ostream &ofile;		//stream object
    bool little_endian;			//stream endianess flag
	Endian endian;				//machine endianess object
};

class Unpacker 
{
public:
    DllExport Unpacker(std::istream &stream, bool little);

    DllExport bool read(const char *fmt, void *target);

private:
    std::istream &ifile;		//stream object
    bool little_endian;			//stream endianess flag
	Endian endian;				//machine endianess object
};

#endif

