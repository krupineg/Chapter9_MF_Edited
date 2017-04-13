#pragma once
#include "Common.h"

#include <mfreadwrite.h>
#include <mfidl.h>
class SampleGrabberCB : public IMFSampleGrabberSinkCallback, IMFMediaSink
{
protected:
    bool stopped;
    std::wstring _name;
    long m_cRef;
    long count;
    IMFSinkWriter *m_pWriter = NULL;
    IMFMediaSink*  _sink;

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
    static HRESULT CreateTimingInstance(std::wstring name, SampleGrabberCB **ppCB);
    virtual HRESULT StopImpl() = 0;
    virtual HRESULT ShutdownImpl() = 0;
    virtual HRESULT StartImpl() = 0;
    virtual HRESULT ProcessSampleImpl(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
        LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
        DWORD dwSampleSize, unsigned __int64 time) = 0;
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

class TimingSampleGrabber : public SampleGrabberCB {
public:
    TimingSampleGrabber(std::wstring name) : SampleGrabberCB (name){        
    }   
    HRESULT StopImpl();
    HRESULT ShutdownImpl();
    HRESULT StartImpl();
    HRESULT ProcessSampleImpl(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
        LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
        DWORD dwSampleSize, unsigned __int64 time);
};

class ASFSampleGrabber : public SampleGrabberCB {
protected:
    IMFByteStream* _byteStream;
public:
    ASFSampleGrabber(IMFByteStream* byteStream, std::wstring name)
        : SampleGrabberCB(name), _byteStream(byteStream)
    {
        
    }

    HRESULT StopImpl();
    HRESULT ShutdownImpl();
    HRESULT StartImpl();
    HRESULT ProcessSampleImpl(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
        LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
        DWORD dwSampleSize, unsigned __int64 time);
};