package org.csiro.darjeeling.infuser.bytecode.transformations;

import java.util.HashMap;
import java.util.ArrayList;
import java.util.Collections;
import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.CodeBlockTransformation;
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.instructions.SimpleInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.LoadStoreInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.IncreaseInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.WideIncreaseInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.MarkLoopStartInstruction;

public class AddMarkLoopInstructions extends CodeBlockTransformation
{
    public AddMarkLoopInstructions(CodeBlock codeBlock)
    {
        super(codeBlock);
    }

    private final static int RTC_VALUETAG_TYPE_LOCAL     = 0x0000;
    private final static int RTC_VALUETAG_TYPE_STATIC    = 0x4000;
    private final static int RTC_VALUETAG_TYPE_CONSTANT  = 0x8000;
    private final static int RTC_VALUETAG_UNUSED         = 0xFFFF;
    private final static int RTC_VALUETAG_DATATYPE_REF   = 0x0000;
    private final static int RTC_VALUETAG_DATATYPE_SHORT = 0x1000;
    private final static int RTC_VALUETAG_DATATYPE_INT   = 0x2000;
    private final static int RTC_VALUETAG_DATATYPE_INT_L = 0x3000;


    private static int instructionToValuetag(Instruction instruction) {
        Opcode opcode = instruction.getOpcode();
        switch (instruction.getOpcode()) {
            case ALOAD:
            case ASTORE:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_REF   + ((LoadStoreInstruction)instruction).getIndex();
            case ALOAD_0:
            case ALOAD_1:
            case ALOAD_2:
            case ALOAD_3:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_REF + opcode.getOpcode() - Opcode.ALOAD_0.getOpcode();
            case ASTORE_0:
            case ASTORE_1:
            case ASTORE_2:
            case ASTORE_3:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_REF + opcode.getOpcode() - Opcode.ASTORE_0.getOpcode();

            case SLOAD:
            case SSTORE:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_SHORT + ((LoadStoreInstruction)instruction).getIndex();
            case SINC:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_SHORT + ((IncreaseInstruction)instruction).getLocalVariable().getIntegerIndex();
            case SINC_W:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_SHORT + ((WideIncreaseInstruction)instruction).getLocalVariable().getIntegerIndex();
            case SLOAD_0:
            case SLOAD_1:
            case SLOAD_2:
            case SLOAD_3:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_SHORT + opcode.getOpcode() - Opcode.SLOAD_0.getOpcode();
            case SSTORE_0:
            case SSTORE_1:
            case SSTORE_2:
            case SSTORE_3:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_SHORT + opcode.getOpcode() - Opcode.SSTORE_0.getOpcode();

            case ILOAD:
            case ISTORE:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_INT   + ((LoadStoreInstruction)instruction).getIndex();
            case IINC:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_INT   + ((IncreaseInstruction)instruction).getLocalVariable().getIntegerIndex();
            case IINC_W:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_INT   + ((WideIncreaseInstruction)instruction).getLocalVariable().getIntegerIndex();
            case ILOAD_0:
            case ILOAD_1:
            case ILOAD_2:
            case ILOAD_3:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_INT   + opcode.getOpcode() - Opcode.ILOAD_0.getOpcode();
            case ISTORE_0:
            case ISTORE_1:
            case ISTORE_2:
            case ISTORE_3:
                return RTC_VALUETAG_TYPE_LOCAL + RTC_VALUETAG_DATATYPE_INT   + opcode.getOpcode() - Opcode.ISTORE_0.getOpcode();

            case SCONST_M1:
            case SCONST_0:
            case SCONST_1:
            case SCONST_2:
            case SCONST_3:
            case SCONST_4:
            case SCONST_5:
                return RTC_VALUETAG_TYPE_CONSTANT + RTC_VALUETAG_DATATYPE_SHORT + opcode.getOpcode() - Opcode.SCONST_M1.getOpcode();

            case ICONST_M1:
            case ICONST_0:
            case ICONST_1:
            case ICONST_2:
            case ICONST_3:
            case ICONST_4:
            case ICONST_5:
                return RTC_VALUETAG_TYPE_CONSTANT + RTC_VALUETAG_DATATYPE_INT + opcode.getOpcode() - Opcode.ICONST_M1.getOpcode();

            case ACONST_NULL:
                return RTC_VALUETAG_TYPE_CONSTANT + RTC_VALUETAG_DATATYPE_REF + 0;

            default:
                return RTC_VALUETAG_UNUSED;
        }
    }

    private static class ValuetagCount implements Comparable<ValuetagCount> {
        public int valuetag;
        public int count;
        public ValuetagCount(int valuetag, int count) {
            this.valuetag = valuetag;
            this.count = count;
        }
        public int compareTo(ValuetagCount v2) {
            return this.count - v2.count;
        }
    }

    private static void insertMARKLOOP(InstructionList instructions, InstructionHandle loopBeginHandle, InstructionHandle loopEndHandle) {
        HashMap<Integer,Integer> valuetagDict = new HashMap<Integer,Integer>();
        for (int i=0; i<instructions.size(); i++)
        {
            InstructionHandle handle = instructions.get(i);
            Instruction instruction = handle.getInstruction();

            if (handle.getPc() < loopEndHandle.getPc() && handle.getPc() > loopBeginHandle.getPc()) {
                int valuetag = instructionToValuetag(instruction);
                if (valuetag != RTC_VALUETAG_UNUSED) {
                    if (valuetagDict.containsKey(valuetag)) {
                        valuetagDict.put(valuetag, valuetagDict.get(valuetag)+1);
                    } else {
                        valuetagDict.put(valuetag, 1);                        
                    }
                }
            }
        }

        ArrayList<ValuetagCount> valuetagCountList = new ArrayList<ValuetagCount>();
        for (int valuetag : valuetagDict.keySet()) {
            valuetagCountList.add(new ValuetagCount(valuetag, valuetagDict.get(valuetag)));
        }
        Collections.sort(valuetagCountList, Collections.reverseOrder());

        ArrayList<Integer> valuetagList = new ArrayList<Integer>();
        for (ValuetagCount c : valuetagCountList) {
            valuetagList.add(c.valuetag);
            System.err.println("Valuetag: " + c.valuetag);
            System.err.println("Count: " + c.count);
        }

        InstructionHandle markLoopHandle;
        markLoopHandle = new InstructionHandle(new MarkLoopStartInstruction(Opcode.MARKLOOP_START, valuetagList));
        markLoopHandle.setPreState(loopBeginHandle.getPreState());
        markLoopHandle.setPostState(loopBeginHandle.getPostState());
        instructions.insertBefore(loopBeginHandle, markLoopHandle);
        markLoopHandle = new InstructionHandle(new SimpleInstruction(Opcode.MARKLOOP_END));
        markLoopHandle.setPreState(loopEndHandle.getPreState());
        markLoopHandle.setPostState(loopEndHandle.getPostState());
        instructions.insertAfter(loopEndHandle, markLoopHandle);
    }

    @Override
    protected void transformInternal()
    {
        // Add MARKLOOP instructions around each inner loop.
        // An inner loop if found when there is a branch that satisfies:
        //  1 the branch branches backwards (it's a loop)
        //  2 there are no other backward branches in the block between this branch and the branch target (it's the inner loop)
        //  3 there are no forward branches beyond the instruction following this branch (single point of exit)
        // insert a BRTARGET instruction in front of each branch target instruction

        InstructionList instructions = codeBlock.getInstructions();
        for (int i=0; i<instructions.size(); i++)
        {
            InstructionHandle loopEndHandle = instructions.get(i);
            Instruction instruction = loopEndHandle.getInstruction();
            
            if (instruction.getOpcode().isBranch()) {
                InstructionHandle loopBeginHandle = loopEndHandle.getBranchHandle();

                // System.err.println("Considering branch at: " + loopEndHandle.getPc() + ".");

                //  1 the branch branches backwards (it's a loop)
                if (loopEndHandle.getPc() < loopBeginHandle.getPc()) {
                    // System.err.println("Reject: not a back branch.");
                    // Discard this branch.
                    continue;
                }

                //  2 there are no other backward branches in the block between this branch and the branch target (it's the inner loop)
                boolean reject = false;
                for (int j=0; j<instructions.size(); j++) {
                    InstructionHandle handle = instructions.get(j);
                    if (handle.getInstruction().getOpcode().isBranch()
                            && handle.getPc() < loopEndHandle.getPc()
                            && handle.getPc() > loopBeginHandle.getPc()
                            && handle.getPc() > handle.getBranchHandle().getPc()) {
                        // System.err.println("Reject: inner back branch found at " + handle.getPc() + ".");
                        // Discard this branch.
                        reject = true;
                        break;
                    }
                }
                if (reject) {
                    continue;
                }

                //  3 there are no forward branches beyond the instruction following this branch (single point of exit)
                reject = false;
                for (int j=0; j<instructions.size(); j++) {
                    InstructionHandle handle = instructions.get(j);
                    if (handle.getInstruction().getOpcode().isBranch()
                            && handle.getPc() > loopEndHandle.getPc()
                            && handle.getPc() < loopBeginHandle.getPc()
                            && handle.getBranchHandle().getPc() > (loopBeginHandle.getPc() + 1)) {
                        // System.err.println("Reject: inner forward branch to beyond end of loop found at " + handle.getPc() + ".");
                        // Discard this branch.
                        continue;
                    }
                }
                if (reject) {
                    continue;
                }

                // System.err.println("Found inner loop from " + loopBeginHandle.getPc() + " to " + loopEndHandle.getPc() + ".");

                // Found an inner loop. Mark it.
                insertMARKLOOP(instructions, loopBeginHandle, loopEndHandle);
                i += 2; // Skip the MARKLOOP instructions we just added
            }
        }

    }
}
