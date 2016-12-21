#pragma once

#include "Common.h"

// Media Foundation headers
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <evr.h>
#include <Wmcontainer.h>
#include <InitGuid.h>

#include <new>

#include "HttpOutputStreamActivate.h"

class CTopoBuilder
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
    private:
        CComQIPtr<IMFTopology>                  m_pTopology;     // the topology itself
        CComQIPtr<IMFMediaSource>               m_pSource;       // the MF source
        CComQIPtr<IMFVideoDisplayControl>       m_pVideoDisplay; // pointer to the mixer
        HWND                                    m_videoHwnd;     // the target window

        CComPtr<IMFActivate> m_pNetworkSinkActivate;
        CComPtr<IMFMediaSink> m_Sink;
        DWORD m_nextNetworkSinkStreamIndex;
        bool toFile;
        bool m_addRenderers;
        bool m_addNetworkSink;
        HRESULT CreateMediaSource();
        HRESULT CreateMediaSource(PCWSTR sURL);
        HRESULT CreateNetworkSink(DWORD requestPort);
        HRESULT CreateFileSink(PCWSTR filePath, IMFMediaType * out_mf_media_type);
		HRESULT CreateTeeMp4Twig(
			IMFPresentationDescriptor* pPresDescriptor, 
			IMFStreamDescriptor* pStreamDescriptor,
			IMFTopologyNode* pRendererNode,
			IMFTopologyNode** ppTeeNode);
        HRESULT CreateTopology(void);

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

