#pragma once

#include <string>
#include <vector>
#include <openssl/evp.h>
#include <WinCompat.h>

/// 
/// Encrypt/decrypt text wrapper over OpenSSL.

/// Use AES 256 CBC.
///
class Crypto
{
  public:
    Crypto();
    ~Crypto();

    bool init(const std::wstring& keydata);

    void encrypt(const std::vector<BYTE>& plain, std::vector<BYTE>& cipher);
    void decrypt(const std::vector<BYTE>& cipher, std::vector<BYTE>& plain);

  private:
    EVP_CIPHER_CTX m_encodeContext,
                   m_decodeContext;
};
