#include "stdafx.h"

#include <stdlib.h>
#include <sstream>

#include "Integer.h"
#include "Endian.h"
#include "Packer.h"

Packer::Packer(std::ostream &stream, bool little):ofile(stream),endian()
{
    little_endian = little;
	endian.init();
}

bool Packer::write(const char *fmt, void *source)
{
    if(!fmt || !source) return 0;
    int pos = 0;
    int count = 1;
    while(fmt[pos]) 
	{
        switch(fmt[pos]) 
		{
            case 'l':
                for(int i = 0; i < count; ++i) 
				{
                    int32_t l;
                    if(little_endian)
                        l = endian.lToLittle(*(int32_t *)(source));
                    else
                        l = endian.lToBig(*(int32_t *)(source));
                    ofile.write((char*)&l, 4);
                    source = (void*)((int32_t *)source + 1);
                }
                count = 1;
            break;
            case 's': 
                for(int i = 0; i < count; ++i) 
				{
                    short s;
                    if(little_endian)
                        s = endian.wToLittle(*(int16_t *)(source));
                    else
                        s = endian.wToBig(*(int16_t *)(source));
                    ofile.write((char*)&s, 2);
                    source = (void*)((int16_t *)source + 1);
                }
                count = 1;
            break;
            case 'b':
                for(int i = 0; i < count; ++i) 
				{
                    ofile.write((char*)(source), 1);
                    source = (void*)((char*)source + 1);
                }
                count = 1;
            break;
            default:
                char *p;
                count = strtol(&(fmt[pos]), &p, 0);
                pos += (int)(p - &fmt[pos]) - 1;
                if(!count)
                    return false;
        }
        ++pos;
    }
    return true;
}

Unpacker::Unpacker(std::istream &stream, bool little):ifile(stream),little_endian(little)
{
	endian.init();
}


bool Unpacker::read(const char *fmt, void *target)
{
    if(!fmt || !target) return 0;
    int pos = 0;
    int count = 1;
    while(fmt[pos]) 
	{
        char buff[4];
        switch(fmt[pos]) 
		{
            case 'l':
                for(int i = 0; i < count; ++i) 
				{
                    ifile.read(buff, 4);
                    int32_t l;
                    if(little_endian)
                        l = endian.lFromLittle(*(int32_t *)buff);
                    else
                        l = endian.lFromBig(*(int32_t *)buff);
                    *((int32_t *)(target)) = l;
                    target = (void*)((int32_t *)target + 1);
                }
                count = 1;
			break;
            case 's': 
                for(int i = 0; i < count; ++i) 
				{
                    ifile.read(buff, 2);
                    int16_t s;
                    if(little_endian)
                        s = endian.wFromLittle(*(int16_t *)buff);
                    else
                        s = endian.wFromBig(*(int16_t *)buff);
                    *((int16_t *)(target)) = s;
                    target = (void*)((int16_t *)target + 1);
                }
                count = 1;
			break;
            case 'b':
                for(int i = 0; i < count; ++i) 
				{
                    ifile.read(buff, 1);
                    *((char*)(target)) = buff[0];
                    target = (void*)((char*)target + 1);
                }
                count = 1;
            break;
            default:
                char *p;
                count = strtol(&(fmt[pos]), &p, 0);
                pos += (int)(p - &fmt[pos]) - 1;
                if(!count)
                    return false;
        }
        ++pos;
    }
    return true;
}

/*
inline short Packer::host_to_little_s(char *c)
{
    short ret;
    char* pret = (char*)&ret;
#ifdef BIG_ENDIAN
    pret[0] = c[1];
    pret[1] = c[0];
#else
    pret[0] = c[0];
    pret[1] = c[1];
#endif
    return ret;
}

inline short Packer::host_to_big_s(char *c)
{
    short ret;
    char* pret = (char*)&ret;
#ifdef BIG_ENDIAN
    pret[0] = c[0];
    pret[1] = c[1];
#else
    pret[0] = c[1];
    pret[1] = c[0];
#endif
    return ret;
}

inline long Packer::host_to_little_l(char *c)
{
    long ret;
    char* pret = (char*)&ret;
#ifdef BIG_ENDIAN
    pret[0] = c[3];
    pret[1] = c[2];
    pret[2] = c[1];
    pret[3] = c[0];
#else
    pret[0] = c[0];
    pret[1] = c[1];
    pret[2] = c[2];
    pret[3] = c[3];
#endif
    return ret;
}

inline long Packer::host_to_big_l(char *c)
{
    long ret;
    char* pret = (char*)&ret;
#ifdef BIG_ENDIAN
    pret[0] = c[0];
    pret[1] = c[1];
    pret[2] = c[2];
    pret[3] = c[3];
#else
    pret[0] = c[3];
    pret[1] = c[2];
    pret[2] = c[1];
    pret[3] = c[0];
#endif
    return ret;
}

inline short Unpacker::big_to_host_s(char *c)
{
    short ret;
    char* pret = (char*)&ret;
#ifdef BIG_ENDIAN
    pret[0] = c[0];
    pret[1] = c[1];
#else
    pret[0] = c[1];
    pret[1] = c[0];
#endif
    return ret;
}

inline short Unpacker::little_to_host_s(char *c)
{
    short ret;
    char* pret = (char*)&ret;
#ifdef BIG_ENDIAN
    pret[0] = c[1];
    pret[1] = c[0];
#else
    pret[0] = c[0];
    pret[1] = c[1];
#endif
    return ret;
}

inline long Unpacker::big_to_host_l(char *c)
{
    long ret;
    char* pret = (char*)&ret;
#ifdef BIG_ENDIAN
    pret[0] = c[0];
    pret[1] = c[1];
    pret[2] = c[2];
    pret[3] = c[3];
#else
    pret[0] = c[3];
    pret[1] = c[2];
    pret[2] = c[1];
    pret[3] = c[0];
#endif
    return ret;
}

inline long Unpacker::little_to_host_l(char *c)
{
    long ret;
    char* pret = (char*)&ret;
#ifdef BIG_ENDIAN
    pret[0] = c[3];
    pret[1] = c[2];
    pret[2] = c[1];
    pret[3] = c[0];
#else
    pret[0] = c[0];
    pret[1] = c[1];
    pret[2] = c[2];
    pret[3] = c[3];
#endif
    return ret;
}
*/
#ifdef packer_test

#pragma pack(push, 1)
struct t {
    uint32_tl1;
    unsigned char c;
    unsigned char c2[10];
    uint32_tl2;
    unsigned short s;
};
#pragma pack(pop)

int main() {
    struct t ts = { 0x12345678, 0xAA, "123456789", 0xFF0ABCDE, 0x6015 };
    std::ofstream ole("/tmp/little", std::ios::out);
    std::ofstream olb("/tmp/big", std::ios::out);
    Packer le = Packer(ole, true);
    Packer lb = Packer(olb, false);

    le.write("lb10bls", &ts);
    lb.write("lb10bls", &ts);
    
    ole.close();
    olb.close();

    std::ifstream iole("/tmp/little");
    std::ifstream iolb("/tmp/big");
    Unpacker leu = Unpacker(iole, true);
    Unpacker lbu = Unpacker(iolb, false);
    
    struct t tsr;
    printf("0x%X 0x%X %s 0x%X 0x%X\n", ts.l1, ts.c, ts.c2, ts.l2, ts.s);
    
    leu.read("lb10bls", &tsr);
    printf("0x%X 0x%X %s 0x%X 0x%X\n", tsr.l1, tsr.c, tsr.c2, tsr.l2, tsr.s);
   
    lbu.read("lb10bls", &tsr);
    printf("0x%X 0x%X %s 0x%X 0x%X\n", tsr.l1, tsr.c, tsr.c2, tsr.l2, tsr.s);

    iole.close();
    iolb.close();
    return 0;
}

#endif

