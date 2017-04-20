#include <string>
#include <locale>
#include <codecvt>
#include "GvfsService.h"
#include "MountPoint.h"

MountPoint::MountPoint():
    m_bMounted(false),
    m_type(FileSystem::NoFs)
{
}

MountPoint::MountPoint(const std::wstring &resPath, const std::wstring &user, const std::wstring &password) :
    m_bMounted(false),
    m_type(FileSystem::NoFs),
    m_resPath(resPath),
    m_user(user),
    m_password(password)
{
}

bool MountPoint::mount() throw(GvfsServiceException)
{
    if (m_bMounted) return true;

    std::string resPath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(this->m_resPath);
    std::string userName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(this->m_user);
    std::string password = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(this->m_password);

    if (resPath.empty()) return false;

    GvfsService service;
    bool success = service.mount(resPath, userName, password);
    if (success)
    {
        m_bMounted = true;
        m_shareName = std::wstring_convert<std::codecvt_utf8<wchar_t> >()
                      .from_bytes(service.getMountName());
        m_mountPointPath = std::wstring_convert<std::codecvt_utf8<wchar_t> >()
                           .from_bytes(service.getMountPath());
    }
    return success;
}

bool MountPoint::unmount() throw(GvfsServiceException)
{
    if (!m_bMounted) return true;

    std::string resPath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(this->m_resPath);
    if (resPath.empty()) return false;

    GvfsService service;
    bool success = service.umount(resPath);
    if (success)
    {
        m_mountPointPath.clear();
        m_shareName.clear();
        m_bMounted = false;
    }
    return success;
}
