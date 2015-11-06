ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS=cleaner grapher includer expander
cleaner_SOURCES=\
		cleaner.cpp\
		graphtoBCGtranslator.cpp\
		graphtoBCGtranslator.h\
		types.h
grapher_SOURCES=\
		grapher.cpp\
		BCGtographtranslator.cpp\
		BCGtographtranslator.h\
		types.h
includer_SOURCES=\
		 includer.cpp\
		types.h
expander_SOURCES=\
		 expander.cpp\
		 diagram_marker.cpp\
		 diagram_marker.h\
		 decider.cpp\
		 decider.h\
		 mark.h\
		 mark.cpp\
		 types.h
AM_CXXFLAGS = -std=c++11
AM_CPPFLAGS = $(PTHREAD_CPPFLAGS) $(BOOST_CPPFLAGS) $(CADP_BCG_CPPFLAGS)
AM_LDFLAGS = $(CADP_BCG_LDFLAGS) $(BOOST_GRAPH_LDFLAGS) $(BOOST_REGEX_LDFLAGS)
includer_LDFLAGS =  $(BOOST_GRAPH_LDFLAGS) $(BOOST_REGEX_LDFLAGS)
expander_LDFLAGS =  $(BOOST_GRAPH_LDFLAGS) $(BOOST_REGEX_LDFLAGS)
# we have to pass PTHREAD_CPPFLAGS here, because on some architecture, the compiler
# just requires -pthread and not a particular -l... option
cleaner_LDADD = $(PTHREAD_CPPFLAGS) $(PTHREAD_LIBS) $(CADP_BCG_LIBS) $(BOOST_GRAPH_LIBS) $(BOOST_REGEX_LIBS)
grapher_LDADD = $(PTHREAD_CPPFLAGS) $(PTHREAD_LIBS) $(CADP_BCG_LIBS) $(BOOST_GRAPH_LIBS) $(BOOST_REGEX_LIBS)
includer_LDADD = $(PTHREAD_CPPFLAGS) $(PTHREAD_LIBS) $(CADP_BCG_LIBS) $(BOOST_GRAPH_LIBS) $(BOOST_REGEX_LIBS)
expander_LDADD = $(BOOST_GRAPH_LIBS) $(BOOST_REGEX_LIBS)

