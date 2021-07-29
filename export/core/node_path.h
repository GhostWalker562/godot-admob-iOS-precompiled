/*************************************************************************/
/*  node_path.h                                                          */
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

#ifndef NODE_PATH_H
#define NODE_PATH_H

#include "core/string_name.h"
#include "core/ustring.h"

class NodePath {

	struct Data {

		SafeRefCount refcount;
		Vector<StringName> path;
		Vector<StringName> subpath;
		StringName concatenated_subpath;
		bool absolute;
		bool has_slashes;
		mutable bool hash_cache_valid;
		mutable uint32_t hash_cache;
	};

	mutable Data *data;
	void unref();

	void _update_hash_cache() const;

public:
	_FORCE_INLINE_ StringName get_sname() const {

		if (data && data->path.size() == 1 && data->subpath.empty()) {
			return data->path[0];
		} else {
			return operator String();
		}
	}

	bool is_absolute() const;
	int get_name_count() const;
	StringName get_name(int p_idx) const;
	int get_subname_count() const;
	StringName get_subname(int p_idx) const;
	Vector<StringName> get_names() const;
	Vector<StringName> get_subnames() const;
	StringName get_concatenated_subnames() const;

	NodePath rel_path_to(const NodePath &p_np) const;
	NodePath get_as_property_path() const;

	void prepend_period();

	NodePath get_parent() const;

	_FORCE_INLINE_ uint32_t hash() const {
		if (!data)
			return 0;
		if (!data->hash_cache_valid) {
			_update_hash_cache();
		}
		return data->hash_cache;
	}

	operator String() const;
	bool is_empty() const;

	bool operator==(const NodePath &p_path) const;
	bool operator!=(const NodePath &p_path) const;
	void operator=(const NodePath &p_path);

	void simplify();
	NodePath simplified() const;

	NodePath(const Vector<StringName> &p_path, bool p_absolute);
	NodePath(const Vector<StringName> &p_path, const Vector<StringName> &p_subpath, bool p_absolute);
	NodePath(const NodePath &p_path);
	NodePath(const String &p_path);
	NodePath();
	~NodePath();
};

#endif
