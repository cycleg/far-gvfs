#pragma once

#include <memory>
#include <thread>
#include <gtkmm.h>
#include "JobUnitQueue.h"

/// 
/// @brief Обертка вокруг GVolumeMonitor, монитор точек монтирования GVFS

/// Реализован как синглетон.
///
/// @author cycleg
///
class GvfsServiceMonitor
{
  public:
    ///
    /// Доступ к экземпляру-синглету.
    ///
    static inline GvfsServiceMonitor& instance()
    {
      return GvfsServiceMonitor::m_instance;
    }

    ///
    /// Деструктор.
    ///
    ~GvfsServiceMonitor();

    ///
    /// Слот обработки сигнала "mount added" в процедуре монтирования.
    ///
    /// @param [in] monitor Объект-монитор glib.
    /// @param [in] mount Обрабатываемая точка монтирования glib.
    ///
    /// Сигнал подается по завершению операции монтирования.
    ///
    void onMountAdded(GVolumeMonitor* monitor, GMount* mount);

    ///
    /// Слот обработки сигнала "mount removed" в процедуре отмонтирования.
    ///
    /// @param [in] monitor Объект-монитор glib.
    /// @param [in] mount Обрабатываемая точка монтирования glib.
    ///
    /// Сигнал подается по завершению операции отмонтирования. Если приемник
    /// владеет объектом mount, то он должен освободить его.
    ///
    void onMountRemoved(GVolumeMonitor* monitor, GMount* mount);

    ///
    /// Слот обработки сигнала "mount changed".
    ///
    /// @param [in] monitor Объект-монитор glib.
    /// @param [in] mount Обрабатываемая точка монтирования glib.
    ///
    /// Сигнал подается, когда состояние mount меняется.
    ///
    void onMountChanged(GVolumeMonitor* monitor, GMount* mount);

    ///
    /// Слот обработки сигнала "mount pre-unmount" в процедуре отмонтирования.
    ///
    /// @param [in] monitor Объект-монитор glib.
    /// @param [in] mount Обрабатываемая точка монтирования glib.
    ///
    /// Сигнал извещает о скором начале операции отмонтирования. Если приемник
    /// сигнала может удерживать точку монтирования открытым файлом, он должен
    /// закрыть его.
    ///
    void onMountPreunmount(GVolumeMonitor* monitor, GMount* mount);

    ///
    /// Запуск главного цикла монитора, работает в отдельном потоке.
    ///
    /// Перед запуском цикла подключаются обработчики сигналов.
    ///
    void run();

    ///
    /// Остановка главного цикла монитора.
    ///
    /// После остановки цикла отключаются обработчики сигналов.
    ///
    void quit();

  private:
    ///
    /// @brief Задание на обработку подсоединения или отключения ресурса.
    ///
    struct Job
    {
      bool mount; ///< Ресурс подсоединен или отключен.
      std::string name, ///< Наименование ресурса.
                  path, ///< URL ресурса.
                  scheme; ///< Протокол (схема) из URL.

      ///
      /// Конструктор по умолчанию.
      ///
      /// По умолчанию задание об отключившемся ресурсе.
      ///
      Job(): mount(false) {}
    };
    typedef std::shared_ptr<Job> JobPtr; ///< "Умный указатель" на Job.
    typedef JobUnitQueue< JobPtr > JobQueue; ///< Специализация шаблона для
                                             ///< очереди из "умных указателей"
                                             ///< на Job.

    static GvfsServiceMonitor m_instance; ///< Экземпляр-синглет класса.

    ///
    /// Конструктор.
    ///
    GvfsServiceMonitor();

    ///
    /// Главный цикл приема и обработки сигналов GVFS.
    /// 
    /// Принимает и обрабатывает все сигналы, которые передаются из glib,
    /// включая сигналы, порождаемые в экземплярах класса GvfsService.
    /// Cигналы от GVolumeMonitor преобразуются в экземпляры Job, которые
    /// помещаются в очередь #m_jobs. Она, в свою очередь, обрабатывается в
    /// отдельном потоке #m_worker, в котором запущен метод worker().
    ///
    void loop();

    ///
    /// Цикл асинхронной обработки заданий из #m_jobs.
    ///
    void worker();

    GVolumeMonitor* m_monitor; ///< Монитор точек монтирования GVFS.
    Glib::RefPtr<Glib::MainLoop> m_mainLoop; ///< Главный цикл glib.
    std::shared_ptr<std::thread> m_thread; ///< Поток, в котором работает
                                           ///< главный цикл, принимающий
                                           ///< сигналы от gtkmm.
    std::shared_ptr<std::thread> m_worker; ///< Поток обработки заданий из
                                           ///< #m_jobs.
    bool m_quit; ///< Флаг остановки для потока m_worker.
    JobQueue m_jobs; ///< Очередь заданий для m_worker.
};
