
	function genent(n, s) {
		printf "\t[%s].len = %d,\n", n, length(s);
		printf "\t[%s].num = %s,\n", n, n;
		gsub(/[\\]/, "\\\\", s);
		printf "\t[%s].string = \"%s\",\n", n, s;
	}

	function begent(n) {
		printf "struct _token %s[] = {\n", n;
	}

	function endent() {
		printf "};\n\n";
	}

	BEGIN			{ FS="[	 ,]*"; 
				  printf "\n\n#include \"token.h\"\n\n";
				  begent("start_token_s"); }

	/} start_token_t/	{ endent(); begent("ptype_s"); next; }
	/} ptype_t/		{ endent(); begent("edge_type_s"); next; }
	/} edge_type_t/		{ endent(); begent("node_tag_s"); next; }
	/} node_tag_t/		{ endent(); next; }

	/^\t.*UNKNOWN/		{ genent($2,"unknown"); next; }
	/^\tTOKEN_/		{ genent($2,$4); next; }
	/^\tPTYPE_/		{ genent($2,$4); next; }
	/^\tEDGE_/		{ genent($2,$4); next; }
	/^\tNODE_/		{ genent($2,$4); next; }
	
	END			{ printf "\n\n"; }
