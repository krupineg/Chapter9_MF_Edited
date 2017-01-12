#pragma once
#include "Common.h"

// Media Foundation headers
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <evr.h>
#include <Wmcontainer.h>
#include <InitGuid.h>
#include "SampleGrabberCB.h"
#include <new>
#include "CTopoBuilderBase.h"
#include "AbstractVideoSink.h"
#include "HttpOutputStreamActivate.h"
#include "RendererVideoSink.h"
#include "MediaMP4VideoSink.h"
#include "SampleGrabberVideoSink.h"

class TobiiTopoBuilder : public CTopoBuilderBase
{
public:
    void AddSink(AbstractVideoSink * sink);
    IMFTopology* GetTopology(void) { return m_pTopology; }
    HRESULT AfterSessionClose(IMFMediaSession * m_pSession);
    HRESULT Finish(IMFMediaSession * m_pSession);
    HRESULT RenderURL(PCWSTR fileUrl, HWND videoHwnd, bool addNetwork){
        return RenderCamera(videoHwnd, addNetwork);
    }

    HRESULT TobiiTopoBuilder::RenderCamera(HWND videoHwnd, bool addNetwork)
    {
        HRESULT hr = S_OK;

        if (m_videoHwnd != NULL) {
           // AbstractVideoSink * sink = new (std::nothrow) RendererVideoSink(m_videoHwnd);
           // sinks.push_back(sink);
        }
        if (addNetwork) {
            AbstractVideoSink * sink = new (std::nothrow) SampleGrabberVideoSink(L"C:\\Users\\Public\\Encoded-sink-writer.mp4");
            sinks.push_back(sink);
            AbstractVideoSink * sink2 = new (std::nothrow) SampleGrabberVideoSink(L"C:\\Users\\Public\\Encoded-media-sink.mp4");
            sinks.push_back(sink2);
        }
        
        // The topology can have either a rendering sink (when videoHwnd is not NULL), a 
        // network sink, or both.
        if (videoHwnd == NULL && !addNetwork)
        {
            hr = E_INVALIDARG;
            throw;
        }

        // first create the media source for the file/stream passed in.  Fail and fall out if
        // the media source creation fails (e.g. if the file format is not recognized)
        hr = CreateMediaSource();
        THROW_ON_FAIL(hr);

        // add a network sink if one was requested
        if (addNetwork)
        {
            //hr = CreateNetworkSink(8080);
            THROW_ON_FAIL(hr);
        }

        // create the actual topology
        hr = CreateTopology();

        return hr;
    }
private:
    std::vector<AbstractVideoSink*> sinks;
    CComPtr<IMFTopology>                  m_pTopology;     // the topology itself
    CComPtr<IMFMediaSource>               m_pSource;       // the MF source
    CComPtr<IMFVideoDisplayControl>       m_pVideoDisplay; // pointer to the mixer
    HWND                                    m_videoHwnd;     // the target window
    HRESULT ShutdownSource();
    HRESULT CreateMediaSource();
    HRESULT CreateTopology(void);
    HRESULT AddBranchToPartialTopology(
        CComPtr<IMFPresentationDescriptor> pPresDescriptor,
        DWORD iStream);
    IMFTopologyNode* ConnectOutputs(IMFMediaType * in_type, GUID output_type, GUID out_subtype);
    HRESULT CreateSourceStreamNode(
        CComPtr<IMFPresentationDescriptor> pPresDescriptor,
        CComPtr<IMFStreamDescriptor> pStreamDescriptor,
        CComPtr<IMFTopologyNode> &ppNode);

    HRESULT CreateOutputNode(
        CComPtr<IMFPresentationDescriptor> pPresDescriptor,
        CComPtr<IMFStreamDescriptor> pSourceSD,
        HWND hwndVideo,
        IMFTopologyNode* pSourceNode,
        IMFTopologyNode** ppOutputNode);

};