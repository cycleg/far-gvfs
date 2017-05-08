#pragma once

#include <string>
#include <vector>
#include <windows.h>

/// 
/// Класс с набором методов, общих для всех registry-хранилищ.

/// @author cycleg
///
class RegistryStorage
{
  public:
    RegistryStorage(const std::wstring& registryFolder):
      m_registryFolder(registryFolder) {}

  protected:
    ///
    /// Присваивание поля реестра типа "Binary".
    ///
    /// @param folder Дескриптор папки реестра с полем
    /// @param field Имя поля
    /// @param value Значение поля
    /// @return Признак успешности операции
    ///
    bool SetValue(HKEY folder, const std::wstring& field,
                  const std::vector<BYTE>& value) const;
    ///
    /// Присваивание поля реестра типа "String".
    ///
    /// @param folder Дескриптор папки реестра с полем
    /// @param field Имя поля
    /// @param value Значение поля
    /// @return Признак успешности операции
    ///
    bool SetValue(HKEY folder, const std::wstring& field,
                  const std::wstring& value) const;
    ///
    /// Присваивание поля реестра типа "Dword".
    ///
    /// @param folder Дескриптор папки реестра с полем
    /// @param field Имя поля
    /// @param value Значение поля
    /// @return Признак успешности операции
    ///
    bool SetValue(HKEY folder, const std::wstring& field,
                  const DWORD value) const;
    ///
    /// Извлечение значения поля реестра типа "Binary".
    ///
    /// @param folder Дескриптор папки реестра с полем
    /// @param field Имя поля
    /// @param value Буфер для значения поля
    /// @return Признак успешности операции
    ///
    bool GetValue(HKEY folder, const std::wstring& field,
                  std::vector<BYTE>& value) const;
    ///
    /// Извлечение значения поля реестра типа "String".
    ///
    /// @param folder Дескриптор папки реестра с полем
    /// @param field Имя поля
    /// @param value Буфер для значения поля
    /// @return Признак успешности операции
    ///
    bool GetValue(HKEY folder, const std::wstring& field,
                  std::wstring& value) const;
    ///
    /// Извлечение значения поля реестра типа "Dword".
    ///
    /// @param folder Дескриптор папки реестра с полем
    /// @param field Имя поля
    /// @param value Буфер для значения поля
    /// @return Признак успешности операции
    ///
    bool GetValue(HKEY folder, const std::wstring& field,
                  DWORD& value) const;

    std::wstring m_registryFolder; ///< полный путь к корневой папке хранилища
};
