------------------------------------------------------------------------------
RetroCode(tm)
Copyright (C) 2005, Retro Ringtones LLC, Copyright (C) 2006-2009, MMSGURU
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

==============================================================================
DESCRIPTION:
RetroCode(tm) is a universal mobile content converter.
==============================================================================
OVERVIEW:
For more information, check "Documentation\RetroCode_Documentation.rtf".
==============================================================================
SUPPORT AND CONTACT:
Please use the RetroCode(tm) mailing list in case you have any problems, 
questions or doubts using RetroCode(tm).

For subscribing to that list, go to: 
https://lists.sourceforge.net/lists/listinfo/retrocode-news

Please use this list and its archive instead of contacting the author directly 
for these reasons;
1.	Your question may already be answered in the list-archives.
2.	You will be read by more users, giving you a higher chance of a quick and 
helpful answer.
3.	The author is reading and answering within that list.
==============================================================================
SYNTAX:
code [-SHORT] [--LONG] SOURCE DESTINATION
------------------------------------------------------------------------------
GENERAL PARAMETERS: --LONG (-SHORT)

--version (-v)	default: 
This switch will prevent any file processing and trigger only the display the version information of this tool.

--help (-h)	default: 
This switch will prevent any file processing and trigger onlz the display of a short parameter overview of this tool.

--nometa (-nm)	default: 
The switch will prevent any metadata encoding into the destination file.

--manual (-m)	default: 
This switch will prevent any file processing and trigger only the display this manual.

--output (-o)	default: 2
This attribute allows you to set the verbosity level of this tool. The higher the verbosity level, the more this tool will tell you while working. 0=keep quiet, 1=only errors, 2=warnings & messages, 3=debug1, 4=debug2, 5=debug3.

--samplerate (-s)	default: 0
To adapt the sample rate of your source material, use this rate interpolating function. The parameter is given in hertz [Hz].

--channels (-c)	default: 0
Number of channels (1=mono,2=stereo)

--lowpassfreq (-lpf)	default: 0
Lowpass filter cutoff frequency. Use this in conjunction with any samplerate reduction at the Nyquist frequency. If the samplerate is e.g. adapted towards 8kHz, you should use the lowpass filter at 4kHz. The parameter is given in hertz [Hz].

--highpassfreq (-hpf)	default: 0
Highpass filter cutoff frequency. The parameter is given in hertz [Hz].

--bandpassfreq (-bpf)	default: 0
Bandpass filter center frequency. The parameter is given in hertz [Hz].

--bandpasswidth (-bpw)	default: 15000
Bandpass filter bandwidth. The parameter is given in hertz [Hz].

--volgain (-vg)	default: 0.000000
Amplification gain in dB to amplitude (6.0 = doubled amplification). This parameter is given in decibel [dB].

--vollimitgain (-vl)	default: 0.000000
Peak limiting gain (0.001 - 0.999). For a first attempt, 0.2 might be a good value. This parameter is given in decibel [dB].

--normalize (-n)	default: 
Normalize sample data. Two pass; analyzer and amplifier.

--rmsnormalizer (-rms)	default: 0.000000
[dB].

--autocrop (-sac)	default: 
Prevent gaps by cropping the sample below 0.5db at head and tail. MP3 sources should always be treated with this filter.

--sampleplaytime (-spt)	default: 0
Negative time means the actual playtime reduced by the given value. The value is given in milliseconds [ms].

--ringback (-rb)	default: 0
Render and mix a ringback signal into the original sample. Possible values are 1=USA,2=UK,3=Germany,4=France,5=Fiji

--sampleoffset (-sof)	default: 0
The value is given in milliseconds [ms]. If this value is lower than the actual sample playback length, the sample is cropped to fit.

--samplelooptime (-slt)	default: 0
Ignore the sample playback duration and simply loop it until this value is reached. The value is given in milliseconds [ms]. If this value is lower than the actual sample playback length, the sample is cropped to fit.

--samplefadeintime (-sfi)	default: 0
Use this option in conjunction with the playtime option, set playtime to values like 27000 and fadetime to 3000 to receive a total playback duration of 30000 milliseconds [ms].

--samplefadeouttime (-sfo)	default: 0
Use this option in conjunction with the playtime option, set playtime to values like 27000 and fadetime to 3000 to receive a total playback duration of 30000 milliseconds [ms].

--backgroundcolor (-col)	default: 0
no help available

--framewidth (-frx)	default: 1
no help available

--frameheight (-fry)	default: 1
no help available

--software (-sft)	default: 
Rendering software name.

------------------------------------------------------------------------------
FORMAT SPECIFIC PARAMETERS: --LONG (-SHORT)

Yamaha Synthetic Music Application Format (SMAF/MMF)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Yamaha's SMAF format exists in at least four versions; MA1, MA2, MA3, MA5 and MA7. It allows the encapsulations of various audio formats, grafics and even the usage of a speach synthesizer.

--stream (-ss)	default: false
Samples are stored in streaming format (MA3 and upwards - valid only for 8kHz and below).

--save (-bs)	default: true
If this flag is set, the resulting object will not be savable on many SMAF compatible devices. Set this flag to produce preview files.

--edit (-be)	default: false
If this flag is set, the resulting object will not be editable on most SMAF compatible devices. Set this flag generally to true or 1.

--transfer (-bt)	default: false
If this flag is set, the resulting object will not be copyable on many SMAF compatible devices. Set this flag generally to true or 1.

--protag (-p5)	default: false
If this flag is set, the resulting object contain a tag making the SMAF compatible only on MA5 and above.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--artist (-at)
META: Artist

--note (-no)
META: Note/Message/Remarks

--writer (-wr)
META: Writer

--category (-ca)
META: Category

--subcategory (-sc)
META: SubCategory

--copyright (-cp)
META: Copyright

--vendor (-ve)
META: Vendor

--arranger (-ar)
META: Arranger

--management (-ma)
META: Management info

--managedby (-mb)
META: Managed by

--carrier (-cr)
META: Carrier

--datecreated (-dc)
META: Date created

--daterevised (-dr)
META: Date revised


MacroMedia Flash (SWF)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
MacroMedia Flash is a macro-scripting, audio, graphics and video wrapping format. The audio codec is usually MP3.

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--dtx (-dx)	default: false
Enable DTX encoding.


Wave
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
The Microsoft Wave format allows encapsulation of many audio formats. Among them PCM and ADPCM, the most commonly used.

--reverseorder (-rvo)
Reverse order header output.

--goldwave (-glw)
Goldwave fixed format bug.

--comp (-cc)	default: 0
Determines the compression variant used. 0=no compression, 1=IMA ADPCM, 2=MS ADPCM, 3=QCELP, 4=SAGEM ADPCM.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--artist (-at)
META: Artist

--note (-no)
META: Note/Message/Remarks

--category (-ca)
META: Category

--copyright (-cp)
META: Copyright

--datecreated (-dc)
META: Date created


Beatnik Rich Music Format (RMF)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
The Beatnik Rich Music format allows encapsulation of various audio formats. RMF is one of the few mobile sample formats that allows non-linear playback of sample audio data.

--volume (-vo)	default: 100
Playback volume in percent. Use a value of 100 for best results in most cases.

--tempo (-tm)	default: 100
Playback tempo percentage. Use this option for finetuning/compensating the device dependant playback speed.

--playtime (-pt)	default: 26000
Ignore the sample playback duration and simply loop it until this value is reached. The value is given in milliseconds [ms]. If this value is lower than the actual sample playback length, the sample is cropped to fit.

--paraNumFadetime (-ft)	default: 4000
Fade out duration. The value is given in milliseconds [ms].

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--artist (-at)
META: Artist

--composer (-cm)
META: Composer

--note (-no)
META: Note/Message/Remarks

--category (-ca)
META: Category

--subcategory (-sc)
META: SubCategory

--copyright (-cp)
META: Copyright

--publisher (-pu)
META: Publisher

--licenseuse (-ls)
META: License usage

--licenseterm (-lt)
META: License term

--licenseurl (-lu)
META: License URL

--licenseexp (-le)
META: License expiration date

--source (-sr)
META: Original source

--tempodesc (-td)
META: Tempo description

--index (-id)
META: Index


Panasonic MxxFxxMxx (MFM)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Panasonic's MFM format is a straight linear format with few extra attributes.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--copyright (-cp)
META: Copyright

--encoder (-en)
META: Encoder


VOX
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Dialogic ADPCM usually is used within telephony systems. The used file format is very old, files are sometimes encoded with proprietary or rare codecs - standard however is the Dialog ADPCM codec for VOX files.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--copyright (-cp)
META: Copyright

--encoder (-en)
META: Encoder


AMR
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Narrow Band is a very basic linear sample format. No metadata or any other attribute within the files is possible.

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--dtx (-dx)	default: false
Enable DTX encoding.


AMRWB
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Wide Band is a very basic linear sample format. No metadata or any other attribute within the files is possible.

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--dtx (-dx)	default: false
Enable DTX encoding.


Qualcomm Compact Multimedia Format (CMF/PMD)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Qualcomm's Compact Media Format (CMF / CMX) allows the encapsulation of various audio and graphics formats. It is played only by CDMA devices, hence very limited in compatibility.

--comp (-cc)	default: 0
Determines the compression variant used. 0=no compression, 1=IMA ADPCM, 2=MS ADPCM, 3=QCELP, 4=SAGEM ADPCM.

--volume (-vo)	default: 100
Playback volume in percent. Use a value of 100 for best results in most cases.

--loopcount (-lc)	default: 15
Number of repetitions (2=twice - 15=infinite)

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--copyright (-cp)
META: Copyright

--publisher (-pu)
META: Publisher

--datecreated (-dc)
META: Date created


Advanced Audio Coding (AAC)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Advanced Audio Codec

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--id3 (-i3)	default: false
Enable ID3 tagging.

--mpeg (-mp)	default: 4
2=MPEG2, 4=MPEG4 encoding. Do NOT mix this up with MP4 file encoding, that is a different thing.

--aac (-ac)	default: 1
This attribute allows you to determine the AAC profile used for encoding the audio data. 0=Main, 1=Low Complexity (LC), 2=Scalable Sample Rate (SSR), 3=Long Term Prediction (LTP), 4=High Efficiency (HE).

--adts (-dt)	default: 1
This attribute allows you to determine if the AAC data should be encoded with or without the ADTS frame headers (as needed within MP4 files). 0=raw AAC, 1=ADTS AAC.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--artist (-at)
META: Artist

--note (-no)
META: Note/Message/Remarks

--category (-ca)
META: Category

--copyright (-cp)
META: Copyright


MPEG-4 (MP4)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
n/a

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--id3 (-i3)	default: false
Enable ID3 tagging.

--mpeg (-mp)	default: 4
2=MPEG2, 4=MPEG4 encoding. Do NOT mix this up with MP4 file encoding, that is a different thing.

--aac (-ac)	default: 1
This attribute allows you to determine the AAC profile used for encoding the audio data. 0=Main, 1=Low Complexity (LC), 2=Scalable Sample Rate (SSR), 3=Long Term Prediction (LTP), 4=High Efficiency (HE).

--adts (-dt)	default: 1
This attribute allows you to determine if the AAC data should be encoded with or without the ADTS frame headers (as needed within MP4 files). 0=raw AAC, 1=ADTS AAC.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--note (-no)
META: Note/Message/Remarks

--category (-ca)
META: Category

--copyright (-cp)
META: Copyright


3GPP File Format (3GP)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
n/a

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--id3 (-i3)	default: false
Enable ID3 tagging.

--mpeg (-mp)	default: 4
2=MPEG2, 4=MPEG4 encoding. Do NOT mix this up with MP4 file encoding, that is a different thing.

--aac (-ac)	default: 1
This attribute allows you to determine the AAC profile used for encoding the audio data. 0=Main, 1=Low Complexity (LC), 2=Scalable Sample Rate (SSR), 3=Long Term Prediction (LTP), 4=High Efficiency (HE).

--adts (-dt)	default: 1
This attribute allows you to determine if the AAC data should be encoded with or without the ADTS frame headers (as needed within MP4 files). 0=raw AAC, 1=ADTS AAC.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--note (-no)
META: Note/Message/Remarks

--category (-ca)
META: Category

--copyright (-cp)
META: Copyright


Qualcomm PureVoice (QCELP/QCP)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
n/a


Audio Interchange File Format (AIFF)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
n/a

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--note (-no)
META: Note/Message/Remarks

--writer (-wr)
META: Writer

--copyright (-cp)
META: Copyright


Windows Media Audio (WMA)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
n/a

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--artist (-at)
META: Artist

--note (-no)
META: Note/Message/Remarks

--copyright (-cp)
META: Copyright


Real Audio (RA)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
n/a

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--artist (-at)
META: Artist

--note (-no)
META: Note/Message/Remarks

--copyright (-cp)
META: Copyright


OGG
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
n/a

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 


AVI
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Audio Video Interleaved


uLaw
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
muLaw usually is used within telephony systems originated from the United States. The file format is very old and does not handle any metadata at all.

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--id3 (-i3)	default: false
Enable ID3 tagging.

--vbr (-vb)	default: false
Enable variable bitrate encoding (VBR).

--crc (-cr)	default: false
Enable cyclic redundancy check value encoding (CRC).

--jstereo (-js)	default: false
Enable joint stereo encoding for stereo content. It is recommended to use joint stereo mode whenever the encoded bitrate is lower than 192kbps.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--subtitle (-st)
META: Song/Tone/Object SubTitle

--artist (-at)
META: Artist

--note (-no)
META: Note/Message/Remarks

--category (-ca)
META: Category

--copyright (-cp)
META: Copyright

--encoder (-en)
META: Encoder


aLaw
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
aLaw usually is used within telephony systems originated from Europe. The file format is very old and does not handle any metadata at all.

--reverseorder (-rvo)
Reverse order header output.

--goldwave (-glw)
Goldwave fixed format bug.

--comp (-cc)	default: 0
Determines the compression variant used. 0=no compression, 1=IMA ADPCM, 2=MS ADPCM, 3=QCELP, 4=SAGEM ADPCM.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--artist (-at)
META: Artist

--note (-no)
META: Note/Message/Remarks

--category (-ca)
META: Category

--copyright (-cp)
META: Copyright

--datecreated (-dc)
META: Date created


MPEG-1,2 Level 3 (MP3)
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
MPEG Format 1, 2 and 2.5 Layer 3.

--bitrate (-br)	default: 0
Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant 

--id3 (-i3)	default: false
Enable ID3 tagging.

--vbr (-vb)	default: false
Enable variable bitrate encoding (VBR).

--crc (-cr)	default: false
Enable cyclic redundancy check value encoding (CRC).

--jstereo (-js)	default: false
Enable joint stereo encoding for stereo content. It is recommended to use joint stereo mode whenever the encoded bitrate is lower than 192kbps.

--title (-tt)	default: <FILENAME>
META: Song/Tone/Object Title

--subtitle (-st)
META: Song/Tone/Object SubTitle

--artist (-at)
META: Artist

--note (-no)
META: Note/Message/Remarks

--category (-ca)
META: Category

--copyright (-cp)
META: Copyright

--encoder (-en)
META: Encoder

------------------------------------------------------------------------------
EXAMPLES:
code test_input.wav test_ouput.mmf -tt "Eat my shorts" -at "B.Simpson" --save true
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This will convert the WAVE file "test_input.wav" into a SMAF file named "test_output.mmf", encoding the specified title (-tt) and artist (-at) as metadata and setting the saveable bit (--save) to true.

code hq_input.wav test.aac -br 64000 -c 1 -s 32000 -lpf 16000
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This will read and decode the WAVE file "hq_input.wav", reduce the sample to mono (-c) and the samplerate (-s) into 32kHz. Before reducing the samplerate, a lowpassfilter (-lpf) is applied at 16kHz. The resulting sample is then encoded using the AAC Low Complexity (LC) profile.

code foobar.wav test.mp4 -br 64000 -dt 0 -mp 4
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This will convert the WAVE file "foobar.wav" and encode it into the MP4-file "test.mp4" using the AAC Low Complexity [LC] profile, storing the sample as MPEG4 (-mp) raw AAC (-dt) packets.

code in.rmf out.pmd
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This will read and decode the RMF file "in.rmf" which is then converted into the CMF file "out.pmd"".

code in.mmf out.amr -br 12200
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This will read and decode the SMAF file "in.mmf" which is then converted with a resulting bitrate (-br) of 12200 into the AMR-NB file "out.amr"".

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

RetroCode(tm) and all its components were originally assembled in 2005 by Till Toenshoff for inhouse usage at Retro Ringtones LLC.
------------------------------------------------------------------------------
