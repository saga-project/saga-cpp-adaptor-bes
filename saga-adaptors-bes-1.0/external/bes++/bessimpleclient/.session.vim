let SessionLoad = 1
if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
imap <D-BS> 
imap <M-Down> }
inoremap <D-Down> <C-End>
imap <M-Up> {
inoremap <D-Up> <C-Home>
noremap! <M-Right> <C-Right>
noremap! <D-Right> <End>
noremap! <M-Left> <C-Left>
noremap! <D-Left> <Home>
inoremap <silent> <S-Tab> =BackwardsSnippet()
imap <C-Down> <C-Down>li
imap <C-Up> <C-Up>li
imap <M-BS> 
imap <C-Right> lwi
imap <C-Left> bi
imap <S-Right> lwi
imap <S-Left> bi
imap <S-F12> <S-F12>a
imap <C-F12> <C-F12>a
imap <F12> <F12>a
imap <S-F11> <S-F11>a
imap <C-F11> <C-F11>a
imap <F11> <F11>a
imap <S-F10> <S-F10>a
imap <C-F10> <C-F10>a
imap <F10> <F10>a
imap <S-F9> <S-F9>a
imap <C-F9> <C-F9>a
imap <F9> <F9>a
imap <S-F8> <S-F8>a
imap <C-F8> <C-F8>a
imap <F8> <F8>a
imap <S-F7> <S-F7>a
imap <C-F7> <C-F7>a
imap <F7> <F7>a
imap <S-F6> <S-F6>a
imap <C-F6> <C-F6>a
imap <F6> <F6>a
imap <S-F5> <S-F5>a
imap <C-F5> <C-F5>a
imap <F5> <F5>a
imap <S-F4> <S-F4>a
imap <C-F4> <C-F4>a
imap <F4> <F4>a
imap <S-F3> <S-F3>a
imap <C-F3> <C-F3>a
imap <F3> <F3>a
imap <S-F2> <S-F2>a
imap <C-F2> <C-F2>a
imap <F2> <F2>a
imap <S-F1> <S-F1>a
imap <C-F1> <C-F1>a
imap <F1> <F1>a
imap <C-LeftMouse> <C-LeftMouse>a
map! <D-v> *
vmap  :Inc
map  :r! pbpaste | sed 's/ \*\$//'
snoremap <silent> 	 i<Right>=TriggerSnippet()
map 		 i	
snoremap  b<BS>
snoremap % b<BS>%
snoremap ' b<BS>'
snoremap U b<BS>U
vmap [% [%m'gv``
snoremap \ b<BS>\
map \q ggg?G                     " do a rot13
map \r :reg                  " show registers
map \M :GoMark  ('a')        " jump to mark a
map \m :SetMark ('a')        " set mark a
map \d ma[[k"xyy`a:echo @x   " show function (TODO)
map \t :TagExplorer          " open tag explorer
vmap ]% ]%m'gv``
snoremap ^ b<BS>^
snoremap ` b<BS>`
vmap a% [%v]%
nmap gx <Plug>NetrwBrowseX
nmap <silent> sw "_yiw:s/\(\%#\w\+\)\(\W\+\)\(\w\+\)/\3\2\1/
vmap xw :w!      $HOME/.vim/vimxfer
vmap xr c:r $HOME/.vim/vimxfer
nmap xw :'a,.w!  $HOME/.vim/vimxfer
nmap xr :r       $HOME/.vim/vimxfer
map <M-Down> }
noremap <D-Down> <C-End>
map <M-Up> {
noremap <D-Up> <C-Home>
noremap <M-Right> <C-Right>
noremap <D-Right> <End>
noremap <M-Left> <C-Left>
noremap <D-Left> <Home>
snoremap <Left> bi
snoremap <Right> a
snoremap <BS> b<BS>
snoremap <silent> <S-Tab> i<Right>=BackwardsSnippet()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
map <S-Down> [}
map <S-Up> [{
map <C-Down> 
map <C-Up> 
map <C-Right> w
map <C-Left> b
map <S-Right> w
map <S-Left> b
map <S-F12> :call List_toggle()
map <F12> :noh
map <S-F11> :colorscheme peachpuff
map <F11> :source ~/.vimrc
map <S-F10> :colorscheme peachpuff
map <F10> zi
map <S-F9> :colorscheme peachpuff
map <F9> :syntax clear
map <S-F8> :call XDVI_jump()
map <F8> <F2>:silent !make again < ~/.ok >& makelog & 
map <S-F7> :set ft=txt tw=64 nocindent
map <F7> :set wrap!
map <S-F6> :source .session.vim<F11>
map <F6> :mksession! .session.vim
map <S-F5> :call ToggleMenu()
map <F5> _
map <S-F4> :call GUIFont_Inc ()
map <F4> W_
map <S-F3> :call GUIFont_Dec ()
map <F3> _
map <C-F2> :SpellCheck
map <F2> :wa!
map <C-F1> :SpellProposeAlternatives
map <F1> gqapk
map <S-LeftMouse> :call XDVI_jump()
xmap <BS> "-d
vmap <D-x> "*d
vmap <D-c> "*y
vmap <D-v> "-d"*P
nmap <D-v> "*P
imap  a
inoremap <silent> 	 =TriggerSnippet()
imap 		 	
inoremap <silent> 	 =ShowAvailableSnips()
map √õ :diffget]c
map √ù :diffput]c
let &cpo=s:cpo_save
unlet s:cpo_save
set autoindent
set autowrite
set backspace=2
set cindent
set cinkeys=0{,0},0):,0#,!,o,O,e,<Tab>,*<Return>
set cinoptions=>s,e0,n0,f0,{0,}0,^0,:s,=s,l1,gs,hs,p0,t0,i2s,+0s,c3,C1,/0,(0,u0,U0,W4,w0,m0,)20,*20,30
set cpoptions=aABceFs$
set diffopt=filler,iwhite,icase
set noequalalways
set errorformat=@%t@%n@%m@%f@%l@%c
set expandtab
set fileencodings=ucs-bom,utf-8,default,latin1
set formatoptions=tcrqn1
set guifont=Monaco:h13
set guioptions=agimt
set helplang=en
set hlsearch
set ignorecase
set incsearch
set indentkeys==end,},!^F,o,O,<Return>
set keywordprg=man\ -k
set laststatus=2
set listchars=eol:$,tab:ª-,eol:∂,trail:∑,extends:_,precedes:_
set makeef=.vim##.err
set makeprg=gmake\ -j\ 5
set matchtime=200
set maxmem=2000000
set mouse=a
set pastetoggle=<C-P>
set printexpr=system('open\ -a\ Preview\ '.v:fname_in)\ +\ v:shell_error
set ruler
set scrolloff=5
set shellcmdflag=-ic
set shellpipe=>&
set shiftwidth=2
set shortmess=aoOIA
set showcmd
set showmatch
set smartcase
set smarttab
set softtabstop=2
set splitbelow
set statusline=%2*--%2*[%2.2n]%2*--\ %4*%f%2*\ %<----------------------------------------------------------------------------------------------------------------------------------------------------------->\ %2*%([%M%R%H]\ %)%2*%=%2*[%4.6l,%4.4c]\ [%3.3p%%]%*
set tabstop=2
set termencoding=utf-8
set textwidth=80
set timeoutlen=300
set updatecount=1000
set updatetime=3000
set viminfo=%,h,'100,/20,:20,@20,r/mnt,n~/.vim/viminfo
set virtualedit=block
set visualbell
set whichwrap=b,s,>,<,[,],h,l
set winminheight=0
set writeany
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
cd ~/links/saga/svn/trunk/adaptors/ogf/external/bes++/bessimpleclient
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +0 besclient.cpp
args besclient.cpp
edit besclient.cpp
set splitbelow splitright
set nosplitright
wincmd t
set winheight=1 winwidth=1
argglobal
setlocal keymap=
setlocal noarabic
setlocal autoindent
setlocal nobinary
setlocal bufhidden=
setlocal buflisted
setlocal buftype=
setlocal cindent
setlocal cinkeys=0{,0},0):,0#,!,o,O,e,<Tab>,*<Return>
setlocal cinoptions=>s,e0,n0,f0,{0,}0,^0,:s,=s,l1,gs,hs,p0,t0,i2s,+0s,c3,C1,/0,(0,u0,U0,W4,w0,m0,)20,*20,30
setlocal cinwords=if,else,while,do,for,switch
setlocal comments=sO:*\ -,mO:*\ \ ,exO:*/,s1:/*,mb:*,ex:*/,://
setlocal commentstring=/*%s*/
setlocal complete=.,w,b,u,t,i
setlocal completefunc=
setlocal nocopyindent
setlocal nocursorcolumn
setlocal nocursorline
setlocal define=
setlocal dictionary=
setlocal nodiff
setlocal equalprg=
setlocal errorformat=
setlocal expandtab
if &filetype != 'cpp'
setlocal filetype=cpp
endif
setlocal foldcolumn=0
setlocal foldenable
setlocal foldexpr=0
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldmarker={{{,}}}
setlocal foldmethod=manual
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldtext=foldtext()
setlocal formatexpr=
setlocal formatoptions=n1croql
setlocal formatlistpat=^\\s*\\d\\+[\\]:.)}\\t\ ]\\s*
setlocal grepprg=
setlocal iminsert=2
setlocal imsearch=2
setlocal include=
setlocal includeexpr=
setlocal indentexpr=
setlocal indentkeys==end,},!^F,o,O,<Return>
setlocal noinfercase
setlocal iskeyword=@,48-57,_,192-255
setlocal keywordprg=
setlocal nolinebreak
setlocal nolisp
setlocal nolist
setlocal nomacmeta
setlocal makeprg=
setlocal matchpairs=(:),{:},[:]
setlocal modeline
setlocal modifiable
setlocal nrformats=octal,hex
setlocal nonumber
setlocal numberwidth=4
setlocal omnifunc=ccomplete#Complete
setlocal path=
setlocal nopreserveindent
setlocal nopreviewwindow
setlocal quoteescape=\\
setlocal noreadonly
setlocal norightleft
setlocal rightleftcmd=search
setlocal noscrollbind
setlocal shiftwidth=2
setlocal noshortname
setlocal nosmartindent
setlocal softtabstop=2
setlocal nospell
setlocal spellcapcheck=[.?!]\\_[\\])'\"\	\ ]\\+
setlocal spellfile=
setlocal spelllang=en
setlocal statusline=
setlocal suffixesadd=
setlocal swapfile
setlocal synmaxcol=3000
if &syntax != 'cpp'
setlocal syntax=cpp
endif
setlocal tabstop=2
setlocal tags=
setlocal textwidth=80
setlocal thesaurus=
setlocal nowinfixheight
setlocal nowinfixwidth
setlocal wrap
setlocal wrapmargin=0
silent! normal! zE
let s:l = 114 - ((42 * winheight(0) + 30) / 61)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
114
normal! 02l
tabnext 1
if exists('s:wipebuf')
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 shortmess=aoOIA
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &so = s:so_save | let &siso = s:siso_save
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
