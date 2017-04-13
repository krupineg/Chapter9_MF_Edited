#include "SampleGrabberCB.h"
#include "Wmcontainer.h"
HRESULT SampleGrabberCB::CreateTimingInstance(std::wstring name, SampleGrabberCB **ppCB)
{
    *ppCB = new (std::nothrow) TimingSampleGrabber(name);

    if (ppCB == NULL)
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::QueryInterface(REFIID riid, void** ppv)
{

    HRESULT hr = S_OK;

    if (ppv == NULL)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFMediaSink*>(this));
    }
    else if (riid == IID_IMFMediaSink)
    {
        *ppv = static_cast<IMFMediaSink*>(this);
    }
    else if (riid == IID_IMFClockStateSink)
    {
        *ppv = static_cast<IMFClockStateSink*>(this);
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
        AddRef();

    return hr;
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
    try {
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
    catch(exception){
        SafeRelease(&pMediaBuffer);
        SafeRelease(&pSample);
        throw;
    }   
}

STDMETHODIMP SampleGrabberCB::OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
    LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
    DWORD dwSampleSize)
{
    HRESULT hr = S_OK;
    if (!stopped) {
        unsigned __int64 time;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));
        hr = ProcessSampleImpl(guidMajorMediaType, dwSampleFlags, llSampleTime, llSampleDuration, pSampleBuffer, dwSampleSize, time);
        count++;
    }
    // Display information about the sample.

    return hr;
}

void SampleGrabberCB::Stop() {
    stopped = true;
    THROW_ON_FAIL(StopImpl());
}

STDMETHODIMP SampleGrabberCB::OnShutdown()
{
    return S_OK;
}



//
// Get the characteristics of the sink
//
HRESULT SampleGrabberCB::GetCharacteristics(DWORD* pdwCharacteristics)
{
    if (pdwCharacteristics == NULL)
        return E_POINTER;

    // rateless sink with a fixed number of streams
    *pdwCharacteristics = MEDIASINK_RATELESS | MEDIASINK_FIXED_STREAMS;

    return S_OK;
}


// 
// Add a new stream to the sink - not supported, since the sink supports a fixed number of 
// streams.
//
HRESULT SampleGrabberCB::AddStreamSink(
    DWORD dwStreamSinkIdentifier,   // new stream ID
    IMFMediaType* pMediaType,       // media type of the new stream - can be NULL
    IMFStreamSink** ppStreamSink)   // resulting stream
{
    return MF_E_STREAMSINKS_FIXED;
}


//
// Remove/delete a stream from the sink, identified by stream ID.  Not implemented since the
// sink supports a fixed number of streams.
//
HRESULT SampleGrabberCB::RemoveStreamSink(DWORD dwStreamSinkIdentifier)
{
    return MF_E_STREAMSINKS_FIXED;
}



//
// Get the number of stream sinks currently registered with the sink
//
HRESULT SampleGrabberCB::GetStreamSinkCount(DWORD* pcStreamSinkCount)
{
    HRESULT hr = S_OK;
    *pcStreamSinkCount = 1;
    return hr;
}



//
// Get stream by index
//
HRESULT SampleGrabberCB::GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink** ppStreamSink)
{
    HRESULT hr = MF_E_SINK_NO_STREAMS;
    return hr;
}



//
// Get stream by sink ID
//
HRESULT SampleGrabberCB::GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink** ppStreamSink)
{
    HRESULT hr = MF_E_SINK_NO_STREAMS;
    return hr;
}



//
// Set the presentation clock on the sink
//
HRESULT SampleGrabberCB::SetPresentationClock(IMFPresentationClock* pPresentationClock)
{
    return S_OK;
}


//
// Get the current presentation clock
//
HRESULT SampleGrabberCB::GetPresentationClock(IMFPresentationClock** ppPresentationClock)
{
    return MF_E_NO_CLOCK;
}


//
// Shut down the sink
//
HRESULT SampleGrabberCB::Shutdown(void)
{
    return ShutdownImpl();
}

HRESULT TimingSampleGrabber::StopImpl() {
    return S_OK;
}

HRESULT TimingSampleGrabber::StartImpl() {
    return S_OK;
}

HRESULT TimingSampleGrabber::ShutdownImpl() {
    return S_OK;
}

HRESULT TimingSampleGrabber::ProcessSampleImpl(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
    LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
    DWORD dwSampleSize, unsigned __int64 time) {
    DebugLog(L"====");
    DebugLog(_name);
    DebugLongLong(L"Sample: index ", count);
    DebugLongLong(L"Sample timestamp ", llSampleTime);
    DebugLongLong(L"Sample qpc time ", time);
    DebugLog(L"====");
    return S_OK;
}

HRESULT ASFSampleGrabber::StopImpl() {
    return _sink->Shutdown();
}

HRESULT ASFSampleGrabber::StartImpl() {
    return S_OK;
}

HRESULT ASFSampleGrabber::ShutdownImpl() {
    return S_OK;
}

HRESULT ASFSampleGrabber::ProcessSampleImpl(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
    LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
    DWORD dwSampleSize, unsigned __int64 time) {   
    return S_OK;
}