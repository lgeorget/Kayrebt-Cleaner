ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS=cleaner
cleaner_SOURCES=\
		main.cpp\
		graphtoBCGtranslator.cpp\
		graphtoBCGtranslator.h\
		types.h
cleaner_CXXFLAGS = -std=c++11
cleaner_CPPFLAGS = $(BOOST_CPPFLAGS) $(CADP_BCG_CPPFLAGS)
cleaner_LDFLAGS = $(BOOST_GRAPH_LDFLAGS) $(BOOST_REGEX_LDFLAGS)
cleaner_LDADD = $(BOOST_GRAPH_LIBS) $(BOOST_REGEX_LIBS)

