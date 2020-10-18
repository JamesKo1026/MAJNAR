CC=gcc
CP=g++

CFLAGS=-c -Wall

all: MAJNAR

MAJNAR: main.o st.o Basic.o ReadNtk.o WriteNtk.o FindDom.o Imply.o NodeMGR.o Simplify.o RecurImply.o Checking.o NodeAR.o
		$(CP) -O3 -std=c11 -o MAJNAR main.o st.o Basic.o ReadNtk.o WriteNtk.o FindDom.o Imply.o NodeMGR.o Simplify.o RecurImply.o Checking.o NodeAR.o

main.o: main.cpp
		$(CP) $(CFLAGS) main.cpp
		
st.o: st.c
		$(CC) $(CFLAGS) st.c
		
Basic.o: Basic.cpp
		$(CP) $(CFLAGS) Basic.cpp
		
ReadNtk.o: ReadNtk.cpp
		$(CP) $(CFLAGS) ReadNtk.cpp
		
WriteNtk.o: WriteNtk.cpp
		$(CP) $(CFLAGS) WriteNtk.cpp
		
FindDom.o: FindDom.cpp
		$(CP) $(CFLAGS) FindDom.cpp
		
Imply.o: Imply.cpp
		$(CP) $(CFLAGS) Imply.cpp
		
NodeMGR.o: NodeMGR.cpp
		$(CP) $(CFLAGS) NodeMGR.cpp
		
Simplify.o: Simplify.cpp
		$(CP) $(CFLAGS) Simplify.cpp
		
RecurImply.o: RecurImply.cpp
		$(CP) $(CFLAGS) RecurImply.cpp
		
Checking.o: Checking.cpp
		$(CP) $(CFLAGS) Checking.cpp

NodeAR.o: NodeAR.cpp
		$(CP) $(CFLAGS) NodeAR.cpp
		
clean:
		rm -rf *o MAJNAR
