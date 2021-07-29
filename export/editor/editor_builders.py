"""Functions used to generate source files during build time

All such functions are invoked in a subprocess on Windows to prevent build flakiness.

"""
import os
import os.path
from platform_methods import subprocess_main
from compat import encode_utf8, byte_to_str, open_utf8


def make_doc_header(target, source, env):

    dst = target[0]
    g = open_utf8(dst, "w")
    buf = ""
    docbegin = ""
    docend = ""
    for src in source:
        if not src.endswith(".xml"):
            continue
        with open_utf8(src, "r") as f:
            content = f.read()
        buf += content

    buf = encode_utf8(docbegin + buf + docend)
    decomp_size = len(buf)
    import zlib

    buf = zlib.compress(buf)

    g.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    g.write("#ifndef _DOC_DATA_RAW_H\n")
    g.write("#define _DOC_DATA_RAW_H\n")
    g.write("static const int _doc_data_compressed_size = " + str(len(buf)) + ";\n")
    g.write("static const int _doc_data_uncompressed_size = " + str(decomp_size) + ";\n")
    g.write("static const unsigned char _doc_data_compressed[] = {\n")
    for i in range(len(buf)):
        g.write("\t" + byte_to_str(buf[i]) + ",\n")
    g.write("};\n")

    g.write("#endif")

    g.close()


def make_fonts_header(target, source, env):

    dst = target[0]

    g = open_utf8(dst, "w")

    g.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    g.write("#ifndef _EDITOR_FONTS_H\n")
    g.write("#define _EDITOR_FONTS_H\n")

    # saving uncompressed, since freetype will reference from memory pointer
    xl_names = []
    for i in range(len(source)):
        with open(source[i], "rb") as f:
            buf = f.read()

        name = os.path.splitext(os.path.basename(source[i]))[0]

        g.write("static const int _font_" + name + "_size = " + str(len(buf)) + ";\n")
        g.write("static const unsigned char _font_" + name + "[] = {\n")
        for j in range(len(buf)):
            g.write("\t" + byte_to_str(buf[j]) + ",\n")

        g.write("};\n")

    g.write("#endif")

    g.close()


def make_translations_header(target, source, env):

    dst = target[0]

    g = open_utf8(dst, "w")

    g.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n")
    g.write("#ifndef _EDITOR_TRANSLATIONS_H\n")
    g.write("#define _EDITOR_TRANSLATIONS_H\n")

    import zlib
    import os.path

    sorted_paths = sorted(source, key=lambda path: os.path.splitext(os.path.basename(path))[0])

    xl_names = []
    for i in range(len(sorted_paths)):
        with open(sorted_paths[i], "rb") as f:
            buf = f.read()
        decomp_size = len(buf)
        buf = zlib.compress(buf)
        name = os.path.splitext(os.path.basename(sorted_paths[i]))[0]

        g.write("static const unsigned char _translation_" + name + "_compressed[] = {\n")
        for j in range(len(buf)):
            g.write("\t" + byte_to_str(buf[j]) + ",\n")

        g.write("};\n")

        xl_names.append([name, len(buf), str(decomp_size)])

    g.write("struct EditorTranslationList {\n")
    g.write("\tconst char* lang;\n")
    g.write("\tint comp_size;\n")
    g.write("\tint uncomp_size;\n")
    g.write("\tconst unsigned char* data;\n")
    g.write("};\n\n")
    g.write("static EditorTranslationList _editor_translations[] = {\n")
    for x in xl_names:
        g.write('\t{ "' + x[0] + '", ' + str(x[1]) + ", " + str(x[2]) + ", _translation_" + x[0] + "_compressed},\n")
    g.write("\t{NULL, 0, 0, NULL}\n")
    g.write("};\n")

    g.write("#endif")

    g.close()


if __name__ == "__main__":
    subprocess_main(globals())
