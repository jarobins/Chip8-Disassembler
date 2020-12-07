// Test for c code

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define swap16(r) (((r&0xff)<<8)|(r>>8))
#define comb2(r, s) ((r<<4)|s)
#define comb3(r, s, t) ((r<<8)|(s<<4)|(t))

#define MEM_LEN 0x100

struct Inst {
    uint8_t nib1 : 4;
    uint8_t nib2 : 4;
    uint8_t nib3 : 4;
    uint8_t nib4 : 4;
};

void byteswap_array(uint16_t a[], int len) {
    int i = 0;
    for (i;i<len;i++) {
        a[i] = swap16(a[i]);
    }
}

int main (int argc, char *argv[]) {
    uint8_t c;
    uint8_t mem8[MEM_LEN] = {0};
    uint16_t mem16[MEM_LEN/2] = {0};
    uint8_t cond_check;
    FILE *fptr;

    printf("Opening File: %s\n\n", argv[1]);
    fptr = fopen(argv[1], "rb");

    // Check for filename
    if (fptr==NULL) {
        printf("Cannot open file \n");
        return -1;
    }

    // printf("Loading Memory...\n");
    int i = 0;
    c = fgetc(fptr);
    while (i<MEM_LEN) {
        // printf("%02x -> $%02x\n", c, i);
        mem8[i] = (uint8_t)c;
        c = fgetc(fptr);
        i++;
    }

    // Casting into 16bit array and fixing bytes
    memcpy(&mem16, mem8, sizeof(mem8));
    byteswap_array(mem16, MEM_LEN/2);

    // Decode the instructions
    struct Inst inst;

    for (i=0;i<MEM_LEN/2;i++) {
        inst.nib1 = (mem16[i]&0xF000) >> 12;
        inst.nib2 = (mem16[i]&0x0F00) >> 8;
        inst.nib3 = (mem16[i]&0x00F0) >> 4;
        inst.nib4 = (mem16[i]&0x000F);

        printf("$%04x\t%04x\t", (i*2)+0x200, mem16[i]);

        switch(inst.nib1) {
            case 0:
                if (mem16[i] == 0x00E0) {
                    //printf("Clear the Display\n");
                    printf("CLR\n");
                }
                else if (mem16[i] == 0x00EE) {
                    //printf("Return from Subroutine\n");
                    printf("RET\n");
                }
                else {
                    //printf("System Address Jump\n");
                    printf("SYS $%03x\n", comb3(inst.nib2, inst.nib3, inst.nib4));
                }
                break;
            case 1:
                printf("JP %03x\n", comb3(inst.nib2, inst.nib3, inst.nib4));
                break;
            case 2:
                printf("CALL %03x\n", comb3(inst.nib2, inst.nib3, inst.nib4));
                break;
            case 3:
                printf("SE V%X, %02x\n", inst.nib2, comb2(inst.nib3, inst.nib4));
                break;
            case 4:
                printf("SE V%X, %02x\n", inst.nib2, comb2(inst.nib3, inst.nib4));
                break;
            case 5:
                printf("SE V%X, V%x\n", inst.nib2, inst.nib3);
                break;
            case 6:
                //printf("Load Literal Instruction\n");
                printf("LD V%X, #%02x\n", inst.nib2, comb2(inst.nib3, inst.nib4));
                break;
            case 7:
                printf("V%X = V%X + %02x\n", inst.nib2, inst.nib2, comb2(inst.nib3, inst.nib4));
                break;
            case 8:
                switch(inst.nib4) {
                    case 0:
                        printf("V%X = V%X\n", inst.nib2, inst.nib3);
                        break;
                    case 1:
                        printf("V%X = V%X OR V%X\n", inst.nib2, inst.nib3);
                        break;
                    case 2:
                        printf("AND V%X, V%X\n", inst.nib2, inst.nib3);
                        break;
                    case 3:
                        printf("XOR V%X, V%X\n", inst.nib2, inst.nib3);
                        break;
                    case 4:
                        printf("ADD V%X, V%X\n", inst.nib2, inst.nib3);
                        break;
                    case 5:
                        printf("SUB V%X, V%X\n", inst.nib2, inst.nib3);
                        break;
                    case 6:
                        printf("SHR V%X {, V%X}\n", inst.nib2, inst.nib3);
                        break;
                    case 7:
                        printf("SUBN V%X, V%X\n", inst.nib2, inst.nib3);
                        break;
                    case 0xE:
                        printf("SHL V%X {, V%X}\n", inst.nib2, inst.nib3);
                        break;
                    default:
                        printf("UNK\n");
                        break;
                }
                break;
            case 9:
                printf("SNE V%X, V%X\n", inst.nib2, inst.nib3);
                break;
            case 0xA:
                printf("LD I, %03x\n", comb3(inst.nib2, inst.nib3, inst.nib4));
                break;
            case 0xB:
                printf("JP V0, #%03x\n", comb3(inst.nib2, inst.nib3, inst.nib4));
                break;
            case 0xC:
                printf("RND V%X, #%02X\n", inst.nib2, comb2(inst.nib3, inst.nib4));
                break;
            case 0xD:
                printf("DRW V%X, V%X, #%X\n", inst.nib2, inst.nib3, inst.nib4);
                break;
            case 0xE:
                switch(comb2(inst.nib3, inst.nib4)) {
                    case 0x9E:
                        printf("SKP V%X\n", inst.nib2);
                        break;
                    case 0xA1:
                        printf("SKNP V%X\n", inst.nib2);
                        break;
                    default:
                        printf("UNK\n");
                        break;
                }
                break;
            case 0xF:
                switch(comb2(inst.nib3, inst.nib4)) {
                    case 0x07:
                        printf("LD V%X, DT\n", inst.nib2);
                        break;
                    case 0x0A:
                        printf("LD V%X, K\n", inst.nib2);
                        break;
                    case 0x15:
                        printf("LD DT, V%X\n", inst.nib2);
                        break;
                    case 0x18:
                        printf("LD DT, V%X\n", inst.nib2);
                        break;
                    case 0x1E:
                        printf("ADD I, V%X\n", inst.nib2);
                        break;
                    case 0x29:
                        printf("LD F, V%X\n", inst.nib2);
                        break;
                    case 0x33:
                        printf("LD B, V%X\n", inst.nib2);
                        break;
                    case 0x55:
                        printf("LD [I], V%X\n", inst.nib2);
                        break;
                    case 0x65:
                        printf("LD V%X, [I]\n", inst.nib2);
                        break;
                    default:
                        printf("UNK\n");
                        break;
                }
                break;
            default:
                //printf("Unknown Instruction\n");
                printf("UNK\n");
                break;
        }
    }

    fclose(fptr);
    return 0;
}