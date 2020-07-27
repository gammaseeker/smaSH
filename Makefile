OBJ = smash.o symbol_table.o job_control.o
CC = gcc
CFLAGS = -g -O2 -Wall -Werror
HEADERS = smash.h symbol_table.h
DEP = symbol_table.o smash.o job_control.o

all: smash
smash: $(DEP) $(HEADERS)
	$(CC) -o smash $(OBJ) 

smash.o: smash.c smash.h
	$(CC) $(CFLAGS) -c smash.c

symbol_table.o: symbol_table.c symbol_table.h
	$(CC) $(CFLAGS) -c symbol_table.c

job_control.o: job_control.c job_control.h
	$(CC) $(CFLAGS) -c job_control.c

clean:
	rm -rf smash
	rm -rf redirectout
	rm -rf redirectin
	rm -rf redirecterr
	rm -rf envp
	rm -rf test_job
	rm -rf grp_test
	rm -rf control
	rm -rf $(OBJ)

redirout:
	$(CC) $(CFLAGS) -o redirectout redirectout.c

redirin:
	$(CC) $(CFLAGS) -o redirectin redirectin.c

redirerr:
	$(CC) $(CFLAGS) -o redirecterr redirecterr.c

envp_test:
	$(CC) $(CFLAGS) -o envp envp.c symbol_table.c

job_test:
	$(CC) $(CFLAGS) -o test_job test_job_control.c job_control.c

grp_test:
	$(CC) -o grp_test grp_test.c

control:
	$(CC) -o control control.c
