#pragma once

#include <string>
#include "GvfsServiceException.h"

class MountPointStorage;

class MountPoint
{
  friend class MountPointStorage;

  public:
    enum class FileSystem {
        NoFs,
        DiskFs,
        Scp,
        Nfs,
        Samba,
        WebDav
    };

    MountPoint();
    MountPoint(const std::wstring& resPath, const std::wstring& user, const std::wstring& password);
    MountPoint(const MountPoint& other);

    MountPoint& operator=(const MountPoint& other);

    inline bool isMounted() const { return m_bMounted; }
    inline FileSystem getFsType() const { return m_type; }
    inline const std::wstring& getFsPath() const { return m_mountPointPath; }
    inline const std::wstring& getMountName() const { return m_shareName; }
    inline const std::wstring& getResPath() const { return m_resPath; }
    inline const std::wstring& getUser() const { return m_user; }
    inline const std::wstring& getPassword() const { return m_password; }

    inline void setResPath(const std::wstring& s) { this->m_resPath = s; }
    inline void setUser(const std::wstring& s) { this->m_user = s; }
    inline void setPassword(const std::wstring& s) { this->m_password = s; }

    bool mount() throw(GvfsServiceException);
    bool unmount() throw(GvfsServiceException);
    void mountCheck();

  private:
    bool m_bMounted;
    FileSystem m_type;
    std::wstring m_resPath;
    std::wstring m_user;
    std::wstring m_password;
    std::wstring m_mountPointPath;
    std::wstring m_shareName;
    std::wstring m_storageId;
};
