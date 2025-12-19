; Macro Test Program
ORG 10

MACRO ADD_TWO &A &B &C
  LOAD &A
  ADD &B
  STORE &C
MEND

START:  ADD_TWO 50 51 52   ; Should expand to logic
        HALT

ORG 50
DATA1: DW 5
DATA2: DW 10
RES:   DW 0
