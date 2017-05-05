#include "Configuration.h"

Configuration* Configuration::instance = nullptr;

Configuration::Configuration(const std::wstring& registryFolder):
  m_registryFolder(registryFolder),
  m_unmountAtExit(true)
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
  DWORD l_unmountAtExit = m_unmountAtExit;
  SetValue(hKey, L"UnmountAtExit", l_unmountAtExit);
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
    DWORD l_unmountAtExit;
    bool ret = GetValue(hKey, L"UnmountAtExit", l_unmountAtExit);
    if (ret)
      {
        m_unmountAtExit = l_unmountAtExit;
      }
      else
      {
        // save default configuration
        l_unmountAtExit = m_unmountAtExit;
        ret = SetValue(hKey, L"UnmountAtExit", l_unmountAtExit);
      }
    WINPORT(RegCloseKey)(hKey);
  }
}

bool Configuration::SetValue(HKEY folder, const std::wstring& field,
                                 const DWORD value) const
{
  if (!folder || field.empty()) return false;
  LONG res = WINPORT(RegSetValueEx)(folder, field.c_str(), 0, REG_DWORD,
                                    (BYTE *)&value, sizeof(value));
  return res == ERROR_SUCCESS;
}

bool Configuration::GetValue(HKEY folder, const std::wstring& field,
                                 DWORD& value) const
{
  if (!folder || field.empty()) return false;
  DWORD Type, size = sizeof(DWORD);
  LONG res = WINPORT(RegQueryValueEx)(folder, field.c_str(), 0, &Type,
                                      (BYTE*)&value, &size);
  return res == ERROR_SUCCESS;
}
