/*************************************************************************/
/*  gdscript_text_document.h                                             */
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

#ifndef GDSCRIPT_TEXT_DOCUMENT_H
#define GDSCRIPT_TEXT_DOCUMENT_H

#include "core/os/file_access.h"
#include "core/reference.h"
#include "lsp.hpp"

class GDScriptTextDocument : public Reference {
	GDCLASS(GDScriptTextDocument, Reference)
protected:
	static void _bind_methods();

	FileAccess *file_checker;

	void didOpen(const Variant &p_param);
	void didClose(const Variant &p_param);
	void didChange(const Variant &p_param);

	void sync_script_content(const String &p_path, const String &p_content);
	void show_native_symbol_in_editor(const String &p_symbol_id);

	Array native_member_completions;

private:
	Array find_symbols(const lsp::TextDocumentPositionParams &p_location, List<const lsp::DocumentSymbol *> &r_list);
	lsp::TextDocumentItem load_document_item(const Variant &p_param);
	void notify_client_show_symbol(const lsp::DocumentSymbol *symbol);

public:
	Variant nativeSymbol(const Dictionary &p_params);
	Array documentSymbol(const Dictionary &p_params);
	Array completion(const Dictionary &p_params);
	Dictionary resolve(const Dictionary &p_params);
	Array foldingRange(const Dictionary &p_params);
	Array codeLens(const Dictionary &p_params);
	Array documentLink(const Dictionary &p_params);
	Array colorPresentation(const Dictionary &p_params);
	Variant hover(const Dictionary &p_params);
	Array definition(const Dictionary &p_params);
	Variant declaration(const Dictionary &p_params);
	Variant signatureHelp(const Dictionary &p_params);

	void initialize();

	GDScriptTextDocument();
	virtual ~GDScriptTextDocument();
};

#endif
