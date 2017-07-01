#include <string>
#include <utils.h> // far2l/utils
#include "GvfsService.h"
#include "MountPoint.h"

MountPoint::MountPoint(const MountPoint& other)
{
    m_proto = other.m_proto;
    m_url = other.m_url;
    m_user = other.m_user;
    m_password = other.m_password;
    m_mountPointPath = other.m_mountPointPath;
    m_shareName = other.m_shareName;
    m_storageId = other.m_storageId;
    m_askPassword = other.m_askPassword;
}

MountPoint& MountPoint::operator=(const MountPoint& other)
{
    m_proto = other.m_proto;
    m_url = other.m_url;
    m_user = other.m_user;
    m_password = other.m_password;
    m_mountPointPath = other.m_mountPointPath;
    m_shareName = other.m_shareName;
    m_storageId = other.m_storageId;
    m_askPassword = other.m_askPassword;
    return *this;
}

MountPoint::MountPoint():
    m_proto(EProtocol::Unknown),
    m_askPassword(false)
{
}

MountPoint::EProtocol MountPoint::SchemeToProto(const std::string& scheme)
{
    MountPoint::EProtocol ret = EProtocol::Unknown;
    if (scheme == "file") ret = EProtocol::File;
    else if (scheme == "ftp") ret = EProtocol::Ftp;
    else if (scheme == "http") ret = EProtocol::Http;
    else if (scheme == "smb") ret = EProtocol::Samba;
    else if (scheme == "sftp") ret = EProtocol::Sftp;
    return ret;
}

bool MountPoint::mount(GvfsService* service) throw(GvfsServiceException)
{
    std::string url(StrWide2MB(m_url));
    std::string userName(StrWide2MB(m_user));
    std::string password(StrWide2MB(m_password));

    // пароль спрашивают перед монтированием, зачищаем его
    if (m_askPassword) m_password.clear();
    if (isMounted()) return true;
    if (url.empty()) return false;

    bool success = service->mount(url, userName, password);
    if (success)
    {
        m_proto = MountPoint::SchemeToProto(service->getMountScheme());
        StrMB2Wide(service->getMountPath(), m_mountPointPath);
        StrMB2Wide(service->getMountName(), m_shareName);
    }
    return success;
}

bool MountPoint::unmount(GvfsService* service) throw(GvfsServiceException)
{
    if (!isMounted()) return true;

    std::string url(StrWide2MB(this->m_url));
    if (url.empty()) return false;

    bool success = false;
    try
    {
        success = service->umount(url);
    }
    catch (const GvfsServiceException& e)
    {
        // exception equal to unmount
        m_shareName.clear();
        m_mountPointPath.clear();
        m_proto = EProtocol::Unknown;
        throw; // escalate error
    }
    if (success)
    {
        m_shareName.clear();
        m_mountPointPath.clear();
        m_proto = EProtocol::Unknown;
    }
    return success;
}

void MountPoint::mountCheck(GvfsService* service)
{
    std::string url(StrWide2MB(m_url));
    if (url.empty())
    {
        m_mountPointPath.clear();
        m_shareName.clear();
        m_proto = EProtocol::Unknown;
        return;
    }
    if (service->mounted(url))
        {
            m_proto = MountPoint::SchemeToProto(service->getMountScheme());
            StrMB2Wide(service->getMountPath(), m_mountPointPath);
            StrMB2Wide(service->getMountName(), m_shareName);
        }
        else
        {
            m_shareName.clear();
            m_mountPointPath.clear();
            m_proto = EProtocol::Unknown;
        }
}
