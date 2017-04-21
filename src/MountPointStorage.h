#pragma once

#include <string>
#include <map>
#include <vector>
#include <WinCompat.h>
#include "MountPoint.h"

class MountPointStorage
{
  public:
    MountPointStorage(const std::wstring& registryFolder);

    static MountPoint Factory();

    void LoadAll(std::map<std::wstring, MountPoint>& storage) const;
    bool Save(const MountPoint& point) const;
    void Delete(const std::wstring& id) const;

  private:
    static const wchar_t* StoragePath;

    static void GenerateId(std::wstring& id);
    static void Encrypt(std::wstring& in, std::vector<BYTE>& out);
    static void Decrypt(std::vector<BYTE>& in, std::wstring& out);

    bool Load(MountPoint& point) const;

    bool SetRegKey(HKEY folder, std::wstring& field, std::vector<BYTE>& value) const;
    bool SetRegKey(HKEY folder, std::wstring& field, std::wstring& value) const;
    bool SetRegKey(HKEY folder, std::wstring& field, DWORD value) const;
    bool GetRegKey(HKEY folder, std::wstring& field, std::vector<BYTE>& value) const;
    bool GetRegKey(HKEY folder, std::wstring& field, std::wstring& value) const;
    bool GetRegKey(HKEY folder, std::wstring& field, DWORD& value) const;

    std::wstring m_registryFolder;
};
