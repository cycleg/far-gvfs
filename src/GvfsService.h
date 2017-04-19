#pragma once

#include <memory>
#include <string>
#include <gtkmm.h>
#include "GvfsServiceException.h"

class GvfsService
{
public:
    GvfsService();

    bool mount(const std::string& resPath, const std::string &userName,
               const std::string &password) throw(GvfsServiceException);
    bool umount(const std::string& resPath) throw(GvfsServiceException);

    std::string m_mountName;
    std::string m_mountPath;

private:
    void mount_cb(Glib::RefPtr<Gio::AsyncResult>& result);
    bool unmount_cb(Glib::RefPtr<Gio::AsyncResult>& result);

    int m_mountCount;
    Glib::RefPtr<Gio::File> file;
    Glib::RefPtr<Glib::MainLoop> main_loop;
    std::shared_ptr<GvfsServiceException> m_exception;
};
