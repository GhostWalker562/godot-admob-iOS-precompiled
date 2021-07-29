/*************************************************************************/
/*  string_buffer.h                                                      */
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

#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include "core/ustring.h"

template <int SHORT_BUFFER_SIZE = 64>
class StringBuffer {

	CharType short_buffer[SHORT_BUFFER_SIZE];
	String buffer;
	int string_length;

	_FORCE_INLINE_ CharType *current_buffer_ptr() {
		return static_cast<String &>(buffer).empty() ? short_buffer : buffer.ptrw();
	}

public:
	StringBuffer &append(CharType p_char);
	StringBuffer &append(const String &p_string);
	StringBuffer &append(const char *p_str);
	StringBuffer &append(const CharType *p_str, int p_clip_to_len = -1);

	_FORCE_INLINE_ void operator+=(CharType p_char) {
		append(p_char);
	}

	_FORCE_INLINE_ void operator+=(const String &p_string) {
		append(p_string);
	}

	_FORCE_INLINE_ void operator+=(const char *p_str) {
		append(p_str);
	}

	_FORCE_INLINE_ void operator+=(const CharType *p_str) {
		append(p_str);
	}

	StringBuffer &reserve(int p_size);

	int length() const;

	String as_string();

	double as_double();
	int64_t as_int();

	_FORCE_INLINE_ operator String() {
		return as_string();
	}

	StringBuffer() {
		string_length = 0;
	}
};

template <int SHORT_BUFFER_SIZE>
StringBuffer<SHORT_BUFFER_SIZE> &StringBuffer<SHORT_BUFFER_SIZE>::append(CharType p_char) {
	reserve(string_length + 2);
	current_buffer_ptr()[string_length++] = p_char;
	return *this;
}

template <int SHORT_BUFFER_SIZE>
StringBuffer<SHORT_BUFFER_SIZE> &StringBuffer<SHORT_BUFFER_SIZE>::append(const String &p_string) {
	return append(p_string.c_str());
}

template <int SHORT_BUFFER_SIZE>
StringBuffer<SHORT_BUFFER_SIZE> &StringBuffer<SHORT_BUFFER_SIZE>::append(const char *p_str) {
	int len = strlen(p_str);
	reserve(string_length + len + 1);

	CharType *buf = current_buffer_ptr();
	for (const char *c_ptr = p_str; *c_ptr; ++c_ptr) {
		buf[string_length++] = *c_ptr;
	}
	return *this;
}

template <int SHORT_BUFFER_SIZE>
StringBuffer<SHORT_BUFFER_SIZE> &StringBuffer<SHORT_BUFFER_SIZE>::append(const CharType *p_str, int p_clip_to_len) {
	int len = 0;
	while ((p_clip_to_len < 0 || len < p_clip_to_len) && p_str[len]) {
		++len;
	}
	reserve(string_length + len + 1);
	memcpy(&(current_buffer_ptr()[string_length]), p_str, len * sizeof(CharType));
	string_length += len;

	return *this;
}

template <int SHORT_BUFFER_SIZE>
StringBuffer<SHORT_BUFFER_SIZE> &StringBuffer<SHORT_BUFFER_SIZE>::reserve(int p_size) {
	if (p_size < SHORT_BUFFER_SIZE || p_size < buffer.size())
		return *this;

	bool need_copy = string_length > 0 && buffer.empty();
	buffer.resize(next_power_of_2(p_size));
	if (need_copy) {
		memcpy(buffer.ptrw(), short_buffer, string_length * sizeof(CharType));
	}

	return *this;
}

template <int SHORT_BUFFER_SIZE>
int StringBuffer<SHORT_BUFFER_SIZE>::length() const {
	return string_length;
}

template <int SHORT_BUFFER_SIZE>
String StringBuffer<SHORT_BUFFER_SIZE>::as_string() {
	current_buffer_ptr()[string_length] = '\0';
	if (buffer.empty()) {
		return String(short_buffer);
	} else {
		buffer.resize(string_length + 1);
		return buffer;
	}
}

template <int SHORT_BUFFER_SIZE>
double StringBuffer<SHORT_BUFFER_SIZE>::as_double() {
	current_buffer_ptr()[string_length] = '\0';
	return String::to_double(current_buffer_ptr());
}

template <int SHORT_BUFFER_SIZE>
int64_t StringBuffer<SHORT_BUFFER_SIZE>::as_int() {
	current_buffer_ptr()[string_length] = '\0';
	return String::to_int(current_buffer_ptr());
}

#endif
