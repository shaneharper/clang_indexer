"=============================================================================
" $Id$
" File:         clang_indexer/_vimrc_local.vim {{{1
" Author:       Luc Hermitte <EMAIL:hermitte {at} c-s {dot} fr>
" Created:      24th Jan 2013
" Last Update:  $Date$
"------------------------------------------------------------------------
" Description:
"       Project configuration for clang_indexer
" }}}1
"=============================================================================

" abort
let s:k_version = 3
" echo "Loading version .".s:k_version
" Always loaded {{{1
let s:cpo_save=&cpo
set cpo&vim

" Alternate
" let g:alternateSearchPath = 'reg:#\<src\>$#inc,reg:#\<inc\>$#src#'
let g:alternateSearchPath='sfr:.'

" let g:force_reload_CLIC_code_vimrc_local = 0
"=============================================================================
" Global Definitions {{{1
if &cp
  let &cpo=s:cpo_save
  finish
endif
" The following needs to be loaded once, and before anything else (i.e. before
" the buffer-local definitions)
if ! (exists("g:loaded_CLIC_code_vimrc_local")
      \ && (g:loaded_CLIC_code_vimrc_local >= s:k_version)
      \ && (!exists('g:force_reload_CLIC_code_vimrc_local')
      \ || g:force_reload_CLIC_code_vimrc_local==0))
  source <sfile>:p:h/_vimrc_local_global_defs.vim
  " To be continued at the end of the file...
  " g:loaded_CLIC_code_vimrc_local will be set at this moment.
endif
let s:script = expand("<sfile>:p")

"=============================================================================
" Buffer-local Definitions {{{1
" Avoid local reinclusion {{{2
if &cp || (exists("b:loaded_CLIC_code_vimrc_local")
      \ && (b:loaded_CLIC_code_vimrc_local >= s:k_version)
      \ && (!exists('g:force_reload_CLIC_code_vimrc_local')
      \ || g:force_reload_CLIC_code_vimrc_local==0))
  finish
endif
let b:loaded_CLIC_code_vimrc_local = s:k_version
" Avoid local reinclusion }}}2

"------------------------------------------------------------------------
" ======================[ &path {{{2

" don't clang_indexerrch into included file how to complete
setlocal complete-=i
setlocal sw=8
setlocal expandtab

" No sub project
" let b:project_crt_sub_project = matchstr(lh#path#strip_common([g:clang_indexer_config.paths.trunk, expand('%:p:h')])[1], '[^/\\]*[/\\][^/\\]*')
" Tells BTW the compilation directory
let b:BTW_compilation_dir = g:clang_indexer_config.paths._build
" Local vimrc variable for source dir
let b:project_sources_dir = g:clang_indexer_config.paths.sources
" Option for Mu-Template-> |s:path_from_root()|
let b:sources_root = g:clang_indexer_config.paths.sources

" Configures lh-cpp complete includes sub-plugin
let b:includes = [ b:project_sources_dir . '/**']
if SystemDetected() == 'unix'
  call BTW_AddPathIfExists('b:includes', '/home/lhermitte/dev/boost/1_51_0/install/include/')
  call BTW_AddPathIfExists('b:includes', '/usr/local/include/**')
  call BTW_AddPathIfExists('b:includes', '/usr/include/**')
endif
" Setting &path
exe 'setlocal path+='.FixPathName(b:project_sources_dir).'/**'
for p in b:includes
  exe 'setlocal path+='.FixPathName(p)
endfor

" gcov output path
let b:gcov_files_path = g:clang_indexer_config.paths.sources.'/obj/debug/Testing/CoverageInfo'

" spell files
setlocal spellfile=
exe 'setlocal spellfile+='.FixPathName(b:project_sources_dir).'/ignore.utf-8.add'
let b:tags_to_spellfile = 'code-symbols.add'
for p in ['src', 'inc', 'tst/tests-unitaires']
  " echo 'setlocal spellfile+='.FixPathName(b:project_sources_dir.'/'.p.'/'.b:tags_to_spellfile)
  exe 'setlocal spellfile+='.FixPathName(b:project_sources_dir.'/'.p.'/'.b:tags_to_spellfile)
endfor


" ======================[ Settings for BTW {{{2
if SystemDetected() == 'msdos'
  :BTW setlocal cmake
  " echomsg SystemDetected()
  if SystemDetected() == 'unix' " cygwin
    " then cygwin's cmake does not work -> use win32 cmake
    let $PATH=substitute($PATH, '\(.*\);\([^;]*CMake[^;]*\)', '\2;\1', '')
  endif
endif
:BTW addlocal STLFilt
" silent! unlet b:BTW_project_executable
LetIfUndef b:BTW_project_executable.type 'ctest'
" sets b:BTW_project_executable.rule
call g:clang_indexer_config_menu.def_ctest_targets.set_ctest_argument()

" if exists('s:run_lint_menu')
" call s:UpdateSconsOptions()
" else
let b:BTW_project_target = ''
" endif
let b:BTW_project_config = {
      \ 'type': 'ccmake',
      \ 'arg': (b:project_sources_dir), 
      \ 'wd' : lh#function#bind('b:BTW_compilation_dir')}
" \ 'wd' : (b:BTW_compilation_dir)}


" ======================[ tags generation {{{2
let b:tags_dirname = expand('%:p:h')
let b:tags_options = ' --exclude="*.dox" --exclude="html" --exclude=".*sw*"'
let b:tags_options .= ' --exclude="*.txt" --exclude="*.o" --exclude="*.os" --exclude="*.tags" --exclude=tags --exclude=buttons --exclude="*.png" --exclude="*.tar"'
exe 'setlocal tags+='.FixPathName(b:project_sources_dir).'/*/tags'
if SystemDetected() == 'msdos'
else
  setlocal tags+=/usr/local/include/otb/tags
endif
" let BTW_make_in_background_in = 'xterm -e'


" ======================[ Project's style {{{2
" let b:cpp_namespace = 'clic'

" ======================[ Menus {{{2
call lh#menu#make('nic', '50.11', '&Project.Edit local &CMake file', '<localleader><F7>', '<buffer>', ':call g:clang_indexer_config.functions.EditLocalCMakeFile()<cr>')
" call lh#menu#make('nic', '50.11', '&Project.Edit local &Scons file', '', '<buffer>', ':call EditLocalSconsFile()<cr>')
call lh#menu#make('nic', '50.76', '&Project.Edit local &vimrc', '<localleader>le', '<buffer>', ':call lh#buffer#jump('.string(s:script).', "sp")<cr>' )
"
" ======================[ Other commands {{{2
command! -b -nargs=0 CleanEmptyBuffers call s:CleanEmptyBuffers()
command! -b -nargs=* LVEcho :echo <sid>Echo(<args>)


"=============================================================================
" Global Definitions {{{1
" Avoid global reinclusion {{{2
if &cp || (exists("g:loaded_CLIC_code_vimrc_local")
      \ && (g:loaded_CLIC_code_vimrc_local >= s:k_version)
      \ && (!exists('g:force_reload_CLIC_code_vimrc_local')
      \ || g:force_reload_CLIC_code_vimrc_local==0))
  let &cpo=s:cpo_save
  finish
else
  let g:loaded_CLIC_code_vimrc_local = s:k_version
endif
" Avoid global reinclusion }}}2
"------------------------------------------------------------------------
" ======================[ Misc function {{{2
" Function: s:CleanEmptyBuffers() {{{3
function! s:CleanEmptyBuffers()
  let buffers = lh#list#copy_if(range(0, bufnr('$')), [], 'buflisted(v:1_) && empty(bufname(v:1_)) && bufwinnr(v:1_)<0')
  if !empty(buffers)
    exe 'bw '.join(buffers, ' ')
  endif
endfunction

" Function: s:Echo(expr) {{{3
function! s:Echo(expr)
  return a:expr
  " return eval(a:expr)
endfunction

" Functions }}}2
"------------------------------------------------------------------------
let &cpo=s:cpo_save
"=============================================================================
" vim600: set fdm=marker:
