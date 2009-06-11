# $Id: Makefile,v 1.13 2005/07/09 12:17:03 heinz Exp $

FINDLIB=ocamlfind
OCAMLC_OPS=-syntax camlp4o -package camlp4

# opt
OCAMLC = ocamlopt $(OCAMLC_OPS)
LIBSUFFIX = cmxa
OBJSUFFIX = cmx

# bytecode
#OCAMLC = ocamlc -g $(OCAMLC_OPS)
#LIBSUFFIX = cma
#OBJSUFFIX = cmo

BINARIES = tsim hsim totest noopCop handtaschendieb resi
LIBRARIES = str unix 
MODULES = debug lexer world unparser parser pqueue adjgraph worldmap timeout \
	  utils coploop robberTrackingUtils constants gravity

all: $(BINARIES)

%.mli: %.ml
	-$(FINDLIB) $(OCAMLC) -c -i $< > $@

%.$(OBJSUFFIX): %.ml
	$(FINDLIB) $(OCAMLC) -c $<

%.cmi: %.mli
	$(FINDLIB) $(OCAMLC) -c $<

hsim.$(OBJSUFFIX): hsim.ml
	$(FINDLIB) $(OCAMLC) -package lablgtk2 -linkpkg -c $<

$(addsuffix .$(OBJSUFFIX), $(BINARIES)): $(addsuffix .$(OBJSUFFIX), $(MODULES)) $(addsuffix .cmi, $(MODULES))

%: %.$(OBJSUFFIX)
	$(FINDLIB) $(OCAMLC) $(addsuffix .$(LIBSUFFIX), $(LIBRARIES)) $(addsuffix .$(OBJSUFFIX), $(MODULES)) $^ -o $@

hsim: hsim.$(OBJSUFFIX) hsim.$(OBJSUFFIX)
	$(FINDLIB) $(OCAMLC) -package lablgtk2 -linkpkg $(addsuffix .$(LIBSUFFIX), $(LIBRARIES)) $(addsuffix .$(OBJSUFFIX), $(MODULES)) $^ -o $@


clean:
	rm -f *~ *.$(OBJSUFFIX) *.cmi *.o *.cmo *.mli  $(BINARIES)

.PHONY: all opt bytecode
