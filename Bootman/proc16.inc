macro stdcall proc , [arg]{
    reverse push arg
    common call proc
}