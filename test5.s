ADD x7, x1, x2
SUB x8, x3, x4
ADDI x9, x0, 3
ADD x10, x5, x6
LOOP:
SUB x9, x9, x1
BNE x9, x0, FUNC2
ADD x11, x7, x8
SLLI x11, x11, 1
BNE x11, x10, FUNC1
FUNC2:
ADD x12, x12, x9
SUb x13, x3, x1
BEQ x13, x12, LOOP
SUB x14, x12, x10
BNE x14, x8, LOOP
FUNC1:
OR x15, x11, x8
XORI x16, x15, 7
BEQ x16, x10, LOOP
SLL x17, x16, x1
BNE x17, x9, FUNC3
fUNC3:
ADD x18, x17, x16
SUB x19, x18, x7
ADD x20, x19, x10
EXIT