org 100h
.code
main proc
    mov ax, 10
    push ax
    call my_proc
    pop bx ; bx should be 10
    
    mov ah, 4Ch
    int 21h
main endp

my_proc proc
    mov cx, ax
    add cx, 5
    ret
my_proc endp
end main
