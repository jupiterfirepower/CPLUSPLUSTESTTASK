CC=gcc
CPPC=g++
LINK=g++
SRCDIR=.
OBJDIR=obj
LEVEL1_DCACHE_LINESIZE=$(shell getconf LEVEL1_DCACHE_LINESIZE)
FLAGS=-g -Wall -std=c++11 -pthread -DLEVEL1_DCACHE_LINESIZE=$(LEVEL1_DCACHE_LINESIZE)
LIBS=
OBJS=$(OBJDIR)/main.o
EXEC=shared_mutex
DATE=$(shell date +"%Y-%m-%d")

$(EXEC) : $(OBJS)
	$(LINK) $(OBJS) -o $(EXEC) $(FLAGS) $(LIBS)

$(OBJDIR)/main.o: main.cpp shared_mutex.hpp shared_recursive_mutex.hpp  $(OBJDIR)/__setup_obj_dir
	$(CPPC) $(FLAGS) main.cpp -c -o $@

$(OBJDIR)/__setup_obj_dir :
	mkdir -p $(OBJDIR)
	touch $(OBJDIR)/__setup_obj_dir

.PHONY: clean bzip release

clean :
	rm -rf $(OBJDIR)/*.o
	rm -rf $(EXEC)

bzip :
	tar -cvf "$(DATE).$(EXEC).tar" $(SRCDIR)/* Makefile
	bzip2 "$(DATE).$(EXEC).tar"

release : FLAGS +=-O3 -D_RELEASE
release : $(EXEC)