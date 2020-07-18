#pragma once

#include "RegistryStorage.h"

/// 
/// @brief Класс для работы с конфигурацией плагина.

/// Конфигурационные параметры сохраняются в реестре far2l в ветке плагина
/// "Software/Far2/gvfspanel". В текущей реализации имеются три параметра:
/// * отключение известных подмонтированных ресурсов при выходе из far2l
///   (да/нет);
/// * отключение только тех известных подмонтированных ресурсов при выходе из
///   far2l, которые были смонтированы в текущем сеансе (да/нет);
/// * ииспользовать для хранения паролей системное безопасное хранилище
///   (да/нет).
///
/// Класс реализован как синглетон для доступа к параметрам из любой точки
/// плагина.
///
/// @author cycleg
///
class Configuration: public RegistryStorage
{
  public:
    ///
    /// Доступ к экземпляру-синглету.
    ///
    /// @param [in] registryFolder Путь в реестре к папке плагина.
    /// @return Указатель на экземпляр Configuration.
    ///
    /// Путь к папке сохраняется в свойстве класса m_registryFolder,
    /// унаследованном от RegistryStorage.
    ///
    /// Этот метод в плагине должен вызываться до обращения к любыи другим
    /// методам класса, включая статические.
    ///
    static Configuration* Instance(const std::wstring& registryFolder);

    ///
    /// Доступ к экземпляру-синглету.
    ///
    /// @return Указатель на экземпляр Configuration.
    ///
    /// Может вернуть nullptr, если вызван до метода Instance(registryFolder).
    ///
    inline static Configuration* Instance() { return Configuration::instance; }
    ///
    /// Извлечь значение параметра "отключать ресурсы при выходе".
    ///
    /// @return Значение параметра "отключать ресурсы при выходе".
    ///
    inline bool unmountAtExit() const { return m_unmountAtExit; }
    ///
    /// Присвоить значение параметру "отключать ресурсы при выходе".
    ///
    /// @param [in] v Новое значение.
    /// @return Указатель на синглет.
    ///
    inline Configuration* setUnmountAtExit(bool v)
    { m_unmountAtExit = v; return this; }
    ///
    /// Извлечь значение параметра "отключать при выходе только ресурсы,
    /// смонтированные в данном сеансе".
    ///
    /// @return Значение параметра "отключать при выходе только ресурсы,
    ///         смонтированные в данном сеансе".
    ///
    inline bool unmountThisSessionOnly() const
    { return m_unmountThisSessionOnly; }
    ///
    /// Присвоить значение параметру "отключать при выходе только ресурсы,
    /// смонтированные в данном сеансе".
    ///
    /// @param [in] v Новое значение.
    /// @return Указатель на синглет.
    ///
    inline Configuration* setUnmountThisSessionOnly(bool v)
    { m_unmountThisSessionOnly = v; return this; }
#ifdef USE_SECRET_STORAGE
    ///
    /// Извлечь значение параметра "использовать безопасное хранилище".
    ///
    /// @return Значение параметра "использовать безопасное хранилище".
    ///
    inline bool useSecretStorage() const { return m_useSecretStorage; }
    ///
    /// Присвоить значение параметру "использовать безопасное хранилище".
    ///
    /// @param [in] v Новое значение.
    /// @return Указатель на синглет.
    ///
    inline Configuration* setUseSecretStorage(bool v)
    { m_useSecretStorage = v; return this; }
#endif

    ///
    /// Сохранить конфигурацию плагина в реестр.
    ///
    void save() const;

  private:
    static Configuration* instance; ///< Экземпляр-синглет класса.

    ///
    /// Конструктор.
    ///
    /// @param [in] registryFolder Путь в реестре к папке плагина.
    ///
    Configuration(const std::wstring& registryFolder);

    ///
    /// Загрузить конфигурацию плагина из реестра.
    ///
    void load();

    bool m_unmountAtExit; ///< Значение параметра "отключать ресурсы при
                          ///< выходе".
    bool m_unmountThisSessionOnly; ///< Значение параметра "отключать при
                                   ///< выходе только ресурсы, смонтированные
                                   ///< в данном сеансе".
#ifdef USE_SECRET_STORAGE
    bool m_useSecretStorage; ///< Значение параметра "использовать безопасное
                             ///< хранилище".
#endif
};
