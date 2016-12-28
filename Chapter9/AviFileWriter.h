#pragma once
#include "Common.h"

using namespace std;

class CAviFileWriter
{
    public:
        CAviFileWriter(const WCHAR* pFilename);
        ~CAviFileWriter(void);

        HRESULT AddStream(IMFMediaType* pMediaType, DWORD id);
        HRESULT WriteSample(BYTE* pData, DWORD dataLength, DWORD streamId, bool isKeyframe = false);

    private:
        struct AviStreamData
        {
            IAVIStream* pStream;
            ULONG nNextSample;
            bool isAudio;
        };

        IAVIFile* m_pAviFile;        
        hash_map<DWORD, AviStreamData*> m_streamHash;
        WAVEFORMATEX*  m_pAudioFormat;

        HRESULT AddAudioStream(IMFMediaType* pMT, IAVIStream** pStream);
        HRESULT AddVideoStream(IMFMediaType* pMT, IAVIStream** pStream);
};

