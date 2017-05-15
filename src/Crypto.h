#pragma once

#include <string>
#include <vector>
#include <openssl/evp.h>
#include <WinCompat.h>

/// 
/// @brief Encrypt/decrypt text, wrapper over OpenSSL

/// Use AES 256 CBC.
///
/// @author cycleg
///
class Crypto
{
  public:
    ///
    /// Конструктор.
    ///
    Crypto();
    ///
    /// Деструктор.
    ///
    ~Crypto();

    ///
    /// Генерация контекстов шифрования.
    ///
    /// @param [in] keydata Данные для ключевания.
    /// @return Признак успешности операции.
    ///
    /// Операция может завершиться неудачно, если длина сгенерированного по
    /// переданным данным ключа не равна 32 байтам.
    ///
    bool init(const std::wstring& keydata);

    ///
    /// Зашифровать данные в сгенерированном init() контексте.
    ///
    /// @param [in] plain Исходные данные.
    /// @param [out] cipher Шифрованные данные (результат операции).
    ///
    void encrypt(const std::vector<BYTE>& plain, std::vector<BYTE>& cipher);

    ///
    /// Расшифровать данные в сгенерированном init() контексте.
    ///
    /// @param [in] cipher Шифрованные данные.
    /// @param [out] plain Исходные данные (результат операции).
    ///
    void decrypt(const std::vector<BYTE>& cipher, std::vector<BYTE>& plain);

  private:
    EVP_CIPHER_CTX* m_encodeContext, ///< Контекст шифрации.
                  * m_decodeContext; ///< Контекст дешифрации.
};
