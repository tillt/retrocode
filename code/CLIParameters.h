//commandline parameter buffers (currently all static which is a bit tasteless)
converting_parameters code;

//commandline option array
//type - cmdParaSwitch, cmdParaNone, cmdParaNumber, cmdParaString, cmdParaFloat, cmdParaBool 
//id - identifier; unique ONLY in conjunction with the type
//mask - collection of flags signalling the usage of this parameter; formatParaMaskAny, formatParaMaskUpdate
//value - pointer to the buffer that will be filled with the commandline value received for this parameter
//short - short parameter format
//long - long parameter format
//info - quick information
//description - complete description
cmdLinePara cmdPara[]=
{
	{	cmdParaSwitch,	paraSwitchVersion,	formatParaMaskAny,					&code.m_bSwitch[paraSwitchVersion],		
		"v",	"version",		"display application version information", 
		"This switch will prevent any file processing and trigger only the display the version information of this tool."			},

	{	cmdParaSwitch,	paraSwitchHelp,		formatParaMaskAny,					&code.m_bSwitch[paraSwitchHelp],
		"h",	"help",			"display a quick help",
		"This switch will prevent any file processing and trigger onlz the display of a short parameter overview of this tool."	},

	{	cmdParaSwitch,	paraSwitchNoMeta,	formatParaMaskAny,					&code.m_bSwitch[paraSwitchNoMeta],
		"nm",	"nometa",		"suppress meta data insertion",
		"The switch will prevent any metadata encoding into the destination file."},

	{	cmdParaSwitch,	paraSwitchAutoAdapt,formatParaMaskAny,					&code.m_nParameter[paraSwitchAutoAdapt],
		"aa",	"autoadapt",	"automized rate and channel adaption",
		"Automized rate and channel adaption."},

	{	cmdParaSwitch,	paraSwitchManual,	formatParaMaskAny,					&code.m_bSwitch[paraSwitchManual],		
		"m",	"manual",		"display manual pages",
		"This switch will prevent any file processing and trigger only the display this manual."	},

	{	cmdParaNumber,	paraNumVerbosity,	formatParaMaskAny,					&code.m_nParameter[paraNumVerbosity],	
		"o",	"output",		"debugging output verbosity (0-5)",
		"This attribute allows you to set the verbosity level of this tool. The higher the verbosity level, the more this tool will tell you while working. 0=keep quiet, 1=only errors, 2=warnings & messages, 3=debug1, 4=debug2, 5=debug3."	},

	{	cmdParaString,	paraStrPathCodecs,	formatParaMaskAny,					&code.m_strParameter[paraStrPathCodecs],	
		"cod",	"codecdir",		"path to codecs folder",
		"Path to codecs folder"	},

	{	cmdParaNone,	0,					formatParaMaskAny,					NULL,				
		NULL,	NULL,			"editing",
		NULL	},

	{	cmdParaNumber,	paraNumSamplerate,	formatParaMaskAny,					&code.m_nParameter[paraNumSamplerate],	"s",	"samplerate",		
		"sample rate in hz",
		"To adapt the sample rate of your source material, use this rate interpolating function. The parameter is given in hertz [Hz]."},

	{	cmdParaNumber,	paraNumChannels,	formatParaMaskAny,						&code.m_nParameter[paraNumChannels],			
		"c",	"channels",			"number of channels (1=mono,2=stereo)",
		"Number of channels (1=mono,2=stereo)"	},

	{	cmdParaNumber,	paraNumLowpassFreq,	formatParaMaskAny,						&code.m_nParameter[paraNumLowpassFreq],			
		"lpf",	"lowpassfreq",		"lowpass cutoff frequency",
		"Lowpass filter cutoff frequency. Use this in conjunction with any samplerate reduction at the Nyquist frequency. If the samplerate is e.g. adapted towards 8kHz, you should use the lowpass filter at 4kHz. The parameter is given in hertz [Hz]."	},

	{	cmdParaNumber,	paraNumHighpassFreq,formatParaMaskAny,&code.m_nParameter[paraNumHighpassFreq],		
		"hpf",	"highpassfreq",		"highpass cutoff frequency",
		"Highpass filter cutoff frequency. The parameter is given in hertz [Hz]."	},

	{	cmdParaNumber,	paraNumBandpassFreq,formatParaMaskAny,&code.m_nParameter[paraNumBandpassFreq],		
		"bpf",	"bandpassfreq",		"bandpass center frequency",
		"Bandpass filter center frequency. The parameter is given in hertz [Hz]."	},

	{	cmdParaNumber,	paraNumBandpassWidth,formatParaMaskAny,&code.m_nParameter[paraNumBandpassWidth],		
		"bpw",	"bandpasswidth",	"bandpass width/range",
		"Bandpass filter bandwidth. The parameter is given in hertz [Hz]."	},

	{	cmdParaFloat,	paraFloatVolGain,	formatParaMaskAny,&code.m_fParameter[paraFloatVolGain],			
		"vg",	"volgain",			"amplification gain in dB to amplitude",
		"Amplification gain in dB to amplitude (6.0 = doubled amplification). This parameter is given in decibel [dB]."	},

	{	cmdParaFloat,	paraFloatVolLimitGain,formatParaMaskAny,&code.m_fParameter[paraFloatVolLimitGain],		
		"vl",	"vollimitgain",		"peak limiting gain in dB (0.001 - 0.999)",
		"Peak limiting gain (0.001 - 0.999). For a first attempt, 0.2 might be a good value. This parameter is given in decibel [dB]."	},

	{	cmdParaSwitch,	paraSwitchNormalize,formatParaMaskAny,&code.m_bSwitch[paraSwitchNormalize],			
		"n",	"normalize",		"normalize sample data",
		"Normalize sample data. Two pass; analyzer and amplifier."	},

	{	cmdParaFloat,	paraFloatVolRmsNorm,formatParaMaskAny,	&code.m_fParameter[paraFloatVolRmsNorm],		
		"rms",	"rmsnormalizer",	"foo",
		"[dB]."	},

	{	cmdParaString,	paraStrRingbackCountry,	formatParaMaskAny,	&code.m_strParameter[paraStrRingbackCountry],
		"rbc",	"ringbackcountry", "render and mix ringback signal - USA=USA, UK=Great Brittian, DE=Germany, FR=France, FJ=Fiji",
		"Render and mix a ringback signal into the original sample. Possible values are USA, UK, DE, FR, FJ"	},

	{	cmdParaString,	paraStrRingbackXml,	formatParaMaskAny,	&code.m_strParameter[paraStrRingbackXml],
		"rbx",	"ringbackxml", "Location of the ringback tone definition config file.",
		"Path of the itu-ringback-signal configuration file."	},

	{	cmdParaSwitch,	paraSwitchAutoCrop,	formatParaMaskAny,&code.m_bSwitch[paraSwitchAutoCrop],			
		"sac",	"autocrop",		"remove heading and trailing silence",
		"Prevent gaps by cropping the sample below 0.5db at head and tail. MP3 sources should always be treated with this filter."	},

	{	cmdParaNumber,	paraNumSmpPlaytime,	formatParaMaskAny,	&code.m_nParameter[paraNumSmpPlaytime],			
		"spt",	"sampleplaytime", "sample playback duration in milliseconds",
		"Negative time means the actual playtime reduced by the given value. The value is given in milliseconds [ms]."	},

	{	cmdParaNumber,	paraNumSmpOffset,	formatParaMaskAny,	&code.m_nParameter[paraNumSmpOffset],			
		"sof",	"sampleoffset", "sample playback offset in milliseconds",
		"The value is given in milliseconds [ms]. If this value is lower than the actual sample playback length, the sample is cropped to fit."	},

	{	cmdParaNumber,	paraNumSmpLooptime,	formatParaMaskAny,	&code.m_nParameter[paraNumSmpLooptime],			
		"slt",	"samplelooptime", "sample loop time in milliseconds",
		"Ignore the sample playback duration and simply loop it until this value is reached. The value is given in milliseconds [ms]. If this value is lower than the actual sample playback length, the sample is cropped to fit."	},

	{	cmdParaNumber,	paraNumSmpFadeIn,	formatParaMaskAny,	&code.m_nParameter[paraNumSmpFadeIn],			
		"sfi",	"samplefadeintime",	"sample fade in duration in milliseconds",
		"Use this option in conjunction with the playtime option, set playtime to values like 27000 and fadetime to 3000 to receive a total playback duration of 30000 milliseconds [ms]."	},

	{	cmdParaNumber,	paraNumSmpFadeOut,	formatParaMaskAny,	&code.m_nParameter[paraNumSmpFadeOut],			
		"sfo",	"samplefadeouttime","sample fade out duration in milliseconds",
		"Use this option in conjunction with the playtime option, set playtime to values like 27000 and fadetime to 3000 to receive a total playback duration of 30000 milliseconds [ms]."	},

	{	cmdParaNumber,	paraNumBackgroundRGB,	formatParaMaskUpdate,	&code.m_nParameter[paraNumBackgroundRGB],			
		"col",	"backgroundcolor", "background fill color",
		"The value is a 24bit integer representing an RGB value (RRGGBBh). That color is used for the background fill color of the Flash window."},

	{	cmdParaNumber,	paraNumFrameWidth,	formatParaMaskUpdate,	&code.m_nParameter[paraNumFrameWidth],			
		"frx",	"framewidth", "display frame width",
		"no help available"	},

	{	cmdParaNumber,	paraNumFrameHeight,	formatParaMaskUpdate,	&code.m_nParameter[paraNumFrameHeight],			
		"fry",	"frameheight", "display frame height",
		"no help available"	},

	{	cmdParaSwitch,	
		paraSwitchReverseOrder,
		formatParaMaskUpdate,
		&code.m_bSwitch[paraSwitchReverseOrder],			
		"rvo",	"reverseorder",		"reverse order header output",
		"Reverse order header output."	},

	{	cmdParaNone,	0,					0,						NULL,											
		NULL,	NULL,				"encoding"	},

	{	cmdParaNumber,	
		paraNumBitrate,
		formatParaMaskUpdate,
		&code.m_nParameter[paraNumBitrate],	
		"br",	"bitrate","encoded stream bitrate",
		"Encoded stream bitrate. Use carefully, some formats only allow specific bitrates. RA: 12000,24000 AMR-NB: 4750,5150,5900,6700,7400,7950,10020,12200 AMR-WB: 6600,8850,12650,14250,15850,18250,19850,23050,23850 MP3: 8000,16000,24000,32000,40000,48000,56000,64000,80000,96000,112000,128000,160000,192000,224000,256000,320000 AAC: min=8000, max is sample frequency dependant "	},
	{	cmdParaBool,	
		paraBoolStreamSamples,	
		formatParaMaskUpdate,
		&code.m_bParameter[paraBoolStreamSamples],			
		"ss",	"stream",	"use streaming samples",
		"Samples are stored in streaming format (MA3 and upwards - valid only for 8kHz and below)."	},

	{	cmdParaBool,	
		paraBoolAllowDTX,	
		formatParaMaskUpdate,
		&code.m_bParameter[paraBoolAllowDTX],			
		"dx",	"dtx",	"allow DTX encoding",
		"Enable DTX encoding."	},

	{	cmdParaBool,	
		paraBoolAllowID3,	
		formatParaMaskUpdate,
		&code.m_bParameter[paraBoolAllowID3],			
		"i3",	"id3",	"allow ID3 tagging",
		"Enable ID3 tagging."	},

	{	cmdParaBool,	
		paraBoolAllowVBR,	
		formatParaMaskUpdate,
		&code.m_bParameter[paraBoolAllowVBR],			
		"vb",	"vbr",	"variable bitrate",
		"Enable variable bitrate encoding (VBR)."	},

	{	cmdParaBool,	
		paraBoolCopyrighted,	
		formatParaMaskUpdate,
		&code.m_bParameter[paraBoolCopyrighted],			
		"cpb",	"copyrightbit",	"copyright bit",
		"Enable the copyright flag."	},

	{	cmdParaBool,	
		paraBoolAllowCRC,	
		formatParaMaskUpdate,
		&code.m_bParameter[paraBoolAllowCRC],			
		"cr",	"crc",	"set CRC encoding",
		"Set cyclic redundancy check value encoding (CRC)."	},

	{	cmdParaBool,	
		paraBoolAllowJointStereo,	
		formatParaMaskUpdate,
		&code.m_bParameter[paraBoolAllowJointStereo],			
		"js",	"jstereo",	"enable joint-stereo encoding",
		"Set joint stereo encoding for stereo content. It is recommended to use joint stereo mode whenever the encoded bitrate is lower than 192kbps."	},

	{	cmdParaNumber,	
		paraNumQuality,	
		formatParaMaskUpdate,	
		&code.m_nParameter[paraNumQuality],			
		"ql",	"quality", "0=best, 2=high, 5=medium, 7=low",
		"Determine the encoding quality; 0=best, 2=high, 5=medium, 7=low."	},

	{	cmdParaSwitch,	
		paraSwitchGoldWave,
		formatParaMaskUpdate,
		&code.m_bSwitch[paraSwitchGoldWave],			
		"glw",	"goldwave",			"goldwave fixed format bug",
		"Goldwave fixed format bug."	},

	{	cmdParaSwitch,	
		paraSwitchFact,
		formatParaMaskUpdate,
		&code.m_bSwitch[paraSwitchFact],			
		"fct",	"fact",				"include the fact-chunk",
		"Include the fact-chunk in compressed Wave-files. This chunk is marked as being mandatory in the latest Wave-format-specifications. However it is usually NOT needed for proper processing/playback and will slightly increase the resulting filesize. Use this for Siemens S55 handsets."	},

	{	cmdParaNumber,	
		paraNumCompression,		
		formatParaMaskUpdate,
		&code.m_nParameter[paraNumCompression],				
		"cc",	"comp",	"1=IMA ADPCM, 2=MS ADPCM, 3=QCELP compression, 4=SAGEM ADPCM",
		"Determines the compression variant used. 0=no compression, 1=IMA ADPCM, 2=MS ADPCM, 3=QCELP, 4=SAGEM ADPCM."	},

	{	cmdParaNumber,	
		paraNumEncode,		
		formatParaMaskUpdate,
		&code.m_nParameter[paraNumEncode],
		"mp",	"mpeg", "2=MPEG2, 4=MPEG4 encoding",
		"2=MPEG2, 4=MPEG4 encoding. Do NOT mix this up with MP4 file encoding, that is a different thing."	},

	{	cmdParaNumber,	
		paraNumAac,
		formatParaMaskUpdate,
		&code.m_nParameter[paraNumAac],					
		"ac",	"aac",	"0=Main, 1=LC, 2=SSR, 3=LTP, 4=HE profile",
        "This attribute allows you to determine the AAC profile used for encoding the audio data. 0=Main, 1=Low Complexity (LC), 2=Scalable Sample Rate (SSR), 3=Long Term Prediction (LTP), 4=High Efficiency (HE)."	},

	{	cmdParaNumber,	paraNumBlockSize,	formatParaMaskAny,	&code.m_nParameter[paraNumBlockSize],			
		"sbs",	"blocksize",	"IMA-/MS-ADPCM number of bytes per block",
		"Number of samples per block. Default is 256 at 8kHz sample rate. By default, that value doubles with raising sample rate in steps of 8kHz. Higher values lead to slightly better compression ratios with the cost of slight quality reduction. Make sure the value is devideable by 32."	},

	{	cmdParaNumber,	
		paraNumAdts,
		formatParaMaskUpdate,
		&code.m_nParameter[paraNumAdts],
		"dt",	"adts",	"0=raw, 1=ADTS format",
		"This attribute allows you to determine if the AAC data should be encoded with or without the ADTS frame headers (as needed within MP4 files). 0=raw AAC, 1=ADTS AAC."	},

	{	cmdParaNumber,	
		paraNumVolume,
		formatParaMaskUpdate,
		&code.m_nParameter[paraNumVolume],
		"vo",	"volume","playback volume percentage (1-100)",
		"Playback volume in percent. Use a value of 100 for best results in most cases."	},

	{	cmdParaNumber,	paraNumTempo,		formatParaMaskUpdate,	&code.m_nParameter[paraNumTempo],				
		"tm",	"tempo", "playback tempo percentage",
		"Playback tempo percentage. Use this option for finetuning/compensating the device dependant playback speed."	},

	{	cmdParaNumber,	paraNumPlaytime,	formatParaMaskUpdate,	&code.m_nParameter[paraNumPlaytime],			
		"pt",	"playtime", "sequence playback duration in milliseconds",
		"Ignore the sample playback duration and simply loop it until this value is reached. The value is given in milliseconds [ms]. If this value is lower than the actual sample playback length, the sample is cropped to fit."	},

	{	cmdParaNumber,	paraNumFadetime,	formatParaMaskUpdate,	&code.m_nParameter[paraNumFadetime],			
		"ft",	"paraNumFadetime", "sequence fade duration in milliseconds",
		"Fade out duration. The value is given in milliseconds [ms]."	},

	{	cmdParaNumber,	paraNumLoopcount,	formatParaMaskUpdate,	&code.m_nParameter[paraNumLoopcount],			
		"lc",	"loopcount", "number of repetitions (2=twice - 15=infinite)",
		"Number of repetitions (2=twice - 15=infinite)"	},

	{	cmdParaBool,	paraBoolSaveEnabled,formatParaMaskUpdate,	&code.m_bParameter[paraBoolSaveEnabled],		
		"bs",	"save",	"save protection flag",
		"If this flag is set, the resulting object will not be savable on many SMAF compatible devices. Set this flag to produce preview files."	},

	{	cmdParaBool,	paraBoolEditEnabled,formatParaMaskUpdate,	&code.m_bParameter[paraBoolEditEnabled],		
		"be",	"edit",	"edit protection flag",
		"If this flag is set, the resulting object will not be editable on most SMAF compatible devices. Set this flag generally to true or 1."	},
	
	{	cmdParaBool,	paraBoolTransferEnabled,formatParaMaskUpdate,&code.m_bParameter[paraBoolTransferEnabled],	
		"bt",	"transfer",	"transfer protection flag",
		"If this flag is set, the resulting object will not be copyable on many SMAF compatible devices. Set this flag generally to true or 1."	},

	{	cmdParaBool,	paraBoolProTag,formatParaMaskUpdate,&code.m_bParameter[paraBoolProTag],	
		"p5",	"protag",	"pro tag insertion flag",
		"If this flag is set, the resulting object contain a tag making the SMAF compatible only on MA5 and above. Valid only when the streaming sample format has been selected (stream/ss)."},

	{	cmdParaNumber,	paraNumDevice,formatParaMaskUpdate,	&code.m_nParameter[paraNumDevice],			
		"did",	"deviceid",	"0=Auto, 1=MA2, 2=MA3, 3=MA5, 4=MA7",
			"Device specific messages targeting a Yamaha SMAF chip model. Within a SMAF file, a few system exclusive messages are used - that means it contains device specific data marked by a device identifier. Set implicitely by the attributes of the input sample format when 0=Auto is used. 1*=MA2 (YMU786), 2=MA3 (YMU762), 3=MA5 (YMU765), 4=MA7 (YMU786). Valid only when the streaming sample format has been selected (stream/ss). (*) Do not use as streaming samples dont work on MA2 devices."	},

	{	cmdParaString,	paraStrImageExportPath,	formatParaMaskUpdate,	&code.m_strParameter[paraStrImageExportPath],			
		"iep",	"imageexportprefix",	"path prefix for the image data",
		""	},

	{	cmdParaNone,	0,						0,					NULL,				
		NULL,	NULL,	"metadata"	},

	{	cmdParaString,	
		paraStrTitle,
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrTitle],	
		"tt",	"title",	"meta title",
		"META: Song/Tone/Object Title"	},

	{	cmdParaString,	paraStrSubTitle,	formatParaMaskUpdate,	&code.m_strParameter[paraStrSubTitle],			
		"st",	"subtitle",	"meta sub title",
		"META: Song/Tone/Object SubTitle"	},

	{	cmdParaString,	
		paraStrArtist,
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrArtist],			
		"at",	"artist",	"meta artist",
		"META: Artist"	},

	{	cmdParaString,	
		paraStrComposer,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrComposer],			
		"cm",	"composer",	"meta composer",
		"META: Composer"	},

	{	cmdParaString,	
		paraStrNote,
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrNote],				
		"no",	"note",	"meta note/message/remarks",
		"META: Note/Message/Remarks"	},
	
	{	cmdParaString,
		paraStrWriter,		
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrWriter],			
		"wr",	"writer","meta writer",
		"META: Writer"	},

	{	cmdParaString,	
		paraStrCategory,
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrCategory],			
		"ca",	"category",	"meta category",
		"META: Category"	},
	
	{	cmdParaString,	
		paraStrSubcategory,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrSubcategory],		
		"sc",	"subcategory",	"meta sub-category",
		"META: SubCategory"	},
	
	{	cmdParaString,	
		paraStrCopyright,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrCopyright],			
		"cp",	"copyright","meta copyright",
		"META: Copyright"	},
	
	{	cmdParaString,	paraStrVendor,		formatParaMaskUpdate,	&code.m_strParameter[paraStrVendor],			
		"ve",	"vendor",	"meta vendor",
		"META: Vendor"	},
	
	{	cmdParaString,	
		paraStrPublisher,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrPublisher],			
		"pu",	"publisher","meta publisher",
		"META: Publisher"	},
	
	{	cmdParaString,	
		paraStrArranger,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrArranger],			
		"ar",	"arranger",	"meta arranger",
		"META: Arranger"	},
	
	{	cmdParaString,	
		paraStrEncoder,		
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrEncoder],			
		"en",	"encoder",	"meta encoder",
		"META: Encoder"	},

	{	cmdParaString,	paraStrSoftware,0,	&code.m_strParameter[paraStrSoftware],			
		"sft",	"software",			"software name",
		"META: Rendering software name."	},
	
	{	cmdParaString,	
		paraStrManagement,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrManagement],		
		"ma",	"management",	"meta management info",
		"META: Management info"	},

	{	cmdParaString,	
		paraStrManagedBy,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrManagedBy],			
		"mb",	"managedby",	"meta managed by",
		"META: Managed by"	},

	{	cmdParaString,		
		paraStrCarrier,		
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrCarrier],			
		"cr",	"carrier",	"meta carrier",
		"META: Carrier"	},

	{	cmdParaString,	
		paraStrLicenseUse,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrLicenseUse],		
		"ls",	"licenseuse","meta license usage",
		"META: License usage"	},
	
	{	cmdParaString,	paraStrLicenseTerm,	formatParaMaskUpdate,	&code.m_strParameter[paraStrLicenseTerm],		
		"lt",	"licenseterm","meta license term",
		"META: License term"	},

	{	cmdParaString,	paraStrLicenseUrl,	formatParaMaskUpdate,	&code.m_strParameter[paraStrLicenseUrl],		
		"lu",	"licenseurl",	"meta license url",
		"META: License URL"	},

	{	cmdParaString,	paraStrLicenseExp,	formatParaMaskUpdate,	&code.m_strParameter[paraStrLicenseExp],
		"le",	"licenseexp",	"meta license expiration date",
		"META: License expiration date"	},
	
	{	cmdParaString,	
		paraStrDateCreated,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrDateCreated],
		"dc",	"datecreated",	"meta date created",
		"META: Date created"	},

	{	cmdParaString,
		paraStrDateRevised,	
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrDateRevised],
		"dr",	"daterevised",	"meta date revised",
		"META: Date revised"	},

	{	cmdParaString,
		paraStrSource,
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrSource],
		"sr",	"source",		"meta original source",
		"META: Original source"},

	{	cmdParaString,	
		paraStrTempo,		
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrTempo],
		"td",	"tempodesc",	"meta tempo description",
		"META: Tempo description"},

	{	cmdParaString,
		paraStrIndex,
		formatParaMaskUpdate,
		&code.m_strParameter[paraStrIndex],
		"id",	"index",	"meta index",
		"META: Index"},

		{	0,0,0,NULL,NULL,NULL,NULL	}
};	
