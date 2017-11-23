
CFLAGS  = -Wall -Werror 
CFLAGS += -std=c++11 -I/opt/clang/3_5/include 
LIBS += -L/opt/clang/3_5/lib/ -lclang
# LIBS += -L/opt/clang/3_5/lib/ -l:libclang.a
# LIBS += -static -lclang

EXE = clic_add clic_clear clic_rm clic_query
OBJECTS = build/clic_printer.o build/ClicDb.o

CXX = clang++

.PHONY: 
all: $(EXE)

clic_%: clic_%.cpp $(OBJECTS)
	$(CXX) $(CFLAGS) $(LIBS) -o $@ $< $(OBJECTS)

build/%.o: %.cpp %.hpp | build
	$(CXX) $(CFLAGS) -c -o "$@" "$<"

build/%.o: %.cpp | build
	$(CXX) $(CFLAGS) -c -o "$@" "$<"

build:
	mkdir build

.PHONY: 
clean: 
	rm -r ./build
	rm $(EXE)

.PHONY: 
run: clic_add clic_query
	./clic_add ./tmp ./tmp_i clic_add.cpp -std=c++11 -I/opt/clang/3_5/include -isystem /usr/lib64/gcc/x86_64-suse-linux/4.8/include/ 
	echo "==="
	./clic_query ./ClicDb.cpp 18 15 -i -std=c++11 -I/opt/clang/3_5/include -isystem /usr/lib64/gcc/x86_64-suse-linux/4.8/include/ 
	echo "==="
	./clic_query ./ClicDb.cpp 18 15 -d -std=c++11 -I/opt/clang/3_5/include -isystem /usr/lib64/gcc/x86_64-suse-linux/4.8/include/ 

