#!/usr/bin/python
# File:       clic_update.py                                            {{{1
# Maintainer: Hermitte <EMAIL:hermitte {at} free {dot} fr>
#                      <URL:https://github.com/LucHermitte/clang_indexer>
# Purpose:                                                     {{{2
#     The purpose of this script is to automate the generation and the update of
#     the clang_indexer database associated to source files.
#     The script automatically fetchs the compilation options to use from a
#     compile_commands.json file. compile_commands.json is a file that is meant
#     to be automatically generated when executing
#         cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ${compilation_dir}
#
# Copying:                                                     {{{2
#   Copyright 2013 Luc Hermitte
#     This program is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Usage:                                                       {{{2
#     Run with -h to get the usage.
#
# Todo:                                                        {{{2
# - Option to specify how to find a matching C++ file (see the alternatepath
#   option of a.vim)
# - Option to specify the usual header/inline file extensions
# - Version string, (c)
# - Verbose/quiet
# - Ignore files under {project-root}/cmake/
# - Execute on only one file (a non-header file)
# - Execute on a header file and all the files that include it.

## Imports {{{1
from subprocess import Popen
from subprocess import call
import glob
import json
import os.path
import re
import string
import subprocess

## globals {{{1
included_files_extentions = ['h', 'hpp', 'hxx', 'H', 'inl', 'txx']
source_files_extentions   = ['c', 'cpp', 'cxx', 'C']


## find_clang_include_files {{{1
# While clang executable knows where to find its system include files, libclang
# v3.* can't. Hence this little hack to help find it.
def find_clang_include_dir():
    if global_options.libclang_path==None:
        return None
    library_dir = global_options.libclang_path + "/../lib/clang"
    library_dir = os.path.normpath(library_dir)
    include_dirs = sorted(glob.glob(library_dir+'/*/include'))
    if len(include_dirs)==0:
        return None
    include_path = include_dirs[-1]
    return include_path

## remove_from_index {{{1
def do_remove_from_index(index_filename):
    # TODO: check the file exists
    command = ["clic_rm", "index.db", index_filename]
    call (command)

def remove_from_index(filename):
    index_filename = string.replace(filename, '/', '%')+'.i.gz'
    print "Removing '"+os.path.basename(filename)+"'"
    do_remove_from_index(index_filename)
    command = ["rm", index_filename]
    call (command)


## add_to_index {{{1
# re_valid_error = re.compile("'linker' input unused when '-fsyntax-only' is present")
re_empty = re.compile("^\s*$")
re_valid_error = re.compile(".*'linker' input unused .*")

def do_add_to_index(index_filename, filename, flags):
    command = ["clic_add", "index.db", index_filename]+flags+[filename]
    # print command
    # call(command, shell=False) 
    output = Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=False) 
    result = output.stdout.read().splitlines()
    result = filter(lambda l: not re_empty.match(l) and not re_valid_error.match(l), result)
    result = map(lambda l: "\t"+l, result)
    str_result = "\n".join(result)
    if len(str_result)>0:
        print str_result

def add_to_index(filename, flags):
    index_filename = string.replace(filename, '/', '%')+'.i.gz'
    print "Adding '"+os.path.basename(filename)+"'"
    do_remove_from_index(index_filename)
    do_add_to_index(index_filename, filename, flags)

## extract_compile_flags {{{1
re_valid_flag1 = re.compile('^-I|^-D|^-std|^-no')
re_valid_flag2 = re.compile('^-i')
def extract_compile_flags(flags):
    # + headers files that libclang isn't able to find by itself
    result = []
    include_path = find_clang_include_dir()
    if include_path != None:
        result += ['-isystem', include_path]
    if global_options.compile_options!=None and len(global_options.compile_options)>0:
        result += global_options.compile_options
    options=flags.split(' ')
    use_next=False
    for option in options:
        if use_next:
            result += [option]
            use_next=False
        elif re_valid_flag1.match(option):
            result += [option]
        elif re_valid_flag2.match(option):
            result += [option]
            use_next=True
        #endif
    #endfor
    return result

## find_matching_source_file {{{1
# todo: use vim's alternate format to find source files
re_match_cpp_extension = re.compile(".*\.("+ "|".join(source_files_extentions)+")$" )
def find_matching_source_file(included_file):
    (root, ext) = os.path.splitext(included_file)
    files = glob.glob(root+".*")
    files = filter(lambda f: re_match_cpp_extension.match(f), files)
    return files[0] if len(files)>0 else None

## update_index_with_flags(compile_commands) {{{1
def update_index_with_flags(compile_flags, filename):
    index_filename = string.replace(filename, '/', '%')+'.i.gz'
    if (not os.path.exists(index_filename)) or os.path.getmtime(index_filename) < os.path.getmtime(filename):
        # print compile_flags
        add_to_index(filename, compile_flags)

## update_index(compile_commands) {{{1
def update_index(command, filename):
    compile_flags=extract_compile_flags(command)
    update_index_with_flags(compile_flags, filename)

## parse_source_files(compile_commands) {{{1
def parse_source_files(compile_commands):
    for compile_command in compile_commands:
        # test pour ne pas parser ce qui est already connu
        # compile_flags=extract_compile_flags(compile_command['command'])
        # add_to_index(compile_command['file'], compile_flags)
        update_index(compile_command['command'], compile_command['file'])

## build_compile_commands_per_directory(compile_commands) {{{1
def build_compile_commands_per_directory(compile_commands):
    res = {}
    for compile_command in compile_commands:
        file = compile_command['file']
        if global_options.excluded_files_for_options!=None and os.path.basename(file) in global_options.excluded_files_for_options:
            continue
        dir = os.path.dirname(file)
        options = extract_compile_flags(compile_command['command'])
        if res.has_key(dir):
            if options != res[dir]:
                # print "options mismatching for", compile_command['file'], "->", res[dir], "!=", options
                res[dir] = ""
        else:
            res[dir] = options
    return res


## Main {{{1
# Check options  {{{2
from optparse import OptionParser
usage="%prog source_path compile_command_json_path"
parser = OptionParser(usage)
parser.add_option("-c", "--compile_option", dest="compile_options",
                  help="Option to pass to libclang to parse the source files. Can be use multiple times.",
                  metavar="C(XX)FLAGS", action="append"
                  )
parser.add_option("-l", "--libclang_path", dest="libclang_path",
                  help="Where libclang is",
                  metavar="PATH", action="store"
                  )
parser.add_option("-x", "--exclude-file", dest="excluded_files_for_options",
                  help="Files to exclude when trying to determine global compilation options",
                  metavar="PATH", action="append"
                  )
(global_options, args) = parser.parse_args()

if len(args) < 2:
    parser.error("Invalid number of arguments supplied")

source_path=os.path.abspath(args[0])
if not os.path.exists(source_path):
    parser.error("Source path "+ source_path+ " does not exist.") 
elif not os.path.isdir(source_path):
    parser.error("Source path "+ source_path+ " is not a directory.") 

compile_command_filename = args[1]
if not os.path.exists(compile_command_filename):
    parser.error(compile_command_filename+ " does not exist.") 

# Parse compile_command_filename {{{2
compile_command_file = open(compile_command_filename)
compile_commands = json.load(compile_command_file)

# Iterate over source files (.c, .cpp) {{{2
parse_source_files(compile_commands)

# Determine compile options common to all .cpp files in a directory
compile_commands_per_directory = build_compile_commands_per_directory(compile_commands)
# print compile_commands_per_directory

# Header files {{{2
# For each .h, .hpp, ... file, search for the matching .cpp to take its
# compilation options
re_match_included_extension = re.compile(".*\.("+ "|".join(included_files_extentions)+")$" )
included_files = []
for root, dirs, files in os.walk(source_path):
    new_files = filter(lambda f: re_match_included_extension.match(f), files)
    included_files += map(lambda f: root+'/'+f, new_files)

for included_file in included_files:
    source_file = find_matching_source_file(included_file)
    if source_file==None:
        path = os.path.dirname(included_file)
        if compile_commands_per_directory.has_key(path):
            path_commands = compile_commands_per_directory[path]
            if path_commands != "":
                update_index_with_flags(path_commands, included_file)
                continue
        print included_file+ ":0: Warning: header file ignored - Cannot find any related source file."
    else:
        compile_command = next((cc for cc in compile_commands if cc['file']==source_file), None)
        if compile_command == None:
            print included_file+ ":0: Warning: header file ignored - No compilation options know for", source_file
        else:
            # compile_flags=extract_compile_flags(compile_command['command'])
            # add_to_index(included_file, compile_flags)
            update_index(compile_command['command'], included_file)

# Removed files {{{2
index_files = sorted(glob.glob(".*.i.gz"))
index_files = map(lambda f: string.replace(f[:-5],'%', '/'), index_files)
files_to_remove = filter(lambda f: not os.path.isfile(f), index_files)
for file in files_to_remove:
    remove_from_index(file)




#======================================================================
# vim: set fdm=marker:tw=80:sw=4:
