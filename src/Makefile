
FLAGS=-DVERBOSE -Wall -O3 -std=c++11

all:  align dump paths accessibility compute-bps

libbpm.a : graph.o bp.o alignment.o config.o traces.o relax.o util.o pugixml.o 
	ar -rs libbpm.a graph.o bp.o alignment.o config.o traces.o relax.o util.o pugixml.o

pugixml.o : pugixml.cpp pugiconfig.hpp pugixml.hpp
	g++ -c -o pugixml.o pugixml.cpp $(FLAGS)

graph.o : graph.cc graph.h util.h alignment.h
	g++ -c -o graph.o graph.cc $(FLAGS)

bp.o : bp.cc bp.h
	g++ -c -o bp.o bp.cc $(FLAGS)

alignment.o : alignment.cc alignment.h
	g++ -c -o alignment.o alignment.cc $(FLAGS)

traces.o : traces.cc traces.h
	g++ -c -o traces.o traces.cc $(FLAGS)

relax.o : relax.cc relax.h
	g++ -c -o relax.o relax.cc $(FLAGS)

util.o : util.cc util.h
	g++ -c -o util.o util.cc $(FLAGS)

config.o : config.cc config.h
	g++ -c -o config.o config.cc $(FLAGS)

align : align.cc libbpm.a
	g++ -o align align.cc -lbpm $(FLAGS) -L.
	cp align ../bin

paths : paths.cc libbpm.a
	g++ -o paths paths.cc -lbpm $(FLAGS) -L.
	cp paths ../bin

accessibility : accessibility.cc libbpm.a
	g++ -o accessibility accessibility.cc -lbpm $(FLAGS) -L.
	cp accessibility ../bin

compute-bps : compute-bps.cc libbpm.a
	g++ -o compute-bps compute-bps.cc -lbpm $(FLAGS) -L.
	cp compute-bps ../bin

dump : dump.cc libbpm.a
	g++ -o dump dump.cc -lbpm $(FLAGS) -L.
	cp dump ../bin

clean:
	rm -f align dump paths accessibility compute-bps *.o *.a
