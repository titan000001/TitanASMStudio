org 100h

.code
main proc

    ; input 1
    mov ah,1
    int 21h
    sub al,48
    mov bl,al

    ; input 2
    mov ah,1
    int 21h
    sub al,48
    mov bh,al

    ; add
    add bl,bh

    ; output result
    add bl,48
    mov dl,bl
    mov ah,2
    int 21h

    ; exit
    mov ah,4Ch
    int 21h

main endp
end main
