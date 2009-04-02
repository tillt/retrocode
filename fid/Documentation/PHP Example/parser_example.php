<?php
/*\
 *	 parser_example.php
 *
 *	 Copyright (C) 2007, MMSGURU c/o Till Toenshoff
 *   Created 16.04.2004 by Till Toenshoff
 *	 
\*/


class ContentInfo
{
	var $sFileName;
	
	//
	//XML attributes
	//
	var $FilePath;			
	var $FileSize;			
	
	var $Format;			
	var $Formatid;			
	
	var $SubFormat;			
	var $SubFormatid;
	
	var $ContainsMIP;
	var $UsesVibra;
	var $DP2Vibra;			
	var $UsesExtraPerc;
	var $SequencePolyphony;
	
	var $ContainsBackground;
	var $ContainsWallpaper;
	var $ContainsScreensaver;
	var $ContainsRingtone;

	var $StatusBitSave;
	var $StatusBitCopy;
	var $StatusBitEdit;
	var $UsesSamples;
	var $UsesGraphix;
	
	var $Width;
	var $Height;
	var $Colors;
	var $BitsPerPixel;
	var $Frames;
	
	var $Interlaced;
	var $Transparent;
	var $Progressive;
	var $ResolutionX;
	var $ResolutionY;
	var $AvgIFrameDelay;
	
	var $BitRate;
	var $VariableBitrate;
	var $Channels;
	var $SamplesPerSecond;
	var $BitsPerSample;
	var $MaxSamplePlaytime;
		
	var $EncodingScheme;
	var $EncodingSchemeid;
	
	var $MetaCopyright;				
	var $MetaVendor;				
	var $MetaTitle;					
	var $MetaSubTitle;				
	var $MetaArtist;				
	var $MetaComposer;				
	var $MetaWriter;				
	var $MetaYear;					
	var $MetaCategory;				
	var $MetaSubCategory;			
	var $MetaComment;				
	var $MetaPublisher;				
	var $MetaArranger;				
	var $MetaEncodedBy;				
	var $MetaManagedBy;				
	var $MetaManagementInfo;		
	var $MetaCarrier;				
	var $MetaLicenseUse;			
	var $MetaLicenseTerm;			
	var $MetaLicenseUrl;			
	var $MetaExpireDate;			
	var $MetaDateCreated;			
	var $MetaDateRevised;			
	
	var $PlayTime;

    function ContentInfo() 
    {
    }
}

class CFIDParser
{
    var $m_oParser;
    
    var $m_oActive;
    var $m_oParent;
    
    var $m_sLastResult;

    function CFIDParser()
    {
        $this->m_oParser = xml_parser_create();
		xml_parser_set_option($this->m_oParser,XML_OPTION_CASE_FOLDING,0);
	    xml_parser_set_option($this->m_oParser,XML_OPTION_SKIP_WHITE,1);
        xml_set_object($this->m_oParser, &$this);
    }
    
    function sGetLastError()
    {
    	return $this->m_sLastResult;
    }
	
	/*\
	 * <---------------- TraverseStruct---------------->
	 * walk through XML tree and get data
	 * --> I N <--
	 * vals - value pairs
	 * &$i - depth indicator
	\*/
	function TraverseStruct($vals,&$i) 
	{ 
		while (++$i < count($vals))
		{ 
			switch ($vals[$i]['type']) 
			{ 
				case 'complete': 
					eval("\$this->m_oActive->".$vals[$i]['tag']."=\"".$vals[$i]['value']."\";");
					if (is_array($vals[$i]['attributes']))
					{
						foreach($vals[$i]['attributes'] as $key => $value) 
						{
							//attributes are initialized: "tag"+"attribute"="value"
							eval("\$this->m_oActive->".$vals[$i]['tag'].$key."=\"".$value."\";");
						}
					}
  				break; 
				case 'open': 
					if (class_exists($vals[$i]['tag']))
					{
						$iCount=count($this->m_oParent->aoChilds);						
						//echo "\$this->m_oParent->aoChilds[\$iCount]=new ".$vals[$i]['tag']."();<br />\n";												
						eval("\$this->m_oParent->aoChilds[\$iCount]=new ".$vals[$i]['tag']."();");
						$this->m_oActive=&$this->m_oParent->aoChilds[$iCount];
						
						if (is_array($vals[$i]['attributes']))
						{
							foreach($vals[$i]['attributes'] as $key => $value) 
							{
								//attributes are initialized: "tag"+"attribute"="value"
								eval("\$this->m_oActive->".$vals[$i]['tag'].$key."=\"".$value."\";");
							}
						}
						$this->TraverseStruct($vals,$i);
					}
				break; 				
				case 'close': 
					return;
			} 
		};
	} 
    
    /*\
     * <---------------- oParseRequest---------------->
     * parse XML data and fill a ContentInfo object with its data
     * --> I N <--
     * sData - raw XML request 
     * <-- OUT -->
     * CBillRequest object or NULL
    \*/
    function oParseRequest($sData)
    { 
    	$iTransaction=0;
    	$sInput=$sData;
    	$oRequest=NULL;
    	
    	if (isset($this->m_oParent))
    	{
	    	unset($this->m_oParent);
	    	if (isset($this->m_oActive))
	    		unset($this->m_oActive);
	    }
    	
        if(!xml_parse_into_struct($this->m_oParser,$sData,$aVals,$aTags)) 
        {
        	//failed to parse XML
        	$sMsg=sprintf( "XML error: %s -- ", xml_error_string(xml_get_error_code($this->m_oParser)));
        	$nLine=xml_get_current_line_number($this->m_oParser);
        	$asOutput=split("\n",$sInput);
        	$sMsg.="Line ".$nLine.": \"".htmlspecialchars($asOutput[$nLine-1])."\"<br />\n";
        	$this->m_sLastResult=$sMsg;
		}
		else
		{
			//parsing XML accomplished -> fill object with data
	   	   	$oRequest=new ContentInfo();	
		   	$this->m_oActive=&$oRequest;
		   	$this->m_oParent=&$oRequest;
		   	$i=0;
		   	$this->TraverseStruct($aVals,$i);
		}	   	
	    return $oRequest;
    }
}

function sGetNiceMemoryValue($nSize,$bShort=false)
{
    $kb = 1024; 
    $mb = 1024 * $kb; 
    $gb = 1024 * $mb; 
    $tb = 1024 * $gb;
    
    $iStyle=$bShort ? 1 : 0;
    $nFloats=$bShort ? 1 : 2;
    
    $sUnit=ARRAY( ARRAY(	"bytes",
				    		"kilobytes",
				    		"megabytes",
				    		"gigabytes",
				    		"terrabytes"	),
				    ARRAY(	"bytes",
				    		"KB",
				    		"MB",
				    		"GB",
				    		"TB"	));

    if ($nSize < $kb) 
    { 
        $file_size = $nSize." ".$sUnit[$iStyle][0]; 
    } 
    elseif ($nSize < $mb) 
    { 
        $final = round($nSize/$kb,$nFloats); 
        $file_size = $final." ".$sUnit[$iStyle][1]; 
    } 
    elseif ($size < $gb) 
    { 
        $final = round($nSize/$mb,$nFloats); 
        $file_size = $final." ".$sUnit[$iStyle][2]; 
    } 
    elseif($size < $tb) 
    { 
        $final = round($size/$gb,$nFloats); 
        $file_size = $final." ".$sUnit[$iStyle][3]; 
    } 
    else 
    { 
        $final = round($size/$tb,$nFloats); 
        $file_size = $final." ".$sUnit[$iStyle][4]; 
    } 
    return $file_size; 
} 

function sGetNiceValue($nSize)
{
    $kb = 1000; 
    $mb = 1000 * $kb; 
    $gb = 1000 * $mb; 
    $tb = 1000 * $gb; 

    if ($nSize < $kb) 
    { 
        $file_size = $nSize; 
    } 
    elseif ($nSize < $mb) 
    {         
        $file_size = sprintf("%d,%03d",intval($nSize/$kb),($nSize%$kb)); 
    } 
    elseif ($size < $gb) 
    { 
    	$file_size = sprintf("%d,%03d,%03d",intval($nSize/$mb),intval(intval($nSize%$mb)/$kb),intval(intval($nSize%$mb)%$kb)); 
    } 
    elseif($size < $tb) 
    { 
    	$file_size = sprintf("%d,%03d,%03d,%03d",intval($nSize/$gb),intval(intval($nSize%$gb)/$mb),intval(intval(intval($nSize%$gb)%$mb)/$kb),intval(intval(intval($nSize%$gb)%$mb)%$kb)); 
    } 
    else 
    { 
        $file_size=$nSize;
    } 
    return $file_size; 
} 

function sGetNiceDuration($nSeconds)
{
    $nMinute = 60; 
    $nHour = 60 * $nMinute; 
    $nDay = 24 * $nHour; 
    $nWeek = 7 * $nDay; 
    $nMonth = 30 * $nDay; 
    $nYear = 12 * $nMonth; 

    if ($nSeconds < $nMinute) 
    { 
       	$file_size = $nSeconds." second/s"; 
    } 
    elseif ($nSeconds < $nHour) 
    {         
        $file_size = sprintf("%d:%02d minute/s",intval($nSeconds/$nMinute),($nSeconds%$nMinute));
    } 
    elseif ($nSeconds < $nDay) 
    { 
    	$file_size = sprintf("%d:%02d:%02d hour/s",intval($nSeconds/$nHour),intval(($nSeconds%$nHour)/$nMinute),intval(($nSeconds%$nHour)%$nMinute));
    } 
    elseif($nSeconds < $nWeek) 
    { 
        $file_size=$nSeconds;
    } 
    elseif($nSeconds < $nMonth) 
    { 
        $file_size=$nSeconds;
    } 
    elseif($nSeconds < $nYear) 
    { 
        $file_size=$nSeconds;
    } 
    else 
    { 
        $file_size=$nSeconds;
    } 
    return $file_size; 
} 


function sGetProperty (&$oAttribute)
{
	$mapBool=ARRAY(0 => "no", 1 => "yes");
	$sRet="";
	switch($oAttribute->m_iType)
	{
		case typeBoolean:		$sRet=$mapBool[$oAttribute->m_sValue];								break;
		case typeDuration:		$sRet=sGetNiceDuration(intval(intval($oAttribute->m_sValue)/1000));	break;
		case typeShortDuration:	$sRet=$oAttribute->m_sValue." ms";									break;
		case typeValue:			$sRet=sGetNiceValue($oAttribute->m_sValue);							break;
		case typeString:		$sRet=$oAttribute->m_sValue;										break;
		case typeSize:			
			$sRet=sGetNiceMemoryValue($oAttribute->m_sValue)." (".sGetNiceValue($oAttribute->m_sValue)." bytes)";	
		break;
	}
	return $sRet;
}

class ContentInfoAttribute
{
	var $m_iType;
	var $m_sVarName;
	var $m_sValue;
	var $m_sTitle;

	function ContentInfoAttribute($iType,$sVar,$sTitle)
	{
		$this->m_iType=$iType;
		$this->m_sVarName=$sVar;
		$this->m_sTitle=$sTitle;
		$this->m_sValue=NULL;
	}
};

/*\
 * <---------------- DisplayFileProperties---------------->
 * 
 * --> I N <--
 * oContentInfo
 * <-- OUT -->
 * function -
\*/
function DisplayFileProperties(&$oContentInfo)
{	
	$aoAttributes=ARRAY(	new ContentInfoAttribute(typeString,	"Format",				"format"),
							new ContentInfoAttribute(typeString,	"SubFormat",			"type"),
							new ContentInfoAttribute(typeDuration,	"PlayTime",				"playtime"),
							new ContentInfoAttribute(typeSize,		"FileSize",				"size"),
							
							new ContentInfoAttribute(typeBoolean,	"ContainsMIP",			"contains MIP message"),
							new ContentInfoAttribute(typeBoolean,	"UsesVibra",			"uses vibra triggering"),
							new ContentInfoAttribute(typeBoolean,	"DP2Vibra",				"Series40 DP2 compatible vibra"),
							new ContentInfoAttribute(typeBoolean,	"UsesExtraPerc",		"uses extra percussions"),
							new ContentInfoAttribute(typeString,	"SequencePolyphony",	"estimated sequence polyphony"),

							new ContentInfoAttribute(typeBoolean,	"StatusBitSave",		"save protected"),
							new ContentInfoAttribute(typeBoolean,	"StatusBitCopy",		"copy protected"),
							new ContentInfoAttribute(typeBoolean,	"StatusBitEdit",		"edit protected"),
							new ContentInfoAttribute(typeBoolean,	"UsesSamples",			"uses sampled sounds"),
							new ContentInfoAttribute(typeBoolean,	"UsesGraphix",			"uses embedded bitmaps"),
							new ContentInfoAttribute(typeBoolean,	"UsesHumanVoice",		"uses speech synthesis"),
							new ContentInfoAttribute(typeBoolean,	"UsesSynthesizer",		"uses synthesizer"),
							new ContentInfoAttribute(typeShortDuration,	"MaxSamplePlaytime","largest sample playtime"),
							
							new ContentInfoAttribute(typeBoolean,	"ContainsBackground",	"contains background"),
							new ContentInfoAttribute(typeBoolean,	"ContainsWallpaper",	"contains wallpaper"),
							new ContentInfoAttribute(typeBoolean,	"ContainsScreensaver",	"contains screensaver"),
							new ContentInfoAttribute(typeBoolean,	"ContainsRingtone",		"contains ringtone"),

							new ContentInfoAttribute(typeValue,		"SamplesPerSecond",		"samples per second"),
							new ContentInfoAttribute(typeValue,		"BitsPerSecond",		"bit rate"),
							new ContentInfoAttribute(typeValue,		"BitsPerSample",		"bits per sample"),
							new ContentInfoAttribute(typeString,	"Channels",				"channels"),
							
							new ContentInfoAttribute(typeValue,		"Width",				"width"),
							new ContentInfoAttribute(typeValue,		"Height",				"height"),
							new ContentInfoAttribute(typeValue,		"Colors",				"colors"),
							new ContentInfoAttribute(typeValue,		"BitsPerPixel",			"bits per pixel"),
							new ContentInfoAttribute(typeValue,		"Frames",				"frames"),
							
							new ContentInfoAttribute(typeBoolean,	"Interlaced",			"interlaced"),
							new ContentInfoAttribute(typeBoolean,	"Transparent",			"transparent"),
							new ContentInfoAttribute(typeBoolean,	"Progressive",			"progressive"),
							
							new ContentInfoAttribute(typeValue,		"ResolutionX",			"horizontal resolution"),
							new ContentInfoAttribute(typeValue,		"ResolutionY",			"vertical resolution"),
							new ContentInfoAttribute(typeShortDuration,	"AvgIFrameDelay",	"average frame delay"),
							
							new ContentInfoAttribute(typeValue,		"MPEGVersion",			"MPEG version"),
							new ContentInfoAttribute(typeString,	"HeaderType",			"header type"),

							new ContentInfoAttribute(typeString,	"EncodingScheme",		"text encoding"),
							
							new ContentInfoAttribute(typeBoolean,	"Looping",				"is looping"),
							new ContentInfoAttribute(typeValue,		"EventCount",			"events"),
							new ContentInfoAttribute(typeValue,		"SmsCount",				"sms count"),
							
							new ContentInfoAttribute(typeSize,		"BankMemUsage",			"Series40 instrument memory"),
							new ContentInfoAttribute(typeSize,		"TotalMemUsage",		"Series40 total used memory"),
							
							new ContentInfoAttribute(typeString,	"MetaCopyright",		"copyright"),
							new ContentInfoAttribute(typeString,	"MetaPublisher",		"publisher"),
							new ContentInfoAttribute(typeString,	"MetaLicenseUse",		"license use"),
							new ContentInfoAttribute(typeString,	"MetaLicenseTerm",		"license term"),
							new ContentInfoAttribute(typeString,	"MetaLicenseUrl",		"license URL"),
							new ContentInfoAttribute(typeString,	"MetaExpireDate",		"license expire date"),
							new ContentInfoAttribute(typeString,	"MetaVendor",			"vendor"),
							new ContentInfoAttribute(typeString,	"MetaTitle",			"title"),
							new ContentInfoAttribute(typeString,	"MetaSubTitle",			"sub-title"),
							new ContentInfoAttribute(typeString,	"MetaArtist",			"artist"),
							new ContentInfoAttribute(typeString,	"MetaComposer",			"composer"),
							new ContentInfoAttribute(typeString,	"MetaWriter",			"writer"),
							new ContentInfoAttribute(typeString,	"MetaYear",				"year"),
							new ContentInfoAttribute(typeString,	"MetaCategory",			"category"),
							new ContentInfoAttribute(typeString,	"MetaSubCategory",		"subcategory"),
							new ContentInfoAttribute(typeString,	"MetaComment",			"comment"),
							new ContentInfoAttribute(typeString,	"MetaArranger",			"arranger"),
							new ContentInfoAttribute(typeString,	"MetaEncodedBy",		"encoded by"),
							new ContentInfoAttribute(typeString,	"MetaSoftware",			"software"),
							new ContentInfoAttribute(typeString,	"MetaManagedBy",		"managed by"),
							new ContentInfoAttribute(typeString,	"MetaManagementInfo",	"management info"),
							new ContentInfoAttribute(typeString,	"MetaCarrier",			"carrier"),
							new ContentInfoAttribute(typeString,	"MetaDateCreated",		"date created"),
							new ContentInfoAttribute(typeString,	"MetaDateRevised",		"date revised")	);
	
	echo "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" class=\"fol_details\" width=\"100%\">\n";
	for ($i=0;$i < count($aoAttributes);$i++)
	{
		$bAttrSet=false;
		
		//evaluate if the oContentInfo attribute is set
		eval("\$bAttrSet=isset(\$oContentInfo->".$aoAttributes[$i]->m_sVarName.");");
		
		//is attribute set?
		if ($bAttrSet)
		{	//yes->set the attribute property from oContentInfo data
			eval("\$aoAttributes[\$i]->m_sValue=\$oContentInfo->".$aoAttributes[$i]->m_sVarName.";");

			echo "<tr valign=\"top\">";
			echo "<td width=\"30%\">".$aoAttributes[$i]->m_sTitle."</td>";
			echo "<td width=\"70%\">".sGetProperty($aoAttributes[$i])."</td>";
			echo "</tr>\n";
		}
	}
	echo "</table>\n";
}

echo "<html>\n";
echo "<hody>\n";
$sRetroFidPath="/usr/local/bin/fid";
$sExec=$sRetroFidPath." -d \"dontgo.mmf\"";							//create a commanline call

echo "<p>\n";
echo "input: ".$sExec."<br/>\n";
echo "</p>\n";
$sRet=`$sExec`;    										//run RetroFid on that file
echo "<p>\n";
echo "output: <textarea class=\"listview\" name=\"sList\" readonly=\"readonly\" style=\"width: 800px; height: 350px\">";
echo $sRet;
echo  "</textarea>\n";
echo "</p>\n";
echo "parsed to: ";
echo "<p>\n";
$oParser=new CFIDParser();								//instantiate a parser
if (($oRet=$oParser->oParseRequest($sRet)) != NULL)		//did we manage to parse this xml result?
	DisplayFileProperties($oRet);						//yes->display its file's information
else
	echo "internal error: <b>".$oParser->sGetLastError()."</b<br/>\n";	//nope->bomb out
echo "</p>\n";
echo "</hody>\n";
echo "</html>\n";
