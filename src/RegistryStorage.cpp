#include <utils.h>
#include "RegistryStorage.h"

bool RegistryStorage::SetValue(HKEY folder, const std::wstring& field,
                                 const std::vector<BYTE>& value) const
{
  if (!folder || field.empty()) return false;
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_BINARY,
                                    &value[0], value.size());
  return res == ERROR_SUCCESS;
}

bool RegistryStorage::SetValue(HKEY folder, const std::wstring& field,
                                 const std::wstring& value) const
{
  if (!folder || field.empty()) return false;
  std::string buf(StrWide2MB(value));
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_SZ_MB,
                                    (const BYTE*)buf.c_str(), buf.size() + 1);
  return res == ERROR_SUCCESS;
}

bool RegistryStorage::SetValue(HKEY folder, const std::wstring& field,
                                 const DWORD value) const
{
  if (!folder || field.empty()) return false;
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_DWORD,
                                    (BYTE *)&value, sizeof(value));
  return res == ERROR_SUCCESS;
}

bool RegistryStorage::GetValue(HKEY folder, const std::wstring& field,
                                 std::vector<BYTE>& value) const
{
  value.clear();
  if (!folder || field.empty()) return false;
  BYTE* buf = new BYTE[MAX_PATH];
  DWORD Type,
        sz = MAX_PATH;
  LONG res;
  do
  {
    memset(buf, 0, sz);
    res = WINPORT(RegQueryValueEx)(folder, field.c_str(), 0, &Type, buf, &sz);
    if (res == ERROR_MORE_DATA)
    {
      // buffer too small; resize it and repeat query
      delete[] buf;
      // TODO: out of memory
      buf = new BYTE[sz];
    }
    if (res  == ERROR_SUCCESS)
    {
      for (DWORD i = 0; i < sz; i++) value.push_back(buf[i]);
    }
  } while (res == ERROR_MORE_DATA);
  delete[] buf;
  return res == ERROR_SUCCESS;
}

bool RegistryStorage::GetValue(HKEY folder, const std::wstring& field,
                                 std::wstring& value) const
{
  value.clear();
  if (!folder || field.empty()) return false;
  char* buf = new char[MAX_PATH];
  DWORD Type,
        sz = MAX_PATH;
  LONG res;
  do
  {
    memset(buf, 0, sz);
    res = WINPORT(RegQueryValueEx)(folder, field.c_str(), 0, &Type, (LPBYTE)buf, &sz);
    if (res == ERROR_MORE_DATA)
    {
      // buffer too small; resize it and repeat query
      delete[] buf;
      // TODO: out of memory
      buf = new char[sz];
    }
    if (res == ERROR_SUCCESS)
    {
      MB2Wide(buf, value);
    }
  } while (res == ERROR_MORE_DATA); 
  delete[] buf;
  return res == ERROR_SUCCESS;
}

bool RegistryStorage::GetValue(HKEY folder, const std::wstring& field,
                                 DWORD& value) const
{
  if (!folder || field.empty()) return false;
  DWORD Type, size = sizeof(DWORD);
  LONG res = WINPORT(RegQueryValueEx)(folder, field.c_str(), 0, &Type,
                                      (BYTE*)&value, &size);
  return res == ERROR_SUCCESS;
}
