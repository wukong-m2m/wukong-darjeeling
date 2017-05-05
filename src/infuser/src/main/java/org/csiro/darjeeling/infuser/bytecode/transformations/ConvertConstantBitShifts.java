package org.csiro.darjeeling.infuser.bytecode.transformations;

import java.util.HashMap;
import java.util.ArrayList;
import java.util.Collections;

import org.csiro.darjeeling.infuser.Infuser;
import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.CodeBlockTransformation;
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.instructions.PushInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.ConstantBitShiftInstruction;

public class ConvertConstantBitShifts extends CodeBlockTransformation
{
    public ConvertConstantBitShifts(CodeBlock codeBlock)
    {
        super(codeBlock);
    }

    public boolean isNormalBitShift(Opcode opcode) {
        return opcode == Opcode.SSHL
            || opcode == Opcode.SSHR
            || opcode == Opcode.SUSHR
            || opcode == Opcode.ISHL
            || opcode == Opcode.ISHR
            || opcode == Opcode.IUSHR;
    }

    public Opcode getConstantBitShiftVersion(Opcode opcode) {
            if (opcode == Opcode.SSHL) return Opcode.SSHL_CONST;
            if (opcode == Opcode.SSHR) return Opcode.SSHR_CONST;
            if (opcode == Opcode.SUSHR) return Opcode.SUSHR_CONST;
            if (opcode == Opcode.ISHL) return Opcode.ISHL_CONST;
            if (opcode == Opcode.ISHR) return Opcode.ISHR_CONST;
            if (opcode == Opcode.IUSHR) return Opcode.IUSHR_CONST;
            throw new Error("opcode not a bit shift opcode.");
    }

    @Override
    protected void transformInternal()
    {
                // addInstruction(new ImmediateBytePushInstruction(Opcode.BSPUSH, 6));
                // addInstruction(new ArithmeticInstruction(Opcode.SSHR));
                // addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));

        if (!Infuser.getUseConstantShiftOptimisation()) {
            // Dan niet.
            return;
        }

        InstructionList instructions = codeBlock.getInstructions();
        for (int i=0; i<instructions.size()-1; i++)
        {
            InstructionHandle firstHandle = instructions.get(i);
            if (firstHandle.getInstruction() instanceof PushInstruction) {
                // Current instruction is a constant push
                InstructionHandle secondHandle = instructions.get(i+1);
                Opcode opcode = secondHandle.getInstruction().getOpcode();
                if (isNormalBitShift(opcode)) {
                    // And the next is a bit shift.

                    PushInstruction constantPushInstruction = (PushInstruction)firstHandle.getInstruction();
                    int bitsToShiftBy = (int)constantPushInstruction.getValue();

                    // We will only optimise cases that avr-gcc also optimises. There's no reason to want to do this, other than making a fair comparison to what's possible in native code, since avr-gcc could do better here.

                    // avr-gcc 4.9.1
                    //     SSHL       ISHL       SSHR       ISHR    SUSHR      IUSHR
                    // 1                    all single shifts
                    // 2                    all double shifts
                    // 3   shifts     shifts*    shifts     loop    shifts     loop
                    // 4   special    shifts*    shifts     loop    special    loop
                    // 5   special    shifts*    shifts     loop    special    loop
                    // 6   special    shifts*    special    loop    special    loop
                    // 7   special    shifts*    special    loop    special    loop
                    // 8   move       move       move       move    move       move
                    // 9   move+shift loop       move+shift loop    move+shift loop
                    // 10  move+shift loop       move+shift loop    move+shift loop
                    // 11  move+shift loop       move+shift loop    move+shift loop
                    // 12  mov+swp    loop       move+shift loop    mov+swp    loop
                    // 13  mov+swp+sh loop       move+shift loop    mov+swp+sh loop
                    // 14  mov+swp+sh loop       special    loop    special    loop
                    // 15  special    loop       special    loop    special    loop
                    // 16  0          move       special    move    0          move
                    // 17             loop                  loop               loop
                    // 18             loop                  loop               loop
                    // 19             loop                  loop               loop
                    // 20             loop                  loop               loop
                    // 21             loop                  loop               loop
                    // 22             loop                  loop               loop
                    // 23             loop                  loop               loop
                    // 24             move                  move               move
                    // 25             loop                  loop               loop
                    // 26             loop                  loop               loop
                    // 27             loop                  loop               loop
                    // 28             loop                  loop               loop
                    // 29             loop                  loop               loop
                    // 30             loop                  loop               loop
                    // 31             special               special            special
                    // 32  0          0          special    special 0          0
                    // * = seems to do some completely unnecessary moves for no apparent reason


                    // Keep the loop when opcode = ISHL, ISHR or IUSHR
                    if (opcode == Opcode.ISHL || opcode == Opcode.ISHR || opcode == Opcode.IUSHR) {
                        // and number of bits > 2
                        if (bitsToShiftBy > 2) {
                            // and number of bits%8 != 0
                            if (bitsToShiftBy%8 !=0) {
                                if (opcode == Opcode.ISHL) {
                                    if (bitsToShiftBy > 8) {
                                        continue;
                                    }
                                } else { // ISHR or IUSHR
                                    continue;
                                }
                            }
                        }
                    }

                    ConstantBitShiftInstruction constantShift = new ConstantBitShiftInstruction(getConstantBitShiftVersion(opcode), bitsToShiftBy);

                    // Remove the constant push
                    instructions.remove(firstHandle);
                    // And replace the bit shift with a constant bit shift
                    secondHandle.setInstruction(constantShift);
                }
            }
        }
    }
}
