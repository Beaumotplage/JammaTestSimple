//-----------------------------------------------------------
// Play an audio stream on the default audio rendering
// device. The PlayAudioStream function allocates a shared
// buffer big enough to hold one second of PCM audio data.
// The function uses this buffer to stream data to the
// rendering device. The inner loop runs every 1/2 second.
//-----------------------------------------------------------

#include <iostream>
#include <windows.h>

// Windows multimedia device
#include <Mmdeviceapi.h>

// WASAPI
#include <audioclient.h>


// Temporary
#include "media\ridgeracer.c"
#include "media\modfile.h"
#include "hxcmod.h"
#include "..\..\App\App_main.h"
modcontext modloaded;
unsigned char* modfile;
tracker_buffer_state trackbuf_state1;
tracker_buffer_state trackbuf_state2;
//-----------------------------------------------------------
class MyAudioSource
{
public:
    MyAudioSource();
    MyAudioSource(float freq) {
        m_frequency = freq;
    };
    ~MyAudioSource();

    HRESULT SetFormat(WAVEFORMATEX*);
    HRESULT LoadData(UINT32, BYTE*, DWORD*);
    HRESULT LoadMoreMod(UINT32 totalFrames, BYTE* dataOut, DWORD* flags);

private:
    void init();
    bool m_initialised = false;
    WAVEFORMATEXTENSIBLE  m_format;
    unsigned int m_pcmPos = 0;
    UINT32 m_bufferSize;
    UINT32 m_bufferPos = 0;
    static const unsigned int m_sampleCount = 96000 * 5;
    float m_frequency = 440;
    float* m_pcmAudio = nullptr;
};
//-----------------------------------------------------------
MyAudioSource::MyAudioSource()
{

}
//-----------------------------------------------------------

MyAudioSource::~MyAudioSource()
{
    if (m_pcmAudio)
    {
        delete[] m_pcmAudio;
    }
}
//-----------------------------------------------------------

void MyAudioSource::init()
{

    m_pcmAudio = new float[m_sampleCount];
#ifdef MOD
    // Get next 1/2-second of data from the audio source.
    msample buffer[96000 * 5];

    hxcmod_fillbuffer(&modloaded, &buffer[0], (96000 * 5) / 4, &trackbuf_state1);


    for (int x = 0; x < (96000 * 5) / 4; x++)
    {
        float y = (float)x;
        y *= 22.0f / 48.0f;
        unsigned char data = buffer[(int)y];

        m_pcmAudio[x] = (float)data * (1.0f / 256.0f);

    }



#else
    for (int x = 0; x < m_sampleCount; x++)
    {
        float y = (float)x;
        y *= 22.0f / 48.0f;
        unsigned char data = ridgeracer[(unsigned int)y];

        m_pcmAudio[x] = (float)data * (1.0f / 256.0f);

    }
#endif
    /*
    const float radsPerSec = 2 * 3.1415926536 * frequency / (float)format.Format.nSamplesPerSec;
    for (unsigned long i = 0; i < sampleCount; i++)
    {
        float sampleValue = sin(radsPerSec * (float)i);
        pcmAudio[i] = sampleValue;
    }
    */
    m_initialised = true;
}
//-----------------------------------------------------------

HRESULT MyAudioSource::SetFormat(WAVEFORMATEX* wfex)
{
    if (wfex->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        m_format = *reinterpret_cast<WAVEFORMATEXTENSIBLE*>(wfex);
    }
    else
    {
        m_format.Format = *wfex;
        m_format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        INIT_WAVEFORMATEX_GUID(&m_format.SubFormat, wfex->wFormatTag);
        m_format.Samples.wValidBitsPerSample = m_format.Format.wBitsPerSample;
        m_format.dwChannelMask = 0;
    }
    const UINT16 formatTag = EXTRACT_WAVEFORMATEX_ID(&m_format.SubFormat);

    //std::cout << "Channel Count: " << m_format.Format.nChannels << '\n';
    //std::cout << "Audio Format: ";
    switch (formatTag)
    {
    case WAVE_FORMAT_IEEE_FLOAT:
        //std::cout << "WAVE_FORMAT_IEEE_FLOAT\n";
        break;
    case WAVE_FORMAT_PCM:
        //std::cout << "WAVE_FORMAT_PCM\n";
        break;
    default:
        //std::cout << "Wave Format Unknown\n";
        break;
    }
    return 0;
}
//-----------------------------------------------------------

HRESULT MyAudioSource::LoadData(UINT32 totalFrames, BYTE* dataOut, DWORD* flags)
{
    float* fData = (float*)dataOut;
    UINT32 totalSamples = totalFrames * m_format.Format.nChannels;
    if (!m_initialised)
    {
        init();
        m_bufferSize = totalFrames * m_format.Format.nChannels;
        //std::cout << "bufferSize: " << m_bufferSize << '\n';
        //std::cout << "sampsPerChan: " << totalFrames / m_format.Format.nChannels << '\n';
        //std::cout << "fData[totalSamples]: " << fData[totalFrames] << '\n';
        //std::cout << "fData[bufferSize]: " << fData[m_bufferSize] << '\n';
        //std::cout << "buffer address: " << int(dataOut) << '\n';
    }
    else
    {
        //std::cout << "Frames to Fill: " << totalFrames << '\n';
        //std::cout << "Samples to Fill: " << totalSamples << '\n';
        //std::cout << "bufferPos: " << m_bufferPos << '\n';
        //std::cout << "buffer address: " << int(dataOut) << '\n';

    }

    if (m_pcmPos >= m_sampleCount)
    {
        m_pcmPos = 0;
    }

    if (m_pcmPos < m_sampleCount)
    {
        for (UINT32 i = 0; i < totalSamples; i += m_format.Format.nChannels)
        {
            for (size_t chan = 0; chan < m_format.Format.nChannels; chan++)
            {
                fData[i + chan] = (m_pcmPos < m_sampleCount) ? m_pcmAudio[m_pcmPos] : 0.0f;
            }
            m_pcmPos++;
        }
        m_bufferPos += totalSamples;
        m_bufferPos %= m_bufferSize;
    }
    else
    {
        *flags = AUDCLNT_BUFFERFLAGS_SILENT;
    }
    return 0;
}


HRESULT MyAudioSource::LoadMoreMod(UINT32 totalFrames, BYTE* dataOut, DWORD* flags)
{
    float* fData = (float*)dataOut;
    UINT32 totalSamples = totalFrames * m_format.Format.nChannels;
    if (!m_initialised)
    {
        init();
        m_bufferSize = totalFrames * m_format.Format.nChannels;
        /*
        std::cout << "bufferSize: " << m_bufferSize << '\n';
        std::cout << "sampsPerChan: " << totalFrames / m_format.Format.nChannels << '\n';
        std::cout << "fData[totalSamples]: " << fData[totalFrames] << '\n';
        std::cout << "fData[bufferSize]: " << fData[m_bufferSize] << '\n';
        std::cout << "buffer address: " << int(dataOut) << '\n';
        */
    }
    else
    {
        /*
        std::cout << "Frames to Fill: " << totalFrames << '\n';
        std::cout << "Samples to Fill: " << totalSamples << '\n';
        std::cout << "bufferPos: " << m_bufferPos << '\n';
        std::cout << "buffer address: " << int(dataOut) << '\n';
        */

    }


    // Get next 1/2-second of data from the audio source.
    msample buffer[96000 ];

    
//    hxcmod_fillbuffer(&modloaded, &buffer[0], totalFrames/2, &trackbuf_state1);

    extern AppMain app_main; // Tidy this up
    // MONO so only loading in half the frames
    int modsamples = 1+(totalFrames * 22) / 44;
    app_main.m_audio.run(&modloaded, &buffer[0], modsamples, &trackbuf_state1);


    // Increment 2 as PC is stereo, but RP is mono at the moment
    for (unsigned int x = 0; x < totalSamples; x+=2)
    {
        float y = (float)x;
        y *= 22.0f / 96.0f;
        
        unsigned char data = buffer[(int)y];

        float outputdata = 0.0f;
        if (m_pcmPos < m_sampleCount)
        {
            outputdata = ( (float)data);
            outputdata -= 128.0f;
            outputdata *= (1.0f / 128.0f);

        }

        // Mono
        fData[x] = outputdata;
        fData[x + 1] = outputdata;
      

    }


#if 0
 //   if (m_pcmPos < m_sampleCount)
    {
        for (UINT32 i = 0; i < totalSamples; i += m_format.Format.nChannels)
        {
            for (size_t chan = 0; chan < m_format.Format.nChannels; chan++)
            {
                fData[i + chan] = (m_pcmPos < m_sampleCount) ? m_pcmAudio[m_pcmPos] : 0.0f;
            }
            m_pcmPos++;
        }
        m_bufferPos += totalSamples;
        m_bufferPos %= m_bufferSize;
    }
    else
    {
        *flags = AUDCLNT_BUFFERFLAGS_SILENT;
    }
#endif
    return 0;
}


//-----------------------------------------------------------
// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  5000000
#define REFTIMES_PER_MILLISEC  10000
//-----------------------------------------------------------

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) {std::cout << hres<< '\n'; goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

//-----------------------------------------------------------
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
//-----------------------------------------------------------
HRESULT PlayAudioStream(MyAudioSource* pMySource)
{
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
    BYTE* pData;
    DWORD flags = 0;

    //std::cout << "CoCreateInstance\n";
    hr = CoCreateInstance(CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);
    //std::cout << "GetDefaultAudioEndpoint\n";

    hr = pEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr);

    //std::cout << "Activate\n";
    hr = pDevice->Activate(
        IID_IAudioClient, CLSCTX_ALL,
        NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr);

    //std::cout << "GetMixFormat\n";
    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr);

    //std::cout << "Initialize\n";
    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL);
    EXIT_ON_ERROR(hr);

    // Tell the audio source which format to use.
    //std::cout << "pMySource->SetFormat\n";
    hr = pMySource->SetFormat(pwfx);
    EXIT_ON_ERROR(hr);

    // Get the actual size of the allocated buffer.
    //std::cout << "GetBufferSize\n";
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr);

    //std::cout << "GetService\n";

    hr = pAudioClient->GetService(
        IID_IAudioRenderClient,
        (void**)&pRenderClient);
    EXIT_ON_ERROR(hr);

    // Grab the entire buffer for the initial fill operation.
    hr = pRenderClient->GetBuffer(bufferFrameCount, &pData);
    EXIT_ON_ERROR(hr);

    // Load the initial data into the shared buffer.
    hr = pMySource->LoadData(bufferFrameCount, pData, &flags);
    EXIT_ON_ERROR(hr);

    hr = pRenderClient->ReleaseBuffer(bufferFrameCount, flags);
    EXIT_ON_ERROR(hr);

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC *
        bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->Start();  // Start playing.
    EXIT_ON_ERROR(hr);

    // Each loop fills about half of the shared buffer.
    while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
        // Sleep for half the buffer duration.
        Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

        // See how much buffer space is available.
        hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
        EXIT_ON_ERROR(hr);

        numFramesAvailable = bufferFrameCount - numFramesPadding;

        // Grab all the available space in the shared buffer.
        hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
        EXIT_ON_ERROR(hr);

        hr = pMySource->LoadMoreMod(numFramesAvailable, pData, &flags);

//        hr = pMySource->LoadData(numFramesAvailable, pData, &flags);
        EXIT_ON_ERROR(hr);

        hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags);
        EXIT_ON_ERROR(hr);
    }
#if 0
    {

        pwhOut = (struct wavehdr_tag*)lParam;

        if (pwhOut->lpData == (char*)&sndbuffer1)
        {
            trackbuf_state1.nb_of_state = 0;
            hxcmod_fillbuffer(&modloaded, (unsigned short*)pwhOut->lpData, pwhOut->dwBufferLength / 4, &trackbuf_state1);
        }
        else
        {
            trackbuf_state2.nb_of_state = 0;
            hxcmod_fillbuffer(&modloaded, (unsigned short*)pwhOut->lpData, pwhOut->dwBufferLength / 4, &trackbuf_state2);
        }

        waveOutWrite((HWAVEOUT)wParam, pwhOut, sizeof(WAVEHDR));
        nb_wr_block++;

    }
#endif
    // Wait for last data in buffer to play before stopping.
    Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

    hr = pAudioClient->Stop();  // Stop playing.
    EXIT_ON_ERROR(hr);

Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pAudioClient);
    SAFE_RELEASE(pRenderClient);

    return hr;
}
//-----------------------------------------------------------
int audio_main(void)
{
    //std::cout << "start main\n";
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    MyAudioSource audio(440);


    hxcmod_init(&modloaded);

    hxcmod_setcfg(&modloaded, 22000, 1, 1);

    //	modfile = unpack(data_cartoon_dreams_n_fantasies_mod->data,data_cartoon_dreams_n_fantasies_mod->csize ,data_cartoon_dreams_n_fantasies_mod->data, data_cartoon_dreams_n_fantasies_mod->size);
  // modfile = escape1;

    hxcmod_load(&modloaded, (void*)escape1, sizeof(escape1));

    PlayAudioStream(&audio);

    CoUninitialize();

    return (0);
}
