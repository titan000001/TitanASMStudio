org 100h
.code
main proc
    mov cl, 5
loop_start:
    print "*"
    sub cl, 1
    cmp cl, 0
    jnz loop_start
    
    mov ah, 4Ch
    int 21h
main endp
end main
