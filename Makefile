
include Makefile.in

#############################################################################

INCLUDE = -I $(LIB_DIR)
RPATH = -rpath=$(LIB_DIR)

COPTS += $(INCLUDE)
COPTS += $(DEBUG)
CXXOPTS += $(INCLUDE)
CXXOPTS += $(DEBUG)

###### targets ##############################################################
all: lib
	$(CXX) $(CXXOPTS) main.cpp intecplot.o -Wl,$(RPATH) -L $(LIB_DIR) -lINutils

lib:
	$(CXX) $(CXXOPTS) -c intecplot.cpp


test:
	$(CC) $(COPTS) main.c -Wl,$(RPATH) -L $(LIB_DIR) -lINutils

clean:
	rm -f *.o a.out *.so 

