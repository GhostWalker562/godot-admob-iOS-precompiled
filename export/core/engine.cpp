/*************************************************************************/
/*  engine.cpp                                                           */
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

#include "engine.h"

#include "core/authors.gen.h"
#include "core/donors.gen.h"
#include "core/license.gen.h"
#include "core/version.h"
#include "core/version_hash.gen.h"

void Engine::set_iterations_per_second(int p_ips) {

	ERR_FAIL_COND_MSG(p_ips <= 0, "Engine iterations per second must be greater than 0.");
	ips = p_ips;
}
int Engine::get_iterations_per_second() const {

	return ips;
}

void Engine::set_physics_jitter_fix(float p_threshold) {
	if (p_threshold < 0)
		p_threshold = 0;
	physics_jitter_fix = p_threshold;
}

float Engine::get_physics_jitter_fix() const {
	return physics_jitter_fix;
}

void Engine::set_target_fps(int p_fps) {
	_target_fps = p_fps > 0 ? p_fps : 0;
}

int Engine::get_target_fps() const {
	return _target_fps;
}

uint64_t Engine::get_frames_drawn() {

	return frames_drawn;
}

void Engine::set_frame_delay(uint32_t p_msec) {

	_frame_delay = p_msec;
}

uint32_t Engine::get_frame_delay() const {

	return _frame_delay;
}

void Engine::set_time_scale(float p_scale) {

	_time_scale = p_scale;
}

float Engine::get_time_scale() const {

	return _time_scale;
}

Dictionary Engine::get_version_info() const {

	Dictionary dict;
	dict["major"] = VERSION_MAJOR;
	dict["minor"] = VERSION_MINOR;
	dict["patch"] = VERSION_PATCH;
	dict["hex"] = VERSION_HEX;
	dict["status"] = VERSION_STATUS;
	dict["build"] = VERSION_BUILD;
	dict["year"] = VERSION_YEAR;

	String hash = VERSION_HASH;
	dict["hash"] = hash.length() == 0 ? String("unknown") : hash;

	String stringver = String(dict["major"]) + "." + String(dict["minor"]);
	if ((int)dict["patch"] != 0)
		stringver += "." + String(dict["patch"]);
	stringver += "-" + String(dict["status"]) + " (" + String(dict["build"]) + ")";
	dict["string"] = stringver;

	return dict;
}

static Array array_from_info(const char *const *info_list) {
	Array arr;
	for (int i = 0; info_list[i] != NULL; i++) {
		arr.push_back(info_list[i]);
	}
	return arr;
}

static Array array_from_info_count(const char *const *info_list, int info_count) {
	Array arr;
	for (int i = 0; i < info_count; i++) {
		arr.push_back(info_list[i]);
	}
	return arr;
}

Dictionary Engine::get_author_info() const {
	Dictionary dict;

	dict["lead_developers"] = array_from_info(AUTHORS_LEAD_DEVELOPERS);
	dict["project_managers"] = array_from_info(AUTHORS_PROJECT_MANAGERS);
	dict["founders"] = array_from_info(AUTHORS_FOUNDERS);
	dict["developers"] = array_from_info(AUTHORS_DEVELOPERS);

	return dict;
}

Array Engine::get_copyright_info() const {
	Array components;
	for (int component_index = 0; component_index < COPYRIGHT_INFO_COUNT; component_index++) {
		const ComponentCopyright &cp_info = COPYRIGHT_INFO[component_index];
		Dictionary component_dict;
		component_dict["name"] = cp_info.name;
		Array parts;
		for (int i = 0; i < cp_info.part_count; i++) {
			const ComponentCopyrightPart &cp_part = cp_info.parts[i];
			Dictionary part_dict;
			part_dict["files"] = array_from_info_count(cp_part.files, cp_part.file_count);
			part_dict["copyright"] = array_from_info_count(cp_part.copyright_statements, cp_part.copyright_count);
			part_dict["license"] = cp_part.license;
			parts.push_back(part_dict);
		}
		component_dict["parts"] = parts;

		components.push_back(component_dict);
	}
	return components;
}

Dictionary Engine::get_donor_info() const {
	Dictionary donors;
	donors["platinum_sponsors"] = array_from_info(DONORS_SPONSOR_PLATINUM);
	donors["gold_sponsors"] = array_from_info(DONORS_SPONSOR_GOLD);
	donors["silver_sponsors"] = array_from_info(DONORS_SPONSOR_SILVER);
	donors["bronze_sponsors"] = array_from_info(DONORS_SPONSOR_BRONZE);
	donors["mini_sponsors"] = array_from_info(DONORS_SPONSOR_MINI);
	donors["gold_donors"] = array_from_info(DONORS_GOLD);
	donors["silver_donors"] = array_from_info(DONORS_SILVER);
	donors["bronze_donors"] = array_from_info(DONORS_BRONZE);
	return donors;
}

Dictionary Engine::get_license_info() const {
	Dictionary licenses;
	for (int i = 0; i < LICENSE_COUNT; i++) {
		licenses[LICENSE_NAMES[i]] = LICENSE_BODIES[i];
	}
	return licenses;
}

String Engine::get_license_text() const {
	return String(GODOT_LICENSE_TEXT);
}

void Engine::add_singleton(const Singleton &p_singleton) {

	singletons.push_back(p_singleton);
	singleton_ptrs[p_singleton.name] = p_singleton.ptr;
}

Object *Engine::get_singleton_object(const String &p_name) const {

	const Map<StringName, Object *>::Element *E = singleton_ptrs.find(p_name);
	ERR_FAIL_COND_V_MSG(!E, NULL, "Failed to retrieve non-existent singleton '" + p_name + "'.");
	return E->get();
};

bool Engine::has_singleton(const String &p_name) const {

	return singleton_ptrs.has(p_name);
};

void Engine::get_singletons(List<Singleton> *p_singletons) {

	for (List<Singleton>::Element *E = singletons.front(); E; E = E->next())
		p_singletons->push_back(E->get());
}

Engine *Engine::singleton = NULL;

Engine *Engine::get_singleton() {
	return singleton;
}

Engine::Engine() {

	singleton = this;
	frames_drawn = 0;
	ips = 60;
	physics_jitter_fix = 0.5;
	_physics_interpolation_fraction = 0.0f;
	_frame_delay = 0;
	_fps = 1;
	_target_fps = 0;
	_time_scale = 1.0;
	_gpu_pixel_snap = false;
	_physics_frames = 0;
	_idle_frames = 0;
	_in_physics = false;
	_frame_ticks = 0;
	_frame_step = 0;
	editor_hint = false;
}

Engine::Singleton::Singleton(const StringName &p_name, Object *p_ptr) :
		name(p_name),
		ptr(p_ptr) {
#ifdef DEBUG_ENABLED
	Reference *ref = Object::cast_to<Reference>(p_ptr);
	if (ref && !ref->is_referenced()) {
		WARN_PRINT("You must use Ref<> to ensure the lifetime of a Reference object intended to be used as a singleton.");
	}
#endif
}
