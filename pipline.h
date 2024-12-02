#pragma once
#include"risc_v.h"
#include"C_type.h"
#include"S_type.h"
#include"I_type.h"
#include"U_type.h"
#include"R_type.h"
#include"J_type.h"
#include"cache.h"

unsigned int jc;
long cy2 = 0;
int nw = 0;

#define MODE 1
FILE* out2;
FILE* cycle_infom;
struct pipl
{
    int id;
    unsigned int insn;
    long pc;
    unsigned int pc_add;
    long addr;
    int rd_fg;
}IF, ID, EX, MEM, WB;

int FLUSH;
int LW_Stall;
int F_Stall;
int hit;
int hit_in_icache;

void Decoder_jud(unsigned int insn) {
    OPCODE = insn & 0x7f;
    imm_4_0 = RD = (insn >> 7) & 0x1f;
    FUNCT3 = (insn >> 12) & 0x7;
    RS1 = (insn >> 15) & 0x1f;
    RS2 = (insn >> 20) & 0x1f;
    RS3 = (insn >> 27) & 0x1f;
    FUNCT2 = (insn >> 25) & 0x3;
    FUNCT7 = (insn >> 25) & 0x7f;
    imm_11_0 = (insn >> 20) & 0xfff;
    imm_11_5 = (insn >> 25) & 0x7f;
    imm_12 = (insn >> 31) & 0x1;
    imm_10_5 = (insn >> 25) & 0x3f;
    imm_4_1 = (insn >> 8) & 0xf;
    imm_B_11 = (insn >> 7) & 0x1;
    imm_31_12 = (insn >> 12) & 0xfffff;
    imm_19_12 = (insn >> 12) & 0xff;
    imm_J_11 = (insn >> 20) & 0x1;
    imm_10_1 = (insn >> 21) & 0x3ff;
    imm_20 = (insn >> 31) & 0x1;
}

void CDecoder_jud(unsigned int insn_16)
{
    COPCODE = insn_16 & 0b11;
    CRD = CRS1 = (insn_16 >> 7) & 0b11111;
    CRS2 = (insn_16 >> 2) & 0b11111;
    CRD_ = CRS2_ = (insn_16 >> 2) & 0b111;
    CRS1_ = (insn_16 >> 7) & 0b111;
    CFUNCT2 = (insn_16 >> 5) & 0b11;
    CFUNCT3 = (insn_16 >> 13) & 0b111;
    CFUNCT4 = (insn_16 >> 12) & 0b1111;
    CFUNCT6 = (insn_16 >> 10) & 0b111111;
    cimm_12 = (insn_16 >> 12) & 0b1;
    cimm_6_2 = coffset_6_2 = (insn_16 >> 2) & 0b11111;
    cimm_12_7 = (insn_16 >> 7) & 0b111111;
    cimm_12_5 = (insn_16 >> 5) & 0b11111111;
    cimm_12_10 = coffset_12_10 = (insn_16 >> 10) & 0b111;
    cimm_6_5 = (insn_16 >> 5) & 0b11;
    cimm_j = (insn_16 >> 2) & 0x7ff;
}


int judge_stall(int pos) {
    int pcd = ((instr_flow[pos - 1] & 0b11) == 0b11) ? 4 : 2;
    int rt = cycles;
    if (pcd == 4) {
        Decoder_jud(instr_flow[pos - 1]);
        if (LB || LBU || LH || LHU || LW) {
            int rd0 = RD;
            int pcd2 = ((instr_flow[pos] & 0b11) == 0b11) ? 4 : 2;
            if (pcd2 == 4) {
                Decoder_jud(instr_flow[pos]);
                if (JALR || ADDI || SLTI || SLTIU || XORI || ORI || ANDI || SLLI || SRLI || SRAI || FCVTDW || LB || LBU || LH || LHU || LW || FLW || FLD) {
                    if (rd0 == RS1) cycles++;
                }
                else if (BEQ || BGE || BGEU || BLT || BLTU || BNE || FSD || SB || SH || SW || (OPCODE == R_type)) {
                    if (RS1 == rd0 || RS2 == rd0) cycles++;
                }
            }
            else {
                CDecoder_jud(instr_flow[pos]);
                if (CFLD || CLW || CSRLI || CSRAI || CANDI || CBEQZ || CBNEZ) {
                    if (CRS1_ + 8 == rd0)cycles++;
                }
                else if (CFSD || CSW || ((COPCODE == 0b1) && (CFUNCT3 == 0b100))) {
                    if (CRS1_ + 8 == rd0 || CRS2_ + 8 == rd0) cycles++;
                }
                else if (CADDI_16sp || CADDI_4spn || CFLD_sp || CLW_sp) {
                    if (rd0 == 2) cycles++;
                }
                else if (CADDI || CSLLI || CJR || CJALR) {
                    if (rd0 == CRS1)cycles++;
                }
                else if (CADD) {
                    if (rd0 == CRS1 || rd0 == CRS2) cycles++;
                }
                else if (CSW_sp || CFSD_sp) {
                    if (rd0 == CRS2 || rd0 == 2) cycles++;
                }
                else if (CMV) {
                    if (CRS2 == rd0) cycles++;
                }
            }
        }
        else if (FLW || FLD) {
            int rd0 = RD;
            int pcd2 = ((instr_flow[pos] & 0b11) == 0b11) ? 4 : 2;
            if (pcd2 == 4) {
                Decoder_jud(instr_flow[pos]);
                if (FSD) {
                    if (RS2 == rd0) cycles++;
                }
                else if (FADDD || FEQD || FLTD || FMULD) {
                    if (RS1 == rd0 || RS2 == rd0) cycles++;
                }
            }
            else {
                CDecoder_jud(instr_flow[pos]);
                if (CFSD) {
                    if (CRS2_ + 8 == rd0) cycles++;
                }
                else if (CFSD_sp) {
                    if (rd0 == CRS2) cycles++;
                }
            }
        }
    }
    else {
        CDecoder_jud(instr_flow[pos - 1]);
        if (CLW || CLW_sp) {
            int rd0 = CRD_ + 8;
            if (CLW_sp) rd0 = CRD;
            int pcd2 = ((instr_flow[pos] & 0b11) == 0b11) ? 4 : 2;
            if (pcd2 == 4) {
                Decoder_jud(instr_flow[pos]);
                if (JALR || ADDI || SLTI || SLTIU || XORI || ORI || ANDI || SLLI || SRLI || SRAI || FCVTDW || LB || LBU || LH || LHU || LW || FLW || FLD) {
                    if (rd0 == RS1) cycles++;
                }
                else if (BEQ || BGE || BGEU || BLT || BLTU || BNE || FSD || SB || SH || SW || (OPCODE == R_type) || FADDD || FEQD || FLTD || FMULD) {
                    if (RS1 == rd0 || RS2 == rd0) cycles++;
                }
            }
            else {
                CDecoder_jud(instr_flow[pos]);
                if (CFLD || CLW || CSRLI || CSRAI || CANDI || CBEQZ || CBNEZ) {
                    if (CRS1_ + 8 == rd0)cycles++;
                }
                else if (CFSD || CSW || ((COPCODE == 0b1) && (CFUNCT3 == 0b100))) {
                    if (CRS1_ + 8 == rd0 || CRS2_ + 8 == rd0) cycles++;
                }
                else if (CADDI_16sp || CADDI_4spn || CFLD_sp || CLW_sp) {
                    if (rd0 == 2) cycles++;
                }
                else if (CADDI || CSLLI || CJR || CJALR) {
                    if (rd0 == CRS1)cycles++;
                }
                else if (CADD) {
                    if (rd0 == CRS1 || rd0 == CRS2) cycles++;
                }
                else if (CSW_sp || CFSD_sp) {
                    if (rd0 == CRS2 || rd0 == 2) cycles++;
                }
                else if (CMV) {
                    if (CRS2 == rd0) cycles++;
                }
            }
        }
        else if (CFLD || CFLD_sp) {
            int rd0 = CRD_ + 8;
            if (CFLD_sp) rd0 = CRD;
            int pcd2 = ((instr_flow[pos] & 0b11) == 0b11) ? 4 : 2;
            if (pcd2 == 4) {
                Decoder_jud(instr_flow[pos]);
                if (FSD) {
                    if (RS2 == rd0) cycles++;
                }
                else if (FADDD || FEQD || FLTD || FMULD) {
                    if (RS1 == rd0 || RS2 == rd0) cycles++;
                }
            }
            else {
                CDecoder_jud(instr_flow[pos]);
                if (CFSD) {
                    if (CRS2_ + 8 == rd0) cycles++;
                }
                else if (CFSD_sp) {
                    if (rd0 == CRS2) cycles++;
                }
            }
        }
    }
    return cycles - rt;
}

int Stall() {
    if (EX.insn == 0 || ID.insn == 0) return 0;
    int pcd = ((EX.insn & 0b11) == 0b11) ? 4 : 2;
    int res = 0;
    if (pcd == 4) {
        Decoder_jud(EX.insn);
        if (LB || LBU || LH || LHU || LW) {
            int rd0 = RD;
            int pcd2 = ((ID.insn & 0b11) == 0b11) ? 4 : 2;
            if (pcd2 == 4) {
                Decoder_jud(ID.insn);
                if (JALR || ADDI || SLTI || SLTIU || XORI || ORI || ANDI || SLLI || SRLI || SRAI || FCVTDW || LB || LBU || LH || LHU || LW || FLW || FLD) {
                    if (rd0 == RS1) res++;
                }
                else if (BEQ || BGE || BGEU || BLT || BLTU || BNE || FSD || SB || SH || SW || (OPCODE == R_type)) {
                    if (RS1 == rd0 || RS2 == rd0) res++;
                }
            }
            else {
                CDecoder_jud(ID.insn);
                if (CFLD || CLW || CSRLI || CSRAI || CANDI || CBEQZ || CBNEZ) {
                    if (CRS1_ + 8 == rd0)res++;
                }
                else if (CFSD || CSW || ((COPCODE == 0b1) && (CFUNCT3 == 0b100))) {
                    if (CRS1_ + 8 == rd0 || CRS2_ + 8 == rd0) res++;
                }
                else if (CADDI_16sp || CADDI_4spn || CFLD_sp || CLW_sp) {
                    if (rd0 == 2) res++;
                }
                else if (CADDI || CSLLI || CJR || CJALR) {
                    if (rd0 == CRS1)res++;
                }
                else if (CADD) {
                    if (rd0 == CRS1 || rd0 == CRS2) res++;
                }
                else if (CSW_sp || CFSD_sp) {
                    if (rd0 == CRS2 || rd0 == 2) res++;
                }
                else if (CMV) {
                    if (CRS2 == rd0) res++;
                }
            }
        }
        else if (FLW || FLD) {
            int rd0 = RD;
            int pcd2 = ((ID.insn & 0b11) == 0b11) ? 4 : 2;
            if (pcd2 == 4) {
                Decoder_jud(ID.insn);
                if (FSD) {
                    if (RS2 == rd0) res++;
                }
                else if (FADDD || FEQD || FLTD || FMULD) {
                    if (RS1 == rd0 || RS2 == rd0) res++;
                }
            }
            else {
                CDecoder_jud(ID.insn);
                if (CFSD) {
                    if (CRS2_ + 8 == rd0) res++;
                }
                else if (CFSD_sp) {
                    if (rd0 == CRS2) res++;
                }
            }
        }
    }
    else {
        CDecoder_jud(EX.insn);
        if (CLW || CLW_sp) {
            int rd0 = CRD_ + 8;
            if (CLW_sp) rd0 = CRD;
            int pcd2 = ((ID.insn & 0b11) == 0b11) ? 4 : 2;
            if (pcd2 == 4) {
                Decoder_jud(ID.insn);
                if (JALR || ADDI || SLTI || SLTIU || XORI || ORI || ANDI || SLLI || SRLI || SRAI || FCVTDW || LB || LBU || LH || LHU || LW || FLW || FLD) {
                    if (rd0 == RS1) res++;
                }
                else if (BEQ || BGE || BGEU || BLT || BLTU || BNE || FSD || SB || SH || SW || (OPCODE == R_type) || FADDD || FEQD || FLTD || FMULD) {
                    if (RS1 == rd0 || RS2 == rd0) res++;
                }
            }
            else {
                CDecoder_jud(ID.insn);
                if (CFLD || CLW || CSRLI || CSRAI || CANDI || CBEQZ || CBNEZ) {
                    if (CRS1_ + 8 == rd0)res++;
                }
                else if (CFSD || CSW || ((COPCODE == 0b1) && (CFUNCT3 == 0b100))) {
                    if (CRS1_ + 8 == rd0 || CRS2_ + 8 == rd0) res++;
                }
                else if (CADDI_16sp || CADDI_4spn || CFLD_sp || CLW_sp) {
                    if (rd0 == 2) res++;
                }
                else if (CADDI || CSLLI || CJR || CJALR) {
                    if (rd0 == CRS1)res++;
                }
                else if (CADD) {
                    if (rd0 == CRS1 || rd0 == CRS2) res++;
                }
                else if (CSW_sp || CFSD_sp) {
                    if (rd0 == CRS2 || rd0 == 2) res++;
                }
                else if (CMV) {
                    if (CRS2 == rd0) res++;
                }
            }
        }
        else if (CFLD || CFLD_sp) {
            int rd0 = CRD_ + 8;
            if (CFLD_sp) rd0 = CRD;
            int pcd2 = ((ID.insn & 0b11) == 0b11) ? 4 : 2;
            if (pcd2 == 4) {
                Decoder_jud(ID.insn);
                if (FSD) {
                    if (RS2 == rd0) res++;
                }
                else if (FADDD || FEQD || FLTD || FMULD) {
                    if (RS1 == rd0 || RS2 == rd0) res++;
                }
            }
            else {
                CDecoder_jud(ID.insn);
                if (CFSD) {
                    if (CRS2_ + 8 == rd0) res++;
                }
                else if (CFSD_sp) {
                    if (rd0 == CRS2) res++;
                }
            }
        }
    }
    return res;
}

int Flush() {
    return (!FLUSH) && (IF.pc != pc_flow[nw]);
}

void F_stall() {
    int pcd = ((EX.insn & 0b11) == 0b11) ? 4 : 2;
    if (F_Stall) return;
    if (pcd == 4) {
        Decoder_jud(EX.insn);
        if (FADDD) {
            F_Stall += 3;
        }
        if (FMULD) {
            F_Stall += 8;
        }
    }
}

int fstall(int i) {
    int pcd = ((instr_flow[i-1] & 0b11) == 0b11) ? 4 : 2;
    if (pcd == 4) {
        Decoder_jud(instr_flow[i - 1]);
        if (FADDD) {
            cycles += 3;
            return 2;
        }
        if (FMULD) {
            cycles += 8;
            return 7;
        }
    }
    return 0;
}

void count_cy() {
    cycles = 2;
    jc = 0;
    int lstt = 0;
    int fstt = 0;
    out2 = fopen("lwtime.txt", "w");
    count_cycles[1] = cycles;
    for (int i = 1; i < cnt; i++) {
        cycles++;
        count_cycles[i + 1] = cycles;
        int pcd = ((instr_flow[i - 1] & 0b11) == 0b11) ? 4 : 2;

        if (pc_flow[i] - pc_flow[i - 1] != pcd)
            cycles++, jc++;
        fstt += fstall(i);
        int tpp = judge_stall(i);
        //if (!MODE && tpp == 1) fprintf(out2,"cycles:%8d.    i-th%8d.    ins:%#8X.    pc:%#8x.\n", cycles, i,instr_flow[i-1],pc_flow[i]);
        lstt += tpp;
    }
    printf("lw_stall:%d\n", lstt);
    printf("fd_stall:%d\n", fstt);
    fclose(out2);
}
char ss[6][10] = { "IF","ID","EX","MEM","WB" };
void cycle_info(pipl np) {
    //printf("cycle:%d\n", cy2);
    if (!MODE) return;
    fprintf(cycle_infom,"%4s:", ss[np.id]);
    if (!np.id && (LW_Stall || FLUSH || F_Stall)) {
        fprintf(cycle_infom, "\t NOP\n");
    }
    else {
        if (np.insn)
            fprintf(cycle_infom, "\t PC: %#8X \t INSN: %#10X\n", (np.id) ? np.pc : np.pc - np.pc_add, np.insn);
        else fprintf(cycle_infom, "\t NOP\n");
    }
}

void STAGE_IF() {
    if (!hit);
    else if (F_Stall);
    else if (!(LW_Stall || FLUSH)) {
        IF.pc = pc_flow[nw];
        IF.insn = instr_flow[nw];
        IF.addr = addr_flow[nw];
        hit_in_icache = accessICache(IF.addr);
        if (hit_in_icache) {
            IF.rd_fg = rd_rt[nw];
            IF.pc_add = ((IF.insn & 0b11) == 0b11) ? 4 : 2;
            ID.pc = IF.pc;
            ID.addr = IF.addr;
            ID.rd_fg = IF.rd_fg;
            IF.pc = IF.pc + IF.pc_add;
            ID.insn = IF.insn;
            nw++;
        }
    }
    else if (LW_Stall);
    else if (FLUSH) ID.insn = 0;

    cycle_info(IF);
    if (FLUSH && LW_Stall) FLUSH = 0;
}

void STAGE_ID() {
    LW_Stall = Stall();
    FLUSH = Flush();
    cycle_info(ID);
    if (!hit);
    else if (F_Stall);
    else if (LW_Stall || FLUSH)
        EX.insn = 0;
    else
        EX.insn = ID.insn, EX.pc = ID.pc, EX.addr = ID.addr, EX.rd_fg = ID.rd_fg;
}

void STAGE_EX() {
    F_stall();
    cycle_info(EX);
    if (!hit) {
        if (F_Stall) F_Stall--;
    }
    else if (F_Stall) {
        F_Stall--;
        if (F_Stall == 0) {
            MEM.insn = EX.insn;
            MEM.pc = EX.pc;
            MEM.addr = EX.addr;
            MEM.rd_fg = EX.rd_fg;
        }
        else 
            MEM.insn = 0;
    }
    else {
        MEM.insn = EX.insn;
        MEM.pc = EX.pc;
        MEM.addr = EX.addr;
        MEM.rd_fg = EX.rd_fg;
    }
}

void STAGE_MEM() {
    cycle_info(MEM);
    if (MEM.rd_fg == 0) 
        hit = 1;
    else 
        hit = accessDCache(MEM.rd_fg, MEM.addr);
    if (!hit) 
        WB.insn = 0;
    else {
        WB.insn = MEM.insn;
        WB.pc = MEM.pc;
    }
}

void STAGE_WB() {
    cycle_info(WB);
}

void pipline() {
    IF.pc = pc_flow[0];
    IF.insn = ID.insn = EX.insn = MEM.insn = WB.insn = 0;
    IF.id = 0, ID.id = 1, EX.id = 2, MEM.id = 3, WB.id = 4;
    F_Stall = 0;
    if (MODE) {
        cycle_infom = fopen("cycle_info.txt", "w");
    }
    for (;;) {
        cy2++;
        if (MODE)
            fprintf(cycle_infom,"\n=================CYCLE:%d=================\n", cy2);
        STAGE_WB();
        STAGE_MEM();
        STAGE_EX();
        STAGE_ID();
        STAGE_IF();
        if (MODE)
            fprintf(cycle_infom,"=========================================\n");
        if (nw > cnt) break;
    }
}