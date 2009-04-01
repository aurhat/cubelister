
AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\"

CLEANFILES = Csl.h.gch

noinst_HEADERS = Csl.h

if ENABLE_PCH
noinst_PROGRAMS = dummy
dummy_SOURCES = dummy.cpp

noinst_DATA = Csl.h.gch

include ./$(DEPDIR)/Csl.Po

%.h.gch: %.h
	if $(CXXCOMPILE) $(CXXFLAGS) -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" -c -o $@ $<; \
	then \
	    mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else \
	    rm -f "$(DEPDIR)/$*.Tpo"; exit 1; fi
endif

copyheader:
	if [ "$(abs_srcdir)" != "$(abs_builddir)" ]; then \
		cp -a $(srcdir)/Csl.h $(builddir)/; \
	fi

all: copyheader
