#pragma once
#include "VideoSink.h"

const DWORD VIDEO_SINK_STREAM_ID = 1;


// Video_FILE_HEADER
// This structure contains the first part of the .Video file, up to the
// data portion. (Videoe files are so simple there is no reason to write
// a general-purpose RIFF authoring object.)
//struct Video_FILE_HEADER
//{
//    RIFFCHUNK       FileHeader;
//    DWORD           fccVideoeType;    // must be 'VideoE'
//    RIFFCHUNK       VideoeHeader;
//    VideoEFORMATEX    VideoeFormat;
//    RIFFCHUNK       DataHeader;
//};

// PCM_Audio_Format_Params
// Defines parameters for uncompressed PCM audio formats.
// The remaining fields can be derived from these.
struct PCM_Audio_Format_Params
{
    DWORD   nSamplesPerSec; // Samples per second.
    WORD    wBitsPerSample; // Bits per sample.
    WORD    nChannels;      // Number of channels.
};


// g_AudioFormats: Static list of our preferred formats.

// This is an ordered list that we use to hand out formats in the 
// stream's IMFMediaTypeHandler::GetMediaTypeByIndex method. The 
// stream will accept other bit rates not listed here.

PCM_Audio_Format_Params g_AudioFormats[] =
{
    { 48000, 16, 2 },
    { 48000, 8, 2 },
    { 44100, 16, 2 },
    { 44100, 8, 2 },
    { 22050, 16, 2 },
    { 22050, 8, 2 },

    { 48000, 16, 1 },
    { 48000, 8, 1 },
    { 44100, 16, 1 },
    { 44100, 8, 1 },
    { 22050, 16, 1 },
    { 22050, 8, 1 },
};

DWORD g_NumAudioFormats = 12;

// Forward declares
HRESULT ValidateVideoFormat(const WAVEFORMATEX *pWav, DWORD cbSize);

HRESULT CreateRawVideoType(
    UINT32 frameRate,        // Samples per second
    UINT32 width,     // Bits per sample
    UINT32 hieght,         // Number of channels
    IMFMediaType **ppType     // Receives a pointer to the media type.
);


//HRESULT CreatePCMAudioType(
//    UINT32 sampleRate,        // Samples per second
//    UINT32 bitsPerSample,     // Bits per sample
//    UINT32 cChannels,         // Number of channels
//    IMFMediaType **ppType     // Receives a pointer to the media type.
//    );
/////////////////////////////////////////////////////////////////////////////////////////////
//
// CVideoSink class. - Implements the media sink.
//
// Notes:
// - Most public methods calls CheckShutdown. This method fails if the sink was shut down.
//
/////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------
// Name: CreateInstance 
// Description: Creates an instance of the VideoSink object. 
// [See CreateVideoSink]
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// CVideoSink constructor.
//-------------------------------------------------------------------

CVideoSink::CVideoSink(IMFByteStream *pStream, IMFMediaType *type) :
    m_nRefCount(1), m_IsShutdown(FALSE), m_pStream(NULL), m_pClock(NULL)
{
    this->Initialize(pStream, type);
}

//-------------------------------------------------------------------
// CVideoSink destructor.
//-------------------------------------------------------------------

CVideoSink::~CVideoSink()
{
    DebugInfo(L"~CVideoSink\n");
    assert(m_IsShutdown);
}

// IUnknown methods
//
ULONG CVideoSink::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG  CVideoSink::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CVideoSink::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFMediaSink*>(this));
    }
    else if (iid == __uuidof(IMFMediaSink))
    {
        *ppv = static_cast<IMFMediaSink*>(this);
    }
    else if (iid == __uuidof(IMFMediaSink))
    {
        *ppv = static_cast<IMFMediaSink*>(this);
    }
    else if (iid == __uuidof(IMFClockStateSink))
    {
        *ppv = static_cast<IMFClockStateSink*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}




///  IMFMediaSink methods.


//-------------------------------------------------------------------
// Name: GetCharacteristics 
// Description: Returns the characteristics flags. 
//
// Note: This sink has a fixed number of streams and is rateless.
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::GetCharacteristics(DWORD *pdwCharacteristics)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    if (pdwCharacteristics == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pdwCharacteristics = MEDIASINK_FIXED_STREAMS | MEDIASINK_RATELESS;
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: AddStreamSink 
// Description: Adds a new stream to the sink. 
//
// Note: This sink has a fixed number of streams, so this method
//       always returns MF_E_STREAMSINKS_FIXED.
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::AddStreamSink(
    DWORD dwStreamSinkIdentifier,
    IMFMediaType *pMediaType,
    IMFStreamSink **ppStreamSink)
{
    return MF_E_STREAMSINKS_FIXED;
}



//-------------------------------------------------------------------
// Name: RemoveStreamSink 
// Description: Removes a stream from the sink. 
//
// Note: This sink has a fixed number of streams, so this method
//       always returns MF_E_STREAMSINKS_FIXED.
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::RemoveStreamSink(DWORD dwStreamSinkIdentifier)
{
    return MF_E_STREAMSINKS_FIXED;
}


//-------------------------------------------------------------------
// Name: GetStreamSinkCount 
// Description: Returns the number of streams. 
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::GetStreamSinkCount(DWORD *pcStreamSinkCount)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    if (pcStreamSinkCount == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pcStreamSinkCount = 1;  // Fixed number of streams.
    }

    return hr;

}


//-------------------------------------------------------------------
// Name: GetStreamSinkByIndex 
// Description: Retrieves a stream by index. 
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::GetStreamSinkByIndex(
    DWORD dwIndex,
    IMFStreamSink **ppStreamSink)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    if (ppStreamSink == NULL)
    {
        return E_INVALIDARG;
    }

    // Fixed stream: Index 0. 
    if (dwIndex > 0)
    {
        return MF_E_INVALIDINDEX;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *ppStreamSink = m_pStream;
        (*ppStreamSink)->AddRef();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetStreamSinkById 
// Description: Retrieves a stream by ID. 
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::GetStreamSinkById(
    DWORD dwStreamSinkIdentifier,
    IMFStreamSink **ppStreamSink)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    if (ppStreamSink == NULL)
    {
        return E_INVALIDARG;
    }

    // Fixed stream ID.
    if (dwStreamSinkIdentifier != VIDEO_SINK_STREAM_ID)
    {
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *ppStreamSink = m_pStream;
        (*ppStreamSink)->AddRef();
    }

    return hr;

}


//-------------------------------------------------------------------
// Name: SetPresentationClock 
// Description: Sets the presentation clock. 
//
// pPresentationClock: Pointer to the clock. Can be NULL.
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::SetPresentationClock(IMFPresentationClock *pPresentationClock)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    // If we already have a clock, remove ourselves from that clock's
    // state notifications.
    if (SUCCEEDED(hr))
    {
        if (m_pClock)
        {
            hr = m_pClock->RemoveClockStateSink(this);
        }
    }

    // Register ourselves to get state notifications from the new clock.
    if (SUCCEEDED(hr))
    {
        if (pPresentationClock)
        {
            hr = pPresentationClock->AddClockStateSink(this);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Release the pointer to the old clock.
        // Store the pointer to the new clock.

        SAFE_RELEASE(m_pClock);
        m_pClock = pPresentationClock;
        if (m_pClock)
        {
            m_pClock->AddRef();
        }
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetPresentationClock 
// Description: Returns a pointer to the presentation clock. 
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::GetPresentationClock(IMFPresentationClock **ppPresentationClock)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);
    if (ppPresentationClock == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        if (m_pClock == NULL)
        {
            hr = MF_E_NO_CLOCK; // There is no presentation clock.
        }
        else
        {
            // Return the pointer to the caller.
            *ppPresentationClock = m_pClock;
            (*ppPresentationClock)->AddRef();
        }
    }


    return hr;
}


//-------------------------------------------------------------------
// Name: Shutdown 
// Description: Releases resources held by the media sink. 
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::Shutdown()
{
    DebugInfo(L"CVideoSink::Shutdown\n");
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        BeginFinalize(NULL, NULL);

        m_IsShutdown = true;
    }

    return hr;
}

/// IMFMediaSink methods


//-------------------------------------------------------------------
// Name: BeginFinalize 
// Description: Starts the asynchronous finalize operation.
//
// Note: We use the Finalize operation to write the RIFF headers.
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::BeginFinalize(
    IMFAsyncCallback *pCallback,
    IUnknown *punkState)
{
    DebugInfo(L"CVideoSink::BeginFinalize\n");

    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    // Tell the stream to finalize.
    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Finalize(pCallback, punkState);
    }
    return hr;
}


//-------------------------------------------------------------------
// Name: EndFinalize 
// Description: Completes the asynchronous finalize operation.
//-------------------------------------------------------------------

IFACEMETHODIMP CVideoSink::EndFinalize(IMFAsyncResult *pResult)
{
    DebugInfo(L"CVideoSink::EndFinalize\n");

    HRESULT hr = S_OK;

    // Return the status code from the async result.
   /* if (pResult == NULL)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = pResult->GetStatus();
    }*/
    hr = m_pStream->Shutdown();
    THROW_ON_FAIL(hr);
    SAFE_RELEASE(m_pClock);
    SAFE_RELEASE(m_pStream);

    return hr;
}

//-------------------------------------------------------------------
// Name: OnClockStart 
// Description: Called when the presentation clock starts.
//
// hnsSystemTime: System time when the clock started.
// llClockStartOffset: Starting presentatation time.
//
// Note: For an archive sink, we don't care about the system time.
//       But we need to cache the value of llClockStartOffset. This 
//       gives us the earliest time stamp that we archive. If any 
//       input samples have an earlier time stamp, we discard them.
//-------------------------------------------------------------------

HRESULT CVideoSink::OnClockStart(
    /* [in] */ MFTIME hnsSystemTime,
    /* [in] */ LONGLONG llClockStartOffset)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Start(llClockStartOffset);
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: OnClockStop 
// Description: Called when the presentation clock stops.
//
// Note: After this method is called, we stop accepting new data.
//-------------------------------------------------------------------

HRESULT CVideoSink::OnClockStop(
    /* [in] */ MFTIME hnsSystemTime)
{
    DebugInfo((L"CVideoSink::OnClockStop\n"));
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Stop();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: OnClockPause 
// Description: Called when the presentation clock paused.
//
// Note: For an archive sink, the paused state is equivalent to the
//       running (started) state. We still accept data and archive it.
//-------------------------------------------------------------------

HRESULT CVideoSink::OnClockPause(
    /* [in] */ MFTIME hnsSystemTime)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Pause();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: OnClockRestart 
// Description: Called when the presentation clock restarts.
//-------------------------------------------------------------------

HRESULT CVideoSink::OnClockRestart(
    /* [in] */ MFTIME hnsSystemTime)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Restart();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: OnClockSetRate 
// Description: Called when the presentation clock's rate changes.
//
// Note: For a rateless sink, the clock rate is not important.
//-------------------------------------------------------------------

HRESULT CVideoSink::OnClockSetRate(
    /* [in] */ MFTIME hnsSystemTime,
    /* [in] */ float flRate)
{
    return S_OK;
}


/// Private methods


//-------------------------------------------------------------------
// Name: Initialize 
// Description: Initializes the media sink.
//
// Note: This method is called once when the media sink is first
//       initialized.
//-------------------------------------------------------------------

HRESULT CVideoSink::Initialize(IMFByteStream *pByteStream, IMFMediaType * type)
{
    HRESULT hr = S_OK;

    m_pStream = new CVideoStream();
    if (m_pStream == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    IMFMediaTypeHandler* typeHandler;


    // Initialize the stream.
    if (SUCCEEDED(hr))
    {
        hr = m_pStream->Initialize(this, pByteStream);
    }

    if (SUCCEEDED(hr))
       {
    	m_pStream->GetMediaTypeHandler(&typeHandler);
    	//typeHandler->GetCurrentMediaType(&type);
    	//CreateRawVideoType(30,352,288,&type);

    	typeHandler->SetCurrentMediaType(type);
    }
    return hr;
}



/////////////////////////////////////////////////////////////////////////////////////////////
//
// CAsyncOperation class. - Private class used by CVideoStream class.
//
/////////////////////////////////////////////////////////////////////////////////////////////

CVideoStream::CAsyncOperation::CAsyncOperation(StreamOperation op)
    : m_nRefCount(1), m_op(op)
{
}

CVideoStream::CAsyncOperation::~CAsyncOperation()
{
    assert(m_nRefCount == 0);
}

ULONG CVideoStream::CAsyncOperation::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG CVideoStream::CAsyncOperation::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CVideoStream::CAsyncOperation::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
// CVideoStream class. - Implements the stream sink.
//
// Notes: 
// - Most of the real work gets done in this class. 
// - The sink has one stream. If it had multiple streams, it would need to coordinate them.
// - Most operations are done asynchronously on a work queue.
// - Async methods are handled like this:
//      1. Call ValidateOperation to check if the operation is permitted at this time
//      2. Create an CAsyncOperation object for the operation.
//      3. Call QueueAsyncOperation. This puts the operation on the work queue.
//      4. The workqueue calls OnDispatchWorkItem.
// - Locking:
//      To avoid deadlocks, do not hold the CVideoStream lock followed by the CVideoSink lock.
//      The other order is OK (CVideoSink, then CVideoStream).
// 
/////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------
// CVideoStream constructor
//-------------------------------------------------------------------

CVideoStream::CVideoStream()
    : m_nRefCount(1), m_state(State_TypeNotSet), m_IsShutdown(FALSE),
    m_pSink(NULL), m_pEventQueue(NULL), m_pByteStream(NULL),
    m_pCurrentType(NULL), m_pFinalizeResult(NULL),
    m_StartTime(0), m_cbDataWritten(0), m_WorkQueueId(0),
    m_WorkQueueCB(this, &CVideoStream::OnDispatchWorkItem)
{

}


//-------------------------------------------------------------------
// CVideoStream destructor
//-------------------------------------------------------------------

CVideoStream::~CVideoStream()
{
    DebugInfo(L"~CVideoStream\n");
    assert(m_IsShutdown);
}


// IUnknown methods

ULONG CVideoStream::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG  CVideoStream::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CVideoStream::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFStreamSink*>(this));
    }
    else if (iid == __uuidof(IMFStreamSink))
    {
        *ppv = static_cast<IMFStreamSink *>(this);
    }
    else if (iid == __uuidof(IMFMediaEventGenerator))
    {
        *ppv = static_cast<IMFMediaEventGenerator*>(this);
    }
    else if (iid == __uuidof(IMFMediaTypeHandler))
    {
        *ppv = static_cast<IMFMediaTypeHandler*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}


// IMFMediaEventGenerator methods.
// Note: These methods call through to the event queue helper object.

HRESULT CVideoStream::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    HRESULT hr = S_OK;

    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->BeginGetEvent(pCallback, punkState);
    }

    return hr;
}

HRESULT CVideoStream::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->EndGetEvent(pResult, ppEvent);
    }

    return hr;
}

HRESULT CVideoStream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    // NOTE: 
    // GetEvent can block indefinitely, so we don't hold the lock.
    // This requires some juggling with the event queue pointer.

    HRESULT hr = S_OK;

    IMFMediaEventQueue *pQueue = NULL;

    { // scope for lock

        CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

        // Check shutdown
        hr = CheckShutdown();

        // Get the pointer to the event queue.
        if (SUCCEEDED(hr))
        {
            pQueue = m_pEventQueue;
            pQueue->AddRef();
        }

    }   // release lock

        // Now get the event.
    if (SUCCEEDED(hr))
    {
        hr = pQueue->GetEvent(dwFlags, ppEvent);
    }

    SAFE_RELEASE(pQueue);

    return hr;
}

HRESULT CVideoStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{

    HRESULT hr = S_OK;

    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);
    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    return hr;
}




/// IMFStreamSink methods


//-------------------------------------------------------------------
// Name: GetMediaSink 
// Description: Returns the parent media sink.
//-------------------------------------------------------------------

HRESULT CVideoStream::GetMediaSink(IMFMediaSink **ppMediaSink)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    if (ppMediaSink == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *ppMediaSink = (IMFMediaSink*)m_pSink;
        (*ppMediaSink)->AddRef();
    }

    return hr;

}


//-------------------------------------------------------------------
// Name: GetIdentifier 
// Description: Returns the stream identifier.
//-------------------------------------------------------------------

HRESULT CVideoStream::GetIdentifier(DWORD *pdwIdentifier)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    if (pdwIdentifier == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pdwIdentifier = VIDEO_SINK_STREAM_ID;
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetMediaTypeHandler 
// Description: Returns a media type handler for this stream.
//-------------------------------------------------------------------

HRESULT CVideoStream::GetMediaTypeHandler(IMFMediaTypeHandler **ppHandler)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    if (ppHandler == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    // This stream object acts as its own type handler, so we QI ourselves.
    if (SUCCEEDED(hr))
    {
        hr = this->QueryInterface(IID_IMFMediaTypeHandler, (void**)ppHandler);
    }
    return hr;
}


//-------------------------------------------------------------------
// Name: ProcessSample 
// Description: Receives an input sample. [Asynchronous]
//
// Note: The client should only give us a sample after we send an
//       MEStreamSinkRequestSample event.
//-------------------------------------------------------------------

HRESULT CVideoStream::ProcessSample(IMFSample *pSample)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    if (pSample == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    hr = CheckShutdown();

    // Validate the operation.
    if (SUCCEEDED(hr))
    {
        hr = ValidateOperation(OpProcessSample);
    }

    // Add the sample to the sample queue.
    if (SUCCEEDED(hr))
    {
        hr = m_SampleQueue.InsertBack(pSample);
    }

    // Unless we are paused, start an async operation to dispatch the next sample.
    if (SUCCEEDED(hr))
    {
        if (m_state != State_Paused)
        {
            // Queue the operation.
            hr = QueueAsyncOperation(OpProcessSample);
        }
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: PlaceMarker 
// Description: Receives a marker. [Asynchronous]
//
// Note: The client can call PlaceMarker at any time. In response,
//       we need to queue an MEStreamSinkMarer event, but not until
//       *after* we have processed all samples that we have received
//       up to this point. 
//
//       Also, in general you might need to handle specific marker
//       types, although this sink does not.
//-------------------------------------------------------------------

HRESULT CVideoStream::PlaceMarker(
    MFSTREAMSINK_MARKER_TYPE eMarkerType,
    const PROPVARIANT *pvarMarkerValue,
    const PROPVARIANT *pvarContextValue)
{

    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = S_OK;

    IMarker *pMarker = NULL;

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = ValidateOperation(OpPlaceMarker);
    }

    // Create a marker object and put it on the sample queue.
    if (SUCCEEDED(hr))
    {
        hr = CMarker::Create(
            eMarkerType,
            pvarMarkerValue,
            pvarContextValue,
            &pMarker);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_SampleQueue.InsertBack(pMarker);
    }

    // Unless we are paused, start an async operation to dispatch the next sample/marker.
    if (SUCCEEDED(hr))
    {
        if (m_state != State_Paused)
        {
            // Queue the operation.
            hr = QueueAsyncOperation(OpPlaceMarker); // Increments ref count on pOp.
        }
    }

    SAFE_RELEASE(pMarker);

    return hr;
}


//-------------------------------------------------------------------
// Name: Flush 
// Description: Discards all samples that were not processed yet.
//-------------------------------------------------------------------

HRESULT CVideoStream::Flush()
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        // Note: Even though we are flushing data, we still need to send 
        // any marker events that were queued.
        hr = ProcessSamplesFromQueue(DropSamples);
    }

    return hr;
}


/// IMFMediaTypeHandler methods

//-------------------------------------------------------------------
// Name: IsMediaTypeSupported 
// Description: Check if a media type is supported.
//
// pMediaType: The media type to check.
// ppMediaType: Optionally, receives a "close match" media type.
//-------------------------------------------------------------------


HRESULT CVideoStream::IsMediaTypeSupported(
    /* [in] */ IMFMediaType *pMediaType,
    /* [out] */ IMFMediaType **ppMediaType)
{
    if (pMediaType == NULL)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    GUID majorType = GUID_NULL;
    //WAVEFORMATEX *pWav = NULL;
    UINT cbSize = 0;

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = pMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
    }

    // First make sure it's audio. 
    if (SUCCEEDED(hr))
    {
        if (majorType != MFMediaType_Video)
        {
            hr = MF_E_INVALIDTYPE;
        }
    }

    //// Get a WAVEFORMATEX structure to validate against.
    //if (SUCCEEDED(hr))
    //{
    //    hr = MFCreateWaveFormatExFromMFMediaType(pMediaType, &pWav, &cbSize);
    //}

    //// Validate the WAVEFORMATEX structure.
    //if (SUCCEEDED(hr))
    //{
    //    hr = ValidateVideoFormat(pWav, cbSize);
    //}

    // We don't return any "close match" types.
    if (ppMediaType)
    {
        *ppMediaType = NULL;
    }


    //CoTaskMemFree(pWav);

    return hr;
}


//-------------------------------------------------------------------
// Name: GetMediaTypeCount 
// Description: Return the number of preferred media types.
//-------------------------------------------------------------------

HRESULT CVideoStream::GetMediaTypeCount(DWORD *pdwTypeCount)
{
    if (pdwTypeCount == NULL)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        *pdwTypeCount = g_NumAudioFormats;
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetMediaTypeByIndex 
// Description: Return a preferred media type by index.
//-------------------------------------------------------------------

HRESULT CVideoStream::GetMediaTypeByIndex(
    /* [in] */ DWORD dwIndex,
    /* [out] */ IMFMediaType **ppType)
{
    if (ppType == NULL)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        if (dwIndex >= g_NumAudioFormats)
        {
            hr = MF_E_NO_MORE_TYPES;
        }
    }

    if (SUCCEEDED(hr))
    {
        /*const DWORD   nSamplesPerSec = g_AudioFormats[dwIndex].nSamplesPerSec;
        const WORD    wBitsPerSample = g_AudioFormats[dwIndex].wBitsPerSample;
        const WORD    nChannels = g_AudioFormats[dwIndex].nChannels;*/

        hr = CreateRawVideoType(30, 640, 480, ppType);
        m_pCurrentType = *ppType;

    }

    return hr;
}


//-------------------------------------------------------------------
// Name: SetCurrentMediaType 
// Description: Set the current media type.
//-------------------------------------------------------------------

HRESULT CVideoStream::SetCurrentMediaType(IMFMediaType *pMediaType)
{
    if (pMediaType == NULL)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = CheckShutdown();

    // We don't allow format changes after streaming starts,
    // because this would invalidate the .Video file.
    if (SUCCEEDED(hr))
    {
        hr = ValidateOperation(OpSetMediaType);
    }

    if (SUCCEEDED(hr))
    {
        hr = IsMediaTypeSupported(pMediaType, NULL);
    }


    if (SUCCEEDED(hr))
    {
        SAFE_RELEASE(m_pCurrentType);
        m_pCurrentType = pMediaType;
        m_pCurrentType->AddRef();

        m_state = State_Ready;
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: GetCurrentMediaType 
// Description: Return the current media type, if any.
//-------------------------------------------------------------------

HRESULT CVideoStream::GetCurrentMediaType(IMFMediaType **ppMediaType)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    if (ppMediaType == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        if (m_pCurrentType == NULL)
        {
            hr = MF_E_NOT_INITIALIZED;
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppMediaType = m_pCurrentType;
        (*ppMediaType)->AddRef();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: GetMajorType 
// Description: Return the major type GUID.
//-------------------------------------------------------------------

HRESULT CVideoStream::GetMajorType(GUID *pguidMajorType)
{
    if (pguidMajorType == NULL)
    {
        return E_INVALIDARG;
    }

    *pguidMajorType = MFMediaType_Video;

    return S_OK;
}


// private methods



//-------------------------------------------------------------------
// Name: Initialize 
// Description: Initializes the stream sink.
//
// Note: This method is called once when the media sink is first
//       initialized.
//-------------------------------------------------------------------

HRESULT CVideoStream::Initialize(CVideoSink *pParent, IMFByteStream *pByteStream)
{
    assert(pParent != NULL);
    //assert(pByteStream != NULL);

    HRESULT hr = S_OK;

    DWORD dwCaps = 0;
    //const DWORD dwRequiredCaps = (MFBYTESTREAM_IS_WRITABLE | MFBYTESTREAM_IS_SEEKABLE);

    // Make sure the byte stream has the necessary caps bits.
    //hr = pByteStream->GetCapabilities(&dwCaps);

    /*if (SUCCEEDED(hr))
    {
    if ((dwCaps & dwRequiredCaps) != dwRequiredCaps)
    {
    hr = E_FAIL;
    }
    }*/

    //// Move the file pointer to leave room for the RIFF headers.
    //if (SUCCEEDED(hr))
    //{
    //    hr = pByteStream->SetCurrentPosition(sizeof(VIDEO_FILE_HEADER));
    //}

    // Create the event queue helper.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateEventQueue(&m_pEventQueue);
    }

    // Allocate a new work queue for async operations.
    if (SUCCEEDED(hr))
    {
        hr = MFAllocateSerialWorkQueue(MFASYNC_CALLBACK_QUEUE_STANDARD, &m_WorkQueueId);
    }


    if (SUCCEEDED(hr))
    {
        m_pByteStream = pByteStream;
        m_pByteStream->AddRef();

        m_pSink = pParent;
        m_pSink->AddRef();
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: Start 
// Description: Called when the presentation clock starts.
//
// Note: Start time can be PRESENTATION_CURRENT_POSITION meaning
//       resume from the last current position.
//-------------------------------------------------------------------

HRESULT CVideoStream::Start(MFTIME start)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpStart);

    if (SUCCEEDED(hr))
    {
        if (start != PRESENTATION_CURRENT_POSITION)
        {
            m_StartTime = start;        // Cache the start time.
        }
        m_state = State_Started;
        hr = QueueAsyncOperation(OpStart);
    }
    return hr;

}

//-------------------------------------------------------------------
// Name: Stop
// Description: Called when the presentation clock stops.
//-------------------------------------------------------------------

HRESULT CVideoStream::Stop()
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpStop);

    if (SUCCEEDED(hr))
    {
        m_state = State_Stopped;
        hr = QueueAsyncOperation(OpStop);
    }
    return hr;
}


//-------------------------------------------------------------------
// Name: Pause
// Description: Called when the presentation clock pauses.
//-------------------------------------------------------------------

HRESULT CVideoStream::Pause()
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpPause);

    if (SUCCEEDED(hr))
    {
        m_state = State_Paused;
        hr = QueueAsyncOperation(OpPause);
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: Restart
// Description: Called when the presentation clock restarts.
//-------------------------------------------------------------------

HRESULT CVideoStream::Restart()
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpRestart);

    if (SUCCEEDED(hr))
    {
        m_state = State_Started;
        hr = QueueAsyncOperation(OpRestart);
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: Finalize
// Description: Starts the async finalize operation.
//-------------------------------------------------------------------

HRESULT CVideoStream::Finalize(IMFAsyncCallback *pCallback, IUnknown *punkState)
{
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = S_OK;

    hr = ValidateOperation(OpFinalize);

    if (SUCCEEDED(hr))
    {
        if (m_pFinalizeResult != NULL)
        {
            hr = MF_E_INVALIDREQUEST;  // The operation is already pending.
        }
    }

    // Create and store the async result object.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateAsyncResult(NULL, pCallback, punkState, &m_pFinalizeResult);
    }

    if (SUCCEEDED(hr))
    {
        m_state = State_Finalized;
        hr = QueueAsyncOperation(OpFinalize);
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: ValidStateMatrix
// Description: Class-static matrix of operations vs states.
//
// If an entry is TRUE, the operation is valid from that state.
//-------------------------------------------------------------------

BOOL CVideoStream::ValidStateMatrix[CVideoStream::State_Count][CVideoStream::Op_Count] =
{
    // States:    Operations:
    //            SetType   Start     Restart   Pause     Stop      Sample    Marker    Finalize
    /* NotSet */  TRUE,     FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE,

    /* Ready */   TRUE,     TRUE,     FALSE,    TRUE,     TRUE,     FALSE,    TRUE,     TRUE,

    /* Start */   FALSE,    TRUE,     FALSE,    TRUE,     TRUE,     TRUE,     TRUE,     TRUE,

    /* Pause */   FALSE,    TRUE,     TRUE,     TRUE,     TRUE,     TRUE,     TRUE,     TRUE,

    /* Stop */    FALSE,    TRUE,     FALSE,    FALSE,    TRUE,     FALSE,    TRUE,     TRUE,

    /* Final */   FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE,    FALSE

    // Note about states:
    // 1. OnClockRestart should only be called from paused state.
    // 2. While paused, the sink accepts samples but does not process them.

};


//-------------------------------------------------------------------
// Name: ValidateOperation
// Description: Checks if an operation is valid in the current state.
//-------------------------------------------------------------------

HRESULT CVideoStream::ValidateOperation(StreamOperation op)
{
    assert(!m_IsShutdown);

    HRESULT hr = S_OK;

    BOOL bTransitionAllowed = ValidStateMatrix[m_state][op];

    if (bTransitionAllowed)
    {
        return S_OK;
    }
    else
    {
        return MF_E_INVALIDREQUEST;
    }
}

//-------------------------------------------------------------------
// Name: Shutdown
// Description: Shuts down the stream sink.
//-------------------------------------------------------------------

HRESULT CVideoStream::Shutdown()
{
    assert(!m_IsShutdown);

    if (m_pEventQueue)
    {
        m_pEventQueue->Shutdown();
    }

    MFUnlockWorkQueue(m_WorkQueueId);

    m_SampleQueue.Clear();

    SAFE_RELEASE(m_pSink);
    SAFE_RELEASE(m_pEventQueue);
    SAFE_RELEASE(m_pByteStream);
    SAFE_RELEASE(m_pCurrentType);
    SAFE_RELEASE(m_pFinalizeResult);

    m_IsShutdown = TRUE;

    return S_OK;
}


//-------------------------------------------------------------------
// Name: QueueAsyncOperation
// Description: Puts an async operation on the work queue.
//-------------------------------------------------------------------

HRESULT CVideoStream::QueueAsyncOperation(StreamOperation op)
{
    HRESULT hr = S_OK;
    CAsyncOperation *pOp = new CAsyncOperation(op); // Created with ref count = 1
    if (pOp == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = MFPutWorkItem2(m_WorkQueueId, 0, &m_WorkQueueCB, pOp);
    }

    SAFE_RELEASE(pOp);

    return hr;
}



//-------------------------------------------------------------------
// Name: OnDispatchWorkItem
// Description: Callback for MFPutWorkItem.
//-------------------------------------------------------------------

HRESULT CVideoStream::OnDispatchWorkItem(IMFAsyncResult* pAsyncResult)
{
    // Called by work queue thread. Need to hold the critical section.
    CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

    HRESULT hr = S_OK;

    IUnknown *pState = NULL;

    hr = pAsyncResult->GetState(&pState);

    if (SUCCEEDED(hr))
    {
        // The state object is a CAsncOperation object.
        CAsyncOperation *pOp = (CAsyncOperation*)pState;

        StreamOperation op = pOp->m_op;

        switch (op)
        {
        case OpStart:
        case OpRestart:
            // Send MEStreamSinkStarted.
            hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL);

            // Kick things off by requesting two samples...
            if (SUCCEEDED(hr))
            {
                hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL);
            }
            if (SUCCEEDED(hr))
            {
                hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL);
            }

            // There might be samples queue from earlier (ie, while paused).
            if (SUCCEEDED(hr))
            {
                hr = ProcessSamplesFromQueue(WriteSamples);
            }
            break;

        case OpStop:
            // Drop samples from queue.
            hr = ProcessSamplesFromQueue(DropSamples);

            // Send the event even if the previous call failed.
            hr = QueueEvent(MEStreamSinkStopped, GUID_NULL, hr, NULL);
            break;

        case OpPause:
            hr = QueueEvent(MEStreamSinkPaused, GUID_NULL, hr, NULL);
            break;

        case OpProcessSample:
        case OpPlaceMarker:
            hr = DispatchProcessSample(pOp);
            break;

        case OpFinalize:
            hr = DispatchFinalize(pOp);
            break;
        }
    }

    SAFE_RELEASE(pState);

    return hr;
}


//-------------------------------------------------------------------
// Name: DispatchProcessSample
// Description: Complete a ProcessSample or PlaceMarker request.
//-------------------------------------------------------------------

HRESULT CVideoStream::DispatchProcessSample(CAsyncOperation* pOp)
{
    HRESULT hr = S_OK;
    assert(pOp != NULL);

    hr = ProcessSamplesFromQueue(WriteSamples);

    // Ask for another sample
    if (SUCCEEDED(hr))
    {
        if (pOp->m_op == OpProcessSample)
        {
            hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, S_OK, NULL);
        }
    }

    // We are in the middle of an asynchronous operation, so if something failed, send an error.
    if (FAILED(hr))
    {
        hr = QueueEvent(MEError, GUID_NULL, hr, NULL);
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: ProcessSamplesFromQueue
// Description: 
//
// Removes all of the samples and markers that are currently in the
// queue and processes them.
//
// If bFlushData = DropSamples:
//     For each marker, send an MEStreamSinkMarker event, with hr = E_ABORT.
//     For each sample, drop the sample.
//
// If bFlushData = WriteSamples
//     For each marker, send an MEStreamSinkMarker event, with hr = S_OK.
//     For each sample, write the sample to the file.
//
// This method is called when we flush, stop, restart, receive a new
// sample, or receive a marker.
//-------------------------------------------------------------------

HRESULT CVideoStream::ProcessSamplesFromQueue(FlushState bFlushData)
{
    HRESULT hr = S_OK;

    ComPtrList<IUnknown>::POSITION pos = m_SampleQueue.FrontPosition();

    // Enumerate all of the samples/markers in the queue.

    while (pos != m_SampleQueue.EndPosition())
    {
        IUnknown *pUnk = NULL;
        IMarker  *pMarker = NULL;
        IMFSample *pSample = NULL;

        hr = m_SampleQueue.GetItemPos(pos, &pUnk);

        assert(pUnk != NULL); // GetItemPos should not fail unless we reached the end of the list.

                              // Figure out if this is a marker or a sample.
        if (SUCCEEDED(hr))
        {
            hr = pUnk->QueryInterface(__uuidof(IMarker), (void**)&pMarker);
            if (hr == E_NOINTERFACE)
            {
                // If this is a sample, write it to the file.
                hr = pUnk->QueryInterface(IID_IMFSample, (void**)&pSample);
            }
        }

        // Now handle the sample/marker appropriately.
        if (SUCCEEDED(hr))
        {
            if (pMarker)
            {
                hr = SendMarkerEvent(pMarker, bFlushData);
            }
            else
            {
                assert(pSample != NULL);    // Not a marker, must be a sample
                if (bFlushData == WriteSamples)
                {
                    hr = WriteSampleToFile(pSample);
                }
            }
        }
        SAFE_RELEASE(pUnk);
        SAFE_RELEASE(pMarker);
        SAFE_RELEASE(pSample);

        if (FAILED(hr))
        {
            break;
        }

        pos = m_SampleQueue.Next(pos);

    }       // while loop

            // Now clear the list.
    m_SampleQueue.Clear();

    return hr;
}

//-------------------------------------------------------------------
// Name: WriteSampleToFile
// Description: Output one media sample to the file.
//-------------------------------------------------------------------

HRESULT CVideoStream::WriteSampleToFile(IMFSample *pSample)
{
    HRESULT hr = S_OK;

    LONGLONG time = 0;
    DWORD cBufferCount = 0; // Number of buffers in the sample.
    BYTE *pData = NULL;
    DWORD cbData = 0;
    DWORD cbWritten = 0;

    // Get the time stamp
    hr = pSample->GetSampleTime(&time);

    if (SUCCEEDED(hr))
    {
        // If the time stamp is too early, just discard this sample.
        if (time < m_StartTime)
        {
            return S_OK;
        }
    }
    // Note: If there is no time stamp on the sample, proceed anyway.

    // Find how many buffers are in this sample.
    hr = pSample->GetBufferCount(&cBufferCount);
    if (SUCCEEDED(hr))
    {
        // Loop through all the buffers in the sample.
        for (DWORD iBuffer = 0; iBuffer < cBufferCount; iBuffer++)
        {
            IMFMediaBuffer *pBuffer = NULL;

            hr = pSample->GetBufferByIndex(iBuffer, &pBuffer);

            // Lock the buffer and write the data to the file.
            if (SUCCEEDED(hr))
            {
                hr = pBuffer->Lock(&pData, NULL, &cbData);
            }

            if (SUCCEEDED(hr))
            {
                hr = m_pByteStream->Write(pData, cbData, &cbWritten);
               /* FILE* file = 0;
                fopen_s(&file, "C:\\Users\\Public\\file.rgb", "a+");
                int res = fwrite(pData, sizeof(BYTE), cbData, file);
                fclose(file);*/
                pBuffer->Unlock();
            }

            // Update the running tally of bytes written.
            if (SUCCEEDED(hr))
            {
                m_cbDataWritten += cbData;
            }

            SAFE_RELEASE(pBuffer);

            if (FAILED(hr))
            {
                break;
            }
        }   // for loop
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: SendMarkerEvent
// Description: Saned a marker event.
// 
// pMarker: Pointer to our custom IMarker interface, which holds
//          the marker information.
//-------------------------------------------------------------------

HRESULT CVideoStream::SendMarkerEvent(IMarker *pMarker, FlushState FlushState)
{
    HRESULT hr = S_OK;
    HRESULT hrStatus = S_OK;  // Status code for marker event.

    if (FlushState == DropSamples)
    {
        hrStatus = E_ABORT;
    }

    PROPVARIANT var;
    PropVariantInit(&var);

    // Get the context data.
    hr = pMarker->GetContext(&var);

    if (SUCCEEDED(hr))
    {
        hr = QueueEvent(MEStreamSinkMarker, GUID_NULL, hrStatus, &var);
    }

    PropVariantClear(&var);
    return hr;
}


//-------------------------------------------------------------------
// Name: DispatchFinalize
// Description: Complete a BeginFinalize request.
//-------------------------------------------------------------------

HRESULT CVideoStream::DispatchFinalize(CAsyncOperation* pOp)
{
    HRESULT hr = S_OK;

    // Write any samples left in the queue...
    hr = ProcessSamplesFromQueue(WriteSamples);



    // Close the byte stream.
    if (SUCCEEDED(hr))
    {
        hr = m_pByteStream->Close();

    }

    // Set the async status and invoke the callback.
    m_pFinalizeResult->SetStatus(hr);
   // hr = MFInvokeCallback(m_pFinalizeResult);
    m_pSink->EndFinalize(NULL);
    return hr;
}




//////////////////////
// CMarker class
// Holds information from IMFStreamSink::PlaceMarker
// 

CMarker::CMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType) : m_nRefCount(1), m_eMarkerType(eMarkerType)
{
    PropVariantInit(&m_varMarkerValue);
    PropVariantInit(&m_varContextValue);
}

CMarker::~CMarker()
{
    assert(m_nRefCount == 0);

    PropVariantClear(&m_varMarkerValue);
    PropVariantClear(&m_varContextValue);
}

/* static */
HRESULT CMarker::Create(
    MFSTREAMSINK_MARKER_TYPE eMarkerType,
    const PROPVARIANT* pvarMarkerValue,     // Can be NULL.
    const PROPVARIANT* pvarContextValue,    // Can be NULL.
    IMarker **ppMarker
)
{
    if (ppMarker == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    CMarker *pMarker = new CMarker(eMarkerType);

    if (pMarker == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    // Copy the marker data.
    if (SUCCEEDED(hr))
    {
        if (pvarMarkerValue)
        {
            hr = PropVariantCopy(&pMarker->m_varMarkerValue, pvarMarkerValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (pvarContextValue)
        {
            hr = PropVariantCopy(&pMarker->m_varContextValue, pvarContextValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppMarker = pMarker;
        (*ppMarker)->AddRef();
    }

    SAFE_RELEASE(pMarker);

    return hr;
}

// IUnknown methods.

ULONG CMarker::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG CMarker::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

HRESULT CMarker::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == __uuidof(IMarker))
    {
        *ppv = static_cast<IMarker*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

// IMarker methods
HRESULT CMarker::GetMarkerType(MFSTREAMSINK_MARKER_TYPE *pType)
{
    if (pType == NULL)
    {
        return E_POINTER;
    }

    *pType = m_eMarkerType;
    return S_OK;
}

HRESULT CMarker::GetMarkerValue(PROPVARIANT *pvar)
{
    if (pvar == NULL)
    {
        return E_POINTER;
    }
    return PropVariantCopy(pvar, &m_varMarkerValue);

}
HRESULT CMarker::GetContext(PROPVARIANT *pvar)
{
    if (pvar == NULL)
    {
        return E_POINTER;
    }
    return PropVariantCopy(pvar, &m_varContextValue);
}


//-------------------------------------------------------------------
// CreatePCMAudioType
//
// Creates a media type that describes an uncompressed PCM audio
// format.
//-------------------------------------------------------------------

HRESULT CreateRawVideoType(
    UINT32 frameRate,        // Frame Rate
    UINT32 width,
    UINT32 height,
    IMFMediaType **ppType     // Receives a pointer to the media type.
)
{
    HRESULT hr = S_OK;

    LONG    lStride = 0;
    UINT    cbImage = 0;

    IMFMediaType *pType = NULL;



    hr = MFCreateMediaType(&pType);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, width, height);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pType->SetUINT32(MF_MT_COMPRESSED, FALSE);
    if (FAILED(hr))
    {
        goto done;
    }

    lStride = 0;
    // Calculate the default stride value.
    hr = pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pType->SetUINT32(MF_MT_SAMPLE_SIZE, ((width * 32 + 31) & ~31) / 8 * height);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    if (FAILED(hr))
    {
        goto done;
    }

    // Frame rate
    hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, frameRate, 1);
    if (FAILED(hr))
    {
        goto done;
    }

    // Pixel aspect ratio
    hr = MFSetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    if (FAILED(hr))
    {
        goto done;
    }

    // Return the pointer to the caller.
    *ppType = pType;
    (*ppType)->AddRef();

done:
    SafeRelease(&pType);
    return hr;
}
//
//
//HRESULT CreatePCMAudioType(
//    UINT32 sampleRate,        // Samples per second
//    UINT32 bitsPerSample,     // Bits per sample
//    UINT32 cChannels,         // Number of channels
//    IMFMediaType **ppType     // Receives a pointer to the media type.
//    )
//{
//    HRESULT hr = S_OK;
//
//    IMFMediaType *pType = NULL;
//
//    // Calculate derived values.
//    UINT32 blockAlign = cChannels * (bitsPerSample / 8);
//    UINT32 bytesPerSecond = blockAlign * sampleRate;
//
//    // Create the empty media type.
//    hr = MFCreateMediaType(&pType);
//
//    // Set attributes on the type.
//    if (SUCCEEDED(hr))
//    {
//        hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
//    }
//
//    if (SUCCEEDED(hr))
//    {
//        hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
//    }
//
//    if (SUCCEEDED(hr))
//    {
//        hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, cChannels);
//    }
//
//    if (SUCCEEDED(hr))
//    {
//        hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
//    }
//
//    if (SUCCEEDED(hr))
//    {
//        hr = pType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, blockAlign);
//    }
//
//    if (SUCCEEDED(hr))
//    {
//        hr = pType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bytesPerSecond);
//    }
//
//    if (SUCCEEDED(hr))
//    {
//        hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
//    }
//
//    if (SUCCEEDED(hr))
//    {
//        hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
//    }
//
//    if (SUCCEEDED(hr))
//    {
//        // Return the type to the caller.
//        *ppType = pType;
//        (*ppType)->AddRef();
//    }
//
//    SAFE_RELEASE(pType);
//    return hr;
//}



//-------------------------------------------------------------------
// Name: ValidateVideoFormat
// Description: Validates a WAVEFORMATEX structure. 
//
// Just to keep the sample as simple as possible, we only accept 
// uncompressed PCM formats.
//-------------------------------------------------------------------

HRESULT ValidateVideoFormat(const WAVEFORMATEX *pWav, DWORD cbSize)
{
    if (pWav->wFormatTag != WAVE_FORMAT_PCM)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    if (pWav->nChannels != 1 && pWav->nChannels != 2)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    if (pWav->wBitsPerSample != 8 && pWav->wBitsPerSample != 16)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    if (pWav->cbSize != 0)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    // Make sure block alignment was calculated correctly.
    if (pWav->nBlockAlign != pWav->nChannels * (pWav->wBitsPerSample / 8))
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    // Check possible overflow...
    if (pWav->nSamplesPerSec  > (DWORD)(MAXDWORD / pWav->nBlockAlign))        // Is (nSamplesPerSec * nBlockAlign > MAXDWORD) ?
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    // Make sure average bytes per second was calculated correctly.
    if (pWav->nAvgBytesPerSec != pWav->nSamplesPerSec * pWav->nBlockAlign)
    {
        return MF_E_INVALIDMEDIATYPE;
    }

    // Everything checked out.
    return S_OK;
}


#pragma warning( pop )