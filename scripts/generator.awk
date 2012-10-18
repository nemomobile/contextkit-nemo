
BEGIN {
        FS=":"
        if (mode == "test") {
                OFS = "\n";
                print "#include <QtCore/QCoreApplication>",
                        "#include <QString>",
                        "#include <contextkit_test.hpp>",
                        "#include <QDebug>";

                printf("#include <contextkit_props/%s.hpp>\n", interface);
                print;

                print "int main(int argc, char *argv[])",
                        "{",
                        "    QCoreApplication app(argc, argv);";
        } else if (mode == "header") { 
                OFS = ":";
                printf("#ifndef _CONTEXTKIT_PROPS_%s_HPP_\n", toupper(interface))
                printf("#define _CONTEXTKIT_PROPS_%s_HPP_\n", toupper(interface))
        } else if (mode == "xml") {
                OFS = "\n";
                print "<?xml version=\"1.0\"?>",
                        "<provider xmlns=\"http://contextkit.freedesktop.org/Provider\"";
                printf("plugin=\"/%s\"\n", provider);
                printf("constructionString=\"%s\">\n", interface);
        }
}

END {
        OFS = "\n";
        if (mode == "test") {
                print "    return app.exec();", "}";
        } else if (mode == "header") {
                print "#endif"
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
                        printf("    CKitProperty %s_%s_impl(%s_%s, print_%s);\n",
                               interface, var_name, interface, var_name, type_name);
                } else if (mode == "header") {
                        printf("static char const *%s_%s = \"%s\";\n",
                               interface, var_name, prop_name)
                } else if (mode == "xml") {
                        printf("<key name=\"%s\"/>\n", prop_name);
                }
}
