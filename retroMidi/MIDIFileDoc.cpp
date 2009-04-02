/*\
 * MIDIFileDoc.cpp
 * Copyright (C) 2004-2007, MMSGURU - written by Till Toenshoff
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
#include <iostream>
#include <fstream>
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/MIDIFile.h"
#include "../retroBase/MIDIFileWriter.h"
#include "MIDIFileDoc.h"
#include "MIDIProperty.h"

DYNIMPPROPERTY(CMIDIFileDoc,CMIDIProperty)

/*\
 *<------------ CMIDIFileDoc ------------>
 @m default constructor
 */
CMIDIFileDoc::CMIDIFileDoc()
{
	m_nMagicSize=4;
	m_pcMagic="MThd";
	m_sFormatName=_T("MIDI");
	m_sDefaultExtension=_T("mid");
}

/*\
 *<------------ ~CMIDIFileDoc ------------>
 @m destructor
 */
CMIDIFileDoc::~CMIDIFileDoc()
{
}

/*\
 *<------------ Load ------------>
 @m load midi file data
 *--> I N <-- @p
 * const char *szPath - pointer to path
 *<-- OUT --> @r
 * BOOL - TRUE=done
 */
bool CMIDIFileDoc::Load(const char *szPath)
{
	return CMIDIFileLoader::Load(szPath) == CMIDIFile::ok;
}

bool CMIDIFileDoc::Save(const char *szFile)
{
	bool bRet=true;
	ofstream f;
	try
	{  
		f.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );
		f.open(szFile,ios::binary);
		//CArchive ar(&f,CArchive::store);
		CMIDIFileWriter::Save(f);
		f.close();
	}
	catch (ofstream::failure const &e)
	{
		Log2(verbLevErrors,"MIDI read file exception: %s\n",e.what());
		bRet=false;
	}
	return bRet;
}

void CMIDIFileDoc::Read(std::istream &ar)
{
	CMIDIFileLoader::Load(ar);
}

void CMIDIFileDoc::Write(std::ostream &ar)
{
	CMIDIFileWriter::Save(ar);
}
