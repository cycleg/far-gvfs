#include <openssl/aes.h>
#include <utils.h>
#include "Crypto.h"

Crypto::Crypto():
  m_encodeContext(nullptr),
  m_decodeContext(nullptr)
{
  m_encodeContext = EVP_CIPHER_CTX_new();
  m_decodeContext = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_init(m_encodeContext);
  EVP_CIPHER_CTX_init(m_decodeContext);
}

Crypto::~Crypto()
{
  EVP_CIPHER_CTX_free(m_encodeContext);
  EVP_CIPHER_CTX_free(m_decodeContext);
}

bool Crypto::init(const std::wstring& keydata)
{
  std::string l_keydata = StrWide2MB(keydata);
  int i, nrounds = 5;
  BYTE key[32], iv[32];
 
  /*
   * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
   * nrounds is the number of times the we hash the material. More rounds are more secure but
   * slower. Salt not used.
   */
  i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), nullptr,
                     (const unsigned char*)l_keydata.c_str(), l_keydata.size(),
                     nrounds, key, iv);
  if (i != 32) return false;
 
  EVP_CIPHER_CTX_cleanup(m_encodeContext);
  EVP_CIPHER_CTX_cleanup(m_decodeContext);
  EVP_CIPHER_CTX_init(m_encodeContext);
  EVP_CIPHER_CTX_init(m_decodeContext);
  EVP_EncryptInit_ex(m_encodeContext, EVP_aes_256_cbc(), nullptr, key, iv);
  EVP_DecryptInit_ex(m_decodeContext, EVP_aes_256_cbc(), nullptr, key, iv);
 
  return true;
}

void Crypto::encrypt(const std::vector<BYTE>& plain, std::vector<BYTE>& cipher)
{
  /* max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes */
  int c_len = plain.size() + AES_BLOCK_SIZE, f_len = 0;
  BYTE* ciphertext = new BYTE[c_len];
 
  /* allows reusing of 'e' for multiple encryption cycles */
  EVP_EncryptInit_ex(m_encodeContext, nullptr, nullptr, nullptr, nullptr);
 
  /* update ciphertext, c_len is filled with the length of ciphertext
     generated, *len is the size of plaintext in bytes */
  EVP_EncryptUpdate(m_encodeContext, ciphertext, &c_len, plain.data(),
                    plain.size());
 
  /* update ciphertext with the final remaining bytes */
  EVP_EncryptFinal_ex(m_encodeContext, ciphertext + c_len, &f_len);

  cipher.clear();
  for (int i = 0; i < c_len + f_len; i++) cipher.push_back(ciphertext[i]);
  delete[] ciphertext;
}

void Crypto::decrypt(const std::vector<BYTE>& cipher, std::vector<BYTE>& plain)
{
  /* plaintext will always be equal to or lesser than length of ciphertext*/
  int p_len = cipher.size(), f_len = 0;
  BYTE* plaintext = new BYTE[p_len];
 
  EVP_DecryptInit_ex(m_decodeContext, nullptr, nullptr, nullptr, nullptr);
  EVP_DecryptUpdate(m_decodeContext, plaintext, &p_len, cipher.data(),
                    cipher.size());
  EVP_DecryptFinal_ex(m_decodeContext, plaintext + p_len, &f_len);
 
  plain.clear();
  for (int i = 0; i < p_len + f_len; i++) plain.push_back(plaintext[i]);
  delete[] plaintext;
}
