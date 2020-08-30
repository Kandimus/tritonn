//=================================================================================================
//===
//=== hash.cpp
//===
//=== Copyright (c) 2019 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Описание типовых функций и утилит для работ с хешами SHA-1
//===
//=================================================================================================

#include "string.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include "stringex.h"
#include "def.h"
#include "hash.h"

using std::string;


//-------------------------------------------------------------------------------------------------
// Вычисление SHA-1 строки
UDINT GetStrHash(const string &str, USINT *buff)
{
	return GetBufHash(str.c_str(), (UDINT)str.size(), buff);
}


//
UDINT GetBufHash(const void *data, UDINT size, USINT *buff)
{
	UDINT       result = -1;
	EVP_MD_CTX *context = EVP_MD_CTX_create();

	if(!context) return result;

	if(EVP_DigestInit_ex(context, EVP_sha1(), NULL))
	{
		if(EVP_DigestUpdate(context, data, size))
		{
			unsigned int lengthOfHash = 0;

			if(EVP_DigestFinal_ex(context, buff, &lengthOfHash))
			{
				result = 0;
			}
		}
	}

	EVP_MD_CTX_destroy(context);

	return result;
}


UDINT CmpHash(const USINT *buff1, const USINT *buff2)
{
	for(UDINT ii = 0; ii < MAX_HASH_SIZE; ++ii)
	{
		if(buff1[ii] != buff2[ii]) return 0;
	}
	return 1;
}


int EncryptEAS(const string &text, const unsigned char key[16]/* 128- битный ключ */, const unsigned char iv[16], string &result)
{
	EVP_CIPHER_CTX *context    = EVP_CIPHER_CTX_new();
	unsigned char  *plaintext  = nullptr;
	unsigned char  *ciphertext = nullptr;
	int             plaintext_len;
	int             ciphertext_len;
	int             len;

	// Вычисляем длину ближайшего числа кратного 128
	plaintext_len = ((text.size() % 128) == 0) ? text.size() : ((text.size() / 128) + 1) * 128;

	// Создаем массивы
	result     = "";
	plaintext  = new unsigned char[plaintext_len + 1];
	ciphertext = new unsigned char[plaintext_len + 1];
	memset(plaintext , 0, plaintext_len + 1);
	memset(ciphertext, 0, plaintext_len + 1);
	memcpy(plaintext , text.c_str(), text.size());

	// Инициализируем алгоритм шифрования
	if(1 != EVP_EncryptInit_ex(context, EVP_aes_128_cbc(), nullptr, key, iv))
	{
		delete[] plaintext;
		delete[] ciphertext;

		return 1;
	}

	if(1 != EVP_EncryptUpdate(context, ciphertext, &len, plaintext, text.size()))
	{
		EVP_CIPHER_CTX_free(context);

		delete[] plaintext;
		delete[] ciphertext;

		return 2;
	}
	ciphertext_len = len;

	if(1 != EVP_EncryptFinal_ex(context, ciphertext + len, &len))
	{
		EVP_CIPHER_CTX_free(context);

		delete[] plaintext;
		delete[] ciphertext;

		return 3;
	}
	ciphertext_len += len;

	result = String_FromBuffer(ciphertext, ciphertext_len);

	/* Clean up */
	EVP_CIPHER_CTX_free(context);

	delete[] plaintext;
	delete[] ciphertext;

	return 0;
}


int DecryptEAS(const string &text, const unsigned char key[16]/* 128- битный ключ */, const unsigned char iv[16], string &result)
{
	EVP_CIPHER_CTX *context        = EVP_CIPHER_CTX_new();
	unsigned char  *ciphertext     = nullptr;
	unsigned char  *plaintext      = nullptr;
	int             ciphertext_len = text.size() >> 1;
	int             plaintext_len  = 0;
	int             len;

	result     = "";
	ciphertext = new unsigned char[ciphertext_len + 1];
	plaintext  = new unsigned char[ciphertext_len + 1];
	memset(ciphertext, 0, ciphertext_len + 1);
	memset(plaintext , 0, ciphertext_len + 1);

	String_ToBuffer(text.c_str(), ciphertext, ciphertext_len);

	if(1 != EVP_DecryptInit_ex(context, EVP_aes_128_cbc(), nullptr, key, iv))
	{
		delete[] ciphertext;
		delete[] plaintext;

		return 1;
	}

	if(1 != EVP_DecryptUpdate(context, plaintext, &len, ciphertext, ciphertext_len))
	{
		EVP_CIPHER_CTX_free(context);

		delete[] ciphertext;
		delete[] plaintext;

		return 2;
	}
	plaintext_len = len;

	if(1 != EVP_DecryptFinal_ex(context, plaintext + len, &len))
	{
		EVP_CIPHER_CTX_free(context);

		delete[] ciphertext;
		delete[] plaintext;

		return 3;
	}
	plaintext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(context);

	plaintext[plaintext_len] = 0;
	result = (char *)plaintext;

	delete[] ciphertext;
	delete[] plaintext;

	return 0;
}










