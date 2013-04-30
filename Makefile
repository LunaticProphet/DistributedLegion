PVM_HOME=.
PVM_DIR_SRC=src
PVM_DIR_BIN=bin
PVM_BIN=$(PVM_HOME)/$(PVM_DIR_BIN)
PVM_SRC=$(PVM_HOME)/$(PVM_DIR_SRC)
PVMINC=$(PVM_ROOT)/include
PVMLIB=$(PVM_ROOT)/lib/$(PVM_ARCH) 

all:	$(PVM_BIN)/master $(PVM_BIN)/slave

run:
	$(PVM_BIN)/master

$(PVM_BIN)/master:	$(PVM_DIR_SRC)/master.c $(PVM_DIR_SRC)/def.h
	cc -g $(PVM_DIR_SRC)/master.c -o $(PVM_BIN)/master -L$(PVMLIB) -I$(PVMINC) -lpvm3 -lgpvm3

$(PVM_BIN)/slave:	$(PVM_DIR_SRC)/slave.c $(PVM_DIR_SRC)/def.h
	cc -g $(PVM_DIR_SRC)/slave.c -o $(PVM_BIN)/slave -L$(PVMLIB) -I$(PVMINC) -lpvm3 -lgpvm3

clean:
	rm $(PVM_BIN)/master $(PVM_BIN)/slave
	rm -f *.o

