/*************************************************************************/
/*  editor_plugin_settings.cpp                                           */
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

#include "editor_plugin_settings.h"

#include "core/io/config_file.h"
#include "core/os/file_access.h"
#include "core/os/main_loop.h"
#include "core/project_settings.h"
#include "editor_node.h"
#include "editor_scale.h"
#include "scene/gui/margin_container.h"

void EditorPluginSettings::_notification(int p_what) {

	if (p_what == MainLoop::NOTIFICATION_WM_FOCUS_IN) {
		update_plugins();
	} else if (p_what == Node::NOTIFICATION_READY) {
		plugin_config_dialog->connect("plugin_ready", EditorNode::get_singleton(), "_on_plugin_ready");
		plugin_list->connect("button_pressed", this, "_cell_button_pressed");
	}
}

void EditorPluginSettings::update_plugins() {

	plugin_list->clear();
	updating = true;
	TreeItem *root = plugin_list->create_item();

	Vector<String> plugins = _get_plugins("res://addons");
	plugins.sort();

	for (int i = 0; i < plugins.size(); i++) {

		Ref<ConfigFile> cf;
		cf.instance();
		const String path = plugins[i];

		Error err2 = cf->load(path);

		if (err2 != OK) {
			WARN_PRINTS("Can't load plugin config: " + path);
		} else {
			bool key_missing = false;

			if (!cf->has_section_key("plugin", "name")) {
				WARN_PRINTS("Plugin config misses \"plugin/name\" key: " + path);
				key_missing = true;
			}
			if (!cf->has_section_key("plugin", "author")) {
				WARN_PRINTS("Plugin config misses \"plugin/author\" key: " + path);
				key_missing = true;
			}
			if (!cf->has_section_key("plugin", "version")) {
				WARN_PRINTS("Plugin config misses \"plugin/version\" key: " + path);
				key_missing = true;
			}
			if (!cf->has_section_key("plugin", "description")) {
				WARN_PRINTS("Plugin config misses \"plugin/description\" key: " + path);
				key_missing = true;
			}
			if (!cf->has_section_key("plugin", "script")) {
				WARN_PRINTS("Plugin config misses \"plugin/script\" key: " + path);
				key_missing = true;
			}

			if (!key_missing) {
				String name = cf->get_value("plugin", "name");
				String author = cf->get_value("plugin", "author");
				String version = cf->get_value("plugin", "version");
				String description = cf->get_value("plugin", "description");
				String script = cf->get_value("plugin", "script");

				TreeItem *item = plugin_list->create_item(root);
				item->set_text(0, name);
				item->set_tooltip(0, TTR("Name:") + " " + name + "\n" + TTR("Path:") + " " + path + "\n" + TTR("Main Script:") + " " + script + "\n" + TTR("Description:") + " " + description);
				item->set_metadata(0, path);
				item->set_text(1, version);
				item->set_metadata(1, script);
				item->set_text(2, author);
				item->set_metadata(2, description);
				item->set_cell_mode(3, TreeItem::CELL_MODE_CHECK);
				item->set_text(3, TTR("Enable"));
				bool is_active = EditorNode::get_singleton()->is_addon_plugin_enabled(path);
				item->set_checked(3, is_active);
				item->set_editable(3, true);
				item->add_button(4, get_icon("Edit", "EditorIcons"), BUTTON_PLUGIN_EDIT, false, TTR("Edit Plugin"));
			}
		}
	}

	updating = false;
}

void EditorPluginSettings::_plugin_activity_changed() {

	if (updating)
		return;

	TreeItem *ti = plugin_list->get_edited();
	ERR_FAIL_COND(!ti);
	bool active = ti->is_checked(3);
	String name = ti->get_metadata(0);

	EditorNode::get_singleton()->set_addon_plugin_enabled(name, active, true);

	bool is_active = EditorNode::get_singleton()->is_addon_plugin_enabled(name);

	if (is_active != active) {
		updating = true;
		ti->set_checked(3, is_active);
		updating = false;
	}
}

void EditorPluginSettings::_create_clicked() {
	plugin_config_dialog->config("");
	plugin_config_dialog->popup_centered();
}

void EditorPluginSettings::_cell_button_pressed(Object *p_item, int p_column, int p_id) {
	TreeItem *item = Object::cast_to<TreeItem>(p_item);
	if (!item)
		return;
	if (p_id == BUTTON_PLUGIN_EDIT) {
		if (p_column == 4) {
			String dir = item->get_metadata(0);
			plugin_config_dialog->config(dir);
			plugin_config_dialog->popup_centered();
		}
	}
}

Vector<String> EditorPluginSettings::_get_plugins(const String &p_dir) {
	DirAccessRef da = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	Error err = da->change_dir(p_dir);
	if (err != OK) {
		return Vector<String>();
	}

	Vector<String> plugins;
	da->list_dir_begin();
	for (String path = da->get_next(); path != String(); path = da->get_next()) {
		if (path[0] == '.' || !da->current_is_dir()) {
			continue;
		}

		const String full_path = p_dir.plus_file(path);
		const String plugin_config = full_path.plus_file("plugin.cfg");
		if (FileAccess::exists(plugin_config)) {
			plugins.push_back(plugin_config);
		} else {
			plugins.append_array(_get_plugins(full_path));
		}
	}

	da->list_dir_end();
	return plugins;
}

void EditorPluginSettings::_bind_methods() {

	ClassDB::bind_method("update_plugins", &EditorPluginSettings::update_plugins);
	ClassDB::bind_method("_create_clicked", &EditorPluginSettings::_create_clicked);
	ClassDB::bind_method("_plugin_activity_changed", &EditorPluginSettings::_plugin_activity_changed);
	ClassDB::bind_method("_cell_button_pressed", &EditorPluginSettings::_cell_button_pressed);
}

EditorPluginSettings::EditorPluginSettings() {

	plugin_config_dialog = memnew(PluginConfigDialog);
	plugin_config_dialog->config("");
	add_child(plugin_config_dialog);

	HBoxContainer *title_hb = memnew(HBoxContainer);
	title_hb->add_child(memnew(Label(TTR("Installed Plugins:"))));
	title_hb->add_spacer();
	create_plugin = memnew(Button(TTR("Create")));
	create_plugin->connect("pressed", this, "_create_clicked");
	title_hb->add_child(create_plugin);
	update_list = memnew(Button(TTR("Update")));
	update_list->connect("pressed", this, "update_plugins");
	title_hb->add_child(update_list);
	add_child(title_hb);

	plugin_list = memnew(Tree);
	plugin_list->set_v_size_flags(SIZE_EXPAND_FILL);
	plugin_list->set_columns(5);
	plugin_list->set_column_titles_visible(true);
	plugin_list->set_column_title(0, TTR("Name:"));
	plugin_list->set_column_title(1, TTR("Version:"));
	plugin_list->set_column_title(2, TTR("Author:"));
	plugin_list->set_column_title(3, TTR("Status:"));
	plugin_list->set_column_title(4, TTR("Edit:"));
	plugin_list->set_column_expand(0, true);
	plugin_list->set_column_expand(1, false);
	plugin_list->set_column_expand(2, false);
	plugin_list->set_column_expand(3, false);
	plugin_list->set_column_expand(4, false);
	plugin_list->set_column_min_width(1, 100 * EDSCALE);
	plugin_list->set_column_min_width(2, 250 * EDSCALE);
	plugin_list->set_column_min_width(3, 80 * EDSCALE);
	plugin_list->set_column_min_width(4, 40 * EDSCALE);
	plugin_list->set_hide_root(true);
	plugin_list->connect("item_edited", this, "_plugin_activity_changed");

	VBoxContainer *mc = memnew(VBoxContainer);
	mc->add_child(plugin_list);
	mc->set_v_size_flags(SIZE_EXPAND_FILL);
	mc->set_h_size_flags(SIZE_EXPAND_FILL);

	add_child(mc);

	updating = false;
}
