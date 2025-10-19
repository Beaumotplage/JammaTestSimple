#pragma once
#include "hxcmod.h"


class AppAudio
{
public:


	typedef struct
	{
		uint16_t volume;
		uint16_t play;
		uint16_t reset;
	}SoundCodes;



	void run(modcontext* modctx, msample* outbuffer, mssize nbsample, tracker_buffer_state* trkbuf);

	SoundCodes m_soundcodes;


private:
	
	

};