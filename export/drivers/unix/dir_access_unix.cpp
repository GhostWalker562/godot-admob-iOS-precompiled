/*************************************************************************/
/*  dir_access_unix.cpp                                                  */
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

#include "dir_access_unix.h"

#if defined(UNIX_ENABLED) || defined(LIBC_FILEIO_ENABLED)

#include "core/list.h"
#include "core/os/memory.h"
#include "core/print_string.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ANDROID_ENABLED
#include <sys/statvfs.h>
#endif

#ifdef HAVE_MNTENT
#include <mntent.h>
#endif

DirAccess *DirAccessUnix::create_fs() {

	return memnew(DirAccessUnix);
}

Error DirAccessUnix::list_dir_begin() {

	list_dir_end(); //close any previous dir opening!

	//char real_current_dir_name[2048]; //is this enough?!
	//getcwd(real_current_dir_name,2048);
	//chdir(current_path.utf8().get_data());
	dir_stream = opendir(current_dir.utf8().get_data());
	//chdir(real_current_dir_name);
	if (!dir_stream)
		return ERR_CANT_OPEN; //error!

	return OK;
}

bool DirAccessUnix::file_exists(String p_file) {

	GLOBAL_LOCK_FUNCTION

	if (p_file.is_rel_path())
		p_file = current_dir.plus_file(p_file);

	p_file = fix_path(p_file);

	struct stat flags;
	bool success = (stat(p_file.utf8().get_data(), &flags) == 0);

	if (success && S_ISDIR(flags.st_mode)) {
		success = false;
	}

	return success;
}

bool DirAccessUnix::dir_exists(String p_dir) {

	GLOBAL_LOCK_FUNCTION

	if (p_dir.is_rel_path())
		p_dir = get_current_dir().plus_file(p_dir);

	p_dir = fix_path(p_dir);

	struct stat flags;
	bool success = (stat(p_dir.utf8().get_data(), &flags) == 0);

	return (success && S_ISDIR(flags.st_mode));
}

uint64_t DirAccessUnix::get_modified_time(String p_file) {

	if (p_file.is_rel_path())
		p_file = current_dir.plus_file(p_file);

	p_file = fix_path(p_file);

	struct stat flags;
	bool success = (stat(p_file.utf8().get_data(), &flags) == 0);

	if (success) {
		return flags.st_mtime;
	} else {

		ERR_FAIL_V(0);
	};
	return 0;
};

String DirAccessUnix::get_next() {

	if (!dir_stream)
		return "";

	dirent *entry = readdir(dir_stream);

	if (entry == NULL) {
		list_dir_end();
		return "";
	}

	String fname = fix_unicode_name(entry->d_name);

	// Look at d_type to determine if the entry is a directory, unless
	// its type is unknown (the file system does not support it) or if
	// the type is a link, in that case we want to resolve the link to
	// known if it points to a directory. stat() will resolve the link
	// for us.
	if (entry->d_type == DT_UNKNOWN || entry->d_type == DT_LNK) {
		String f = current_dir.plus_file(fname);

		struct stat flags;
		if (stat(f.utf8().get_data(), &flags) == 0) {
			_cisdir = S_ISDIR(flags.st_mode);
		} else {
			_cisdir = false;
		}
	} else {
		_cisdir = (entry->d_type == DT_DIR);
	}

	_cishidden = is_hidden(fname);

	return fname;
}

bool DirAccessUnix::current_is_dir() const {

	return _cisdir;
}

bool DirAccessUnix::current_is_hidden() const {

	return _cishidden;
}

void DirAccessUnix::list_dir_end() {

	if (dir_stream)
		closedir(dir_stream);
	dir_stream = 0;
	_cisdir = false;
}

#if defined(HAVE_MNTENT) && defined(X11_ENABLED)
static bool _filter_drive(struct mntent *mnt) {
	// Ignore devices that don't point to /dev
	if (strncmp(mnt->mnt_fsname, "/dev", 4) != 0) {
		return false;
	}

	// Accept devices mounted at common locations
	if (strncmp(mnt->mnt_dir, "/media", 6) == 0 ||
			strncmp(mnt->mnt_dir, "/mnt", 4) == 0 ||
			strncmp(mnt->mnt_dir, "/home", 5) == 0 ||
			strncmp(mnt->mnt_dir, "/run/media", 10) == 0) {
		return true;
	}

	// Ignore everything else
	return false;
}
#endif

static void _get_drives(List<String> *list) {

#if defined(HAVE_MNTENT) && defined(X11_ENABLED)
	// Check /etc/mtab for the list of mounted partitions
	FILE *mtab = setmntent("/etc/mtab", "r");
	if (mtab) {
		struct mntent mnt;
		char strings[4096];

		while (getmntent_r(mtab, &mnt, strings, sizeof(strings))) {
			if (mnt.mnt_dir != NULL && _filter_drive(&mnt)) {
				// Avoid duplicates
				if (!list->find(mnt.mnt_dir)) {
					list->push_back(mnt.mnt_dir);
				}
			}
		}

		endmntent(mtab);
	}
#endif

	// Add $HOME
	const char *home = getenv("HOME");
	if (home) {
		// Only add if it's not a duplicate
		if (!list->find(home)) {
			list->push_back(home);
		}

		// Check $HOME/.config/gtk-3.0/bookmarks
		char path[1024];
		snprintf(path, 1024, "%s/.config/gtk-3.0/bookmarks", home);
		FILE *fd = fopen(path, "r");
		if (fd) {
			char string[1024];
			while (fgets(string, 1024, fd)) {
				// Parse only file:// links
				if (strncmp(string, "file://", 7) == 0) {
					// Strip any unwanted edges on the strings and push_back if it's not a duplicate
					String fpath = String(string + 7).strip_edges().split_spaces()[0].percent_decode();
					if (!list->find(fpath)) {
						list->push_back(fpath);
					}
				}
			}

			fclose(fd);
		}
	}

	list->sort();
}

int DirAccessUnix::get_drive_count() {

	List<String> list;
	_get_drives(&list);

	return list.size();
}

String DirAccessUnix::get_drive(int p_drive) {

	List<String> list;
	_get_drives(&list);

	ERR_FAIL_INDEX_V(p_drive, list.size(), "");

	return list[p_drive];
}

bool DirAccessUnix::drives_are_shortcuts() {

	return true;
}

Error DirAccessUnix::make_dir(String p_dir) {

	GLOBAL_LOCK_FUNCTION

	if (p_dir.is_rel_path())
		p_dir = get_current_dir().plus_file(p_dir);

	p_dir = fix_path(p_dir);

	bool success = (mkdir(p_dir.utf8().get_data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0);
	int err = errno;

	if (success) {
		return OK;
	};

	if (err == EEXIST) {
		return ERR_ALREADY_EXISTS;
	};

	return ERR_CANT_CREATE;
}

Error DirAccessUnix::change_dir(String p_dir) {

	GLOBAL_LOCK_FUNCTION

	p_dir = fix_path(p_dir);

	// prev_dir is the directory we are changing out of
	String prev_dir;
	char real_current_dir_name[2048];
	ERR_FAIL_COND_V(getcwd(real_current_dir_name, 2048) == NULL, ERR_BUG);
	if (prev_dir.parse_utf8(real_current_dir_name))
		prev_dir = real_current_dir_name; //no utf8, maybe latin?

	// try_dir is the directory we are trying to change into
	String try_dir = "";
	if (p_dir.is_rel_path()) {
		String next_dir = current_dir.plus_file(p_dir);
		next_dir = next_dir.simplify_path();
		try_dir = next_dir;
	} else {
		try_dir = p_dir;
	}

	bool worked = (chdir(try_dir.utf8().get_data()) == 0); // we can only give this utf8
	if (!worked) {
		return ERR_INVALID_PARAMETER;
	}

	String base = _get_root_path();
	if (base != String() && !try_dir.begins_with(base)) {
		ERR_FAIL_COND_V(getcwd(real_current_dir_name, 2048) == NULL, ERR_BUG);
		String new_dir;
		new_dir.parse_utf8(real_current_dir_name);

		if (!new_dir.begins_with(base)) {
			try_dir = current_dir; //revert
		}
	}

	// the directory exists, so set current_dir to try_dir
	current_dir = try_dir;
	ERR_FAIL_COND_V(chdir(prev_dir.utf8().get_data()) != 0, ERR_BUG);
	return OK;
}

String DirAccessUnix::get_current_dir() {

	String base = _get_root_path();
	if (base != "") {

		String bd = current_dir.replace_first(base, "");
		if (bd.begins_with("/"))
			return _get_root_string() + bd.substr(1, bd.length());
		else
			return _get_root_string() + bd;
	}
	return current_dir;
}

Error DirAccessUnix::rename(String p_path, String p_new_path) {

	if (p_path.is_rel_path())
		p_path = get_current_dir().plus_file(p_path);

	p_path = fix_path(p_path);

	if (p_new_path.is_rel_path())
		p_new_path = get_current_dir().plus_file(p_new_path);

	p_new_path = fix_path(p_new_path);

	return ::rename(p_path.utf8().get_data(), p_new_path.utf8().get_data()) == 0 ? OK : FAILED;
}

Error DirAccessUnix::remove(String p_path) {

	if (p_path.is_rel_path())
		p_path = get_current_dir().plus_file(p_path);

	p_path = fix_path(p_path);

	struct stat flags;
	if ((stat(p_path.utf8().get_data(), &flags) != 0))
		return FAILED;

	if (S_ISDIR(flags.st_mode))
		return ::rmdir(p_path.utf8().get_data()) == 0 ? OK : FAILED;
	else
		return ::unlink(p_path.utf8().get_data()) == 0 ? OK : FAILED;
}

size_t DirAccessUnix::get_space_left() {

#ifndef NO_STATVFS
	struct statvfs vfs;
	if (statvfs(current_dir.utf8().get_data(), &vfs) != 0) {

		return 0;
	};

	return (uint64_t)vfs.f_bavail * (uint64_t)vfs.f_frsize;
#else
	// FIXME: Implement this.
	return 0;
#endif
};

String DirAccessUnix::get_filesystem_type() const {
	return ""; //TODO this should be implemented
}

bool DirAccessUnix::is_hidden(const String &p_name) {
	return p_name != "." && p_name != ".." && p_name.begins_with(".");
}

DirAccessUnix::DirAccessUnix() {

	dir_stream = 0;
	_cisdir = false;

	/* determine drive count */

	// set current directory to an absolute path of the current directory
	char real_current_dir_name[2048];
	ERR_FAIL_COND(getcwd(real_current_dir_name, 2048) == NULL);
	if (current_dir.parse_utf8(real_current_dir_name))
		current_dir = real_current_dir_name;

	change_dir(current_dir);
}

DirAccessUnix::~DirAccessUnix() {

	list_dir_end();
}

#endif //posix_enabled
