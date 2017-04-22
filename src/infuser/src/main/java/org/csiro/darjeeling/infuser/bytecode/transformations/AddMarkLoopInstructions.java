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
import org.csiro.darjeeling.infuser.bytecode.instructions.MarkLoopInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.BranchTargetInstruction;

// TODO Known issue: In the example below, we originally used return to exit the loop, but the infuser won't add the MARKLOOP instructions if we do.
//                   It does if we use break it does, but this is a temporary fix.
    // @Lightweight
    // public static void siftDown(int a[], short start, short end) {
    //     short root = 0;
    //     short child;
    //     while ( (child = (short)((root << 1)+1)) < end ) {
    //         short child_plus_one = (short)(child + 1);
    //         if ((child_plus_one < end) && (a[child] < a[child_plus_one])) {
    //             child += 1;
    //         }
    //         int a_root = a[root];
    //         int a_child = a[child];
    //         if (a_root < a_child) {
    //             // SWAP( a[child], a[root] );
    //             a[root] = a_child;
    //             a[child] = a_root;

    //             root = child;
    //         }
    //         else
    //             break;
    //     }        
    // }



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
    private final static int RTC_VALUETAG_NEEDS_LOAD     = 0x0800;
    private final static int RTC_VALUETAG_NEEDS_STORE    = 0x0400;


    private static boolean isTYPE_LOCAL(int valuetag) { return (valuetag & 0xC000) == RTC_VALUETAG_TYPE_LOCAL; }
    private static boolean isTYPE_STATIC(int valuetag) { return (valuetag & 0xC000) == RTC_VALUETAG_TYPE_STATIC; }
    private static boolean isTYPE_CONSTANT(int valuetag) { return (valuetag & 0xC000) == RTC_VALUETAG_TYPE_CONSTANT; }
    private static boolean isDATATYPE_REF(int valuetag) { return (valuetag & 0x3000) == RTC_VALUETAG_DATATYPE_REF; }


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

    private static boolean isDestructive(Instruction instruction) {
        Opcode opcode = instruction.getOpcode();
        switch (instruction.getOpcode()) {
            case ASTORE:
            case ASTORE_0:
            case ASTORE_1:
            case ASTORE_2:
            case ASTORE_3:
            case SSTORE:
            case SINC:
            case SINC_W:
            case SSTORE_0:
            case SSTORE_1:
            case SSTORE_2:
            case SSTORE_3:
            case ISTORE:
            case IINC:
            case IINC_W:
            case ISTORE_0:
            case ISTORE_1:
            case ISTORE_2:
            case ISTORE_3:
                return true;

            default:
                return false;
        }
    }

    private static boolean isLiveAtInstruction(InstructionHandle instruction, int valuetag) {
        if (isTYPE_CONSTANT(valuetag) || isTYPE_STATIC(valuetag)) {
            return true;
        }
        if (isDATATYPE_REF(valuetag)) {
            return instruction.getLiveVariables().containsReferenceIndex(valuetag & 0x03FF);
        } else {
            return instruction.getLiveVariables().containsIntegerIndex(valuetag & 0x03FF);            
        }
    }

    private static boolean isModifiedInBlock(InstructionList instructions, InstructionHandle beginTargetHandle, InstructionHandle endBranchHandle, int valuetag) {
        if (isTYPE_CONSTANT(valuetag)) {
            return false;
        }
        if (isTYPE_STATIC(valuetag)) {
            return true;
        }


        for (int i=0; i<instructions.size(); i++)
        {
            InstructionHandle handle = instructions.get(i);
            Instruction instruction = handle.getInstruction();

            if (beginTargetHandle.getPc() < handle.getPc() && handle.getPc() < endBranchHandle.getPc()) {
                if (instructionToValuetag(instruction) == valuetag && isDestructive(instruction)) {
                    return true;
                }
            }
        }
        return false;
    }

    private static boolean mayTriggerGC(Instruction instruction) {
        switch (instruction.getOpcode()) {
            case INVOKEVIRTUAL:
            case INVOKESPECIAL:
            case INVOKESTATIC:
            case INVOKEINTERFACE:
            case NEW:
            case NEWARRAY:
            case ANEWARRAY:
                return true;
            default:
                return false;
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

    private static void splitFirstAndLastBranchTargetIfNecessary(InstructionList instructions, InstructionHandle beginTargetHandle, InstructionHandle endBranchHandle) {
        // Examples:
        // 
        // if (!negative) {
        //     i = -i;
        // }
        // while (i <= -radix) {
        //     buf[charPos--] = digits[-(i % radix)];
        //     i = i / radix;
        // }
        // This produces code where the first BRTARGET needs to be split.
        //
        // public static void test(int a, short[] numbers) {
        //     if (a ==1 ) {
        //        for (short i=0; i<NUMNUMBERS; i++) {
        //            numbers[i] = 1;
        //        }
        //     }
        // }
        // This produces code where the final BRTARGET needs to be split.
        //
        // In this case there will be a single branch target at the end of the for loop,
        // but there will be two branches to that location, one from the if, and another
        // when the loop terminates.
        // Because the loop MARKLOOP instruction needs to come after the end of the loop,
        // but shouldn't be executed if we don't enter the loop at all, the branch target
        // needs to be split so the MARKLOOP instruction can sit inbetween.

        if (endBranchHandle.getInstruction().getOpcode() != Opcode.BRTARGET) {
            return; // This only applies to loops that end with a GOTO+BRTARGET.
        }

        InstructionHandle splitBranchTargetHandle = null;

        for (int i=0; i<instructions.size(); i++) {
            InstructionHandle handle = instructions.get(i);
            Instruction instruction = handle.getInstruction();

            // Branch outside the loop to the first branch target
            if (instruction.getOpcode().isBranch()
                    && (handle.getPc() < beginTargetHandle.getPc() || endBranchHandle.getPc() < handle.getPc())
                    && handle.getBranchHandle() == beginTargetHandle) {
                // Insert a new branchtarget if we hadn't already.
                if (splitBranchTargetHandle == null) {
                    splitBranchTargetHandle = endBranchHandle.copyToNewHandleWithSameStateAndLiveVariables(new BranchTargetInstruction(Opcode.BRTARGET));
                    instructions.insertBefore(beginTargetHandle, splitBranchTargetHandle);
                }
                handle.setBranchHandle(splitBranchTargetHandle);
            }

            // Branch outside the loop to the last branch target
            if (instruction.getOpcode().isBranch()
                    && (handle.getPc() < beginTargetHandle.getPc() || endBranchHandle.getPc() < handle.getPc())
                    && handle.getBranchHandle() == endBranchHandle) {
                // Insert a new branchtarget if we hadn't already.
                if (splitBranchTargetHandle == null) {
                    splitBranchTargetHandle = endBranchHandle.copyToNewHandleWithSameStateAndLiveVariables(new BranchTargetInstruction(Opcode.BRTARGET));
                    instructions.insertAfter(endBranchHandle, splitBranchTargetHandle);
                }
                handle.setBranchHandle(splitBranchTargetHandle);
            }
        }
    }

    public static String valuetagToString(int valuetag) {
        String type, datatype;
        switch (valuetag & 0xC000) {
            case RTC_VALUETAG_TYPE_LOCAL: type = "Local"; break;
            case RTC_VALUETAG_TYPE_STATIC: type = "Static"; break;
            case RTC_VALUETAG_TYPE_CONSTANT: type = "Constant"; break;
            default: type = "??";
        }
        switch (valuetag & 0x3000) {
            case RTC_VALUETAG_DATATYPE_REF: datatype = "Ref"; break;
            case RTC_VALUETAG_DATATYPE_SHORT: datatype = "Short"; break;
            case RTC_VALUETAG_DATATYPE_INT: datatype = "Int"; break;
            case RTC_VALUETAG_DATATYPE_INT_L: datatype = "Int_l"; break;
            default: datatype = "??";
        }

        return type + " " + datatype + " " + (valuetag & 0x03FF) + " (" + ((valuetag & 0x0800) == 0x0800 ? "NEEDS LOAD" : "") + "," + ((valuetag & 0x0400) == 0x0400 ? "NEEDS STORE" : "") + ")";
    }

    private static void insertMARKLOOP(InstructionList instructions, InstructionHandle beginTargetHandle, InstructionHandle endBranchHandle) {
        HashMap<Integer,Integer> valuetagDict = new HashMap<Integer,Integer>();
        boolean mayTriggerGC = false;

        for (int i=0; i<instructions.size(); i++)
        {
            InstructionHandle handle = instructions.get(i);
            Instruction instruction = handle.getInstruction();

            if (handle.getPc() < endBranchHandle.getPc() && handle.getPc() > beginTargetHandle.getPc()) {
                int valuetag = instructionToValuetag(instruction);
                if (valuetag != RTC_VALUETAG_UNUSED) {
                    if (valuetagDict.containsKey(valuetag)) {
                        valuetagDict.put(valuetag, valuetagDict.get(valuetag)+1);
                    } else {
                        valuetagDict.put(valuetag, 1);                        
                    }
                }

                // If GC may be triggered in this inner loop, we cannot pin any references since after
                // GC the value wouldn't be correct anymore. It probably doesn't matter much since most
                // operations that may trigger GC are quite expensive, so the gains from pinning the
                // reference variable are limited in comparison.
                mayTriggerGC |= mayTriggerGC(instruction);
            }
        }

        if (mayTriggerGC) {
            // If this inner loop may trigger the GC,
            // remove all reference value tags.
            for (Integer key : valuetagDict.keySet().toArray(new Integer [0])) {
                if (isDATATYPE_REF(key)) {
                    valuetagDict.remove(key);
                }
            }
        }

        InstructionHandle afterBranchHandle = null;
        for (int i=0; i<instructions.size(); i++) {
            if (instructions.get(i) == endBranchHandle) {
                if (i+1 < instructions.size()) {
                    afterBranchHandle = instructions.get(i+1);
                } else {
                    afterBranchHandle = endBranchHandle; // Not sure if this is ever needed, but the branch could be the last instruction in the method.
                }
                break;
            }
        }


        ArrayList<ValuetagCount> valuetagCountList = new ArrayList<ValuetagCount>();
        for (int valuetag : valuetagDict.keySet()) {
            int count = valuetagDict.get(valuetag);
            int flags = 0;
            if (isLiveAtInstruction(beginTargetHandle, valuetag)) {
                flags += RTC_VALUETAG_NEEDS_LOAD;
            }
            if (isLiveAtInstruction(afterBranchHandle, valuetag) && isModifiedInBlock(instructions, beginTargetHandle, endBranchHandle, valuetag)) {
                flags += RTC_VALUETAG_NEEDS_STORE;
            }
            // flags = RTC_VALUETAG_NEEDS_STORE + RTC_VALUETAG_NEEDS_LOAD;
            valuetagCountList.add(new ValuetagCount(valuetag + flags, count));
        }
        Collections.sort(valuetagCountList, Collections.reverseOrder());

        ArrayList<Integer> valuetagList = new ArrayList<Integer>();
        // System.err.println("MARKLOOP_START valuetags: ");
        for (ValuetagCount c : valuetagCountList) {
            valuetagList.add(c.valuetag);
            // System.err.println("Valuetag: " + valuetagToString(c.valuetag));
            // System.err.println("Count: " + c.count);
        }

        InstructionHandle markLoopHandle;
        markLoopHandle = beginTargetHandle.copyToNewHandleWithSameStateAndLiveVariables(new MarkLoopInstruction(Opcode.MARKLOOP_START, valuetagList));
        instructions.insertBefore(beginTargetHandle, markLoopHandle);
        markLoopHandle = endBranchHandle.copyToNewHandleWithSameStateAndLiveVariables(new SimpleInstruction(Opcode.MARKLOOP_END));
        instructions.insertAfter(endBranchHandle, markLoopHandle);
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
            InstructionHandle endBranchHandle = instructions.get(i);
            Instruction endBranchInstruction = endBranchHandle.getInstruction();

            if (endBranchInstruction.getOpcode().isBranch()) {
                int pcAfterBranchInstruction = (i+1) < instructions.size() ? instructions.get(i+1).getPc() : Integer.MAX_VALUE;
                InstructionHandle beginTargetHandle = endBranchHandle.getBranchHandle();

                //  1 the branch branches backwards (it's a loop)
                if (endBranchHandle.getPc() < beginTargetHandle.getPc()) {
                    // Discard this branch.
                    continue;
                }

                // System.err.println("Considering back branch at: " + endBranchHandle.getPc() + ".");

                //  2A there are no other backward branches in the block between this branch and the branch target, to a different location (it's the inner loop)
                boolean reject = false;
                for (int j=0; j<instructions.size(); j++) {
                    InstructionHandle handle = instructions.get(j);
                    if (handle.getInstruction().getOpcode().isBranch()
                            && beginTargetHandle.getPc() < handle.getPc()
                            && handle.getPc() < endBranchHandle.getPc()
                            && handle.getBranchHandle().getPc() < handle.getPc()
                            && handle.getBranchHandle() != beginTargetHandle) {
                        // System.err.println("Reject: inner back branch found at " + handle.getPc() + ".");
                        // Discard this branch.
                        reject = true;
                        break;
                    }
                }
                if (reject) {
                    continue;
                }


                // //  2B there are no other backward branches enveloping this block (it's the outer loop)
                // boolean reject = false;
                // for (int j=0; j<instructions.size(); j++) {
                //     InstructionHandle handle = instructions.get(j);
                //     if (handle.getInstruction().getOpcode().isBranch()
                //             && endBranchHandle.getPc() < handle.getPc()
                //             && handle.getBranchHandle().getPc() < beginTargetHandle.getPc()) {
                //         // System.err.println("Reject: inner back branch found at " + handle.getPc() + ".");
                //         // Discard this branch.
                //         reject = true;
                //         break;
                //     }
                // }
                // if (reject) {
                //     continue;
                // }



                //  3 there are no forward branches beyond the instruction following this branch (single point of exit)
                reject = false;
                for (int j=0; j<instructions.size(); j++) {
                    InstructionHandle handle = instructions.get(j);
                    if (handle.getInstruction().getOpcode().isBranch()
                            && beginTargetHandle.getPc() < handle.getPc()
                            && handle.getPc() < endBranchHandle.getPc()) {
                        // System.err.println("vliegtuig " + pcAfterBranchInstruction + " " + handle.getBranchHandle().getPc());
                            }
                    if (handle.getInstruction().getOpcode().isBranch()
                            && beginTargetHandle.getPc() < handle.getPc()
                            && handle.getPc() < endBranchHandle.getPc()
                            && pcAfterBranchInstruction < handle.getBranchHandle().getPc()) {
                        // System.err.println("Reject: inner forward branch to beyond end of loop found at " + handle.getPc() + ".");
                        // Discard this branch.
                        reject = true;
                        break;
                    }
                }
                if (reject) {
                    continue;
                }

                // 4 there are no later back branches branches to the beginning of the loop (some loops have two, for example if the loop ends in an if block, the compiler may generate a back branch for the reverse condition)
                reject = false;
                for (int j=0; j<instructions.size(); j++) {
                    InstructionHandle handle = instructions.get(j);
                    if (handle.getInstruction().getOpcode().isBranch()
                            && endBranchHandle.getPc() < handle.getPc()
                            && handle.getBranchHandle() == beginTargetHandle) {
                        // System.err.println("Reject: later backbranch to the same loop beginning found at " + handle.getPc() + ".");
                        // Discard this branch.
                        reject = true;
                        break;
                    }
                }
                if (reject) {
                    continue;
                }


                // If this is a conditional back branch, we're done. (not sure if Java ever emits those)
                // If this is a goto, then there will be a branchtarget following, and that branch target
                // should be the end of the loop instead.
                if (endBranchInstruction.getOpcode() == Opcode.GOTO) {
                    InstructionHandle brachTargetHandle = instructions.get(i+1);
                    Instruction brachTargetInstruction = brachTargetHandle.getInstruction();
                    if (brachTargetInstruction.getOpcode() != Opcode.BRTARGET) {
                        // If the back branch marking this block is a GOTO, the next instruction should have been BRTARGET.
                        continue;
                    }
                    endBranchHandle = brachTargetHandle;
                    endBranchInstruction = brachTargetInstruction;
                }

                // System.err.println("Found inner loop from " + beginTargetHandle.getPc() + " to " + endBranchHandle.getPc() + ".");

                // Found an inner loop. If the end is a branch target and other instructions outside the loop branch to it, then we need to split the target.
                splitFirstAndLastBranchTargetIfNecessary(instructions, beginTargetHandle, endBranchHandle);
                // Finally mark the loop.
                insertMARKLOOP(instructions, beginTargetHandle, endBranchHandle);
                i += 2; // Skip the MARKLOOP instructions we just added
            }
        }

    }
}
