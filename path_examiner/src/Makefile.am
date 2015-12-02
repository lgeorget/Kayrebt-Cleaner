bin_PROGRAMS=examine_edges
examine_edges_SOURCES=examine_edges.cpp\
		      types.h\
		      converters.h\
		      constraint_parser.h\
		      constraint_parser.cpp
examine_edges_CXXFLAGS = -std=c++11
examine_edges_CPPFLAGS = $(PTHREAD_CPPFLAGS) $(BOOST_CPPFLAGS)
examine_edges_LDFLAGS =  $(BOOST_GRAPH_LDFLAGS) $(BOOST_REGEX_LDFLAGS)
examine_edges_LDADD = $(BOOST_GRAPH_LIBS) $(BOOST_REGEX_LIBS)

all-local:
	/sbin/paxctl -m examine_edges
