#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include "Plugin.h"
#include "GvfsServiceMonitor.h"

#ifndef USE_GIO_MOUNTOPERATION_ONLY
namespace {

extern "C" void monitor_mount_added_wrapper(GVolumeMonitor* volume_monitor,
                                             GMount* mount)
{
  GvfsServiceMonitor::instance().onMountAdded(volume_monitor, mount);
}

extern "C" void monitor_mount_removed_wrapper(GVolumeMonitor* volume_monitor,
                                              GMount* mount)
{
  GvfsServiceMonitor::instance().onMountRemoved(volume_monitor, mount);
}

extern "C" void monitor_mount_changed_wrapper(GVolumeMonitor* volume_monitor,
                                              GMount* mount)
{
  GvfsServiceMonitor::instance().onMountChanged(volume_monitor, mount);
}

extern "C" void monitor_mount_pre_unmount_wrapper(GVolumeMonitor* volume_monitor,
                                                  GMount* mount)
{
  GvfsServiceMonitor::instance().onMountPreunmount(volume_monitor, mount);
}

} // anonymous namespace
#endif // USE_GIO_MOUNTOPERATION_ONLY

GvfsServiceMonitor GvfsServiceMonitor::m_instance;

GvfsServiceMonitor::GvfsServiceMonitor():
  m_quit(false)
{
}

GvfsServiceMonitor::~GvfsServiceMonitor()
{
  if ((m_mainLoop.operator->() != nullptr) && m_mainLoop->is_running())
    quit();
}

#ifdef USE_GIO_MOUNTOPERATION_ONLY
void GvfsServiceMonitor::onMountAdded(const Glib::RefPtr<Gio::Mount>& mount)
{
  std::string name, path, scheme;
  name = mount->get_name();
  Glib::RefPtr< const Gio::File > file = mount->get_root();
  path = file->get_path();
  scheme = file->get_uri_scheme();
//  Glib::object_unref(file);
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountAdded() name: " << name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountAdded() path: " << path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountAdded() scheme: " << scheme << std::endl;
#endif // NDEBUG
  JobPtr job(new Job());
  job->mount = true;
  m_jobs.put(job);
  m_jobs.notify_one();
}

void GvfsServiceMonitor::onMountRemoved(const Glib::RefPtr<Gio::Mount>& mount)
{
  JobPtr job(new Job());
  job->name = mount->get_name();
  Glib::RefPtr< const Gio::File > file = mount->get_root();
  job->path = file->get_path();
  job->scheme = file->get_uri_scheme();
//  Gio::File::object_unref(file);
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountRemoved() name: " << job->name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountRemoved() path: " << job->path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountRemoved() scheme: " << job->scheme << std::endl;
#endif // NDEBUG
  m_jobs.put(job);
  m_jobs.notify_one();
}

void GvfsServiceMonitor::onMountChanged(const Glib::RefPtr<Gio::Mount>& mount)
{
  std::string name, path, scheme;
  name = mount->get_name();
  Glib::RefPtr< const Gio::File > file = mount->get_root();
  path = file->get_path();
  scheme = file->get_uri_scheme();
//  Gio::File::object_unref(file);
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountChanged() name: " << name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountChanged() path: " << path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountChanged() scheme: " << scheme << std::endl;
#endif // NDEBUG
}

void GvfsServiceMonitor::onMountPreunmount(const Glib::RefPtr<Gio::Mount>& mount)
{
  std::string name, path, scheme;
  name = mount->get_name();
  Glib::RefPtr< const Gio::File > file = mount->get_root();
  path = file->get_path();
  scheme = file->get_uri_scheme();
//  Gio::File::object_unref(file);
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountPreunmount name: " << name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountPreunmount path: " << path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountPreunmount scheme: " << scheme << std::endl;
#endif // NDEBUG
}
#else // USE_GIO_MOUNTOPERATION_ONLY
void GvfsServiceMonitor::onMountAdded(GVolumeMonitor* monitor, GMount* mount)
{
  std::string name, path, scheme;
  char* buffer = nullptr;
  buffer = g_mount_get_name(mount);
  name = buffer;
  g_free(buffer);
  GFile* file = g_mount_get_root(mount);
  if (file)
  {
    buffer = g_file_get_path(file);
    path = buffer;
    g_free(buffer);
    buffer = g_file_get_uri_scheme(file);
    if (buffer)
    {
      scheme = buffer;
      g_free(buffer);
    }
    g_object_unref(file);
  }
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountAdded() name: " << name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountAdded() path: " << path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountAdded() scheme: " << scheme << std::endl;
#endif // NDEBUG
  JobPtr job(new Job());
  job->mount = true;
  m_jobs.put(job);
  m_jobs.notify_one();
}

void GvfsServiceMonitor::onMountRemoved(GVolumeMonitor* monitor, GMount* mount)
{
  char* buffer = nullptr;
  JobPtr job(new Job());
  buffer = g_mount_get_name(mount);
  job->name = buffer;
  g_free(buffer);
  GFile* file = g_mount_get_root(mount);
  if (file)
  {
    buffer = g_file_get_path(file);
    job->path = buffer;
    g_free(buffer);
    buffer = g_file_get_uri_scheme(file);
    if (buffer)
    {
      job->scheme = buffer;
      g_free(buffer);
    }
    g_object_unref(file);
  }
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountRemoved() name: " << job->name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountRemoved() path: " << job->path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountRemoved() scheme: " << job->scheme << std::endl;
#endif // NDEBUG
  m_jobs.put(job);
  m_jobs.notify_one();
}

void GvfsServiceMonitor::onMountChanged(GVolumeMonitor* monitor, GMount* mount)
{
  std::string name, path, scheme;
  char* buffer = nullptr;
  buffer = g_mount_get_name(mount);
  name = buffer;
  g_free(buffer);
  GFile* file = g_mount_get_root(mount);
  if (file)
  {
    buffer = g_file_get_path(file);
    path = buffer;
    g_free(buffer);
    buffer = g_file_get_uri_scheme(file);
    if (buffer)
    {
      scheme = buffer;
      g_free(buffer);
    }
    g_object_unref(file);
  }
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountChanged() name: " << name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountChanged() path: " << path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountChanged() scheme: " << scheme << std::endl;
#endif // NDEBUG
}

void GvfsServiceMonitor::onMountPreunmount(GVolumeMonitor* monitor,
                                           GMount* mount)
{
  std::string name, path, scheme;
  char* buffer = nullptr;
  buffer = g_mount_get_name(mount);
  name = buffer;
  g_free(buffer);
  GFile* file = g_mount_get_root(mount);
  if (file)
  {
    buffer = g_file_get_path(file);
    path = buffer;
    g_free(buffer);
    buffer = g_file_get_uri_scheme(file);
    if (buffer)
    {
      scheme = buffer;
      g_free(buffer);
    }
    g_object_unref(file);
  }
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountPreunmount name: " << name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountPreunmount path: " << path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountPreunmount scheme: " << scheme << std::endl;
#endif // NDEBUG
}
#endif // USE_GIO_MOUNTOPERATION_ONLY

void GvfsServiceMonitor::run()
{
  if ((m_mainLoop.operator->() != nullptr) && m_mainLoop->is_running())
    return;
  // запускаем главный цикл монитора в отдельном потоке
  m_thread = std::make_shared<std::thread>(std::bind(&GvfsServiceMonitor::loop,
                                                     this));
  // запускаем обработчик сигналов в отдельном потоке
  m_quit = false;
  m_worker = std::make_shared<std::thread>(std::bind(&GvfsServiceMonitor::worker,
                                                     this));
}

void GvfsServiceMonitor::quit()
{
  if ((m_mainLoop.operator->() == nullptr) ||
      ((m_mainLoop.operator->() != nullptr) && !m_mainLoop->is_running()))
    return;
  m_quit = true;
  m_mainLoop->quit();
  m_thread->join();
  m_worker->join();
}

void GvfsServiceMonitor::loop()
{
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::loop() run" << std::endl;
#endif // NDEBUG
#ifdef USE_GIO_MOUNTOPERATION_ONLY
  using namespace std::placeholders;
  // извлекаем указатель на Volume Monitor
  Glib::RefPtr<Gio::VolumeMonitor> monitor = Gio::VolumeMonitor::get();
  std::vector<sigc::connection> handlers;
  // подключаем к сигналам наши слоты
  handlers.push_back(monitor->signal_mount_added().connect(
    std::bind(&GvfsServiceMonitor::onMountAdded, this, _1)
  ));
  handlers.push_back(monitor->signal_mount_removed().connect(
    std::bind(&GvfsServiceMonitor::onMountRemoved, this, _1)
  ));
  handlers.push_back(monitor->signal_mount_changed().connect(
    std::bind(&GvfsServiceMonitor::onMountChanged, this, _1)
  ));
  handlers.push_back(monitor->signal_mount_pre_unmount().connect(
    std::bind(&GvfsServiceMonitor::onMountPreunmount, this, _1)
  ));
#else // USE_GIO_MOUNTOPERATION_ONLY
  // создаем volume monitor в контексте потока
  GVolumeMonitor* monitor = g_volume_monitor_get();
  std::vector<gulong> handlers;
  // подключаем к сигналам наши слоты через обертки в виде C-функций
  handlers.push_back(g_signal_connect(monitor, "mount-added",
                                      G_CALLBACK(monitor_mount_added_wrapper),
                                      nullptr));
  handlers.push_back(g_signal_connect(monitor, "mount-removed",
                                      G_CALLBACK(monitor_mount_removed_wrapper),
                                      nullptr));
  handlers.push_back(g_signal_connect(monitor, "mount-changed",
                                      G_CALLBACK(monitor_mount_changed_wrapper),
                                      nullptr));
  handlers.push_back(g_signal_connect(monitor, "mount-pre-unmount",
                                      G_CALLBACK(monitor_mount_pre_unmount_wrapper),
                                      nullptr));
#endif // USE_GIO_MOUNTOPERATION_ONLY
  m_mainLoop = Glib::MainLoop::create(false);
  m_mainLoop->run();
  // отключаем наши слоты от сигналов
  for (auto handler: handlers)
#ifdef USE_GIO_MOUNTOPERATION_ONLY
    handler.disconnect();
#else
    g_signal_handler_disconnect(monitor, handler);
#endif // USE_GIO_MOUNTOPERATION_ONLY
  handlers.clear();
  // больше volume monitor не нужен
#ifndef USE_GIO_MOUNTOPERATION_ONLY
//  Gio::VolumeMonitor::object_unref(monitor);
// #else
  g_object_unref(monitor);
  monitor = nullptr;
#endif // USE_GIO_MOUNTOPERATION_ONLY
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::loop() end" << std::endl;
#endif //
}

void GvfsServiceMonitor::worker()
{
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::worker() run" << std::endl;
#endif // NDEBUG
  while (!m_quit)
  {
    if (!m_jobs.wait_for(std::chrono::milliseconds(500)))
    {
      // есть новое задание
      JobPtr job(m_jobs.get());
      if (job->mount)
        Plugin::getInstance().onPointMounted();
        else Plugin::getInstance().onPointUnmounted(job->name, job->path,
                                                    job->scheme);
    }
  }
#ifndef NDEBUG
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::worker() end" << std::endl;
#endif // NDEBUG
}
