PVM_ARCH=LINUX64
PVM_HOME=../bin/$(PVM_ARCH)
PVM_SLAVE=$(realpath $(PVM_HOME))/slave 
PVM_ROOT=/usr/lib/pvm3
PVMINC=$(PVM_ROOT)/include
PVMLIB=$(PVM_ROOT)/lib/$(PVM_ARCH)

all:	$(PVM_HOME)/master $(PVM_HOME)/slave

run:
	$(PVM_HOME)/master $(PVM_SLAVE)

$(PVM_HOME)/master:	master.c def.h
	cc -g master.c -o $(PVM_HOME)/master -L$(PVMLIB) -I$(PVMINC) -lpvm3 -lgpvm3 -lm

$(PVM_HOME)/slave:	slave.c def.h
	cc -g slave.c -o $(PVM_HOME)/slave -L$(PVMLIB) -I$(PVMINC) -lpvm3 -lgpvm3 -lm

clean:
	rm $(PVM_HOME)/master $(PVM_HOME)/slave
	rm -f *.o
