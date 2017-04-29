#include <cstring>
#include <codecvt>
#include <locale>
#include <windows.h>
#include <uuid/uuid.h>
#include "MountPointStorage.h"

#define UUID_TEXT_SIZE (sizeof(uuid_t) * 2 + 5)

const wchar_t* MountPointStorage::StoragePath = L"Resources";
const wchar_t* MountPointStorage::StorageVersionKey = L"Version";
const DWORD MountPointStorage::StorageVersion = 1;

MountPointStorage::MountPointStorage(const std::wstring& registryFolder):
  m_registryFolder(registryFolder),
  m_version(0)
{
  m_registryFolder.append(WGOOD_SLASH);
  m_registryFolder.append(StoragePath);
  std::wstring key = m_registryFolder;
	HKEY hKey = nullptr;
  bool newStorage = false;
  LONG res = WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, m_registryFolder.c_str(),
                                   0, KEY_READ | KEY_WRITE, &hKey);
  if (res != ERROR_SUCCESS)
    {
      // new storage?
      DWORD disposition;
      res = WINPORT(RegCreateKeyEx)(HKEY_CURRENT_USER, m_registryFolder.c_str(),
                                    0, nullptr, 0, KEY_WRITE, nullptr, &hKey,
                                    &disposition);
      WINPORT(SetLastError)(res);
      newStorage = true;
    }
    else
    {
      if (!GetValue(hKey, StorageVersionKey, m_version)) m_version = 0;
    }
  if (res == ERROR_SUCCESS)
  {
    if (!m_version)
    {
      // set storage version: no version in existing storage - storage v1,
      // else - new storage
      m_version = newStorage ? StorageVersion : 1;
      SetValue(hKey, StorageVersionKey, m_version);
    }
    WINPORT(RegCloseKey)(hKey);
  }
}

MountPoint MountPointStorage::PointFactory()
{
  MountPoint point;
  GenerateId(point.m_storageId);
  return point;
}

void MountPointStorage::LoadAll(std::map<std::wstring, MountPoint>& storage)
{
  // не удалось создать хранилище на диске
  if (!valid()) return;
	HKEY hKey = nullptr;
	LONG res;
  DWORD index = 0;
  storage.clear();
  if (WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, m_registryFolder.c_str(), 0,
                            KEY_ENUMERATE_SUB_KEYS | KEY_READ, &hKey) != ERROR_SUCCESS)
  {
    // хранилище не доступно
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
      // load always converse record to current storage version
      // TODO: load error
      if (Load(point))
          storage.insert(std::pair<std::wstring, MountPoint>(point.getResPath(),
                         point));
    }
    index++;
  } while (res == ERROR_SUCCESS);
  WINPORT(RegCloseKey)(hKey);
  if (m_version < StorageVersion)
  {
    // storage conversion
    // чтобы не попасть в бесконечную рекурсию в Save()
    m_version = StorageVersion;
    for (const auto& mountPt : storage)
    {
      Delete(mountPt.second);
      // TODO: save error
      Save(mountPt.second);
    }
    res = WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, m_registryFolder.c_str(),
                                     0, KEY_WRITE, &hKey);
    if (res == ERROR_SUCCESS)
      {
        SetValue(hKey, StorageVersionKey, m_version);
        WINPORT(RegCloseKey)(hKey);
      }
      else
      {
        // TODO: save error
      }
  }
}

bool MountPointStorage::Save(const MountPoint& point)
{
  // не удалось создать хранилище на диске
  if (!valid()) return false;
	HKEY hKey = nullptr;
  LONG res;
  if (m_version < StorageVersion)
  {
    // LoadAll() не вызывали, придется конвертировать хранилище здесь
    std::map<std::wstring, MountPoint> buffer;
    LoadAll(buffer);
  }
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
  // save always in current storage version
  bool ret = SetValue(hKey, L"Path", point.m_resPath) &&
             SetValue(hKey, L"User", point.m_user)  &&
             SetValue(hKey, L"Password", l_password);
  WINPORT(RegCloseKey)(hKey);
  return ret;
}

void MountPointStorage::Delete(const MountPoint& point) const
{
  // no valid storage - nothing to delete
  if (!valid()) return;
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

void MountPointStorage::Decrypt(const std::vector<BYTE>& in, std::wstring& out) const
{
  out.clear();
  switch (m_version)
  {
    case 1:
      {
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
      break;
    default:
      break;
  }
}

bool MountPointStorage::Load(MountPoint& point) const
{
  // не удалось создать хранилище на диске
  if (!m_version) return false;
	HKEY hKey = nullptr;
	LONG res;
  std::wstring key = m_registryFolder;
  key.append(WGOOD_SLASH);
  key.append(point.m_storageId);
  res = WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, key.c_str(), 0, KEY_READ, &hKey);
  if (res != ERROR_SUCCESS) return false;
  std::vector<BYTE> l_password;
  bool ret = true;
  switch (m_version)
  {
    case 1:
      ret = GetValue(hKey, L"Path", point.m_resPath) &&
            GetValue(hKey, L"User", point.m_user)  &&
            GetValue(hKey, L"Password", l_password);
      if (ret) Decrypt(l_password, point.m_password);
      break;
    default:
      break;
  }
  WINPORT(RegCloseKey)(hKey);
  // TODO: converse to current storage version
  return ret;
}

bool MountPointStorage::SetValue(HKEY folder, const std::wstring& field,
                                  const std::vector<BYTE>& value) const
{
  if (!folder || field.empty()) return false;
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_BINARY,
                                    &value[0], value.size());
  return res == ERROR_SUCCESS;
}

bool MountPointStorage::SetValue(HKEY folder, const std::wstring& field,
                                  const std::wstring& value) const
{
  if (!folder || field.empty()) return false;
  std::string buf = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(value);
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_SZ_MB,
                                    (const BYTE*)buf.c_str(), buf.size() + 1);
  return res == ERROR_SUCCESS;
}

bool MountPointStorage::SetValue(HKEY folder, const std::wstring& field,
                                  const DWORD value) const
{
  if (!folder || field.empty()) return false;
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_DWORD,
                                    (BYTE *)&value, sizeof(value));
  return res == ERROR_SUCCESS;
}

bool MountPointStorage::GetValue(HKEY folder, const std::wstring& field,
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

bool MountPointStorage::GetValue(HKEY folder, const std::wstring& field,
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

bool MountPointStorage::GetValue(HKEY folder, const std::wstring& field,
                                  DWORD& value) const
{
  if (!folder || field.empty()) return false;
  DWORD Type, size = sizeof(DWORD);
  LONG res = WINPORT(RegQueryValueEx)(folder, field.c_str(), 0, &Type,
                                      (BYTE*)&value, &size);
  return res == ERROR_SUCCESS;
}
