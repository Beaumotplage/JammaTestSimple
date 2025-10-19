/* App Audio
	Manipulate the MOD player to suit the testcard page

*/
#include "app_audio.h"


void AppAudio::run(modcontext* modctx, msample* outbuffer, mssize nbsample, tracker_buffer_state* trkbuf)
{
	if (m_soundcodes.reset)
	{
		modctx->tablepos = 0;
		modctx->patternpos = 0;
	}
	// Volume hack by JB
	modctx->volume = m_soundcodes.volume >> 8;

	if (m_soundcodes.play)
	{
		hxcmod_fillbuffer(modctx, outbuffer, nbsample, trkbuf);
	}
	else
	{
		for (mssize x = 0; x < nbsample; x++)
		{
			outbuffer[x] = 128;
		}
	}
}