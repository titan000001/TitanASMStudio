org 100h

main proc
    mov ax, 0
    call my_func
    add ax, 1    ; Should be 5 + 1 = 6
    mov ah, 4Ch  ; Exit
    int 21h
main endp

my_func:
    mov ax, 5
    ret
