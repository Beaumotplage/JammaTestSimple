///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
// File : hxcmod.h
// Contains: a tiny mod player
//
// Written by: Jean François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////
#ifndef MODPLAY_DEF
#define MODPLAY_DEF

#ifdef __cplusplus
extern "C" {
#endif
//#define HXCMOD_USE_PRECALC_VOLUME_TABLE
#define HXCMOD_SLOW_TARGET
#define HXCMOD_MONO_OUTPUT 

#ifndef HXCMOD_SLOW_TARGET
	#define HXCMOD_STATE_REPORT_SUPPORT 0
	#define HXCMOD_OUTPUT_FILTER 0
	#define HXCMOD_OUTPUT_STEREO_MIX 1
	#define HXCMOD_CLIPPING_CHECK 1
#endif
#define HXCMOD_8BITS_OUTPUT 
#define HXCMOD_UNSIGNED_OUTPUT
// Warning : The following option
// required 32KB of additional RAM :
// #define HXCMOD_USE_PRECALC_VOLUME_TABLE 1

// Basic type
typedef unsigned char   uint8_t;
typedef signed   char   int8_t;
typedef unsigned short  uint16_t;
typedef          short  int16_t;
typedef unsigned int   uint32_t;

#ifdef HXCMOD_16BITS_TARGET
	typedef uint16_t  mssize;
#else
	typedef uint32_t   mssize;
#endif

#ifdef HXCMOD_8BITS_OUTPUT
	#ifdef HXCMOD_UNSIGNED_OUTPUT
	typedef unsigned char  msample;
	#else
	typedef signed char    msample;
	#endif
#else
	#ifdef HXCMOD_UNSIGNED_OUTPUT
	typedef unsigned short msample;
	#else
	typedef signed short   msample;
	#endif
#endif

#ifdef HXCMOD_MAXCHANNELS
	#define NUMMAXCHANNELS HXCMOD_MAXCHANNELS
#else
	#define NUMMAXCHANNELS 32
#endif

#define MAXNOTES 12*12

//
// MOD file structures
//

#pragma pack(1)

typedef struct {
	uint8_t  name[22];
	uint16_t   length;
	uint8_t  finetune;
	uint8_t  volume;
	uint16_t   reppnt;
	uint16_t   replen;
} sample;

typedef struct {
	uint8_t  sampperiod;
	uint8_t  period;
	uint8_t  sampeffect;
	uint8_t  effect;
} note;

typedef struct {
	uint8_t  title[20];
	sample  samples[31];
	uint8_t  length;
	uint8_t  protracker;
	uint8_t  patterntable[128];
	uint8_t  signature[4];
	uint8_t  speed;
} module;

#pragma pack()

//
// HxCMod Internal structures
//
typedef struct {
	int8_t * sampdata;
	uint16_t   length;
	uint16_t   reppnt;
	uint16_t   replen;
	uint8_t  sampnum;

	int8_t * nxt_sampdata;
	uint16_t   nxt_length;
	uint16_t   nxt_reppnt;
	uint16_t   nxt_replen;
	uint8_t  update_nxt_repeat;

	int8_t * dly_sampdata;
	uint16_t   dly_length;
	uint16_t   dly_reppnt;
	uint16_t   dly_replen;
	uint8_t  note_delay;

	int8_t * lst_sampdata;
	uint16_t   lst_length;
	uint16_t   lst_reppnt;
	uint16_t   lst_replen;
	uint8_t  retrig_cnt;
	uint8_t  retrig_param;

	uint16_t   funkoffset;
	int16_t    funkspeed;

	int16_t    glissando;

	uint32_t  samppos;
	uint32_t  sampinc;
	uint16_t   period;
	uint8_t  volume;
#ifdef HXCMOD_USE_PRECALC_VOLUME_TABLE
	mint  * volume_table;
#endif
	uint8_t  effect;
	uint8_t  parameffect;
	uint16_t   effect_code;

	uint32_t  last_set_offset;

	int16_t    decalperiod;
	int16_t    portaspeed;
	int16_t    portaperiod;
	int16_t    vibraperiod;
	int16_t    Arpperiods[3];
	uint8_t  ArpIndex;

	uint8_t  volumeslide;

	uint8_t  vibraparam;
	uint8_t  vibrapointeur;

	uint8_t  finetune;

	uint8_t  cut_param;

	uint16_t   patternloopcnt;
	uint16_t   patternloopstartpoint;
} channel;

typedef struct {
	module  song;
	int8_t * sampledata[31];
	note *  patterndata[128];

#ifdef HXCMOD_16BITS_TARGET
	muint   playrate;
#else
	uint32_t  playrate;
#endif

	uint16_t   tablepos;
	uint16_t   patternpos;
	uint16_t   patterndelay;
	uint8_t  jump_loop_effect;
	uint8_t  bpm;

#ifdef HXCMOD_16BITS_TARGET
	muint   patternticks;
	muint   patterntickse;
	muint   patternticksaim;
	muint   patternticksem;
	muint   tick_cnt;
#else
	uint32_t  patternticks;
	uint32_t  patterntickse;
	uint32_t  patternticksaim;
	uint32_t  patternticksem;
	uint32_t  tick_cnt;
#endif

	uint32_t  sampleticksconst;

	channel channels[NUMMAXCHANNELS];

	uint16_t   number_of_channels;

	uint16_t   mod_loaded;

	int16_t    last_r_sample;
	int16_t    last_l_sample;

	int16_t    stereo;
	int16_t    stereo_separation;
	int16_t    bits;
	int16_t    filter;
	uint8_t	   volume; // Master Volume

#ifdef EFFECTS_USAGE_STATE
	int effects_event_counts[32];
#endif

#ifdef HXCMOD_USE_PRECALC_VOLUME_TABLE
	mint    precalc_volume_array[65*256];
	mint  * volume_selection_table[65];
#endif

} modcontext;

//
// Player states structures
//
typedef struct track_state_
{
	unsigned char instrument_number;
	unsigned short cur_period;
	unsigned char  cur_volume;
	unsigned short cur_effect;
	unsigned short cur_parameffect;
}track_state;

typedef struct tracker_state_
{
	int number_of_tracks;
	int bpm;
	int speed;
	int cur_pattern;
	int cur_pattern_pos;
	int cur_pattern_table_pos;
	unsigned int buf_index;
	track_state tracks[NUMMAXCHANNELS];
}tracker_state;

typedef struct tracker_state_instrument_
{
	char name[22];
	int active;
}tracker_state_instrument;

typedef struct tracker_buffer_state_
{
	int nb_max_of_state;
	int nb_of_state;
	int cur_rd_index;
	int sample_step;
	char name[64];
	tracker_state_instrument instruments[31];
	tracker_state * track_state_buf;
}tracker_buffer_state;

///////////////////////////////////////////////////////////////////////////////////
// HxCMOD Core API:
// -------------------------------------------
// int  hxcmod_init(modcontext * modctx)
//
// - Initialize the modcontext buffer. Must be called before doing anything else.
//   Return 1 if success. 0 in case of error.
// -------------------------------------------
// int  hxcmod_load( modcontext * modctx, void * mod_data, int mod_data_size )
//
// - "Load" a MOD from memory (from "mod_data" with size "mod_data_size").
//   Return 1 if success. 0 in case of error.
// -------------------------------------------
// void hxcmod_fillbuffer( modcontext * modctx, unsigned short * outbuffer, mssize nbsample, tracker_buffer_state * trkbuf )
//
// - Generate and return the next samples chunk to outbuffer.
//   nbsample specify the number of stereo 16bits samples you want.
//   The output format is signed 44100Hz 16-bit Stereo PCM samples.
//   The output buffer size in byte must be equal to ( nbsample * 2 * 2 ).
//   The optional trkbuf parameter can be used to get detailed status of the player. Put NULL/0 is unused.
// -------------------------------------------
// void hxcmod_unload( modcontext * modctx )
//
// - "Unload" / clear the player status.
// -------------------------------------------
///////////////////////////////////////////////////////////////////////////////////

int  hxcmod_init( modcontext * modctx );
int  hxcmod_setcfg( modcontext * modctx, int samplerate, int stereo_separation, int filter );
int  hxcmod_load( modcontext * modctx, void * mod_data, int mod_data_size );
void hxcmod_fillbuffer( modcontext * modctx, msample * outbuffer, mssize nbsample, tracker_buffer_state * trkbuf );
void hxcmod_unload( modcontext * modctx );

#ifdef __cplusplus
}
#endif

#endif /* MODPLAY_DEF */
