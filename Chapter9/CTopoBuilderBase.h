#pragma once
#include "Common.h"
class CTopoBuilderBase
{
public:
    virtual HRESULT RenderURL(PCWSTR fileUrl, HWND videoHwnd, bool addNetwork) = 0;
    virtual  HRESULT RenderCamera(HWND videoHwnd, bool addNetwork) = 0;
    virtual IMFTopology* GetTopology(void) = 0;

    virtual  HRESULT ShutdownSource(void) = 0;
    virtual  HRESULT AfterSessionClose() = 0;
    virtual   HRESULT Finish() = 0;


  
    HRESULT SetSession(IMFMediaSession * session) {
        m_pSession = session;
        return S_OK;
    }
protected:
    CComPtr<IMFMediaSession> m_pSession;
};