
CFLAGS  = -Wall -Werror 
CFLAGS += -std=c++11 

# INCLUDES = -I/opt/clang/3_5/include 
INCLUDES = -I/usr/lib/llvm-3.8/include
CFLAGS += $(INCLUDES)

# SYSTEM_INC = /usr/lib64/gcc/x86_64-suse-linux/4.8/include/ 
SYSTEM_INC = /usr/lib/gcc/x86_64-linux-gnu/4.8/include/ 

# LIBS += -L/opt/clang/3_5/lib/ 
LIBS += -lclang

EXE = clic_add clic_clear clic_rm clic_query
CPP_FILES=ClicDb Location Reference
OBJ_FILES=obj/sqlite3.o

OBJ_FILES+=$(addprefix obj/, $(addsuffix .o,${CPP_FILES}))

C = clang
CXX = clang++

.PHONY: 
all: $(EXE) 

clic_%: clic_%.cpp types.hpp $(OBJ_FILES) | sep
	$(CXX) $(CFLAGS) $(LIBS) -o $@ $< $(OBJ_FILES)

.PRECIOUS: obj/%.o
obj/%.o: %.cpp %.hpp | obj
	$(CXX) $(CFLAGS) -c -o "$@" "$<"

obj/%.o: %.cpp | obj
	$(CXX) $(CFLAGS) -c -o "$@" "$<"

obj/sqlite3.o: sqlite3.c sqlite3.h | obj
	$(C) -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -c -o "$@" "$<"

obj:
	mkdir obj

.PHONY: 
clean: 
	rm -rf ./obj
	rm -f $(EXE)

.PHONY: 
run: all
	./clic_add ./tmp ClicDb.cpp $(CFLAGS) -isystem $(SYSTEM_INC)
	echo "==="
	./clic_query ./ClicDb.cpp 18 15 -i $(CFLAGS) -isystem $(SYSTEM_INC)
	echo "==="
	./clic_query ./ClicDb.cpp 18 15 -d $(CFLAGS) -isystem $(SYSTEM_INC)

PHONY: sep
sep:
	@echo ==============================================================================================
	@echo ==============================================================================================
