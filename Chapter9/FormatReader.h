#pragma once

#include <Strsafe.h>
#include <combaseapi.h>
#include <string>




struct IMFMediaType;
struct IMFAttributes;

/// Class for parsing info from IMFMediaType into the local MediaType
class FormatReader
{
public:

    static FormatReader& getInstance();

    void Read(LPCWSTR header, IMFAttributes *pType);
private:
    FormatReader(void);
    ~FormatReader(void);
    FormatReader(const FormatReader&);
    FormatReader& operator=(const FormatReader&);
    HRESULT LogAttributeValueByIndexNew(IMFAttributes *pAttr, DWORD index);
    HRESULT GetGUIDNameNew(const GUID& guid, std::wstring &nameGUID);
    void LogUINT32AsUINT64New(GUID nameGuid, const PROPVARIANT& var, UINT32 &uHigh, UINT32 &uLow);
    HRESULT SpecialCaseAttributeValueNew(GUID guid, const PROPVARIANT& var);
    static LPCWSTR GetGUIDNameConstNew(const GUID& guid);

};
