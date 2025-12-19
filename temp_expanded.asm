
ORG 10
START:  LOAD VAR
; Begin Macro Expansion: INC
  LOAD VAR
  ADD ONE
  STORE VAR
; End Macro Expansion
        HALT

ORG 50
VAR: DW 5
ONE: DW 1
