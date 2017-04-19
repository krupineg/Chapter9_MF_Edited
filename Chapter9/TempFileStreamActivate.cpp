#include "TempFileStreamActivate.h"

HRESULT TempFileStreamActivate::ActivateObject(REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;
    CComPtr<IMFByteStream> pByteStream;

    do
    {
        hr = MFCreateFile(MF_ACCESSMODE_READWRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NOBUFFERING, L"C:\\Users\\Public\\test.asf", &pByteStream);
        BREAK_ON_FAIL(hr);

        hr = pByteStream->QueryInterface(riid, ppv);
    } while (false);

    return hr;
}


//////////////////////////////////////////////////////////////////////////////////////////
//
//  IUnknown interface implementation
//
/////////////////////////////////////////////////////////////////////////////////////////
ULONG TempFileStreamActivate::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG TempFileStreamActivate::Release()
{
    ULONG refCount = InterlockedDecrement(&m_cRef);
    if (refCount == 0)
    {
        delete this;
    }

    return refCount;
}

HRESULT TempFileStreamActivate::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = S_OK;

    if (ppv == NULL)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFActivate*>(this));
    }
    else if (riid == IID_IMFActivate)
    {
        *ppv = static_cast<IMFActivate*>(this);
    }
    else if (riid == IID_IMFAttributes)
    {
        *ppv = static_cast<IMFAttributes*>(this);
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