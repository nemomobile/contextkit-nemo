
BEGIN {
        ident = "([0-9_A-Za-z]+)";
        var_name_re = ident;
        prop_name_re = "\"([0-9./_A-Za-z]+)\"";
        type_name_re = ident;
        comma = "[[:space:]]*,[[:space:]]*";
        prefix = "^[[:space:]]*CKIT_PROP[[:space:]]*\\([[:space:]]*";
        eol = "[[:space:]]*\\)[[:space:]]*;[[:space:]]*";
        var_decl = prefix var_name_re comma prop_name_re comma type_name_re eol;

        OFS = "\n";
        if (mode == "main") {
                print "#include <QtCore/QCoreApplication>",
                        "#include <QString>",
                        "#include <contextkit_test.hpp>",
                        "#include <QDebug>";

                printf("#include <contextkit_props/%s.hpp>\n", interface);
                print;

                print "int main(int argc, char *argv[])","{", "    QCoreApplication app(argc, argv);";
        } else {
                print "<?xml version=\"1.0\"?>",
                        "<provider xmlns=\"http://contextkit.freedesktop.org/Provider\"";
                printf("plugin=\"/%s\"\n", provider);
                printf("constructionString=\"%s\">\n", interface);
        }
}

END {
        if (mode == "main") {
                OFS = "\n";
                print "    return app.exec();", "}";
        } else {
                print "</provider>"
        }
}


$0 ~ var_decl {
        sub(prefix, "", $0);
        sub(eol, "", $0);
        count = split($0, parts, comma);
        prop_name = parts[2];
        gsub("\"", "", prop_name);
        type_name = parts[3];
        if (count != 3) {
                print "ERROR";
                exit 1;
        }
        if (mode == "main") {
                printf("    CKitProperty %s_impl(%s, print_%s);\n", parts[1], parts[1], type_name);
        } else {
                printf("<key name=\"%s\"/>\n", prop_name);
        }
}

{
}
