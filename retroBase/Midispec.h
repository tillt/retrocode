/*
 *  adopted from NeXTstep 3.2 1995 by Till Toenshoff
 *
 *  File:   mididriver/midi_spec.h
 *	Author:	David Jaffe
 *	Copyright (C) 1991, NeXT, Inc.
 *      MIDI definitions
 */

/*
SP-MIDI Vibra Activate
0xB? 0x00 0x79	
0xB? 0x20 0x06
0xC? 0x7C
*/

/*
 * MIDI status bytes (from International MIDI Association document
 * MIDI-1.0, August 5, 1983)
 */
#define MIDI_KARAPHRASE			0x10
#define MIDI_KARALINE			0x20
#define MIDI_KARASCREEN			0x30

#define MIDI_TEMPO              0x60

#define MIDI_NOTEOFF            0x80
#define MIDI_NOTEON             0x90
#define MIDI_POLYPRES           0xA0
#define MIDI_CONTROL            0xB0
#define MIDI_PROGRAM            0xC0
#define MIDI_CHANPRES           0xD0
#define MIDI_PITCH              0xE0 
#define MIDI_SYSTEM             0xF0
#define MIDI_CHANMODE           MIDI_CONTROL

//#define xx 1 20 0 allsounds off
//#define xx 1 21 0 reset all
//#define xx 1 23 0 allnotes off

#define MIDI_SYSEXCL            (MIDI_SYSTEM | 0x0)
#define MIDI_TIMECODEQUARTER	(MIDI_SYSTEM | 0x1)
#define MIDI_SONGPOS            (MIDI_SYSTEM | 0x2)
#define MIDI_SONGSEL            (MIDI_SYSTEM | 0x3)
#define MIDI_TUNEREQ            (MIDI_SYSTEM | 0x6)
#define MIDI_EOX                (MIDI_SYSTEM | 0x7)
#define MIDI_CLOCK              (MIDI_SYSTEM | 0x8)
#define MIDI_START              (MIDI_SYSTEM | 0xa)
#define MIDI_CONTINUE           (MIDI_SYSTEM | 0xb)
#define MIDI_STOP               (MIDI_SYSTEM | 0xc)
#define MIDI_ACTIVE             (MIDI_SYSTEM | 0xe)
#define MIDI_RESET              (MIDI_SYSTEM | 0xf)

#define MIDI_MAXDATA            0x7f
#define MIDI_OP(y)              (y & (MIDI_STATUSMASK))
#define MIDI_DATA(y)            (y & (MIDI_MAXDATA))
#define MIDI_MAXCHAN            0x0f
#define MIDI_MAXTRACKS          120
#define MIDI_NUMCHANS           16
#define MIDI_NUMKEYS            128
#define MIDI_ZEROBEND           0x2000
#define MIDI_DEFAULTVELOCITY    64

/* MIDI Controller numbers */
#define MIDI_BANKSELECT			0
#define MIDI_MODWHEEL           1
#define MIDI_BREATH             2
#define MIDI_FOOT               4
#define MIDI_PORTAMENTOTIME     5
#define MIDI_DATAENTRY          6
#define MIDI_MAINVOLUME         7
#define MIDI_BALANCE            8
#define MIDI_PAN                10
#define MIDI_EXPRESSION         11
#define MIDI_EFFECTCONTROL1     12
#define MIDI_EFFECTCONTROL2     13

/* LSB for above */
#define MIDI_MODWHEELLSB        (1 + 31)
#define MIDI_BREATHLSB          (2 + 31)
#define MIDI_FOOTLSB            (4 + 31)
#define MIDI_PORTAMENTOTIMELSB  (5 + 31)
#define MIDI_DATAENTRYLSB       (6 + 31)
#define MIDI_MAINVOLUMELSB      (7 + 31)
#define MIDI_BALANCELSB         (8 + 31)
#define MIDI_PANLSB             (10 + 31)
#define MIDI_EXPRESSIONLSB      (11 + 31)

#define MIDI_DAMPER             64
#define MIDI_PORTAMENTO         65
#define MIDI_SOSTENUTO          66
#define MIDI_SOFTPEDAL          67
#define MIDI_HOLD2              69
/*
 * Controller 91-95 definitions from original 1.0 MIDI spec
 */
#define MIDI_EXTERNALEFFECTSDEPTH   91
#define MIDI_TREMELODEPTH           92
#define MIDI_CHORUSDEPTH            93
#define MIDI_DETUNEDEPTH            94
#define MIDI_PHASERDEPTH            95
/*
 * Controller 91-95 definitions as of June 1990
 */
#define MIDI_EFFECTS1           91
#define MIDI_EFFECTS2           92
#define MIDI_EFFECTS3           93
#define MIDI_EFFECTS4           94
#define MIDI_EFFECTS5           95
#define MIDI_DATAINCREMENT      96
#define MIDI_DATADECREMENT      97

#define MIDI_ALLSOUNDSOFF		0x78
#define MIDI_RESETCONTROLLERS	0x79
#define MIDI_LOCALCONTROL       0x7A
#define MIDI_ALLNOTESOFF        0x7B
#define MIDI_OMNIOFF            0x7C
#define MIDI_OMNION             0x7D
#define MIDI_MONO               0x7E
#define MIDI_POLY               0x7F



/* Masks for disassembling MIDI status bytes */
#define	MIDI_STATUSBIT	0x80	/* indicates this is a status byte */
#define	MIDI_STATUSMASK	0xf0	/* bits indicating type of status req */
#define	MIDI_SYSRTBIT	0x08	/* differentiates SYSRT from SYSCOM */

/* Some useful parsing macros. */
#define MIDI_TYPE_SYSTEM_REALTIME(byte)	(((byte)&0xf8) == 0xf8)
#define MIDI_TYPE_1BYTE(byte)	(   MIDI_TYPE_SYSTEM_REALTIME(byte) \
				 || (byte) == 0xf6 || (byte) == 0xf7)
#define MIDI_TYPE_2BYTE(byte)	(   (((byte)&0xe0) == 0xc0) \
				 || (((byte)&0xe0) == 0xd0) \
				 || ((byte)&0xfd) == 0xf1)
#define MIDI_TYPE_3BYTE(byte)	(   ((byte)&0xc0) == 0x80 \
				 || ((byte)&0xe0) == 0xe0 \
				 || (byte) == 0xf2)
#define MIDI_TYPE_SYSTEM(byte)	(((byte)&0xf0) == 0xf0)
#define MIDI_EVENTSIZE(byte)    (MIDI_TYPE_1BYTE(byte) ? 1 : \
				 MIDI_TYPE_2BYTE(byte) ? 2 : 3)



//some metaevents
#define SEQUENCENUMBER  0x00
#define KARAOKETEXT		0x01
#define COPYRIGHT		0x02
#define TITLE			0x03
#define CHANNELPREFIX   0x20
#define TRACKCHANGE     0x2F
#define TEMPOCHANGE     0x51
#define SMPTEOFFSET     0x54
#define TIMESIG         0x58
#define KEYSIG          0x59
#define SEQUENCERMETA   0x7F    //


   /*

****     TABLE 1  -  General MIDI Instrument Patch Map      ****
(groups sounds into sixteen families, w/8 instruments in each family)

Prog#     Instrument               Prog#     Instrument

   (1-8        PIANO)                   (9-16      CHROM PERCUSSION)
1         Acoustic Grand             9        Celesta
2         Bright Acoustic           10        Glockenspiel
3         Electric Grand            11        Music Box
4         Honky-Tonk                12        Vibraphone
5         Electric Piano 1          13        Marimba
6         Electric Piano 2          14        Xylophone
7         Harpsichord               15        Tubular Bells
8         Clav                      16        Dulcimer

   (17-24      ORGAN)                      (25-32      GUITAR)
17        Drawbar Organ             25        Acoustic Guitar(nylon)
18        Percussive Organ          26        Acoustic Guitar(steel)
19        Rock Organ                27        Electric Guitar(jazz)
20        Church Organ              28        Electric Guitar(clean)
21        Reed Organ                29        Electric Guitar(muted)
22        Accoridan                 30        Overdriven Guitar
23        Harmonica                 31        Distortion Guitar
24        Tango Accordian           32        Guitar Harmonics

   (33-40      BASS)                        (41-48     STRINGS)
33        Acoustic Bass             41        Violin
34        Electric Bass(finger)     42        Viola
35        Electric Bass(pick)       43        Cello
36        Fretless Bass             44        Contrabass
37        Slap Bass 1               45        Tremolo Strings
38        Slap Bass 2               46        Pizzicato Strings
39        Synth Bass 1              47        Orchestral Strings
40        Synth Bass 2              48        Timpani

   (49-56     ENSEMBLE)                      (57-64      BRASS)
49        String Ensemble 1         57        Trumpet
50        String Ensemble 2         58        Trombone
51        SynthStrings 1            59        Tuba
52        SynthStrings 2            60        Muted Trumpet
53        Choir Aahs                61        French Horn
54        Voice Oohs                62        Brass Section
55        Synth Voice               63        SynthBrass 1
56        Orchestra Hit             64        SynthBrass 2

   (65-72      REED)                         (73-80      PIPE)
65        Soprano Sax               73        Piccolo
66        Alto Sax                  74        Flute
67        Tenor Sax                 75        Recorder
68        Baritone Sax              76        Pan Flute
69        Oboe                      77        Blown Bottle
70        English Horn              78        Skakuhachi
71        Bassoon                   79        Whistle
72        Clarinet                  80        Ocarina

   (81-88      SYNTH LEAD)                   (89-96      SYNTH PAD)
81        Lead 1 (square)           89        Pad 1 (new age)
82        Lead 2 (sawtooth)         90        Pad 2 (warm)
83        Lead 3 (calliope)         91        Pad 3 (polysynth)
84        Lead 4 (chiff)            92        Pad 4 (choir)
85        Lead 5 (charang)          93        Pad 5 (bowed)
86        Lead 6 (voice)            94        Pad 6 (metallic)
87        Lead 7 (fifths)           95        Pad 7 (halo)
88        Lead 8 (bass+lead)        96        Pad 8 (sweep)

   (97-104     SYNTH EFFECTS)                (105-112     ETHNIC)
 97        FX 1 (rain)              105       Sitar
 98        FX 2 (soundtrack)        106       Banjo
 99        FX 3 (crystal)           107       Shamisen
100        FX 4 (atmosphere)        108       Koto
101        FX 5 (brightness)        109       Kalimba
102        FX 6 (goblins)           110       Bagpipe
103        FX 7 (echoes)            111       Fiddle
104        FX 8 (sci-fi)            112       Shanai

   (113-120    PERCUSSIVE)                  (121-128     SOUND EFFECTS)
113        Tinkle Bell              121       Guitar Fret Noise
114        Agogo                    122       Breath Noise
115        Steel Drums              123       Seashore
116        Woodblock                124       Bird Tweet
117        Taiko Drum               125       Telephone Ring
118        Melodic Tom              126       Helicopter
119        Synth Drum               127       Applause
120        Reverse Cymbal           128       Gunshot


****    TABLE 2  -  General MIDI Percussion Key Map    ****
(assigns drum sounds to note numbers. MIDI Channel 10 is for percussion)

MIDI   Drum Sound                MIDI    Drum Sound
Key                              Key

35     Acoustic Bass Drum        59      Ride Cymbal 2
36     Bass Drum 1               60      Hi Bongo
37     Side Stick                61      Low Bongo
38     Acoustic Snare            62      Mute Hi Conga
39     Hand Clap                 63      Open Hi Conga
40     Electric Snare            64      Low Conga
41     Low Floor Tom             65      High Timbale
42     Closed Hi-Hat             66      Low Timbale
43     High Floor Tom            67      High Agogo
44     Pedal Hi-Hat              68      Low Agogo
45     Low Tom                   69      Cabasa
46     Open Hi-Hat               70      Maracas
47     Low-Mid Tom               71      Short Whistle
48     Hi-Mid Tom                72      Long Whistle
49     Crash Cymbal 1            73      Short Guiro
50     High Tom                  74      Long Guiro
51     Ride Cymbal 1             75      Claves
52     Chinese Cymbal            76      Hi Wood Block
53     Ride Bell                 77      Low Wood Block
54     Tambourine                78      Mute Cuica
55     Splash Cymbal             79      Open Cuica
56     Cowbell                   80      Mute Triangle
57     Crash Cymbal 2            81      Open Triangle
58     Vibraslap





Octave||                     Note Numbers
   #  ||
      || C   | C#  | D   | D#  | E   | F   | F#  | G   | G#  | A   | A#  | B
-----------------------------------------------------------------------------
   0  ||   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 |   8 |   9 |  10 | 11
   1  ||  12 |  13 |  14 |  15 |  16 |  17 |  18 |  19 |  20 |  21 |  22 | 23
   2  ||  24 |  25 |  26 |  27 |  28 |  29 |  30 |  31 |  32 |  33 |  34 | 35
   3  ||  36 |  37 |  38 |  39 |  40 |  41 |  42 |  43 |  44 |  45 |  46 | 47
   4  ||  48 |  49 |  50 |  51 |  52 |  53 |  54 |  55 |  56 |  57 |  58 | 59
   5  ||  60 |  61 |  62 |  63 |  64 |  65 |  66 |  67 |  68 |  69 |  70 | 71
   6  ||  72 |  73 |  74 |  75 |  76 |  77 |  78 |  79 |  80 |  81 |  82 | 83
   7  ||  84 |  85 |  86 |  87 |  88 |  89 |  90 |  91 |  92 |  93 |  94 | 95
   8  ||  96 |  97 |  98 |  99 | 100 | 101 | 102 | 103 | 104 | 105 | 106 | 107
   9  || 108 | 109 | 110 | 111 | 112 | 113 | 114 | 115 | 116 | 117 | 118 | 119
  10  || 120 | 121 | 122 | 123 | 124 | 125 | 126 | 127 |

 */

//System Exclusive Manufacturer's ID Numbers
//reserved (used for three byte id's)
#define MIDI_SYSXMID_RESERVED		0x00
//american
#define MIDI_SYSXMID_SEQUENTIAL		0x01
#define MIDI_SYSXMID_IDP			0x02
#define MIDI_SYSXMID_VOYETRA		0x03
#define MIDI_SYSXMID_MOOG			0x04
#define MIDI_SYSXMID_PASSPORT		0x05
#define MIDI_SYSXMID_LEXICON		0x06
#define MIDI_SYSXMID_KUTZWEIL		0x07
#define MIDI_SYSXMID_FENDER			0x08
#define MIDI_SYSXMID_GULBRANSEN		0x09
#define MIDI_SYSXMID_AKG			0x0A
#define MIDI_SYSXMID_VOYCE			0x0B
#define MIDI_SYSXMID_WAVEFRAME		0x0C
#define MIDI_SYSXMID_ADA			0x0D
#define MIDI_SYSXMID_GARFIELD		0x0E
#define MIDI_SYSXMID_ENSONIQ		0x0F
#define MIDI_SYSXMID_OBERHEIM		0x10
#define MIDI_SYSXMID_APPLE			0x11
#define MIDI_SYSXMID_GREYMATTER		0x12
#define MIDI_SYSXMID_DIGIDESIGN		0x13
#define MIDI_SYSXMID_PALMTREE		0x14
#define MIDI_SYSXMID_JLCOOPER		0x15
#define MIDI_SYSXMID_LOWREY			0x16
#define MIDI_SYSXMID_ADAMSSMITH		0x17
#define MIDI_SYSXMID_EMU			0x18
#define MIDI_SYSXMID_HARMONY		0x19
#define MIDI_SYSXMID_ART			0x1A
#define MIDI_SYSXMID_BALDWIN		0x1B
#define MIDI_SYSXMID_EVENTIDE		0x1C
#define MIDI_SYSXMID_INVENTRONICS	0x1D
#define MIDI_SYSXMID_CLARITY		0x1F
//european
#define MIDI_SYSXMID_PASSAC			0x20
#define MIDI_SYSXMID_SIEL			0x21
#define MIDI_SYSXMID_SYNTHAXE		0x22
#define MIDI_SYSXMID_STEPP			0x23
#define MIDI_SYSXMID_HOHNER			0x24
#define MIDI_SYSXMID_TWISTER		0x25
#define MIDI_SYSXMID_SOLTON			0x26
#define MIDI_SYSXMID_JELLINGHAUS	0x27
#define MIDI_SYSXMID_SOUTHWORTH		0x28
#define MIDI_SYSXMID_PPG			0x29
#define MIDI_SYSXMID_JEN			0x2A
#define MIDI_SYSXMID_SSL			0x2B
#define MIDI_SYSXMID_AUDIOVERI		0x2C
#define MIDI_SYSXMID_NEVE			0x2D
#define MIDI_SYSXMID_SOUNTRACS		0x2E
#define MIDI_SYSXMID_ELKA			0x2F
#define MIDI_SYSXMID_DYNACORD		0x30
#define MIDI_SYSXMID_VISCOUNT		0x31
#define MIDI_SYSXMID_DRAWMER		0x32
#define MIDI_SYSXMID_CLAVIA			0x33
#define MIDI_SYSXMID_AUDIOARCH		0x34
#define MIDI_SYSXMID_GENERAL		0x35
#define MIDI_SYSXMID_CHEETAH		0x36
#define MIDI_SYSXMID_CTM			0x37
#define MIDI_SYSXMID_SIMMONS		0x38
#define MIDI_SYSXMID_SOUNDCRAFT		0x39
#define MIDI_SYSXMID_STEINBERG		0x3A
#define MIDI_SYSXMID_WERSI			0x3B
#define MIDI_SYSXMID_AVAB			0x3C
#define MIDI_SYSXMID_DIGIGRAM		0x3D
#define MIDI_SYSXMID_WALDORF		0x3E
#define MIDI_SYSXMID_QUASIMIDI		0x3F
//japanese
#define MIDI_SYSXMID_KAWAI			0x40
#define MIDI_SYSXMID_ROLAND			0x41
#define MIDI_SYSXMID_KORG			0x42
#define MIDI_SYSXMID_YAMAHA			0x43
#define MIDI_SYSXMID_CASIO			0x44
#define MIDI_SYSXMID_KAMIYA			0x46
#define MIDI_SYSXMID_AKAI			0x47
#define MIDI_SYSXMID_JVICTOR		0x48
#define MIDI_SYSXMID_MESOSHA		0x49
#define MIDI_SYSXMID_HOSHINO		0x4A
#define MIDI_SYSXMID_FUJITSU		0x4B
#define MIDI_SYSXMID_SONY			0x4C
#define MIDI_SYSXMID_NISSHIN		0x4D
#define MIDI_SYSXMID_TEAC			0x4E
#define MIDI_SYSXMID_MATSUELEC		0x50
#define MIDI_SYSXMID_FOSTEX			0x51
#define MIDI_SYSXMID_ZOOM			0x52
#define MIDI_SYSXMID_MIDORI			0x53
#define MIDI_SYSXMID_MATSUCOM		0x54
#define MIDI_SYSXMID_SUZUKI			0x55
#define MIDI_SYSXMID_FUJI			0x56
#define MIDI_SYSXMID_ACOUSTIC		0x57

