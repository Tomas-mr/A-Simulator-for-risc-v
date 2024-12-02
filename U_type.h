#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "risc_v.h"

#define U1_type  0b0010111
#define U2_type	 0b0110111

#define AUIPC (OPCODE == U1_type)
#define LUI  (OPCODE == U2_type)

#define ECALL (insn == 0x73)


#define CSRRS (OPCODE == 0b1110011) && (FUNCT3 == 0b010)

long INSN_AUIPC(long pc) {
	if(RD)
		reg32[RD] = pc+(imm_31_12<<12);
	return pc + 4;
} 

long INSN_LUI(long pc) {
	if(RD)
		reg32[RD] = (imm_31_12<<12);
	return pc + 4;
}

long INSN_CSRRS(long pc) {
	int csr = (insn >> 20);
	int t = CSR[csr];
	CSR[csr] |= reg32[RS1];
	reg32[RD] = t;
	return pc + 4;
}

long INSN_ECALL(long pc)
{
	long type = reg32[17];

	switch (type)
	{
	case 57: {
		reg32[10] = 0;
		break;
	}//_close,返回为0，即close成功
	case 64: {
		putchar((char)(*(mem + reg32[11])));
		break; }//_write
		   
	case 80: {
		reg32[10] = 0; 
		break; 
	}//_fstat，返回为0，执行成功
	case 93: {
		end = 1;
		//exit(0); 
		break; 
	} //exit(0);//_exit
	case 214: {
		int inr = reg32[10];
		int heap_end = read_mem_word(0x20a30);
		heap_end += inr;
		write_mem_word(0x20a30, heap_end);
		reg32[10] = heap_end;
		break;
	}//_sbrk

	default:
		printf("Unknown syscall type %d\n", type);
	}
	return pc + 4;
}