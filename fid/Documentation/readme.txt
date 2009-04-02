------------------------------------------------------------------------------
RetroFID(tm)
Copyright (C) 2004-2005, Retro Ringtones LLC; (C) 2006-2009, MMSGURU
$Id: readme.txt,v 1.2 2009/02/21 02:40:46 lobotomat Exp $
------------------------------------------------------------------------------
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------
DESCRIPTION:
                fid reads mobile content files and identifies their content,
                returning the mobile content format and meta-data as xml data.
                Optionally, a PNG thumbnail can be rendered.
------------------------------------------------------------------------------
SYNTAX:
                fid PATH [--details|--version|-d|-v] [--picture|-p WIDTH HEIGHT PNG_PATH]
------------------------------------------------------------------------------
SHORT   LONG            (DEFAULT)       DESCRIPTION
v       version                         display application version information
d       details                         display detailed content information
p       picture                         render a wave display of sample sound
h       help                            show this quick help
------------------------------------------------------------------------------
EXAMPLE:
                fid -d test.mid
OVERVIEW:
For more information, check "Documentation\Documentation.rtf".
------------------------------------------------------------------------------
SUPPORT AND CONTACT:
Please use the RetroCode(tm) mailing list in case you have any problems, 
questions or doubts using RetroCode(tm).

For subscribing to that list, go to: 
https://lists.sourceforge.net/lists/listinfo/retrofid-news

Please use this list and its archive instead of contacting the author directly 
for these reasons;
1.	Your question may already be answered in the list-archives.
2.	You will be read by more users, giving you a higher chance of a quick and 
helpful answer.
3.	The author is reading and answering within that list.
------------------------------------------------------------------------------
SYNTAX:
code [-SHORT] [--LONG] SOURCE DESTINATION
------------------------------------------------------------------------------
GENERAL PARAMETERS: --LONG (-SHORT)

--version (-v)	default: 
This switch will prevent any file processing and trigger only the display the version information of this tool.

--help (-h)	default: 
This switch will prevent any file processing and trigger only the display of a short parameter overview of this tool.
------------------------------------------------------------------------------
CREDITS:
Be aware that some of the used codecs are based on fragmential specifications, released by their original IP holders. Namely Retro's RMF codec is NOT licensed or in any way approved by Beatnik Software.

Retro's SMAF codec is based on: "Specification: Synthetic music Mobile Application Format" Ver.3.06, Copyright (c) 1999-2002 by YAMAHA CORPORATION.

Retro's CMF codec is based on: "Internet-Draft: draft-atarius-cmf-00.txt", Copyright (c) 2004 by The Internet Society; "3GPP2 C.S0050-0 Version 1.0 - 3GPP2 File Formats for Multimedia Services", Copyright (c) 2003 by 3GGP2.

Retro's MFM codec is based on: "Dialogic ADPCM Algorithm", Copyright (c) 1988 by Dialogic Corporation.

The Qcelp codec is entirely based on: "RFC 3625 - The QCP File Format and Media Types for Speech Data", Copyright (c) 2003 by The Internet Society; "Qualcomm Speech Codec Library", Copyright (c) 2003 by QUALCOMM, Inc.

The AAC codec is entirely based on: "FAAC", Copyright (c) 2001 by M. Bakker; "FAAD2", Copyright (c) 2003 by M. Bakker.

The MP3 codec is entirely based on: "LAME", Copyright (c) 1999 by A.L. Faber; "MPG123", Copyright (c) 1995-1997 by Michael Hipp, "libmad", Copyright (c) 2000-2004 by Underbit Technologies, Inc.

The AMR-NB codec is entirely based on: "3GPP specification TS 26.101: Mandatory speech processing functions; AMR speech codec frame structure", Copyright (c) 2003 by 3GPP; "TS 26.104: 3GPP AMR Floating-point Speech Codec V5.1.0"), Copyright (c) 2003 by 3GPP.

The AMR-WB codec is entirely based on: "3GPP specification TS 26.201: Speech Codec speech processing functions; AMR Wideband Speech Codec; Frame Structure", Copyright (c) 2003 by 3GPP; "TS 26.204: 3GPP AMR Wideband Floating-point Speech Codec"), Copyright (c) 2003 by 3GPP.

The SWF codec is entirely based on: "Macromedia Flash (SWF) and Flash Video (FLV) File Format Specification Version 8", Copyright (c) 2006 by Adobe Inc.

The ID3 parser is entirely based on: "id3lib", Copyright (c) 1999, 2000 by Scott Thomas Haug, 2002 by Thijmen Klok.

The MP4 encoder is entirely based on: "mpeg4ip", Copyright (c) 2001 by Cisco Systems Inc.

The Butterworth filter is entirely based on: "Sound Processing Kit - A C++ Class Library for Audio Signal Processing", Copyright (c) 1995-1998 Kai Lassfolk.

The JFIF/JPEG decoder is entirely based on: "The Independent JPEG Group's JPEG software", Copyright (c) 1994-1998, Thomas G. Lane

The GIF decoder is entirely based on "giflib", Copyright(c)  1989, Gershon Elber

The PNG codec is entirely based on "libpng", Copyright (c) 1995-1996, Guy Eric Schalnat

The zip decompression is entirely based on "zlib", Copyright (c) 1995-2004 Jean-loup Gailly and Mark Adler

RetroFID(tm) and all its components were originally assembled in 2005 by Till Toenshoff for inhouse usage at Retro Ringtones LLC.
------------------------------------------------------------------------------
