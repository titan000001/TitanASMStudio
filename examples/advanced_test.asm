.model small
.stack 100h

.data
var1 db 5
var2 db 3

.code
main proc
    ; 1. Arithmetic Test
    mov bl, 10
    mov bh, 20
    add bl, bh   ; BL = 30
    sub bl, 5    ; BL = 25 hiding
    
    ; 2. Variable Test
    mov al, var1 ; AL = 5
    mov cl, var2
    add al, cl   ; AL = 8
    
    ; 3. I/O Test (Prompt)
    printn "Type a char:"
    mov ah, 1
    int 21h      ; Input -> AL
    
    mov dl, al   ; Move input to DL
    mov ah, 2
    printn "You typed:"
    int 21h      ; Output DL
    
    mov ah, 4ch
    int 21h
main endp
end main
