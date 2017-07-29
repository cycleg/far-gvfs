#pragma once

#include <memory>
#include <thread>
#include <vector>
#include <gtkmm.h>

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
    static GvfsServiceMonitor m_instance; ///< Экземпляр-синглет класса.

    ///
    /// Конструктор.
    ///
    GvfsServiceMonitor();

    ///
    /// Главный цикл монитора.
    /// 
    void loop();

    GVolumeMonitor* m_monitor; ///< Монитор точек монтирования GVFS.
    Glib::RefPtr<Glib::MainLoop> m_mainLoop; ///< Главный цикл glib.
    std::shared_ptr<std::thread> m_thread; ///< Поток, в котором работает
                                           ///< главный цикл.
    std::vector<gulong> m_handlers; ///< Идентификаторы установленных
                                    ///< обработчиков сигналов.
};
