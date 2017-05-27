#include "Configuration.h"

Configuration* Configuration::instance = nullptr;

Configuration::Configuration(const std::wstring& registryFolder):
  RegistryStorage(registryFolder),
  m_unmountAtExit(true)
#ifdef USE_SECRET_STORAGE
  , m_useSecretStorage(false)
#endif
{
}

Configuration* Configuration::Instance(const std::wstring& registryFolder)
{
  if (!Configuration::instance)
  {
    Configuration::instance = new Configuration(registryFolder);
    Configuration::instance->load();
  }
  return Configuration::instance;
}

void Configuration::save() const
{
	HKEY hKey = nullptr;
  LONG res = WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, m_registryFolder.c_str(),
                                   0, KEY_WRITE, &hKey);
  // ignore save errors
  if (res != ERROR_SUCCESS) return;
  SetValue(hKey, L"UnmountAtExit", m_unmountAtExit);
#ifdef USE_SECRET_STORAGE
  SetValue(hKey, L"UseSecretStorage", m_useSecretStorage);
#endif
  WINPORT(RegCloseKey)(hKey);
}

void Configuration::load()
{
	HKEY hKey = nullptr;
  LONG res = WINPORT(RegOpenKeyEx)(HKEY_CURRENT_USER, m_registryFolder.c_str(),
                                   0, KEY_READ | KEY_WRITE, &hKey);
  if (res != ERROR_SUCCESS)
  {
    // new configuration?
    DWORD disposition;
    res = WINPORT(RegCreateKeyEx)(HKEY_CURRENT_USER, m_registryFolder.c_str(),
                                  0, nullptr, 0, KEY_WRITE, nullptr, &hKey,
                                  &disposition);
    WINPORT(SetLastError)(res);
  }
  if (res == ERROR_SUCCESS)
  {
    // извлечь-иначе-сохранить значения по умолчанию
    DWORD l_bool;
    if (GetSetValue<DWORD>(hKey, L"UnmountAtExit", l_bool, m_unmountAtExit))
      m_unmountAtExit = l_bool;
#ifdef USE_SECRET_STORAGE
    if (GetSetValue<DWORD>(hKey, L"UseSecretStorage", l_bool, m_useSecretStorage))
      m_useSecretStorage = l_bool;
#endif
    WINPORT(RegCloseKey)(hKey);
  }
}
