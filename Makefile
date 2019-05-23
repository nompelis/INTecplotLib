
include Makefile.in

#############################################################################

INCLUDE = -I $(LIB_DIR)
RPATH = -rpath=$(LIB_DIR)

COPTS += $(INCLUDE)
COPTS += $(DEBUG)
CXXOPTS += $(INCLUDE)
CXXOPTS += $(DEBUG)

###### targets ##############################################################

### (the main target makes use of the "libINtecplot.so" library)
all: lib
	$(CXX) $(CXXOPTS) -I $(LIB_DIR) main.cpp \
         -Wl,$(RPATH) -L $(LIB_DIR) -lINtecplot

### (the software is made into a library: "libINtecplot.so" and makes use of
### the "libINutils.so" library)
lib: utils
	$(CXX) $(CXXOPTS) -I $(LIB_DIR) -c intecplot.cpp
	$(CXX) -shared -Wl,-rpath=$(LIB_DIR),-soname,libINtecplot.so -o libINtecplot.so \
          intecplot.o -lc -L $(LIB_DIR) -lINutils

### (a lean version of the "libINutils.so" library is built here)
utils:
	$(CC) $(COPTS) -c utils.c
	$(CC) -shared -Wl,-soname,libINutils.so  -o libINutils.so \
         utils.o -lc

clean:
	rm -f *.o a.out *.so 

