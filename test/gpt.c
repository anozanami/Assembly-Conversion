#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_START_ADDR 1000
#define MAX_LINE_LENGTH 100
#define MAX_INSTRUCTIONS 1000

typedef struct {
    unsigned int pc;
    unsigned int instr;
} TraceEntry;

typedef struct {
    char mnemonic[10];
    int rd, rs1, rs2, imm;
} Instruction;

int registers[32] = {0, 1, 2, 3, 4, 5, 6};
TraceEntry trace[MAX_INSTRUCTIONS];
int trace_count = 0;

void add_trace(unsigned int pc) {
    trace[trace_count].pc = pc;
    trace_count++;
}

void write_trace_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    for (int i = 0; i < trace_count; i++) {
        fprintf(file, "%u\n", trace[i].pc);
    }
    fclose(file);
}

unsigned int encode_r_type(int opcode, int funct3, int funct7, int rd, int rs1, int rs2) {
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

unsigned int encode_i_type(int opcode, int funct3, int rd, int rs1, int imm) {
    return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

unsigned int encode_s_type(int opcode, int funct3, int rs1, int rs2, int imm) {
    int imm11_5 = (imm & 0xFE0) >> 5;
    int imm4_0 = imm & 0x1F;
    return (imm11_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm4_0 << 7) | opcode;
}

unsigned int encode_b_type(int opcode, int funct3, int rs1, int rs2, int imm) {
    int imm12 = (imm >> 12) & 1;
    int imm10_5 = (imm >> 5) & 0x3F;
    int imm4_1 = (imm >> 1) & 0xF;
    int imm11 = (imm >> 11) & 1;
    return (imm12 << 31) | (imm11 << 7) | (imm10_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm4_1 << 8) | opcode;
}

unsigned int encode_u_type(int opcode, int rd, int imm) {
    return (imm << 12) | (rd << 7) | opcode;
}

unsigned int encode_j_type(int opcode, int rd, int imm) {
    int imm20 = (imm >> 20) & 1;
    int imm10_1 = (imm >> 1) & 0x3FF;
    int imm11 = (imm >> 11) & 1;
    int imm19_12 = (imm >> 12) & 0xFF;
    return (imm20 << 31) | (imm19_12 << 12) | (imm11 << 20) | (imm10_1 << 21) | (rd << 7) | opcode;
}

void process_instruction(Instruction instr, unsigned int *pc) {
    unsigned int machine_code = 0;

    // Example for R-type instruction ADD
    if (strcmp(instr.mnemonic, "add") == 0) {
        machine_code = encode_r_type(0x33, 0x0, 0x0, instr.rd, instr.rs1, instr.rs2);
    }
    // other instructions like SUB, AND, OR...
    else if (strcmp(instr.mnemonic, "exit") == 0) {
        machine_code = 0xFFFFFFFF;
    } else {
        printf("Syntax Error!!\n");
        return;
    }

    *pc += 4;
    add_trace(*pc);
}

int main() {
    char filename[50];
    FILE *inputFile;
    char line[MAX_LINE_LENGTH];
    Instruction instructions[MAX_INSTRUCTIONS];
    unsigned int pc = MEM_START_ADDR;
    int instr_count = 0;

    while (1) {
        printf("Enter input file name (or 'terminate' to exit): ");
        scanf("%s", filename);

        if (strcmp(filename, "terminate") == 0) break;

        inputFile = fopen(filename, "r");
        if (!inputFile) {
            printf("Input file does not exist!!\n");
            continue;
        }

        // Parse the input file
        while (fgets(line, sizeof(line), inputFile)) {
            if (strlen(line) > 1) {
                Instruction instr = {0};
                sscanf(line, "%s x%d, x%d, x%d", instr.mnemonic, &instr.rd, &instr.rs1, &instr.rs2);
                instructions[instr_count++] = instr;
            }
        }
        fclose(inputFile);

        for (int i = 0; i < instr_count; i++) {
            process_instruction(instructions[i], &pc);
        }

        char outputFilename[50];
        sprintf(outputFilename, "%s.o", filename);
        FILE *outputFile = fopen(outputFilename, "w");
        for (int i = 0; i < instr_count; i++) {
            fprintf(outputFile, "%08s\n", instructions[i].mnemonic);
        }
        fclose(outputFile);

        char traceFilename[50];
        sprintf(traceFilename, "%s.trace", filename);
        write_trace_file(traceFilename);

        printf("Successfully generated %s and %s.\n", outputFilename, traceFilename);
    }

    return 0;
}
