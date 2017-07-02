#include <cstring>
#include <windows.h>
#include <utils.h>
#include <uuid/uuid.h>

#ifdef USE_OPENSSL
#include "Crypto.h"
#endif

#ifdef USE_SECRET_STORAGE
#include "Configuration.h"
#include "SecretServiceStorage.h"
#endif

#include "MountPointStorage.h"

#define UUID_TEXT_SIZE (sizeof(uuid_t) * 2 + 5)

const wchar_t* MountPointStorage::StoragePath = L"Resources";
const wchar_t* MountPointStorage::StorageVersionKey = L"Version";
const DWORD MountPointStorage::StorageVersion = 5;

MountPointStorage::MountPointStorage(const std::wstring& registryFolder):
  RegistryStorage(registryFolder),
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
          storage.insert(std::pair<std::wstring, MountPoint>(point.getUrl(),
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
  DWORD l_askPassword = point.m_askPassword;
  bool ret = true;
#ifdef USE_SECRET_STORAGE
  if (Configuration::Instance()->useSecretStorage())
    {
      SecretServiceStorage storage;
      ret = ret &&
            storage.SavePassword(point.m_storageId, point.m_password);
      // если используется безопасное хранилище, вместо пароля в реестр
      // записывается пустая строка
    }
    else
#endif
    {
#ifdef USE_OPENSSL
      Encrypt(point.m_storageId, point.m_password, l_password);
#else
      Encrypt(point.m_password, l_password);
#endif
    }
  // save always in current storage version
  ret = ret &&
        SetValue(hKey, L"URL", point.m_url) &&
        SetValue(hKey, L"User", point.m_user) &&
        SetValue(hKey, L"Password", l_password) &&
        SetValue(hKey, L"AskPassword", l_askPassword);
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
#ifdef USE_SECRET_STORAGE
  // удаляем всегда, во избежание
  SecretServiceStorage storage;
  storage.RemovePassword(point.m_storageId);
#endif
}

void MountPointStorage::GenerateId(std::wstring& id)
{
  uuid_t uuid;
  char* out = new char[UUID_TEXT_SIZE];
  uuid_generate(uuid);
  uuid_unparse(uuid, out);
  MB2Wide(out, id);
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

#ifdef USE_OPENSSL
void MountPointStorage::Encrypt(const std::wstring& keydata,
                                const std::wstring& in,
                                std::vector<BYTE>& out)
{
  std::vector<BYTE> plain;
  std::string buf(StrWide2MB(in));
  Crypto crypto;
  crypto.init(keydata);
  for (const std::string::value_type ch : buf) plain.push_back((BYTE)ch);
  out.clear();
  crypto.encrypt(plain, out);
}
#endif

void MountPointStorage::Decrypt(const std::vector<BYTE>& in, std::wstring& out) const
{
  out.clear();
  switch (m_version)
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
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

#ifdef USE_OPENSSL
void MountPointStorage::Decrypt(const std::wstring& keydata,
                                const std::vector<BYTE>& in,
                                std::wstring& out) const
{
  std::vector<BYTE> plain;
  std::string buf;
  Crypto crypto;
  crypto.init(keydata);
  out.clear();
  switch (m_version)
  {
    case 3:
    case 4:
    case 5:
      crypto.decrypt(in, plain);
      for (const BYTE ch : plain) buf.push_back(ch);
      StrMB2Wide(buf, out);
      break;
    default:
      break;
  }
}
#endif

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
  std::wstring l_url, l_user;
  std::vector<BYTE> l_password;
  DWORD l_askPassword;
  bool ret = GetValue(hKey, L"User", l_user)  &&
             GetValue(hKey, L"Password", l_password);
  switch (m_version)
  {
    case 1:
      ret = ret &&
            GetValue(hKey, L"Path", l_url);
      if (ret)
      {
        // change record only on success
        point.m_url = l_url;
        point.m_user = l_user;
        Decrypt(l_password, point.m_password);
        point.m_askPassword = false; // для наглядности
      }
      break;
    case 2:
      ret = ret &&
            GetValue(hKey, L"Path", l_url) &&
            GetValue(hKey, L"AskPassword", l_askPassword);
      if (ret)
      {
        point.m_url = l_url;
        point.m_user = l_user;
        Decrypt(l_password, point.m_password);
        point.m_askPassword = (l_askPassword == 1);
      }
    case 3:
      ret = ret &&
            GetValue(hKey, L"Path", l_url) &&
            GetValue(hKey, L"AskPassword", l_askPassword);
      if (ret)
      {
        point.m_url = l_url;
        point.m_user = l_user;
#ifdef USE_OPENSSL
        Decrypt(point.m_storageId, l_password, point.m_password);
#else
        Decrypt(l_password, point.m_password);
#endif
        point.m_askPassword = (l_askPassword == 1);
      }
      break;
    case 4:
      ret = ret &&
            GetValue(hKey, L"Path", l_url) &&
            GetValue(hKey, L"AskPassword", l_askPassword);
      if (ret)
      {
        point.m_url = l_url;
        point.m_user = l_user;
#ifdef USE_SECRET_STORAGE
        if (Configuration::Instance()->useSecretStorage())
          {
            SecretServiceStorage storage;
            // Здесь делаем ссылочную целостность слабой: если пароль не
            // удалось извлечь из стороннего хранилища, это не означает
            // порчу всей записи. Пусть пользователь введет пароль заново.
            storage.LoadPassword(point.m_storageId, point.m_password);
          }
          else
#endif
          {
#ifdef USE_OPENSSL
            Decrypt(point.m_storageId, l_password, point.m_password);
#else
            Decrypt(l_password, point.m_password);
#endif
          }
        point.m_askPassword = (l_askPassword == 1);
      }
      break;
    case 5:
      ret = ret &&
            GetValue(hKey, L"URL", l_url) &&
            GetValue(hKey, L"AskPassword", l_askPassword);
      if (ret)
      {
        point.m_url = l_url;
        point.m_user = l_user;
#ifdef USE_SECRET_STORAGE
        if (Configuration::Instance()->useSecretStorage())
          {
            SecretServiceStorage storage;
            storage.LoadPassword(point.m_storageId, point.m_password);
          }
          else
#endif
          {
#ifdef USE_OPENSSL
            Decrypt(point.m_storageId, l_password, point.m_password);
#else
            Decrypt(l_password, point.m_password);
#endif
          }
        point.m_askPassword = (l_askPassword == 1);
      }
      break;
    default:
      break;
  }
  WINPORT(RegCloseKey)(hKey);
  return ret;
}
