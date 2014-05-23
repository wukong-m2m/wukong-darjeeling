// LDD     10q0 qq0d dddd yqqq, with d=dest register, q=offset from Y or Z, y=1 for Y 0 for Z
#define OP_LDD 		0x8008
// PUSH    1001 001d dddd 1111, with d=source register
#define OP_PUSH		0x920F
// POP     1001 000d dddd 1111
#define OP_POP		0x900F
// LDI     1110 KKKK dddd KKKK, with K=constant to load, d=dest register-16 (can only load to r16-r31)
#define OP_LDI		0xE000
// ADD     0000 11rd dddd rrrr, with d=dest register, r=source register
#define OP_ADD		0x0C00
// ADC     0001 11rd dddd rrrr
#define OP_ADC		0x1C00
// RET     1001 0101 0000 1000
#define OP_RET		0x9508

#define R0			0
#define R1			1
#define R2			2
#define R3			3
#define R4			4
#define R5			5
#define R6			6
#define R7			7
#define R8			8
#define R9			9
#define R10			10
#define R11			11
#define R12			12
#define R13			13
#define R14			14
#define R15			15
#define R16			16
#define R17			17
#define R18			18
#define R19			19
#define R20			20
#define R21			21
#define R22			22
#define R23			23
#define R24			24
#define R25			25
#define R26			26
#define R27			27
#define R28			28
#define R29			29
#define R30			30
#define R31			31

#define Y			1
#define Z			0

uint16_t avr_asm (uint16_t opop, ... );


