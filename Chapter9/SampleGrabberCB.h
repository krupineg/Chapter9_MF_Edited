#pragma once
#include "Common.h"

#include <mfreadwrite.h>

class SampleGrabberCB : public IMFSampleGrabberSinkCallback, IMFMediaSink
{
    bool stopped;
    std::wstring _name;
    long m_cRef;
    long count;
    IMFSinkWriter *m_pWriter = NULL;
    SampleGrabberCB(std::wstring name) :
        m_cRef(1),
        stopped(false), 
        _name(name), 
        count(0){
        
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
    static HRESULT CreateInstance(std::wstring name, SampleGrabberCB **ppCB);
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
    // IMFMediaSink interface implementation
    STDMETHODIMP GetCharacteristics(DWORD *pdwCharacteristics);
    STDMETHODIMP AddStreamSink(DWORD dwStreamSinkIdentifier, IMFMediaType* pMediaType, IMFStreamSink** ppStreamSink);
    STDMETHODIMP RemoveStreamSink(DWORD dwStreamSinkIdentifier);
    STDMETHODIMP GetStreamSinkCount(DWORD* pcStreamSinkCount);
    STDMETHODIMP GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink** ppStreamSink);
    STDMETHODIMP GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink** ppStreamSink);
    STDMETHODIMP SetPresentationClock(IMFPresentationClock* pPresentationClock);
    STDMETHODIMP GetPresentationClock(IMFPresentationClock** ppPresentationClock);
    STDMETHODIMP Shutdown(void);
    // IMFSampleGrabberSinkCallback methods
    STDMETHODIMP OnSetPresentationClock(IMFPresentationClock* pClock);
    STDMETHODIMP OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
        LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
        DWORD dwSampleSize);
    STDMETHODIMP OnShutdown();
};
