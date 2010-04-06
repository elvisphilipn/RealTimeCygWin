########## The following definition must be set for uCOS >= V2.8x #############
CFLAGS=-DNO_TYPEDEF_OS_FLAGS

########## Path for uCOS-II core source files #################################
UCOS_SRC=./ucosii

########## Path for uCOS-II WIN32 port source files ###########################
UCOS_PORT_SRC=./ucosii_port

########## Path for uCOS-II WIN32 example source files ########################
UCOS_PORT_EX=.

########## Path for uCOS-II WIN32 port library file ###########################
UCOS_PORT_LIB=$(UCOS_PORT_EX)

########## Name of Example source file ########################################
EXAMPLE=SEG4145_Assignment4

all:	$(EXAMPLE).c  app_cfg.h  os_cfg.h  includes.h  $(UCOS_PORT_LIB)/ucos_ii.a Makefile
	@echo --- Recompiling the application ---
	@gcc -D__WIN32__ $(CFLAGS) -I$(UCOS_SRC) -I$(UCOS_PORT_SRC) -I$(UCOS_PORT_EX) $(EXAMPLE).c $(UCOS_PORT_LIB)/ucos_ii.a /usr/lib/w32api/libwinmm.a /usr/lib/mingw/libcoldname.a  -o $(EXAMPLE).exe

$(UCOS_PORT_LIB)/ucos_ii.a:	os_cfg.h
	@echo --- Recompiling library ucos_ii.a ---
	@gcc -c -D__WIN32__ $(CFLAGS) -I$(UCOS_SRC) -I$(UCOS_PORT_SRC) -I$(UCOS_PORT_EX) $(UCOS_SRC)/ucos_ii.c $(UCOS_PORT_SRC)/pc.c $(UCOS_PORT_SRC)/os_cpu_c.c
	@ar -r $(UCOS_PORT_LIB)/ucos_ii.a ucos_ii.o os_cpu_c.o pc.o

clean:
	@rm -f *.o
	@rm -f *.bak

mrproper:	distclean

distclean:
	@rm -f *.o
	@rm -f *.bak
	@rm -f *.a
	@rm -f *.exe
