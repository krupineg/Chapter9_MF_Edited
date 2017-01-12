#include "SampleGrabberCB.h"
HRESULT SampleGrabberCB::CreateInstance(IMFSinkWriter * writer, CTopoBuilderBase * builder, IMFMediaType *pTypeIn, IMFMediaType *pTypeOut, SampleGrabberCB **ppCB)
{
    *ppCB = new (std::nothrow) SampleGrabberCB(writer, builder, pTypeIn, pTypeOut);

    if (ppCB == NULL)
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(SampleGrabberCB, IMFSampleGrabberSinkCallback),
        QITABENT(SampleGrabberCB, IMFClockStateSink),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) SampleGrabberCB::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) SampleGrabberCB::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;

}

// IMFClockStateSink methods.

// In these example, the IMFClockStateSink methods do not perform any actions. 
// You can use these methods to track the state of the sample grabber sink.

STDMETHODIMP SampleGrabberCB::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnClockStop(MFTIME hnsSystemTime)
{
   
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnClockPause(MFTIME hnsSystemTime)
{
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnClockRestart(MFTIME hnsSystemTime)
{
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
    return S_OK;
}

// IMFSampleGrabberSink methods.

STDMETHODIMP SampleGrabberCB::OnSetPresentationClock(IMFPresentationClock* pClock)
{
    return S_OK;
}

HRESULT WriteSample(IMFSinkWriter* writer, DWORD cbData, LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE *buff)
{
    IMFMediaBuffer*  pMediaBuffer;
    IMFSample* pSample;
    DWORD  cbMaxLength, cbCurrentLength;
    BYTE  *pbBuffer;
    HRESULT hr = MFCreateMemoryBuffer(cbData, &pMediaBuffer);
    THROW_ON_FAIL(hr);
    hr = pMediaBuffer->Lock(&pbBuffer, &cbMaxLength, &cbCurrentLength);
    THROW_ON_FAIL(hr);
    hr = memcpy_s(pbBuffer, cbData, buff, cbData);
    THROW_ON_FAIL(hr);
    //copy your array to pbBuffer (or have the frames come in to this location in the first place)
    hr = pMediaBuffer->Unlock();
    THROW_ON_FAIL(hr);
    hr = pMediaBuffer->SetCurrentLength(cbData);
    THROW_ON_FAIL(hr);
    hr = MFCreateSample(&pSample);
    THROW_ON_FAIL(hr);
    hr = pSample->AddBuffer(pMediaBuffer);
    THROW_ON_FAIL(hr);
    hr = pSample->SetSampleDuration(llSampleDuration);
    THROW_ON_FAIL(hr);
    hr = pSample->SetSampleTime(llSampleTime);
    THROW_ON_FAIL(hr);
    hr = writer->WriteSample(0, pSample);
    THROW_ON_FAIL(hr);
    SafeRelease(&pMediaBuffer);
    SafeRelease(&pSample);
}

STDMETHODIMP SampleGrabberCB::OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
    LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
    DWORD dwSampleSize)
{
    HRESULT hr = S_OK;
    if (!stopped && m_pWriter) {
        if (timeOffset == 0) {
            timeOffset = llSampleTime;
        }
        llSampleTime = llSampleTime - timeOffset;
        DebugLongLong(L"Sample: start ", llSampleTime);
        hr = WriteSample(m_pWriter, dwSampleSize, llSampleTime, llSampleDuration, pSampleBuffer);
        THROW_ON_FAIL(hr);
    }
    // Display information about the sample.
   
    return hr;
}

void SampleGrabberCB::Start() {
    HRESULT hr = m_pWriter->BeginWriting();
    THROW_ON_FAIL(hr);
}

void SampleGrabberCB::Stop() {
    if (!stopped) {
        stopped = true;
        if (m_pWriter)
        {
            HRESULT hr = m_pWriter->Flush(0);
            THROW_ON_FAIL(hr);
            hr = m_pWriter->Finalize();
            THROW_ON_FAIL(hr);
            m_pWriter = NULL;
        }
    }
   
}

STDMETHODIMP SampleGrabberCB::OnShutdown()
{    
    return S_OK;
}
