#include <cstring>
#include <codecvt>
#include <locale>
#include <windows.h>
#include <uuid/uuid.h>
#include "MountPointStorage.h"

#define UUID_TEXT_SIZE (sizeof(uuid_t) * 2 + 5)

const wchar_t* MountPointStorage::StoragePath = L"Resources";

MountPointStorage::MountPointStorage(const std::wstring& registryFolder):
  m_registryFolder(registryFolder)
{
  m_registryFolder.append(WGOOD_SLASH);
  m_registryFolder.append(StoragePath);
}

MountPoint MountPointStorage::PointFactory()
{
  MountPoint point;
  GenerateId(point.m_storageId);
  return point;
}

void MountPointStorage::LoadAll(std::map<std::wstring, MountPoint>& storage) const
{
	HKEY hKey = nullptr;
	LONG res;
  DWORD index = 0;
  storage.clear();
  if (WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, m_registryFolder.c_str(), 0,
                            KEY_ENUMERATE_SUB_KEYS | KEY_READ, &hKey) != ERROR_SUCCESS)
  {
    // no storage
		return;
	}
  do
  {
    wchar_t subKey[MAX_PATH];
    FILETIME tTime;
    DWORD subKeySize = MAX_PATH * sizeof(wchar_t);
    std::memset(subKey, 0, subKeySize);
    res = WINPORT(RegEnumKeyEx)(hKey, index, subKey, &subKeySize, 0, nullptr,
                                nullptr, &tTime);
    WINPORT(SetLastError)(res);
    if (res == ERROR_SUCCESS)
    {
      MountPoint point;
      point.m_storageId = subKey;
      // TODO: load error
      if (Load(point))
        storage.insert(std::pair<std::wstring, MountPoint>(point.getResPath(), point));
    }
    index++;
  } while (res == ERROR_SUCCESS);
  WINPORT(RegCloseKey)(hKey);
}

bool MountPointStorage::Save(const MountPoint& point) const
{
	HKEY hKey = nullptr;
  LONG res;
  std::wstring key = m_registryFolder;
  key.append(WGOOD_SLASH);
  key.append(point.m_storageId);
  res = WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, key.c_str(), 0,
                              KEY_ENUMERATE_SUB_KEYS | KEY_READ, &hKey);
  if (res != ERROR_SUCCESS)
  {
    // new record?
    DWORD disposition;
    res = WINPORT(RegCreateKeyEx)(HKEY_CURRENT_USER, key.c_str(), 0, nullptr, 0,
                                  KEY_WRITE, nullptr, &hKey, &disposition);
    WINPORT(SetLastError)(res);
  }
  if (res != ERROR_SUCCESS) return false;
  std::vector<BYTE> l_password;
  Encrypt(point.m_password, l_password);
  bool ret = SetRegKey(hKey, L"Path", point.m_resPath) &&
             SetRegKey(hKey, L"User", point.m_user)  &&
             SetRegKey(hKey, L"Password", l_password);
  WINPORT(RegCloseKey)(hKey);
  return ret;
}

void MountPointStorage::Delete(const MountPoint& point) const
{
	HKEY hKey = nullptr;
  std::wstring key = m_registryFolder;
  key.append(WGOOD_SLASH);
  key.append(point.m_storageId);
  LONG res = WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, key.c_str(), 0,
                                   KEY_WRITE, &hKey);
  // no key - nothing to delete
  if (res != ERROR_SUCCESS) return;
  WINPORT(RegCloseKey)(hKey);
  res = WINPORT(RegDeleteKey)(HKEY_CURRENT_USER, key.c_str());
  WINPORT(SetLastError)(res);
}

void MountPointStorage::GenerateId(std::wstring& id)
{
  uuid_t uuid;
  char* out = new char[UUID_TEXT_SIZE];
  uuid_generate(uuid);
  uuid_unparse(uuid, out);
  id = std::wstring_convert<std::codecvt_utf8<wchar_t> >().from_bytes(out);
  delete[] out;
}

void MountPointStorage::Encrypt(const std::wstring& in, std::vector<BYTE>& out)
{
  out.clear();
  for (std::wstring::size_type i = 0; i < in.size(); i++)
  {
     wchar_t symbol = in[i];
     BYTE wideSymbol[sizeof(wchar_t) / sizeof(BYTE)];
     int j = sizeof(wchar_t) / sizeof(BYTE);
     memcpy(wideSymbol, &symbol, j);
     while (j > 0)
     {
       out.push_back(wideSymbol[sizeof(wchar_t) / sizeof(BYTE) - j]);
       j--;
     }
  }
}

void MountPointStorage::Decrypt(const std::vector<BYTE>& in, std::wstring& out)
{
  out.clear();
  unsigned int i = 0;
  wchar_t symbol = 0;
  for (const auto& byte : in)
  {
    if (i < sizeof(wchar_t) / sizeof(BYTE) - 1)
      {
        symbol += byte << (8 * i);
        i++;
      }
      else
      {
        out.push_back(symbol);
        i = 0;
        symbol = byte;
      }
  }
}

bool MountPointStorage::Load(MountPoint& point) const
{
	HKEY hKey = nullptr;
	LONG res;
  std::wstring key = m_registryFolder;
  key.append(WGOOD_SLASH);
  key.append(point.m_storageId);
  res = WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, key.c_str(), 0, KEY_READ, &hKey);
  if (res != ERROR_SUCCESS) return false;
  std::vector<BYTE> l_password;
  bool ret = GetRegKey(hKey, L"Path", point.m_resPath) &&
             GetRegKey(hKey, L"User", point.m_user)  &&
             GetRegKey(hKey, L"Password", l_password);
  WINPORT(RegCloseKey)(hKey);
  if (ret) Decrypt(l_password, point.m_password);
  return ret;
}

bool MountPointStorage::SetRegKey(HKEY folder, const std::wstring& field,
                                  const std::vector<BYTE>& value) const
{
  if (!folder || field.empty()) return false;
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_BINARY,
                                    &value[0], value.size());
  return res == ERROR_SUCCESS;
}

bool MountPointStorage::SetRegKey(HKEY folder, const std::wstring& field,
                                  const std::wstring& value) const
{
  if (!folder || field.empty()) return false;
  std::string buf = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(value);
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_SZ_MB,
                                    (const BYTE*)buf.c_str(), buf.size() + 1);
  return res == ERROR_SUCCESS;
}

bool MountPointStorage::SetRegKey(HKEY folder, const std::wstring& field,
                                  const DWORD value) const
{
  if (!folder || field.empty()) return false;
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_DWORD,
                                    (BYTE *)&value, sizeof(value));
  return res == ERROR_SUCCESS;
}

bool MountPointStorage::GetRegKey(HKEY folder, const std::wstring& field,
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

bool MountPointStorage::GetRegKey(HKEY folder, const std::wstring& field,
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
      value = std::wstring_convert<std::codecvt_utf8<wchar_t> >().from_bytes(std::string(buf));
    }
  } while (res == ERROR_MORE_DATA); 
  delete[] buf;
  return res == ERROR_SUCCESS;
}

bool MountPointStorage::GetRegKey(HKEY folder, const std::wstring& field,
                                  DWORD& value) const
{
  if (!folder || field.empty()) return false;
  DWORD Type, size = sizeof(DWORD);
  LONG res = WINPORT(RegQueryValueEx)(folder, field.c_str(), 0, &Type,
                                      (BYTE*)&value, &size);
  return res == ERROR_SUCCESS;
}