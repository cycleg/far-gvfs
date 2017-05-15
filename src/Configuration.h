#pragma once

#include "RegistryStorage.h"

/// 
/// @brief Класс для работы с конфигурацией плагина.

/// Конфигурационные параметры сохраняются в реестре far2l в ветке плагина
/// "Software/Far2/gvfspanel". В текущей реализации имеется один параметр:
/// * отключение известных подмонтированных ресурсов при выходе из far2l
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
    /// @param registryFolder Путь в реестре к папке плагина
    /// @return указатель на экземпляр Configuration
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
    /// @return указатель на экземпляр Configuration
    ///
    /// Может вернуть nullptr, если вызван до метода Instance(registryFolder).
    ///
    inline static Configuration* Instance() { return Configuration::instance; }
    ///
    /// Извлечь значение параметра "отключать ресурсы при выходе".
    ///
    /// @return Значение параметра "отключать ресурсы при выходе"
    ///
    inline bool unmountAtExit() const { return m_unmountAtExit; }
    ///
    /// Присвоить значение параметру "отключать ресурсы при выходе".
    ///
    /// @param u Новое значение
    /// @return Указатель на синглет
    ///
    inline Configuration* setUnmountAtExit(bool u)
    { m_unmountAtExit = u; return this; }

    ///
    /// Сохранить конфигурацию плагина в реестр.
    ///
    void save() const;

  private:
    static Configuration* instance; ///< экземпляр-синглет класса

    ///
    /// Конструктор.
    ///
    Configuration(const std::wstring& registryFolder);

    ///
    /// Загрузить конфигурацию плагина из реестра.
    ///
    void load();

    bool m_unmountAtExit; ///< значение параметра "отключать ресурсы при выходе"
};
