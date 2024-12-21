#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LABELS 100
#define LABEL_LENGTH 20

typedef struct {
    char label[LABEL_LENGTH];
    int address;
} Label;

typedef struct {
    Label labels[MAX_LABELS];
    int count;
} LabelTable;

void addLabel(LabelTable *labelTable, const char *labelName, int address) {
    if (labelTable->count >= MAX_LABELS) {
        printf("Label table is full!\n");
        return;
    }
    strcpy(labelTable->labels[labelTable->count].label, labelName);
    labelTable->labels[labelTable->count].address = address;
    labelTable->count++;
}

int getLabelAddress(LabelTable *labelTable, const char *labelName) {
    for (int i = 0; i < labelTable->count; i++) {
        if (strcmp(labelTable->labels[i].label, labelName) == 0) {
            return labelTable->labels[i].address;
        }
    }
    printf("Label not found: %s\n", labelName);
    return -1; // Label not found
}

int main() {
    LabelTable labelTable;
    labelTable.count = 0;

    // 레이블 추가 예제
    addLabel(&labelTable, "start", 1000);
    addLabel(&labelTable, "loop", 1012);
    addLabel(&labelTable, "end", 1024);

    // 레이블 주소 가져오기 예제
    char labelName[LABEL_LENGTH];
    printf("Enter label name: ");
    scanf("%s", labelName);

    int address = getLabelAddress(&labelTable, labelName);
    if (address != -1) {
        printf("Label %s has address %d\n", labelName, address);
    }

    return 0;
}
