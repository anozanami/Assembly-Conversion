ADD x7, x1, x2
SUB x8, x3, x4
JAL x1, FUNC1
ADDI x9, x0, 3
JAL x1, FUNC2
ADD x10, x5, x6
ADD x11, x7, x8
JAL x2, FUNC3
SLLI x12, x11, 1
ADD x13, x12, x7
ADD x14, x13, x9
ADD x15, x14, x10
EXIT
FUNC1:
XORI x16, x9, 5
JALR x0, 0(x1)
FUNC2:
AND x17, x10, x5
JALR x0, 0(x1)
FUNC3:
ADD x18, x13, x10
JALR x0, 0(x2)