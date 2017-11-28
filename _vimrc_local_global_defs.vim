"=============================================================================
" File:         clang_indexer/_vimrc_local_global_defs.vim {{{1
" Author:       Luc Hermitte <EMAIL:hermitte {at} c-s {dot} fr>
" Version:      001
" Created:      10th Sep 2012
" Last Update:  $Date$
"------------------------------------------------------------------------
" Description:
"       Contains the global definitions that need to exist before defining
"       buffer-local definitions.
"       Meant to be sourced from _vimrc_local.vim
" 
"------------------------------------------------------------------------
" }}}1
"=============================================================================

" ======================[ Global project configuration {{{2
let s:sources_dir = '.'

" echo expand("<sfile>:p:h")
" unlet g:clang_indexer_config
" Mandatory Project options
call lh#let#if_undef('g:clang_indexer_config.paths.trunk', string(expand("<sfile>:p:h")))
LetIfUndef g:clang_indexer_config.paths.project    fnamemodify(g:clang_indexer_config.paths.trunk,':h')
LetIfUndef g:clang_indexer_config.paths.doxyfile   g:clang_indexer_config.paths.project
LetIfUndef g:clang_indexer_config.build.Debug      'build-d'
LetIfUndef g:clang_indexer_config.build.Release    'build-r'

" Here, this matches all the trunk => complete even with test files
" LetIfUndef g:clang_indexer_config.paths.sources    g:clang_indexer_config.paths.project.'/clang_indexer_src'
LetIfUndef g:clang_indexer_config.paths.sources    g:clang_indexer_config.paths.trunk
" Optional Project options
LetIfUndef g:clang_indexer_config.compilation.mode 'Release'
LetIfUndef g:clang_indexer_config.tests.verbosity '-VV'

" ======================[ Menus {{{2
let s:menu_priority = '50.130.'
let s:menu_name     = '&Project.&clang_indexer.'

" Function: s:getSNR([func_name]) {{{3
function! s:getSNR(...)
  if !exists("s:SNR")
    let s:SNR=matchstr(expand('<sfile>'), '<SNR>\d\+_\zegetSNR$')
  endif
  return s:SNR . (a:0>0 ? (a:1) : '')
endfunction

" Function: s:EditLocalCMakeFile() {{{3
function! s:EditLocalCMakeFile()
  let file = lh#path#to_relative(expand('%:p:h').'/CMakeLists.txt')
  call lh#buffer#jump(file, 'sp')
endfunction

call lh#let#if_undef ('g:clang_indexer_config.functions',
      \ string({'EditLocalCMakeFile': function(s:getSNR('EditLocalCMakeFile'))}))

" ======================[ Local variables to automagically import in QuickFix buffer {{{2
QFImport b:tags_select
QFImport &l:path
QFImport b:BTW_project_target
QFImport b:BTW_compilation_dir
QFImport b:BTW_project_build_mode

"------------------------------------------------------------------------
" Compilation mode, & CTest options {{{3
let g:clang_indexer_config_menu = {
      \ '_project': 'clang_indexer_config',
      \ 'menu': {'priority': s:menu_priority, 'name': s:menu_name}
      \ }
call lh#btw#cmake#def_options(g:clang_indexer_config_menu, [
      \ 'def_toggable_compil_mode',
      \ 'def_toggable_ctest_verbosity',
      \ 'def_toggable_ctest_checkmem',
      \ 'def_ctest_targets',
      \ 'add_gen_clic_DB',
      \ 'update_list'])


" ======================[ Misc functions {{{2
" Function: BTW_AddPathIfExists(listname, path) {{{3
function! BTW_AddPathIfExists(listname, path)
  let path = substitute(a:path, '[/\\]\*\*$', '', '')
  if isdirectory(path)
    let {a:listname} += [a:path]
  endif
endfunction

"=============================================================================
" vim600: set fdm=marker:
