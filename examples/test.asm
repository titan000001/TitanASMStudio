; Simple Test Program
ORG 10
START:  LOAD 50     ; Load value from address 50
        ADD 51      ; Add value from address 51
        STORE 52    ; Store result in address 52
        
        SUB 53      ; Subtract
        JZ FINISH   ; Jump if zero to FINISH
        
LOOP:   ADD 51
        JMP LOOP    ; Infinite loop test

FINISH: HALT

ORG 50
DATA1:  DW 5        ; Data at 50
DATA2:  DW 10       ; Data at 51
RESULT: DW 0        ; Data at 52
MINUS:  DW 2        ; Data at 53
