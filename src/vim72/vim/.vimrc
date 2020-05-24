set bs=2

"autocmd! bufwritepost .vimrc source %

highlight Search term=reverse ctermbg=4 ctermfg=7
" highlight Normal ctermbg=black ctermfg=white
" highlight Folded ctermbg=black ctermfg=darkcyan
hi Comment ctermbg=black ctermfg=darkcyan

set ls=2
set statusline=%<%f\ %m%=\ %h%r\ %-19([%p%%]\ %3l,%02c%03V%)%y
highlight StatusLine ctermfg=blue ctermbg=white

set hlsearch

filetype on
" set number
" set cindent
" set autoindent
" set smartindent

" set expandtab
" set shiftwidth=4
" set softtabstop=4
" set tabstop=4

set wildmenu

" set foldmethod=marker

" {{{ UTF-8 Big5 Setting
set fileencodings=big5,utf-8
set termencoding=utf-8
set enc=utf-8
set tenc=utf8
" }}}

"imap <C-n> <Esc>:w<CR><Esc>
nnoremap <silent> <F9> :set paste<CR>

if has("autocmd")
    autocmd BufRead *.txt set tw=78
    autocmd BufReadPost *
    \ if line("'\"") > 0 && line ("'\"") <= line("$") |
    \   exe "normal g'\"" |
    \ endif
endif
