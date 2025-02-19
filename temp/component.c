#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#define MEMORY_SIZE 1024
#define REGISTER_SIZE 32
#define LABEL_COUNT 10
#define MAX_LABELS 20

typedef struct {
    char instruction[20];
    int op1, op2, op3;
    int offset;
    char label[MAX_LABELS];
    char labelName[MAX_LABELS];
} Instruction;

typedef struct {
    char label[MAX_LABELS];
    int address;
} Label;

typedef struct {
    Label labels[MAX_LABELS];
    int count;
} LabelTable;

LabelTable labelTable;

int registers[32] = {
    0, 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}; // 초기값 설정
int memory[5001]; // 메모리 시뮬레이션
int pc;
int labelCnt;

// 완료
void addLabel(LabelTable *labelTable, const char *labelName, int address) {
    strcpy(labelTable->labels[labelTable->count].label, labelName);
    labelTable->labels[labelTable->count].address = address;
    labelTable->count++;
    printf("Adding label %s\n", labelName);
    printf("Label address %d\n", pc);
}

// 완료
unsigned int getLabelAddress(LabelTable *labelTable, const char *labelName) {
    for (int i = 0; i < labelTable->count; i++) {
        if (strcmp(labelTable->labels[i].label, labelName) == 0) {
            return labelTable->labels[i].address;
        }
    }
    return 0; // Label not found
}


// 레지스터 값은
unsigned int rtype(unsigned int func7, unsigned int rd, unsigned int rs1, unsigned int func3, unsigned int rs2) {
    // 입력 받는 값은 모두 이진수
    unsigned int opcode = 0b0110011;
    unsigned int machineCode = 0;
    machineCode |= (func7 << 25);
    machineCode |= (rs2 << 20);
    machineCode |= (rs1 << 15);
    machineCode |= (func3 << 12);
    machineCode |= (rd << 7);
    machineCode |= opcode;
    return machineCode;
}

unsigned int itype(unsigned int imm12, unsigned int rs1, unsigned int func3, unsigned int rd, unsigned int opcode) {
    unsigned int machineCode = 0;
    machineCode |= (imm12 << 20);
    machineCode |= (rs1 << 15);
    machineCode |= (func3 << 12);
    machineCode |= (rd << 7);
    machineCode |= opcode;
    return machineCode;
}

unsigned int stype(unsigned int imm7, unsigned int rs2, unsigned int rs1, unsigned int func3, unsigned int imm5) {
    unsigned int opcode = 0b0100011;
    unsigned int machineCode = 0;
    imm5 = imm5 & 0b11111; // 마스킹
    machineCode |= (imm7 << 25);
    machineCode |= (rs2 << 20);
    machineCode |= (rs1 << 15);
    machineCode |= (func3 << 12);
    machineCode |= (imm5 << 7);
    machineCode |= opcode;
    return machineCode;
}

unsigned int sbtype(unsigned int imm, unsigned int rs2, unsigned int rs1, unsigned int func3) {
    unsigned int opcode = 0b1100011;
    unsigned int machineCode = 0;
    unsigned int imm12 = (imm >> 12) & 1;
    unsigned int imm10_5 = (imm >> 5) & 0x3F; // 0b11 1111
    unsigned int imm4_1 = (imm >> 1) & 0xF;
    unsigned int imm11 = (imm >> 11) & 1;

    machineCode |= (imm12 << 31);
    machineCode |= (imm10_5 << 25);
    machineCode |= (rs2 << 20);
    machineCode |= (rs1 << 15);
    machineCode |= (func3 << 12);
    machineCode |= (imm4_1 << 8);
    machineCode |= (imm11 << 7);
    machineCode |= opcode;

    return machineCode;
}

unsigned int utype(unsigned int imm, unsigned int rd) {
    unsigned int opcode = 0b0110111;
    unsigned int machineCode = 0;
    machineCode |= (imm << 12);
    machineCode |= (rd << 7);
    machineCode |= opcode;
    return machineCode;
}

unsigned int ujtype(int imm, unsigned int rd) {
    unsigned int opcode = 0b1101111;
    unsigned int machineCode = 0;
    unsigned int imm20 = (imm >> 20) & 1;
    unsigned int imm10_1 = (imm >> 1) & 0x3FF;
    unsigned int imm11 = (imm >> 11) & 1;
    unsigned int imm19_12 = (imm >> 12) & 0xFF;

    machineCode |= (imm20 << 31);
    machineCode |= (imm19_12 << 12);
    machineCode |= (imm11 << 20);
    machineCode |= (imm10_1 << 21);
    machineCode |= (rd << 7);
    machineCode |= opcode;

    return machineCode;
}

void initialize() {
    for (int i = 0; i < 5001; i++) {
        memory[i] = 0;
    }
    for (int i = 0; i < 7; i++) {
        registers[i] = i;
    }
    for (int i = 7; i < 32; i++) {
        registers[i] = 0;
    }
    pc = 1000;
    labelTable.count = 0;
    for (int i = 0; i < MAX_LABELS; i++) {
        for (int j = 0; j < MAX_LABELS; j++) {
            labelTable.labels[i].label[i] = '\0';
        }
        // labelTable 안에 있는 배열들도 초기화
    }
    labelCnt = 0;
}

void getUpper(char *instr, char *label, char *labelName) {
    for (int i = 0; i < strlen(instr); i++) {
        instr[i] = toupper(instr[i]);
    }
    for (int i = 0; i < strlen(label); i++) {
        label[i] = toupper(label[i]);
    }
    for (int i = 0; i < strlen(labelName); i++) {
        labelName[i] = toupper(labelName[i]);
    }
}


void parseLine(char *line, Instruction *instr) {
    int chk = 0;
    char inst[20];
    sscanf(line, "%s", inst);
    for (int i = 0; i < 20; i++) {
        inst[i] = toupper(inst[i]);
    }
    if (strcmp(inst, "EXIT") == 0) {
        pc += 4;
        sscanf(line, "%s", instr->instruction);
        return;
    }
    if (strcmp(inst, "LW") == 0 || strcmp(inst, "SW") == 0 || strcmp(inst, "JALR") == 0) {
        chk = 1;
    } else if (strcmp(inst, "ADDI") == 0 || strcmp(inst, "ANDI") == 0 || strcmp(inst, "ORI") == 0 ||
               strcmp(inst, "XORI") == 0 ||
               strcmp(inst, "SLLI") == 0 || strcmp(inst, "SRLI") == 0 || strcmp(inst, "SRAI") == 0) {
        chk = 2;
    } else if (strcmp(inst, "JAL") == 0) {
        chk = 3;
    } else if (strcmp(inst, "BEQ") == 0 || strcmp(inst, "BNE") == 0 || strcmp(inst, "BGE") == 0 || strcmp(inst, "BLT")
               == 0) {
        chk = 4;
    } else if (strcmp(inst, "ADD") == 0 || strcmp(inst, "SUB") == 0 || strcmp(inst, "AND") == 0 || strcmp(inst, "OR") ==
               0 ||
               strcmp(inst, "XOR") == 0 || strcmp(inst, "SLL") == 0 || strcmp(inst, "SRL") == 0 || strcmp(inst, "SRA")
               == 0) {
        chk = 5;
    }

    if (chk == 0) {
        sscanf(line, "%s", instr->labelName);
    } else if (chk == 1) {
        // load, store
        sscanf(line, "%s x%d, %d(x%d)", instr->instruction, &instr->op1, &instr->offset, &instr->op2);
    } else if (chk == 2) {
        // i-type
        sscanf(line, "%s x%d, x%d, %d", instr->instruction, &instr->op1, &instr->op2, &instr->op3);
    } else if (chk == 3) {
        // J-type
        sscanf(line, "%s x%d, %s", instr->instruction, &instr->op1, instr->label);
    } else if (chk == 4) {
        // branch
        sscanf(line, "%s x%d, x%d, %s", instr->instruction, &instr->op1, &instr->op2, instr->label);
    } else if (chk == 5) {
        // R-type
        sscanf(line, "%s x%d, x%d, x%d", instr->instruction, &instr->op1, &instr->op2, &instr->op3);
    }
}

unsigned int convertInstruction(Instruction *instr) {
    for (int i = 0; i < 20; i++) {
        instr->instruction[i] = toupper((unsigned char) instr->instruction[i]);
        if (instr->labelName[i] == ':') {
            continue;
        }
        instr->labelName[i] = toupper((unsigned char) instr->labelName[i]);
        instr->label[i] = toupper((unsigned char) instr->label[i]);
    }
    unsigned int machinecode;
    // 2가지를 마무리해야함
    // 1. pc 값 업데이트(only branch 명령어) in .trace
    // 2. instruction -> binary code (2진수로)
    if (strcmp(instr->instruction, "ADD") == 0) {
        machinecode = rtype(0b0000000, instr->op1, instr->op2, 0b000, instr->op3);
    } else if (strcmp(instr->instruction, "SUB") == 0) {
        machinecode = rtype(0b0100000, instr->op1, instr->op2, 0b000, instr->op3);
    } else if (strcmp(instr->instruction, "ADDI") == 0) {
        machinecode = itype(instr->op3, instr->op2, 0b000, instr->op1, 0b0010011);
    } else if (strcmp(instr->instruction, "AND") == 0) {
        machinecode = rtype(0b0000000, instr->op1, instr->op2, 0b111, instr->op3);
    } else if (strcmp(instr->instruction, "OR") == 0) {
        machinecode = rtype(0b0000000, instr->op1, instr->op2, 0b110, instr->op3);
    } else if (strcmp(instr->instruction, "XOR") == 0) {
        machinecode = rtype(0b0000000, instr->op1, instr->op2, 0b100, instr->op3);
    } else if (strcmp(instr->instruction, "ANDI") == 0) {
        machinecode = itype(instr->op3, instr->op2, 0b111, instr->op1, 0b0010011);
    } else if (strcmp(instr->instruction, "ORI") == 0) {
        machinecode = itype(instr->op3, instr->op2, 0b110, instr->op1, 0b0010011);
    } else if (strcmp(instr->instruction, "XORI") == 0) {
        machinecode = itype(instr->op3, instr->op2, 0b100, instr->op1, 0b0010011);
    } else if (strcmp(instr->instruction, "SLL") == 0) {
        machinecode = rtype(0b0000000, instr->op1, instr->op2, 0b001, instr->op3);
    } else if (strcmp(instr->instruction, "SRL") == 0) {
        machinecode = rtype(0b0000000, instr->op1, instr->op2, 0b101, instr->op3);
    } else if (strcmp(instr->instruction, "SRA") == 0) {
        machinecode = rtype(0b0100000, instr->op1, instr->op2, 0b101, instr->op3);
    } else if (strcmp(instr->instruction, "SLLI") == 0) {
        machinecode = itype(0b000000000000 | instr->op3, instr->op2, 0b001, instr->op1, 0b0010011);
    } else if (strcmp(instr->instruction, "SRLI") == 0) {
        machinecode = itype(0b000000000000 | instr->op3, instr->op2, 0b101, instr->op1, 0b0010011);
    } else if (strcmp(instr->instruction, "SRAI") == 0) {
        machinecode = itype(0b010000000000 | instr->op3, instr->op2, 0b101, instr->op1, 0b0010011);
    } else if (strcmp(instr->instruction, "LW") == 0) {
        machinecode = itype(instr->offset, instr->op2, 0b010, instr->op1, 0b0000011);
    } else if (strcmp(instr->instruction, "SW") == 0) {
        machinecode = stype(instr->offset >> 5, instr->op1, instr->op2, 0b010, (instr->offset));
    } else if (strcmp(instr->instruction, "BEQ") == 0) {
        unsigned int address = getLabelAddress(&labelTable, instr->label);
        machinecode = sbtype(address, instr->op2, instr->op1, 0b000);
    } else if (strcmp(instr->instruction, "BNE") == 0) {
        unsigned int address = getLabelAddress(&labelTable, instr->label);
        machinecode = sbtype(address, instr->op2, instr->op1, 0b001);
    } else if (strcmp(instr->instruction, "BGE") == 0) {
        unsigned int address = getLabelAddress(&labelTable, instr->label);
        machinecode = sbtype(address, instr->op2, instr->op1, 0b101);
    } else if (strcmp(instr->instruction, "BLT") == 0) {
        unsigned int address = getLabelAddress(&labelTable, instr->label);
        machinecode = sbtype(address, instr->op2, instr->op1, 0b100);
    } else if (strcmp(instr->instruction, "JAL") == 0) {
        unsigned int address = getLabelAddress(&labelTable, instr->label);
        machinecode = utype(address, instr->op1);
    } else if (strcmp(instr->instruction, "JALR") == 0) {
        machinecode = utype(instr->op2, instr->op1);
    } else if (strcmp(instr->instruction, "EXIT") == 0) {
        machinecode = 0xFFFFFFFF;
    } else {
        machinecode = 0;
    }
    // 명령어 추가 필요, 대소문자 구분 안되게 조정 필요
    return machinecode;
}

int executionInstruction(Instruction *instr) {
    for (int i = 0; i < 20; i++) {
        instr->instruction[i] = toupper((unsigned char) instr->instruction[i]);
        if (instr->labelName[i] == ':') {
            continue;
        }
        instr->labelName[i] = toupper((unsigned char) instr->labelName[i]);
        instr->label[i] = toupper((unsigned char) instr->label[i]);
    }
    // 2가지를 마무리해야함
    // 1. pc 값 업데이트(only branch 명령어) in .trace
    // 2. instruction -> binary code (2진수로)
    if (strcmp(instr->instruction, "ADD") == 0) {
        registers[instr->op1] = registers[instr->op2] + registers[instr->op3];
    } else if (strcmp(instr->instruction, "SUB") == 0) {
        registers[instr->op1] = registers[instr->op2] - registers[instr->op3];
    } else if (strcmp(instr->instruction, "ADDI") == 0) {
        registers[instr->op1] = registers[instr->op2] + instr->op3;
    } else if (strcmp(instr->instruction, "AND") == 0) {
        registers[instr->op1] = registers[instr->op2] & registers[instr->op3];
    } else if (strcmp(instr->instruction, "OR") == 0) {
        registers[instr->op1] = registers[instr->op2] | registers[instr->op3];
    } else if (strcmp(instr->instruction, "XOR") == 0) {
        registers[instr->op1] = registers[instr->op2] ^ registers[instr->op3];
    } else if (strcmp(instr->instruction, "ANDI") == 0) {
        registers[instr->op1] = registers[instr->op2] & instr->op3;
    } else if (strcmp(instr->instruction, "ORI") == 0) {
        registers[instr->op1] = registers[instr->op2] | instr->op3;
    } else if (strcmp(instr->instruction, "XORI") == 0) {
        registers[instr->op1] = registers[instr->op2] ^ instr->op3;
    } else if (strcmp(instr->instruction, "SLL") == 0) {
        registers[instr->op1] = registers[instr->op2] << registers[instr->op3];
    } else if (strcmp(instr->instruction, "SRL") == 0) {
        registers[instr->op1] = registers[instr->op2] >> registers[instr->op3];
    } else if (strcmp(instr->instruction, "SRA") == 0) {
        registers[instr->op1] = (unsigned int) registers[instr->op2] >> registers[instr->op3];
    } else if (strcmp(instr->instruction, "SLLI") == 0) {
        registers[instr->op1] = registers[instr->op2] << instr->op3;
    } else if (strcmp(instr->instruction, "SRLI") == 0) {
        registers[instr->op1] = registers[instr->op2] >> instr->op3;
    } else if (strcmp(instr->instruction, "SRAI") == 0) {
        registers[instr->op1] = (unsigned int) registers[instr->op2] >> instr->op3;
    } else if (strcmp(instr->instruction, "LW") == 0) {
        registers[instr->op1] = memory[registers[instr->op2] + instr->offset];
    } else if (strcmp(instr->instruction, "SW") == 0) {
        memory[registers[instr->op2] + instr->offset] = instr->op3;
    } else if (strcmp(instr->instruction, "BEQ") == 0) {
        int address = getLabelAddress(&labelTable, instr->label);
        if (registers[instr->op1] == registers[instr->op2]) {
            pc = address;
        } else {
            pc += 4;
        }
        printf("instr : %s, op1 : %d, op2 : %d, op3 : %d, label : %s, labelName : %s, "
               "pc : %d\n", instr->instruction, instr->op1, instr->op2, instr->op3, instr->label, instr->labelName, pc);
        return 0;
    } else if (strcmp(instr->instruction, "BNE") == 0) {
        unsigned int address = getLabelAddress(&labelTable, instr->label);
        if (registers[instr->op1] != registers[instr->op2]) {
            pc = address;
        } else {
            pc += 4;
        }
        return 0;
    } else if (strcmp(instr->instruction, "BGE") == 0) {
        unsigned int address = getLabelAddress(&labelTable, instr->label);
        if (registers[instr->op1] >= registers[instr->op2]) {
            pc = address;
        } else {
            pc += 4;
        }
        printf("BGE 실행 -> pc : %d로 이동\n", pc);
        return 0;
    } else if (strcmp(instr->instruction, "BLT") == 0) {
        int address = getLabelAddress(&labelTable, instr->label);
        if (registers[instr->op1] < registers[instr->op2]) {
            pc = address;
        } else {
            pc += 4;
        }
        return 0;
    } else if (strcmp(instr->instruction, "JAL") == 0) {
        int address = getLabelAddress(&labelTable, instr->label);
        registers[instr->op1] = pc + 4;
        if (instr->op1 == 0) registers[0] = 0;
        pc = address;
        return 0;
    } else if (strcmp(instr->instruction, "JALR") == 0) {
        registers[instr->op1] = pc + 4;
        if (instr->op1 == 0) registers[0] = 0;
        pc = registers[instr->op2] + instr->offset;
        return 0;
    } else if (strcmp(instr->instruction, "EXIT") == 0) {
        return 2; // EXIT
    } else {
        // 레이블 처리
        printf("Label : %s\n", instr->labelName);
        labelCnt++;
        return 1;
    }
    printf("instr : %s, op1 : %d, op2 : %d, op3 : %d, label : %s, labelName : %s, "
           "pc : %d\n", instr->instruction, instr->op1, instr->op2, instr->op3, instr->label, instr->labelName, pc);
    return 0;
}

void decimalToBinary(int n, int binaryNum[]) {
    // 32비트의 정수를 저장할 배열
    int i = 0;

    // 배열 초기화
    for (i = 0; i < 32; i++) {
        binaryNum[i] = 0;
    }

    // 10진수를 2진수로 변환
    i = 31;
    while (n > 0) {
        binaryNum[i--] = n % 2;
        n = n / 2;
    }
}

int main(void) {
    FILE *inputFile, *outputFile, *traceFile;
    char fileName[100];
    char line[100];
    Instruction instr[5001];

    while (1) {
        initialize();
        // 파일 이름 입력
        scanf("%s", fileName);

        // 파일 이름이 terminate가 입력된 경우에는 프로그램 수행 종료 OK
        if ((strcmp(fileName, "terminate") == 0) || strcmp(fileName, "terminate.s") == 0) {
            printf("im sorry");
            break;
        }
        // inputFile에 파일을 저장 OK
        inputFile = fopen(fileName, "r");

        // 입력 파일이 존재하지 않는 경우 메시지 출력 후 파일명 재입력 OK
        if (!inputFile) {
            printf("Input file does not exist!!");
            continue;
        }
        // 하나의 명령어를 한 줄씩 입력 받음 OK

        int address = 1000;
        while (fgets(line, sizeof(line), inputFile)) {
            // 빈 줄 건너뛰기
            if (strlen(line) <= 1) continue;
            Instruction instr = {};
            parseLine(line, &instr);
            if (instr.label[0] != '\0') {
                addLabel(&labelTable, instr.label, address);
                continue;
            }
            address += 4;
        }

        sprintf(fileName + strlen(fileName) - 2, ".o");
        outputFile = fopen(fileName, "w");
        sprintf(fileName + strlen(fileName) - 2, ".trace");
        traceFile = fopen(fileName, "w");
        pc = 1000;
        int hasError = 0;
        // 파일에 직접 쓰는 작업
        while (fgets(line, sizeof(line), inputFile)) {
            if (strlen(line) <= 1) continue;
            Instruction instr = {};
            parseLine(line, &instr);

            if (instr.label[0] != '\0') continue;

            unsigned int machineCode;
            if (strcmp(instr.instruction, "EXIT") == 0) {
                machineCode = 0xFFFFFFFF;
                fprintf(traceFile, "%d\n", pc);
                break;
            } else if (strcmp(instr.instruction, "BEQ") == 0 || strcmp(instr.instruction, "BNE") == 0 ||
                       strcmp(instr.instruction, "BGE") == 0 || strcmp(instr.instruction, "BLT") == 0 ||
                       strcmp(instr.instruction, "JAL") == 0) {
                int labelAddress = getLabelAddress(&labelTable, instr.label);
                if (labelAddress == -1) {
                    printf("Syntax Error!!\n");
                    hasError = 1;
                    break;
                }
                instr.offset = (labelAddress - pc) / 4;
            }

            machineCode = convertInstruction(&instr);
            if (hasError) break;

            for (int i = 31; i >= 0; i--) {
                fprintf(outputFile, "%d", (machineCode >> i) & 1);
            }
            fprintf(outputFile, "\n");
            fprintf(traceFile, "%d\n", pc);
            pc += 4;
        }

        fclose(inputFile);
        fclose(outputFile);
        fclose(traceFile);

        if (!hasError) {
            printf("Output files generated: %s.o and %s.trace\n", fileName, fileName);
        } else {
            remove(strcat(fileName, ".o"));
            remove(strcat(fileName, ".trace"));
        }
    }


    // for (int i = 0; i < cnt; i++) {
    //     unsigned int machinecode = convertInstruction(&instr[i]);
    //     if (machinecode == 0) continue; // label일 때
    //     decimalToBinary(machinecode, binarycode);
    //     for (int i = 0; i < 32; i++) {
    //         fprintf(outputFile, "%d", binarycode[i]);
    //     }
    //     fprintf(outputFile, "\n");
    // } // 완료 but BEQ 했을 때 약간 틀림

    // pc = 1000;
    // printf("pc : %d\n", pc);
    // fprintf(traceFile, "%d\n", pc);
    // while (l + 1 < cnt) {
    //     l = (pc - 1000) / 4 + labelCnt; // 만약 직전이 레이블이었으면 한 칸 뛰어넘음
    //     printf("l : %d, cnt : %d\n", l, cnt);
    //     int chk = executionInstruction(&instr[l]);
    //     if (chk == 1) {
    //         continue; // 레이블인 경우
    //     } else {
    //         fprintf(traceFile, "%d\n", pc - 4);
    //         printf("pc : %d\n", pc - 4);
    //     }
    //     if (chk == 2) break; // exit()
    // }

    // fclose(outputFile);
    // fclose(traceFile);

return 0;
}
