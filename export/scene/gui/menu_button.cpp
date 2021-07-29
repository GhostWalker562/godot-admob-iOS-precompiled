/*************************************************************************/
/*  menu_button.cpp                                                      */
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

#include "menu_button.h"
#include "core/os/keyboard.h"
#include "scene/main/viewport.h"

void MenuButton::_unhandled_key_input(Ref<InputEvent> p_event) {

	if (disable_shortcuts)
		return;

	if (p_event->is_pressed() && !p_event->is_echo() && (Object::cast_to<InputEventKey>(p_event.ptr()) || Object::cast_to<InputEventJoypadButton>(p_event.ptr()) || Object::cast_to<InputEventAction>(*p_event))) {

		if (!get_parent() || !is_visible_in_tree() || is_disabled())
			return;

		bool global_only = (get_viewport()->get_modal_stack_top() && !get_viewport()->get_modal_stack_top()->is_a_parent_of(this));

		if (popup->activate_item_by_event(p_event, global_only))
			accept_event();
	}
}

void MenuButton::pressed() {

	emit_signal("about_to_show");
	Size2 size = get_size();

	Point2 gp = get_global_position();
	popup->set_global_position(gp + Size2(0, size.height * get_global_transform().get_scale().y));
	popup->set_size(Size2(size.width, 0));
	popup->set_scale(get_global_transform().get_scale());
	popup->set_parent_rect(Rect2(Point2(gp - popup->get_global_position()), get_size()));
	popup->popup();
}

void MenuButton::_gui_input(Ref<InputEvent> p_event) {

	BaseButton::_gui_input(p_event);
}

PopupMenu *MenuButton::get_popup() const {

	return popup;
}

void MenuButton::_set_items(const Array &p_items) {

	popup->set("items", p_items);
}

Array MenuButton::_get_items() const {

	return popup->get("items");
}

void MenuButton::set_switch_on_hover(bool p_enabled) {

	switch_on_hover = p_enabled;
}

bool MenuButton::is_switch_on_hover() {

	return switch_on_hover;
}

void MenuButton::_notification(int p_what) {

	if (p_what == NOTIFICATION_VISIBILITY_CHANGED) {

		if (!is_visible_in_tree()) {
			popup->hide();
		}
	}
}

void MenuButton::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_popup"), &MenuButton::get_popup);
	ClassDB::bind_method(D_METHOD("_unhandled_key_input"), &MenuButton::_unhandled_key_input);
	ClassDB::bind_method(D_METHOD("_set_items"), &MenuButton::_set_items);
	ClassDB::bind_method(D_METHOD("_get_items"), &MenuButton::_get_items);
	ClassDB::bind_method(D_METHOD("set_switch_on_hover", "enable"), &MenuButton::set_switch_on_hover);
	ClassDB::bind_method(D_METHOD("is_switch_on_hover"), &MenuButton::is_switch_on_hover);
	ClassDB::bind_method(D_METHOD("set_disable_shortcuts", "disabled"), &MenuButton::set_disable_shortcuts);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "items", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_items", "_get_items");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "switch_on_hover"), "set_switch_on_hover", "is_switch_on_hover");

	ADD_SIGNAL(MethodInfo("about_to_show"));
}

void MenuButton::set_disable_shortcuts(bool p_disabled) {

	disable_shortcuts = p_disabled;
}

MenuButton::MenuButton() {

	switch_on_hover = false;
	set_flat(true);
	set_toggle_mode(true);
	set_disable_shortcuts(false);
	set_focus_mode(FOCUS_NONE);
	set_process_unhandled_key_input(true);
	set_action_mode(ACTION_MODE_BUTTON_PRESS);

	popup = memnew(PopupMenu);
	popup->hide();
	add_child(popup);
	popup->set_pass_on_modal_close_click(false);
	popup->connect("about_to_show", this, "set_pressed", varray(true)); // For when switching from another MenuButton.
	popup->connect("popup_hide", this, "set_pressed", varray(false));
}

MenuButton::~MenuButton() {
}
