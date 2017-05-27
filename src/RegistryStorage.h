#pragma once

#include <string>
#include <vector>
#include <windows.h>

/// 
/// @brief Класс с набором методов, общих для всех registry-хранилищ.

/// @author cycleg
///
class RegistryStorage
{
  public:
    ///
    /// Конструктор.
    ///
    /// @param [in] registryFolder Путь к корневой папке хранилища в реестре.
    ///
    RegistryStorage(const std::wstring& registryFolder):
      m_registryFolder(registryFolder) {}

  protected:
    ///
    /// Присваивание поля реестра типа "Binary".
    ///
    /// @param [in] folder Дескриптор папки реестра с полем.
    /// @param [in] field Имя поля.
    /// @param [in] value Значение поля.
    /// @return Признак успешности операции.
    ///
    bool SetValue(HKEY folder, const std::wstring& field,
                  const std::vector<BYTE>& value) const;
    ///
    /// Присваивание поля реестра типа "String".
    ///
    /// @param [in] folder Дескриптор папки реестра с полем.
    /// @param [in] field Имя поля.
    /// @param [in] value Значение поля.
    /// @return Признак успешности операции.
    ///
    bool SetValue(HKEY folder, const std::wstring& field,
                  const std::wstring& value) const;
    ///
    /// Присваивание поля реестра типа "Dword".
    ///
    /// @param [in] folder Дескриптор папки реестра с полем.
    /// @param [in] field Имя поля.
    /// @param [in] value Значение поля.
    /// @return Признак успешности операции.
    ///
    bool SetValue(HKEY folder, const std::wstring& field,
                  const DWORD value) const;
    ///
    /// Извлечение значения поля реестра типа "Binary".
    ///
    /// @param [in] folder Дескриптор папки реестра с полем.
    /// @param [in] field Имя поля.
    /// @param [out] value Буфер для значения поля.
    /// @return Признак успешности операции.
    ///
    bool GetValue(HKEY folder, const std::wstring& field,
                  std::vector<BYTE>& value) const;
    ///
    /// Извлечение значения поля реестра типа "String".
    ///
    /// @param [in] folder Дескриптор папки реестра с полем.
    /// @param [in] field Имя поля.
    /// @param [out] value Буфер для значения поля.
    /// @return Признак успешности операции.
    ///
    bool GetValue(HKEY folder, const std::wstring& field,
                  std::wstring& value) const;
    ///
    /// Извлечение значения поля реестра типа "Dword".
    ///
    /// @param [in] folder Дескриптор папки реестра с полем.
    /// @param [in] field Имя поля.
    /// @param [out] value Буфер для значения поля.
    /// @return Признак успешности операции.
    ///
    bool GetValue(HKEY folder, const std::wstring& field,
                  DWORD& value) const;
    ///
    /// Извлечение-иначе-присваивание значения поля реестра.
    ///
    /// @param [in] folder Дескриптор папки реестра с полем.
    /// @param [in] field Имя поля.
    /// @param [out] value Буфер для значения поля.
    /// @param [in] default_ Значение поля по умолчанию.
    /// @return Признак успешности операции.
    ///
    /// Сначала поле пытаются прочитать. Если оно отсутствует, то его
    /// создают со значением по умолчанию. В параметре value возвращается
    /// записанное в реестр значение, если операция завершилась успешно.
    ///
    template <class ValueType> bool GetSetValue(HKEY folder,
                                                const std::wstring& field,
                                                ValueType& value,
                                                ValueType default_) const
    {
      ValueType l_value;
      bool ret = GetValue(folder, field, l_value);
      if (ret)
        {
          value = l_value;
        }
        else
        {
          // save default value
          l_value = default_;
          ret = SetValue(folder, field, l_value);
          if (ret) value = l_value;
        }
      return ret;
    }

    std::wstring m_registryFolder; ///< Полный путь к корневой папке хранилища.
};
