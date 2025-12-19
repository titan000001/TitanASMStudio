org 100h
include 'emu8086.inc'

.data
.code

main proc
    
    printn "Hello World"
    
    mov ah, 4Ch
    int 21h
    
main endp
end main
