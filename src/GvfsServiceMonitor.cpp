#include <functional>
#include <iostream>
#include <string>
#include "GvfsServiceMonitor.h"

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

GvfsServiceMonitor GvfsServiceMonitor::m_instance;

GvfsServiceMonitor::GvfsServiceMonitor():
  m_monitor(g_volume_monitor_get())
{
  // подключаем к сигналам наши слоты через обертки в виде C-функций
  g_signal_connect(m_monitor, "mount-added",
                   G_CALLBACK(monitor_mount_added_wrapper), nullptr);
  g_signal_connect(m_monitor, "mount-removed",
                   G_CALLBACK(monitor_mount_removed_wrapper), nullptr);
  g_signal_connect(m_monitor, "mount-changed",
                   G_CALLBACK(monitor_mount_changed_wrapper), nullptr);
  g_signal_connect(m_monitor, "mount-pre-unmount",
                   G_CALLBACK(monitor_mount_pre_unmount_wrapper), nullptr);
}

GvfsServiceMonitor::~GvfsServiceMonitor()
{
  g_object_unref(m_monitor);
}

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
  std::cout << "GvfsServiceMonitor::onMountAdded() name: " << name << std::endl
            << "GvfsServiceMonitor::onMountAdded() path: " << path << std::endl
            << "GvfsServiceMonitor::onMountAdded() scheme: " << scheme << std::endl;
}

void GvfsServiceMonitor::onMountRemoved(GVolumeMonitor* monitor, GMount* mount)
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
  std::cout << "GvfsServiceMonitor::onMountRemoved() name: " << name << std::endl
            << "GvfsServiceMonitor::onMountRemoved() path: " << path << std::endl
            << "GvfsServiceMonitor::onMountRemoved() scheme: " << scheme << std::endl;
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
  std::cout << "GvfsServiceMonitor::onMountChanged() name: " << name << std::endl
            << "GvfsServiceMonitor::onMountChanged() path: " << path << std::endl
            << "GvfsServiceMonitor::onMountChanged() scheme: " << scheme << std::endl;
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
  std::cout << "GvfsServiceMonitor::onMountPreunmount name: " << name << std::endl
            << "GvfsServiceMonitor::onMountPreunmount path: " << path << std::endl
            << "GvfsServiceMonitor::onMountPreunmount scheme: " << scheme << std::endl;
}

void GvfsServiceMonitor::run()
{
  if ((m_mainLoop.operator->() != nullptr) && m_mainLoop->is_running())
    return;
  // запускаем главный цикл монитора в отдельном потоке
  m_thread = std::make_shared<std::thread>(std::bind(&GvfsServiceMonitor::loop,
                                                     this));
}

void GvfsServiceMonitor::quit()
{
  if ((m_mainLoop.operator->() == nullptr) ||
      ((m_mainLoop.operator->() != nullptr) && !m_mainLoop->is_running()))
    return;
  m_mainLoop->quit();
  m_thread->join();
}

void GvfsServiceMonitor::loop()
{
  Gio::init();
  Glib::RefPtr<Glib::MainContext> main_context = Glib::MainContext::create();
  // Чтобы контекст главного цикла Glib::MainLoop не отслеживал ничего,
  // кроме операций с ресурсами! Иначе блокируется пользовательский ввод Far.
  // Из руководства:
  // This will cause certain asynchronous operations (such as most GIO-based
  // I/O) which are started in this thread to run under context and deliver
  // their results to its main loop, rather than running under the global
  // default context in the main thread.
  g_main_context_push_thread_default(main_context->gobj());
  // на будущее запускаем главный цикл glib отдельным оператором
  m_mainLoop = Glib::MainLoop::create(false);
  m_mainLoop->run();
  // Из руководства:
  // In some cases however, you may want to schedule a single operation
  // in a non-default context, or temporarily use a non-default context
  // in the main thread. In that case, you can wrap the call to the
  // asynchronous operation inside a
  // g_main_context_push_thread_default() / g_main_context_pop_thread_default()
  // pair...
  // Второй вариант, видимо, наш случай. Без этого вызова Glib выдает
  // assert.
  g_main_context_pop_thread_default(main_context->gobj());
}
