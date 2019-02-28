
FLAGS=-DVERBOSE -Wall -O3 -std=c++11

all:  align dump paths ComputeBP.class UnfoldPN.class 

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

dump : dump.cc libbpm.a
	g++ -o dump dump.cc -lbpm $(FLAGS) -L.
	cp dump ../bin

ComputeBP.class : ComputeBP.java
	javac -cp ../jbpt/jbpt-bp-0.3.1.jar:../jbpt/jbpt-core-0.3.1.jar:../jbpt/jbpt-petri-0.3.1.jar ComputeBP.java
	cp ComputeBP.class ../bin

UnfoldPN.class : UnfoldPN.java
	javac -cp ../jbpt/jbpt-bp-0.3.1.jar:../jbpt/jbpt-core-0.3.1.jar:../jbpt/jbpt-petri-0.3.1.jar UnfoldPN.java
	cp UnfoldPN.class ../bin

clean:
	rm -f align paths dump *.o *.a *.class
	rm -f ../bin/align ../bin/paths ../bin/dump ../bin/*.class
