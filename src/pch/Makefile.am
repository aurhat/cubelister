
CLEANFILES = Csl.h.gch


if ENABLE_PCH
noinst_DATA = Csl.h.gch

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/engine \
	-I$(top_srcdir)/src/engine/tools -I$(top_srcdir)/src/tools -I$(top_builddir)/src
#AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)/csl\"

CXXCOMPILE = $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)

include ./$(DEPDIR)/Csl.Po

$(DEPDIR)/Csl.h.Po:
	if test ! -d "$(DEPDIR)" \
	then \
	    mkdir $(DEPDIR); \
	fi
	touch $(DEPDIR)/Csl.h.Po

%.h.gch: %.h
	if $(CXXCOMPILE) -x c++-header -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" -c -o $@ $<; \
	then \
	    mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else \
	    rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi
endif

copyheader:
	if [ "$(abs_srcdir)" != "$(abs_builddir)" ]; then \
	    cp -p $(srcdir)/Csl.h $(builddir)/; \
	fi

all: copyheader
include_HEADERS = Csl.h
