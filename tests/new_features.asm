org 100h
.data
msg1 DB "Test passed!$"
val1 DB 3
val2 DB 2
res  DB ?

.code
main proc
    ; Test MUL (3 * 2 = 6)
    mov al, 3
    mov bl, 2
    mul bl      ; AX = AL * BL = 6
    cmp al, 6
    jnz fail

    ; Test DIV (8 / 3 = 2 rem 2)
    mov ax, 8
    mov bl, 3
    div bl      ; AL = 8/3=2, AH = 8%3=2
    cmp al, 2
    jnz fail
    cmp ah, 2
    jnz fail

    ; Test LEA and Print String
    lea dx, msg1
    mov ah, 9
    int 21h
    
    ; Exit
    mov ah, 4Ch
    int 21h

fail:
    mov ah, 4Ch
    int 21h
main endp
end main
