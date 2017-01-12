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
#include "AbstractVideoSink.h"
#include "HttpOutputStreamActivate.h"
#include "CTopoBuilderBase.h"
class CTopoBuilder : public CTopoBuilderBase
{
    public:
        CTopoBuilder()
        { 
            m_addRenderers = false; m_addNetworkSink = false;
        };
        ~CTopoBuilder(void) { 
            ShutdownSource(); 
        };
        void ToFile(bool value) {
            toFile = value;
        }
        HRESULT RenderURL(PCWSTR fileUrl, HWND videoHwnd, bool addNetwork);
        HRESULT RenderCamera(HWND videoHwnd, bool addNetwork);
        IMFTopology* GetTopology(void) { return m_pTopology; }

        HRESULT ShutdownSource(void);
        HRESULT AfterSessionClose(IMFMediaSession * m_pSession);
        HRESULT Finish(IMFMediaSession * m_pSession);
        const LPCWSTR fileName =  L"C:\\Users\\Public\\Encoded2.mp4";
        
    private:
        CComPtr<IMFTopology>                  m_pTopology;     // the topology itself
        CComPtr<IMFMediaSource>               m_pSource;       // the MF source
        CComPtr<IMFVideoDisplayControl>       m_pVideoDisplay; // pointer to the mixer
        HWND                                    m_videoHwnd;     // the target window
        CComPtr<IMFActivate> m_pNetworkSinkActivate;
        CComPtr<IMFSinkWriter>  sink_writer;
        CComPtr<IMFMediaSink>  m_MediaSink;
        CComPtr<IMFByteStream> byte_stream;  
        CComPtr<IMFTransform> transform;
        CComPtr<SampleGrabberCB> sampleGrabber;
        DWORD m_nextNetworkSinkStreamIndex;
        bool toFile;
        bool m_addRenderers;
        bool m_addNetworkSink;
        HRESULT CreateMediaSource();
        HRESULT CreateMediaSource(PCWSTR sURL);
        HRESULT CreateNetworkSink(DWORD requestPort);
        HRESULT CreateFileSink(IMFMediaType * in_mf_media_type_video);
		HRESULT CreateTeeMp4Twig(
            IMFPresentationDescriptor* pPresDescriptor, IMFTransform * transform, IMFStreamDescriptor* pStreamDescriptor,
            IMFTopologyNode* pRendererNode, IMFTopologyNode** ppTeeNode, IMFMediaSink * m_Sink);
        HRESULT CreateTopology(void);
        IMFTopologyNode * AddEncoderIfNeed(IMFTopology * topology, IMFTransform * transform, IMFStreamDescriptor * pStreamDescriptor, IMFTopologyNode * output_node);
        HRESULT AddBranchToPartialTopology(
            CComPtr<IMFPresentationDescriptor> pPresDescriptor, 
            DWORD iStream);

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

        HRESULT CreateTeeNetworkTwig(IMFStreamDescriptor* pStreamDescriptor, 
            IMFTopologyNode* pRendererNode, IMFTopologyNode** ppTeeNode);

};

