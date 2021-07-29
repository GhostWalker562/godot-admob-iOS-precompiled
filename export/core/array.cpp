/*************************************************************************/
/*  array.cpp                                                            */
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

#include "array.h"

#include "core/hashfuncs.h"
#include "core/object.h"
#include "core/variant.h"
#include "core/vector.h"

class ArrayPrivate {
public:
	SafeRefCount refcount;
	Vector<Variant> array;
};

void Array::_ref(const Array &p_from) const {

	ArrayPrivate *_fp = p_from._p;

	ERR_FAIL_COND(!_fp); // should NOT happen.

	if (_fp == _p)
		return; // whatever it is, nothing to do here move along

	bool success = _fp->refcount.ref();

	ERR_FAIL_COND(!success); // should really not happen either

	_unref();

	_p = p_from._p;
}

void Array::_unref() const {

	if (!_p)
		return;

	if (_p->refcount.unref()) {
		memdelete(_p);
	}
	_p = NULL;
}

Variant &Array::operator[](int p_idx) {

	return _p->array.write[p_idx];
}

const Variant &Array::operator[](int p_idx) const {

	return _p->array[p_idx];
}

int Array::size() const {

	return _p->array.size();
}
bool Array::empty() const {

	return _p->array.empty();
}
void Array::clear() {

	_p->array.clear();
}

bool Array::operator==(const Array &p_array) const {

	return _p == p_array._p;
}

uint32_t Array::hash() const {

	uint32_t h = hash_djb2_one_32(0);

	for (int i = 0; i < _p->array.size(); i++) {

		h = hash_djb2_one_32(_p->array[i].hash(), h);
	}
	return h;
}
void Array::operator=(const Array &p_array) {

	_ref(p_array);
}
void Array::push_back(const Variant &p_value) {

	_p->array.push_back(p_value);
}

void Array::append_array(const Array &p_array) {

	_p->array.append_array(p_array._p->array);
}

Error Array::resize(int p_new_size) {

	return _p->array.resize(p_new_size);
}

void Array::insert(int p_pos, const Variant &p_value) {

	_p->array.insert(p_pos, p_value);
}

void Array::erase(const Variant &p_value) {

	_p->array.erase(p_value);
}

Variant Array::front() const {
	ERR_FAIL_COND_V_MSG(_p->array.size() == 0, Variant(), "Can't take value from empty array.");
	return operator[](0);
}

Variant Array::back() const {
	ERR_FAIL_COND_V_MSG(_p->array.size() == 0, Variant(), "Can't take value from empty array.");
	return operator[](_p->array.size() - 1);
}

int Array::find(const Variant &p_value, int p_from) const {

	return _p->array.find(p_value, p_from);
}

int Array::rfind(const Variant &p_value, int p_from) const {

	if (_p->array.size() == 0)
		return -1;

	if (p_from < 0) {
		// Relative offset from the end
		p_from = _p->array.size() + p_from;
	}
	if (p_from < 0 || p_from >= _p->array.size()) {
		// Limit to array boundaries
		p_from = _p->array.size() - 1;
	}

	for (int i = p_from; i >= 0; i--) {

		if (_p->array[i] == p_value) {
			return i;
		}
	}

	return -1;
}

int Array::find_last(const Variant &p_value) const {

	return rfind(p_value);
}

int Array::count(const Variant &p_value) const {

	if (_p->array.size() == 0)
		return 0;

	int amount = 0;
	for (int i = 0; i < _p->array.size(); i++) {

		if (_p->array[i] == p_value) {
			amount++;
		}
	}

	return amount;
}

bool Array::has(const Variant &p_value) const {
	return _p->array.find(p_value, 0) != -1;
}

void Array::remove(int p_pos) {

	_p->array.remove(p_pos);
}

void Array::set(int p_idx, const Variant &p_value) {

	operator[](p_idx) = p_value;
}

const Variant &Array::get(int p_idx) const {

	return operator[](p_idx);
}

Array Array::duplicate(bool p_deep) const {

	Array new_arr;
	int element_count = size();
	new_arr.resize(element_count);
	for (int i = 0; i < element_count; i++) {
		new_arr[i] = p_deep ? get(i).duplicate(p_deep) : get(i);
	}

	return new_arr;
}

int Array::_clamp_slice_index(int p_index) const {

	int arr_size = size();
	int fixed_index = CLAMP(p_index, -arr_size, arr_size - 1);
	if (fixed_index < 0) {
		fixed_index = arr_size + fixed_index;
	}
	return fixed_index;
}

Array Array::slice(int p_begin, int p_end, int p_step, bool p_deep) const { // like python, but inclusive on upper bound

	Array new_arr;

	ERR_FAIL_COND_V_MSG(p_step == 0, new_arr, "Array slice step size cannot be zero.");

	if (empty()) // Don't try to slice empty arrays.
		return new_arr;
	if (p_step > 0) {
		if (p_begin >= size() || p_end < -size())
			return new_arr;
	} else { // p_step < 0
		if (p_begin < -size() || p_end >= size())
			return new_arr;
	}

	int begin = _clamp_slice_index(p_begin);
	int end = _clamp_slice_index(p_end);

	int new_arr_size = MAX(((end - begin + p_step) / p_step), 0);
	new_arr.resize(new_arr_size);

	if (p_step > 0) {
		int dest_idx = 0;
		for (int idx = begin; idx <= end; idx += p_step) {
			ERR_FAIL_COND_V_MSG(dest_idx < 0 || dest_idx >= new_arr_size, Array(), "Bug in Array slice()");
			new_arr[dest_idx++] = p_deep ? get(idx).duplicate(p_deep) : get(idx);
		}
	} else { // p_step < 0
		int dest_idx = 0;
		for (int idx = begin; idx >= end; idx += p_step) {
			ERR_FAIL_COND_V_MSG(dest_idx < 0 || dest_idx >= new_arr_size, Array(), "Bug in Array slice()");
			new_arr[dest_idx++] = p_deep ? get(idx).duplicate(p_deep) : get(idx);
		}
	}

	return new_arr;
}

struct _ArrayVariantSort {

	_FORCE_INLINE_ bool operator()(const Variant &p_l, const Variant &p_r) const {
		bool valid = false;
		Variant res;
		Variant::evaluate(Variant::OP_LESS, p_l, p_r, res, valid);
		if (!valid)
			res = false;
		return res;
	}
};

Array &Array::sort() {

	_p->array.sort_custom<_ArrayVariantSort>();
	return *this;
}

struct _ArrayVariantSortCustom {

	Object *obj;
	StringName func;

	_FORCE_INLINE_ bool operator()(const Variant &p_l, const Variant &p_r) const {

		const Variant *args[2] = { &p_l, &p_r };
		Variant::CallError err;
		bool res = obj->call(func, args, 2, err);
		if (err.error != Variant::CallError::CALL_OK)
			res = false;
		return res;
	}
};
Array &Array::sort_custom(Object *p_obj, const StringName &p_function) {

	ERR_FAIL_NULL_V(p_obj, *this);

	SortArray<Variant, _ArrayVariantSortCustom, true> avs;
	avs.compare.obj = p_obj;
	avs.compare.func = p_function;
	avs.sort(_p->array.ptrw(), _p->array.size());
	return *this;
}

void Array::shuffle() {

	const int n = _p->array.size();
	if (n < 2)
		return;
	Variant *data = _p->array.ptrw();
	for (int i = n - 1; i >= 1; i--) {
		const int j = Math::rand() % (i + 1);
		const Variant tmp = data[j];
		data[j] = data[i];
		data[i] = tmp;
	}
}

template <typename Less>
_FORCE_INLINE_ int bisect(const Vector<Variant> &p_array, const Variant &p_value, bool p_before, const Less &p_less) {

	int lo = 0;
	int hi = p_array.size();
	if (p_before) {
		while (lo < hi) {
			const int mid = (lo + hi) / 2;
			if (p_less(p_array.get(mid), p_value)) {
				lo = mid + 1;
			} else {
				hi = mid;
			}
		}
	} else {
		while (lo < hi) {
			const int mid = (lo + hi) / 2;
			if (p_less(p_value, p_array.get(mid))) {
				hi = mid;
			} else {
				lo = mid + 1;
			}
		}
	}
	return lo;
}

int Array::bsearch(const Variant &p_value, bool p_before) {

	return bisect(_p->array, p_value, p_before, _ArrayVariantSort());
}

int Array::bsearch_custom(const Variant &p_value, Object *p_obj, const StringName &p_function, bool p_before) {

	ERR_FAIL_NULL_V(p_obj, 0);

	_ArrayVariantSortCustom less;
	less.obj = p_obj;
	less.func = p_function;

	return bisect(_p->array, p_value, p_before, less);
}

Array &Array::invert() {

	_p->array.invert();
	return *this;
}

void Array::push_front(const Variant &p_value) {

	_p->array.insert(0, p_value);
}

Variant Array::pop_back() {

	if (!_p->array.empty()) {
		int n = _p->array.size() - 1;
		Variant ret = _p->array.get(n);
		_p->array.resize(n);
		return ret;
	}
	return Variant();
}

Variant Array::pop_front() {

	if (!_p->array.empty()) {
		Variant ret = _p->array.get(0);
		_p->array.remove(0);
		return ret;
	}
	return Variant();
}

Variant Array::min() const {

	Variant minval;
	for (int i = 0; i < size(); i++) {
		if (i == 0) {
			minval = get(i);
		} else {
			bool valid;
			Variant ret;
			Variant test = get(i);
			Variant::evaluate(Variant::OP_LESS, test, minval, ret, valid);
			if (!valid) {
				return Variant(); //not a valid comparison
			}
			if (bool(ret)) {
				//is less
				minval = test;
			}
		}
	}
	return minval;
}

Variant Array::max() const {

	Variant maxval;
	for (int i = 0; i < size(); i++) {
		if (i == 0) {
			maxval = get(i);
		} else {
			bool valid;
			Variant ret;
			Variant test = get(i);
			Variant::evaluate(Variant::OP_GREATER, test, maxval, ret, valid);
			if (!valid) {
				return Variant(); //not a valid comparison
			}
			if (bool(ret)) {
				//is less
				maxval = test;
			}
		}
	}
	return maxval;
}

const void *Array::id() const {
	return _p->array.ptr();
}

Array::Array(const Array &p_from) {

	_p = NULL;
	_ref(p_from);
}

Array::Array() {

	_p = memnew(ArrayPrivate);
	_p->refcount.init();
}
Array::~Array() {

	_unref();
}
