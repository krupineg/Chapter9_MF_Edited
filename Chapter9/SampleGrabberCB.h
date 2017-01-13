#pragma once
#include "Common.h"

#include <mfreadwrite.h>

class SampleGrabberCB : public IMFSampleGrabberSinkCallback
{
    bool stopped;
    long m_cRef;
    IMFSinkWriter           *m_pWriter = NULL;
    SampleGrabberCB(LPCWSTR filePath, IMFMediaType * pTypeIn, IMFMediaType * pTypeOut) : m_cRef(1), stopped(false) {
        HRESULT hr = MFCreateSinkWriterFromURL(filePath,
            NULL,
            NULL,
            &m_pWriter);
        THROW_ON_FAIL(hr);
        CComPtr<IMFMediaType> inTypeCopy = NULL;
        hr = MFCreateMediaType(&inTypeCopy);
        THROW_ON_FAIL(hr);
        hr = inTypeCopy->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        THROW_ON_FAIL(hr);
        
        GUID subtype = GetSubtype(pTypeIn);
        hr = inTypeCopy->SetGUID(MF_MT_SUBTYPE, subtype);
        THROW_ON_FAIL(hr);
        hr = CopyType(pTypeIn, inTypeCopy);
        THROW_ON_FAIL(hr);
        DWORD *sink_stream = NULL;
        hr = ConfigureEncoder(inTypeCopy, pTypeOut, m_pWriter);
        THROW_ON_FAIL(hr);
        hr = m_pWriter->BeginWriting();
        THROW_ON_FAIL(hr);
    }
    const UINT32 TARGET_BIT_RATE = 240 * 1000;
    HRESULT ConfigureEncoder(
        IMFMediaType *pTypeIn,
        IMFMediaType * pTypeOut,
        IMFSinkWriter *pWriter        
    )
    {
        DWORD sink_stream = 0;
        HRESULT hr = S_OK;       
        hr = pWriter->AddStream(pTypeOut, &sink_stream);
        if (hr != MF_E_STREAMSINKS_FIXED) {
            THROW_ON_FAIL(hr);
        }
        hr = pWriter->SetInputMediaType(sink_stream, pTypeIn, NULL);
        THROW_ON_FAIL(hr);
        return hr;
    }
    CRITICAL_SECTION        m_critsec;

public:
    static HRESULT CreateInstance(LPCWSTR path, IMFMediaType *pTypeIn, IMFMediaType *pTypeOut, SampleGrabberCB **ppCB);
    void Stop();
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    MFTIME timeOffset = 0;
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
