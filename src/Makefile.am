ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS=cleaner
cleaner_SOURCES=\
		main.cpp\
		graphtoBCGtranslator.cpp\
		graphtoBCGtranslator.h\
		types.h
cleaner_CXXFLAGS = -std=c++11
cleaner_CPPFLAGS = $(PTHREAD_CPPFLAGS) $(BOOST_CPPFLAGS) $(CADP_BCG_CPPFLAGS)
cleaner_LDFLAGS = $(CADP_BCG_LDFLAGS) $(BOOST_GRAPH_LDFLAGS) $(BOOST_REGEX_LDFLAGS)
# we have to pass PTHREAD_CPPFLAGS here, because on some architecture, the compiler
# just requires -pthread and not a particular -l... option
cleaner_LDADD = $(PTHREAD_CPPFLAGS) $(PTHREAD_LIBS) $(CADP_BCG_LIBS) $(BOOST_GRAPH_LIBS) $(BOOST_REGEX_LIBS)

