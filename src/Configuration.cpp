#include "Configuration.h"

Configuration* Configuration::instance = nullptr;

Configuration::Configuration(const std::wstring& registryFolder):
  RegistryStorage(registryFolder),
  m_unmountAtExit(true),
  m_useSecretService(false)
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
  SetValue(hKey, L"UseSecretService", m_useSecretService);
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
    if (GetSetValue<DWORD>(hKey, L"UseSecretService", l_bool, m_useSecretService))
      m_useSecretService = l_bool;
    WINPORT(RegCloseKey)(hKey);
  }
}
