
BEGIN {
        FS=":"
        if (mode == "test") {
                OFS = "\n";
                print "#include <QtCore/QCoreApplication>",
                        "#include <QString>",
                        "#include <contextkit_test.hpp>",
                        "#include <QDebug>",
                        sprintf("#include <contextkit_props/%s.hpp>\n",
                                interface),
                        sprintf("namespace ns = contextkit::%s;",
                                interface);

                print "int main(int argc, char *argv[])",
                        "{",
                        "    QCoreApplication app(argc, argv);";
        } else if (mode == "header") { 
                OFS = "\n";
                guard = sprintf("_CONTEXTKIT_PROPS_%s_HPP_", toupper(interface))
                print "#ifndef " guard, "#define " guard,
                        "namespace contextkit {",
                        sprintf("namespace %s {\n", interface);
                current_id = 0
        } else if (mode == "xml") {
                OFS = "\n";
                print "<?xml version=\"1.0\"?>",
                        "<provider xmlns=" \
                        "\"http://contextkit.freedesktop.org/Provider\"",
                        sprintf("plugin=\"/%s\"", provider),
                        sprintf("constructionString=\"%s\"", interface),
                        ">";
        }
}

END {
        OFS = "\n";
        if (mode == "test") {
                print "    return app.exec();", "}";
        } else if (mode == "header") {

                for (i = 0; i < current_id; ++i)
                        print char_data[i];

                print "}", "}", "#endif"
        } else if (mode == "xml") { 
                print "</provider>"
        }
}

/#/ { }

/^$/ { }

! /^#/ && NF > 3 {
                prop_name = $1
                var_name = $2
                type_name = $3
                comment = $4
                if (mode == "test") {
                        printf("    CKitProperty %s_impl(ns::%s, print_%s);\n",
                               var_name, var_name, type_name);
                } else if (mode == "header") {
                        char_data[current_id] \
                                = sprintf("static char const *%s = \"%s\";",
                                          var_name, prop_name)
                        ++current_id;
                } else if (mode == "xml") {
                        printf("<key name=\"%s\"/>\n", prop_name);
                }
}
