#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include "Plugin.h"
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
  m_monitor(g_volume_monitor_get()),
  m_quit(false)
{
}

GvfsServiceMonitor::~GvfsServiceMonitor()
{
  if ((m_mainLoop.operator->() != nullptr) && m_mainLoop->is_running()) quit();
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
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountAdded() name: " << name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountAdded() path: " << path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountAdded() scheme: " << scheme << std::endl;
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
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountRemoved() name: " << job->name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountRemoved() path: " << job->path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountRemoved() scheme: " << job->scheme << std::endl;
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
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountChanged() name: " << name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountChanged() path: " << path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountChanged() scheme: " << scheme << std::endl;
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
  std::cout << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountPreunmount name: " << name << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountPreunmount path: " << path << std::endl
            << std::hex << std::this_thread::get_id() << std::dec
            << " GvfsServiceMonitor::onMountPreunmount scheme: " << scheme << std::endl;
}

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
std::cout << std::hex << std::this_thread::get_id() << std::dec
          << " GvfsServiceMonitor::loop() run" << std::endl;
  std::vector<gulong> handlers;
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
  // подключаем к сигналам наши слоты через обертки в виде C-функций
  handlers.push_back(g_signal_connect(m_monitor, "mount-added",
                                      G_CALLBACK(monitor_mount_added_wrapper),
                                      nullptr));
  handlers.push_back(g_signal_connect(m_monitor, "mount-removed",
                                      G_CALLBACK(monitor_mount_removed_wrapper),
                                      nullptr));
  handlers.push_back(g_signal_connect(m_monitor, "mount-changed",
                                      G_CALLBACK(monitor_mount_changed_wrapper),
                                      nullptr));
  handlers.push_back(g_signal_connect(m_monitor, "mount-pre-unmount",
                                      G_CALLBACK(monitor_mount_pre_unmount_wrapper),
                                      nullptr));
  // на будущее запускаем главный цикл glib отдельным оператором
  m_mainLoop = Glib::MainLoop::create(false);
  m_mainLoop->run();
  // отключаем наши слоты от сигналов
  for (auto handler: handlers)
    g_signal_handler_disconnect(m_monitor, handler);
  handlers.clear();
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
std::cout << std::hex << std::this_thread::get_id() << std::dec
          << " GvfsServiceMonitor::loop() end" << std::endl;
}

void GvfsServiceMonitor::worker()
{
std::cout << std::hex << std::this_thread::get_id() << std::dec
          << " GvfsServiceMonitor::worker() run" << std::endl;
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
std::cout << std::hex << std::this_thread::get_id() << std::dec
          << " GvfsServiceMonitor::worker() end" << std::endl;
}
