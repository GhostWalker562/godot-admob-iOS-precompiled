/*************************************************************************/
/*  particles_editor_plugin.h                                            */
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

#ifndef PARTICLES_EDITOR_PLUGIN_H
#define PARTICLES_EDITOR_PLUGIN_H

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/3d/particles.h"
#include "scene/gui/spin_box.h"

class ParticlesEditorBase : public Control {

	GDCLASS(ParticlesEditorBase, Control);

protected:
	Spatial *base_node;
	Panel *panel;
	MenuButton *options;
	HBoxContainer *particles_editor_hb;

	EditorFileDialog *emission_file_dialog;
	SceneTreeDialog *emission_tree_dialog;

	ConfirmationDialog *emission_dialog;
	SpinBox *emission_amount;
	OptionButton *emission_fill;

	PoolVector<Face3> geometry;

	bool _generate(PoolVector<Vector3> &points, PoolVector<Vector3> &normals);
	virtual void _generate_emission_points() = 0;
	void _node_selected(const NodePath &p_path);

	static void _bind_methods();

public:
	ParticlesEditorBase();
};

class ParticlesEditor : public ParticlesEditorBase {

	GDCLASS(ParticlesEditor, ParticlesEditorBase);

	ConfirmationDialog *generate_aabb;
	SpinBox *generate_seconds;
	Particles *node;

	enum Menu {

		MENU_OPTION_GENERATE_AABB,
		MENU_OPTION_CREATE_EMISSION_VOLUME_FROM_NODE,
		MENU_OPTION_CREATE_EMISSION_VOLUME_FROM_MESH,
		MENU_OPTION_CLEAR_EMISSION_VOLUME,
		MENU_OPTION_CONVERT_TO_CPU_PARTICLES,
		MENU_OPTION_RESTART,

	};

	void _generate_aabb();

	void _menu_option(int);

	friend class ParticlesEditorPlugin;

	virtual void _generate_emission_points();

protected:
	void _notification(int p_notification);
	void _node_removed(Node *p_node);
	static void _bind_methods();

public:
	void edit(Particles *p_particles);
	ParticlesEditor();
};

class ParticlesEditorPlugin : public EditorPlugin {

	GDCLASS(ParticlesEditorPlugin, EditorPlugin);

	ParticlesEditor *particles_editor;
	EditorNode *editor;

public:
	virtual String get_name() const { return "Particles"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object *p_object);
	virtual bool handles(Object *p_object) const;
	virtual void make_visible(bool p_visible);

	ParticlesEditorPlugin(EditorNode *p_node);
	~ParticlesEditorPlugin();
};

#endif // PARTICLES_EDITOR_PLUGIN_H
