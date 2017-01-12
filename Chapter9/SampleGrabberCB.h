#pragma once
#include "Common.h"
#include <Wmcodecdsp.h>
#include <mfreadwrite.h>

class SampleGrabberCB : public IMFSampleGrabberSinkCallback
{
    bool stopped;
    long m_cRef;
    CComPtr<IMFSinkWriter> m_pWriter;
    SampleGrabberCB(IMFSinkWriter * writer, IMFMediaType * pTypeIn, IMFMediaType *pTypeOut) : m_cRef(1), stopped(false) {
        m_pWriter = writer;
      
        CComPtr<IMFMediaType> inTypeCopy = CreateMediaType(GetMajorType(pTypeIn), GetSubtype(pTypeIn));
        HRESULT hr = CopyType(pTypeIn, inTypeCopy);
        THROW_ON_FAIL(hr);
        hr = ConfigureEncoder(inTypeCopy, pTypeOut, m_pWriter);
        THROW_ON_FAIL(hr);      
    }

    HRESULT ConfigureEncoder(
        IMFMediaType *pTypeIn,
        IMFMediaType * pTypeOut,
        IMFSinkWriter *pWriter
    )
    {
        HRESULT hr = S_OK;    
        DWORD index = 0;
        hr = pWriter->AddStream(pTypeOut, &index);
        THROW_ON_FAIL(hr);
        hr = pWriter->SetInputMediaType(index, pTypeIn, NULL);
        THROW_ON_FAIL(hr);
        return hr;
    }
    CRITICAL_SECTION        m_critsec;

public:
    static HRESULT CreateInstance(IMFSinkWriter * writer, IMFMediaType *pTypeIn, IMFMediaType *pTypeOut, SampleGrabberCB **ppCB);
    void Stop();
    void Start();
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
