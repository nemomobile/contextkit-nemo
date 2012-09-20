BEGIN {
        provider_up = toupper(PROVIDER)
        print "<?xml version=\"1.0\"?>"
        print "<provider xmlns=\"http://contextkit.freedesktop.org/Provider\""
        printf("plugin=\"/%s\"\n", PROVIDER)
        print  "constructionString=\"battery\">\n"

}

/ *static +char +const.+\/\/PROPERTY */ {
        sub(/^.+= *"/, "", $0)
        sub(/" *;.+$/, "", $0)
        printf("<key name=\"%s\"/>\n", $0)
}

/^$/ {}

END {
        print "</provider>"
}
