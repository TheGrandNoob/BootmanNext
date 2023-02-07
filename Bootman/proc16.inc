macro ccall proc,[arg]                  ; directly call CDECL procedure
 { common
    size@ccall = 0
    if ~ arg eq
   reverse
    pushd arg
    size@ccall = size@ccall+4
   common
    end if
    call proc
    if size@ccall
    add sp,size@ccall
    end if }

macro cinvoke proc,[arg]                ; indirectly call CDECL procedure
 { common
    size@ccall = 0
    if ~ arg eq
   reverse
    pushd arg
    size@ccall = size@ccall+4
   common
    end if
    call [proc]
    if size@ccall
    add sp,size@ccall
    end if }

macro proc [args]                       ; define procedure
 { common
    match name params, args>
    \{ define@proc name,<params \} }