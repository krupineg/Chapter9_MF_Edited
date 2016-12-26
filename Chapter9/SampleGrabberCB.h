#pragma once
#include "Common.h"

class SampleGrabberCB : public IMFSampleGrabberSinkCallback
{
    bool stopped;
    long m_cRef;
    IMFSinkWriter           *m_pWriter = NULL;
    SampleGrabberCB(LPCWSTR filePath, IMFMediaType * pType) : m_cRef(1), stopped(false) {
        HRESULT hr = MFCreateSinkWriterFromURL(filePath,
            NULL,
            NULL,
            &m_pWriter);
        THROW_ON_FAIL(hr);
        DWORD sink_stream;
        hr = ConfigureEncoder(pType, m_pWriter, &sink_stream);
        THROW_ON_FAIL(hr);
        hr = m_pWriter->SetInputMediaType(sink_stream, pType, NULL);
        THROW_ON_FAIL(hr);
        hr = m_pWriter->BeginWriting();
        THROW_ON_FAIL(hr);
    }
    const UINT32 TARGET_BIT_RATE = 240 * 1000;
    HRESULT ConfigureEncoder(
        IMFMediaType *pType,
        IMFSinkWriter *pWriter,
        DWORD *pdwStreamIndex
    )
    {
        HRESULT hr = S_OK;

        IMFMediaType *pType2 = NULL;

        hr = MFCreateMediaType(&pType2);

        if (SUCCEEDED(hr))
        {
            hr = pType2->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        }
        if (SUCCEEDED(hr))
        {
            hr = pType2->SetGUID(MF_MT_SUBTYPE, MEDIASUBTYPE_H264);
        }
        if (SUCCEEDED(hr))
        {
            hr = pType2->SetUINT32(MF_MT_AVG_BITRATE, TARGET_BIT_RATE);
        }

        if (SUCCEEDED(hr))
        {
            hr = CopyAttribute(pType, pType2, MF_MT_FRAME_SIZE);
        }

        if (SUCCEEDED(hr))
        {
            hr = CopyAttribute(pType, pType2, MF_MT_FRAME_RATE);
        }

        if (SUCCEEDED(hr))
        {
            hr = CopyAttribute(pType, pType2, MF_MT_PIXEL_ASPECT_RATIO);
        }

        if (SUCCEEDED(hr))
        {
            hr = CopyAttribute(pType, pType2, MF_MT_INTERLACE_MODE);
        }

        if (SUCCEEDED(hr))
        {
            hr = pWriter->AddStream(pType2, pdwStreamIndex);
        }

        SafeRelease(&pType2);
        return hr;
    }
    CRITICAL_SECTION        m_critsec;

public:
    static HRESULT CreateInstance(LPCWSTR path, IMFMediaType *pType, SampleGrabberCB **ppCB);
    void Stop();
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFClockStateSink methods
    STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
    STDMETHODIMP OnClockStop(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockPause(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate);

    // IMFSampleGrabberSinkCallback methods
    STDMETHODIMP OnSetPresentationClock(IMFPresentationClock* pClock);
    STDMETHODIMP OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
        LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
        DWORD dwSampleSize);
    STDMETHODIMP OnShutdown();
};
