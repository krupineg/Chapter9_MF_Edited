#pragma once
#include "Common.h"
class CTopoBuilderBase
{
public:
    virtual HRESULT RenderURL(PCWSTR fileUrl, HWND videoHwnd, bool addNetwork) = 0;
    virtual  HRESULT RenderCamera(HWND videoHwnd, bool addNetwork) = 0;
    virtual IMFTopology* GetTopology(void) = 0;

    virtual  HRESULT ShutdownSource(void) = 0;
    virtual  HRESULT AfterSessionClose(IMFMediaSession * m_pSession) = 0;
    virtual   HRESULT Finish(IMFMediaSession * m_pSession) = 0;
};