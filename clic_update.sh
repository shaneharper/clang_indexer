#!/bin/bash
# $1: source path
# $2: build path (where to look for .clang_complete)


usage() {
    echo "USAGE: $0 SOURCE_PATH [BUILD_PATH]"
}
if [ $# -lt 1 ] ; then
    usage
    exit -1
fi

# Specify files to index here
SOURCE_PATH=`cd $1; pwd` # convert $1 to an absolute path
find $SOURCE_PATH\
    -name "*.cpp" -or\
    -name "*.hpp" -or\
    -name "*.cxx" -or\
    -name "*.hxx" -or\
    -name "*.cc" -or\
    -name "*.c" -or\
    -name "*.h"\
    | grep -v /cmake/\
    | sort > files2.txt


if [ $# -ge 2 ] ; then
    ADD_CXX_FLAGS_FILE=$2/.clang_complete
else
    ADD_CXX_FLAGS_FILE=${SOURCE_PATH}/.clang_complete
fi
ADD_CXX_FLAGS=''
if [ -e $ADD_CXX_FLAGS_FILE ] ; then
    ADD_CXX_FLAGS=`cat ${ADD_CXX_FLAGS_FILE}`
fi

# + headers files that libclang isn't able to find by itself
ADD_CXX_FLAGS="${ADD_CXX_FLAGS} -isystem /home/lhermitte/local/lib/clang/3.3/include/"

filename_for_user() {
    local FILENAME=$1
    echo ${FILENAME:${#SOURCE_PATH} + 1}
}

add_to_index() {
    INDEX_FILE=`echo ${1}.i.gz | tr "/" "%"`
    echo "Adding '`filename_for_user $1`'"
    # echo clic_add index.db $INDEX_FILE $ADD_CXX_FLAGS $1
    clic_add index.db $INDEX_FILE $ADD_CXX_FLAGS $1 2>&1 |
        grep -v "'linker' input unused when '-fsyntax-only' is present" |
        sed 's/^/\t/'
}

remove_from_index() {
    INDEX_FILE=`echo ${1}.i.gz | tr "/" "%"`
    echo "Removing '`filename_for_user $1`'"
    # echo clic_rm index.db $INDEX_FILE
    clic_rm index.db $INDEX_FILE
    echo rm $INDEX_FILE
    rm $INDEX_FILE
}

if [ ! -f index.db -o ! -f files.txt ]; then
    echo "Creating database from ${ADD_CXX_FLAGS_FILE} with options: ${ADD_CXX_FLAGS}"
    clic_clear index.db
    for i in `cat files2.txt`; do
        add_to_index $i
    done
    mv files2.txt files.txt
    exit
fi

echo "Updating database"
#Generate the list of files added since last time
comm -23 files2.txt files.txt > filesadded.txt

for i in `cat filesadded.txt`; do
    add_to_index $i
done

# Remove removed files
for i in `comm -23 files.txt files2.txt`; do
    remove_from_index $i
done

# Update modified files
for i in `cat files.txt`; do
    if [ -f $i -a $i -nt files.txt ]; then
        remove_from_index $i
        add_to_index $i
    fi
done

rm filesadded.txt
mv files2.txt files.txt
