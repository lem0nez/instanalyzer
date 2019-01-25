let g:ycm_global_ycm_extra_conf = ".ycm_extra_conf.py"
let g:ycm_confirm_extra_conf = 0
let g:todo_search_path = "src/*.{hpp,cpp}"

autocmd BufNewFile *.{cpp,hpp} :LicenseApache
autocmd QuitPre * :mks! .session.vim
