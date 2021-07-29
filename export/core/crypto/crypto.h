/*************************************************************************/
/*  crypto.h                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef CRYPTO_H
#define CRYPTO_H

#include "core/reference.h"
#include "core/resource.h"

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"

class CryptoKey : public Resource {
	GDCLASS(CryptoKey, Resource);

protected:
	static void _bind_methods();
	static CryptoKey *(*_create)();

public:
	static CryptoKey *create();
	virtual Error load(String p_path) = 0;
	virtual Error save(String p_path) = 0;
};

class X509Certificate : public Resource {
	GDCLASS(X509Certificate, Resource);

protected:
	static void _bind_methods();
	static X509Certificate *(*_create)();

public:
	static X509Certificate *create();
	virtual Error load(String p_path) = 0;
	virtual Error load_from_memory(const uint8_t *p_buffer, int p_len) = 0;
	virtual Error save(String p_path) = 0;
};

class Crypto : public Reference {
	GDCLASS(Crypto, Reference);

protected:
	static void _bind_methods();
	static Crypto *(*_create)();
	static void (*_load_default_certificates)(String p_path);

public:
	static Crypto *create();
	static void load_default_certificates(String p_path);

	virtual PoolByteArray generate_random_bytes(int p_bytes) = 0;
	virtual Ref<CryptoKey> generate_rsa(int p_bytes) = 0;
	virtual Ref<X509Certificate> generate_self_signed_certificate(Ref<CryptoKey> p_key, String p_issuer_name, String p_not_before, String p_not_after) = 0;

	Crypto();
};

class ResourceFormatLoaderCrypto : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

class ResourceFormatSaverCrypto : public ResourceFormatSaver {
public:
	virtual Error save(const String &p_path, const RES &p_resource, uint32_t p_flags = 0);
	virtual void get_recognized_extensions(const RES &p_resource, List<String> *p_extensions) const;
	virtual bool recognize(const RES &p_resource) const;
};

#endif // CRYPTO_H
