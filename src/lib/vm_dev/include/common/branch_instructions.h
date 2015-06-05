/*
 * branch_instructions.h
 * 
 * Copyright (c) 2008-2010 CSIRO, Delft University of Technology.
 * 
 * This file is part of Darjeeling.
 * 
 * Darjeeling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darjeeling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Darjeeling.  If not, see <http://www.gnu.org/licenses/>.
 */
 

/**
 * Executes the GOTO instruction. Branches to [offset of the GOTO instruction] + [immediate S16]
 */
static inline void GOTO()
{
	branch(peek16() - 1);
}

/**
 * Executes the IF_ICMPEQ instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a == b
 */
static inline void IF_ICMPEQ()
{
	int32_t temp1, temp2;
	uint16_t offset;
	temp2 = popInt();
	temp1 = popInt();
	offset = fetch16();
	if (temp1==temp2)
		branch(offset-3);
}

/**
 * Executes the IF_ICMPNE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a != b
 */
static inline void IF_ICMPNE()
{
	int32_t temp1, temp2;
	uint16_t offset;
	temp2 = popInt();
	temp1 = popInt();
	offset = fetch16();
	if (temp1!=temp2)
		branch(offset-3);
}

/**
 * Executes the IF_ICMPLT instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a < b
 */
static inline void IF_ICMPLT()
{
	int32_t temp1, temp2;
	uint16_t offset;
	temp2 = popInt();
	temp1 = popInt();
	offset = fetch16();
	if (temp1<temp2)
		branch(offset-3);
}

/**
 * Executes the IF_ICMPGE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a >= b
 */
static inline void IF_ICMPGE()
{
	int32_t temp1, temp2;
	uint16_t offset;
	temp2 = popInt();
	temp1 = popInt();
	offset = fetch16();
	if (temp1>=temp2)
		branch(offset-3);
}

/**
 * Executes the IF_ICMPGT instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a > b
 */
static inline void IF_ICMPGT()
{
	int32_t temp1, temp2;
	uint16_t offset;
	temp2 = popInt();
	temp1 = popInt();
	offset = fetch16();
	if (temp1>temp2)
		branch(offset-3);
}

/**
 * Executes the IF_ICMPLE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a <= b
 */
static inline void IF_ICMPLE()
{
	int32_t temp1, temp2;
	uint16_t offset;
	temp2 = popInt();
	temp1 = popInt();
	offset = fetch16();
	if (temp1<=temp2)
		branch(offset-3);
}

/**
 * Executes the IF_SCMPEQ instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a == b
 */
static inline void IF_SCMPEQ()
{
	int16_t temp1, temp2;
	uint16_t offset;
	temp2 = popShort();
	temp1 = popShort();
	offset = fetch16();
	if (temp1==temp2)
		branch(offset-3);
}

/**
 * Executes the IF_SCMPNE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a != b
 */
static inline void IF_SCMPNE()
{
	int16_t temp1, temp2;
	uint16_t offset;
	temp2 = popShort();
	temp1 = popShort();
	offset = fetch16();
	if (temp1!=temp2)
		branch(offset-3);
}

/**
 * Executes the IF_SCMPLT instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a < b
 */
static inline void IF_SCMPLT()
{
	int16_t temp1, temp2;
	uint16_t offset;
	temp2 = popShort();
	temp1 = popShort();
	offset = fetch16();
	if (temp1<temp2)
		branch(offset-3);
}

/**
 * Executes the IF_SCMPGE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a >= b
 */
static inline void IF_SCMPGE()
{
	int16_t temp1, temp2;
	uint16_t offset;
	temp2 = popShort();
	temp1 = popShort();
	offset = fetch16();
	if (temp1>=temp2)
		branch(offset-3);
}

/**
 * Executes the IF_SCMPGT instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a > b
 */
static inline void IF_SCMPGT()
{
	int16_t temp1, temp2;
	uint16_t offset;
	temp2 = popShort();
	temp1 = popShort();
	offset = fetch16();
	if (temp1>temp2)
		branch(offset-3);
}

/**
 * Executes the IF_SCMPLE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a <= b
 */
static inline void IF_SCMPLE()
{
	int16_t temp1, temp2;
	uint16_t offset;
	temp2 = popShort();
	temp1 = popShort();
	offset = fetch16();
	if (temp1<=temp2)
		branch(offset-3);
}

/**
 * Executes the IFEQ instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a == 0
 */
static inline void IIFEQ()
{
	uint16_t offset = fetch16();
	if (popInt()==0)
		branch(offset-3);
}

/**
 * Executes the IFNE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a != 0
 */
static inline void IIFNE()
{
	uint16_t offset = fetch16();
	if (popInt()!=0)
		branch(offset-3);
}

/**
 * Executes the IFLT instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a < 0
 */
static inline void IIFLT()
{
	uint16_t offset = fetch16();
	if (popInt()<0)
		branch(offset-3);
}

/**
 * Executes the IFGE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a >= 0
 */
static inline void IIFGE()
{
	uint16_t offset = fetch16();
	if (popInt()>=0)
		branch(offset-3);
}

/**
 * Executes the IFGT instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a > 0
 */
static inline void IIFGT()
{
	uint16_t offset = fetch16();
	if (popInt()>0)
		branch(offset-3);
}

/**
 * Executes the IFLE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a <= 0
 */
static inline void IIFLE()
{
	uint16_t offset = fetch16();
	if (popInt()<=0)
		branch(offset-3);
}


/**
 * Executes the IFEQ instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a == 0
 */
static inline void SIFEQ()
{
	uint16_t offset = fetch16();
	if (popShort()==0)
		branch(offset-3);
}

/**
 * Executes the IFNE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a != 0
 */
static inline void SIFNE()
{
	uint16_t offset = fetch16();
	if (popShort()!=0)
		branch(offset-3);
}

/**
 * Executes the IFLT instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a < 0
 */
static inline void SIFLT()
{
	uint16_t offset = fetch16();
	if (popShort()<0)
		branch(offset-3);
}

/**
 * Executes the IFGE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a >= 0
 */
static inline void SIFGE()
{
	uint16_t offset = fetch16();
	if (popShort()>=0)
		branch(offset-3);
}

/**
 * Executes the IFGT instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a > 0
 */
static inline void SIFGT()
{
	uint16_t offset = fetch16();
	if (popShort()>0)
		branch(offset-3);
}

/**
 * Executes the IFLE instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a <= 0
 */
static inline void SIFLE()
{
	uint16_t offset = fetch16();
	if (popShort()<=0)
		branch(offset-3);
}


/**
 * Executes the IFNULL instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a == null
 */
static inline void IFNULL()
{
	uint16_t offset = fetch16();
	if (popRef()==nullref)
		branch(offset-3);
}

/**
 * Executes the IFNULL instruction. Branches to [offset of the GOTO instruction] + [immediate S16] if
 * a == null
 */
static inline void IFNONNULL()
{
	uint16_t offset = fetch16();
	if (popRef()!=nullref)
		branch(offset-3);
}

static inline void IF_ACMPNE()
{
	ref_t a,b;
	uint16_t offset;

	a = popRef();
	b = popRef();
	offset = fetch16();
	if (a!=b)
		branch(offset-3);
}

static inline void IF_ACMPEQ()
{
	ref_t a,b;
	uint16_t offset;
	a = popRef();
	b = popRef();
	offset = fetch16();
	if (a==b)
		branch(offset-3);
}

static inline void ATHROW()
{
	// pop object reference from the stack
	dj_object *obj = REF_TO_VOIDP(popRef());
	dj_exec_throw(obj, pc);
}

/**
 * Executes the TABLESWITCH instruction
 */
static inline void TABLESWITCH()
{
	uint32_t key = popInt();
	uint16_t branchAdress = fetch16();
	uint32_t low = fetch32();
	uint32_t high = fetch32();

	if ((key>=low)&&(key<=high)) branchAdress = peekn16((key-low) * 2);
	branch(branchAdress - 11);
}

/**
 * Executes the LOOKUPSWITCH instruction
 */
static inline void LOOKUPSWITCH()
{
	int i;
	uint32_t key = popInt();
	uint16_t branchAdress = fetch16();
	uint16_t npairs = fetch16();

	// look for the key in the key/adress list
	for (i=0; i<npairs; i++)
	{
		if (peekn32(i*6)==key)
		{
			branchAdress=peekn16(i*6+4);
			continue;
		}
	}
	branch(branchAdress - 5);
}
