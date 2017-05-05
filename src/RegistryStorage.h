#pragma once

#include <string>
#include <vector>
#include <windows.h>

class RegistryStorage
{
  public:
    RegistryStorage(const std::wstring& registryFolder):
      m_registryFolder(registryFolder) {}

  protected:
    bool SetValue(HKEY folder, const std::wstring& field,
                  const std::vector<BYTE>& value) const;
    bool SetValue(HKEY folder, const std::wstring& field,
                  const std::wstring& value) const;
    bool SetValue(HKEY folder, const std::wstring& field,
                  const DWORD value) const;
    bool GetValue(HKEY folder, const std::wstring& field,
                  std::vector<BYTE>& value) const;
    bool GetValue(HKEY folder, const std::wstring& field,
                  std::wstring& value) const;
    bool GetValue(HKEY folder, const std::wstring& field,
                  DWORD& value) const;

    std::wstring m_registryFolder;
};
