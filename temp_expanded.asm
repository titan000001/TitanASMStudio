org 100h
.code
main proc
    mov ah, 1
    mov al, 2
    add ah, al ; ah = 3
    mov bl, 5
    sub bl, 2  ; bl = 3
    
    mov ah, 4Ch
    int 21h
main endp
end main
