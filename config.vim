" NeoVim configuration file
"

" send the contents of the current buffer to the server every time the buffer
" is changed. See clavm.lua
autocmd TextChanged,TextChangedI *.glsl lua require"clavm".send_code()

" listen for error messages, crash reports, and timing statistics and show
" these in the NeoVim status line. This is not yet implemented.
" command! StatusListen lua require"clavm".status_listen()
" command! StatusIgnore lua require"clavm".status_ignore()
" command! TestStatusListen lua require"clavm".test_status_listen()

" I like these settings. Maybe you will as well
filetype plugin indent on
syntax on
set foldmethod=marker
set tabstop=2 softtabstop=2 shiftwidth=2 expandtab smarttab autoindent
set incsearch ignorecase smartcase hlsearch
set wildmode=longest,list,full wildmenu " huh?

" Maybe I'll use these later
"set ruler laststatus=2 showcmd showmode
"set list listchars=trail:»,tab:»-
"set fillchars+=vert:\ 
"set wrap breakindent
"set encoding=utf-8
"set textwidth=0
"set hidden
"set number
"set title



" To use this NeoVim configuration automatically, you have to put this stuff in your main
" init.vim file.
"
" call plug#begin()
" Plug 'embear/vim-localvimrc' " this is the plugin that checks local vimrc
" call plug#end()
" let g:localvimrc_sandbox = 0
" let g:localvimrc_ask = 0


"set runtimepath+=./ " need this?

