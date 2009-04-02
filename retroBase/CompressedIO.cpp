/*\
 * CompressedIO.cpp
 * Copyright (C) 2004-2008, MMSGURU - written by Till Toenshoff
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/
#include "stdafx.h"
#include <memory.h>
#include "zlib.h"
#include "Basics.h"
#include "CompressedIO.h" 

/*\
 * <---------- nDecompressZLIB ----------> 
 * @m decompress data
 * --> I N <-- @p
 * unsigned char *pcSource - compressed source data
 * unsigned int nSrcSize   - size of source data
 * unsigned char *pcDest   - destination data buffer
 * unsigned int nDstSize   - size of destination data buffer
 * <-- OUT --> @r
 * number of bytes used in destination buffer
\*/
int nDecompressZLIB(unsigned char *pcSource,unsigned int nSrcSize,unsigned char *pcDest,unsigned int nDstSize)
{
	#define CHUNK 16384
	int ret=0;
	unsigned int nBufferSize=nDstSize;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

	if (nSrcSize && nDstSize && pcSource && pcDest)
	{
		unsigned int nByteLeft=nSrcSize;
		unsigned have;
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		if ((ret=inflateInit(&strm)) == Z_OK)
		{	
			//decompress until deflate stream ends or end of file
			do 
			{
				//strm.avail_in = fread(in, 1, CHUNK, source);
				strm.avail_in = min(CHUNK,nByteLeft);
				memcpy(in,pcSource,strm.avail_in);
				pcSource+=strm.avail_in;
				nByteLeft-=strm.avail_in;
				
				if (strm.avail_in == 0)
					break;
				strm.next_in = in;
				//run inflate() on input until output buffer not full
				do 
				{
					//just like in def(), the same output space is provided for each call of inflate()
					strm.avail_out = CHUNK;
					strm.next_out = out;

					ret = inflate(&strm, Z_NO_FLUSH);
					if (ret == Z_STREAM_ERROR)
					{
						Log2(verbLevErrors,"zLib stream error\n");
						return false;
					}
					switch (ret) 
					{
						case Z_NEED_DICT:
						case Z_DATA_ERROR:
						case Z_MEM_ERROR:
							(void)inflateEnd(&strm);
							return false;
					}
					have = CHUNK - strm.avail_out;
					if (have > nDstSize)
					{
						Log2(verbLevErrors,"file exceeds buffer by %d bytes\n",have-nDstSize);
						memcpy(pcDest,out,nDstSize);
					}
					else
						memcpy(pcDest,out,have);
					pcDest+=have;
					nDstSize-=have;
					if (nDstSize == 0)
					{
						ret=Z_STREAM_END;
						strm.avail_out=CHUNK;
					}
					//The inner do-loop ends when inflate() has no more output as indicated by not filling the output buffer, just as for deflate(). In this case, we cannot assert that strm.avail_in will be zero, since the deflate stream may end before the file does.
				} while (strm.avail_out == 0);
				//done when inflate() says it's done
			} while (ret != Z_STREAM_END);
			//clean up and return
			(void)inflateEnd(&strm);
		}
		else
			Log2(verbLevErrors,"failed to init ZLib\n");
	}
	return nBufferSize-nDstSize;
}
