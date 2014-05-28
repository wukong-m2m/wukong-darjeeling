import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.Collections;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Enumeration;
import java.util.List;

import org.apache.bcel.Constants;
import org.apache.bcel.generic.Instruction;
import org.apache.bcel.generic.InstructionList;
import org.apache.bcel.generic.InstructionHandle;
import org.apache.bcel.generic.BranchInstruction;
import org.apache.bcel.classfile.ClassParser;
import org.apache.bcel.classfile.Code;
import org.apache.bcel.classfile.JavaClass;
import org.apache.bcel.classfile.Method;

public class JarAnalyser {
	private static int countBranches = 0;
	private static int countTotal = 0;

	private static class BranchSourceAndOffset {
		public int instruction_address;
		public int branch_offset;
		public List<BranchSourceAndOffset> overlap;

		public BranchSourceAndOffset(int instruction_address, int branch_offset) {
			this.instruction_address = instruction_address;
			this.branch_offset = branch_offset;
			this.overlap = new ArrayList<BranchSourceAndOffset>();
		}
	}

	public static int sinterklaas(int x) {
		for (int i=1; i<10000; i++)
			x = x*x;
		return x;
	}

	private static HashMap<JavaClass, HashMap<Method, List<BranchSourceAndOffset>>> branchoffsets;

    public static void main(final String[] args) throws IOException {
    	String zipfilename = args[0];
		ZipFile zipfile = new ZipFile(zipfilename);
		Enumeration<? extends ZipEntry> entries = zipfile.entries();
		System.out.println("Archive " + zipfilename);

		branchoffsets = new HashMap<JavaClass, HashMap<Method, List<BranchSourceAndOffset>>>();
		while(entries.hasMoreElements()) {
			String entryName = entries.nextElement().getName();
			if (entryName.endsWith(".class")) {
				ClassParser parser = new ClassParser(zipfilename, entryName);
				AnalyseClass(parser.parse());
			}
		}


		List<BranchSourceAndOffset> branchoffsets_all = new ArrayList<BranchSourceAndOffset>();
		for (JavaClass c : branchoffsets.keySet()) {
			System.out.println(c.getClassName());
			HashMap<Method, List<BranchSourceAndOffset>> branchoffsets_for_class = branchoffsets.get(c);

			for (Method m : branchoffsets_for_class.keySet()) {
				System.out.println("\t" + m);
				System.out.print("\t\t");
				List<BranchSourceAndOffset> branchoffsets_for_method = branchoffsets_for_class.get(m);
				for (BranchSourceAndOffset i : branchoffsets_for_method)
					System.out.print(i.branch_offset + ", ");
				System.out.println();
				branchoffsets_all.addAll(branchoffsets_for_method);

				// Analyse concurent branches (branches where the source and destination overlap)
				for (BranchSourceAndOffset i : branchoffsets_for_method) {
					for (BranchSourceAndOffset j : branchoffsets_for_method) {
						if (i!=j
							&&  Overlap(i.instruction_address, i.instruction_address+i.branch_offset,
										j.instruction_address, j.instruction_address+j.branch_offset)) {
							i.overlap.add(j);
						}
					}
				}
				System.out.println();				
				for (BranchSourceAndOffset i : branchoffsets_for_method) {
					System.out.print(String.format("\t\tBranch at %d to %d (%d): ", i.instruction_address, i.instruction_address+i.branch_offset, i.branch_offset));
					if (i.overlap.size() == 0)
						System.out.println("no overlap");
					else {
						System.out.print(String.format("overlaps with %d branch(es): ", i.overlap.size()));
						for (BranchSourceAndOffset j : i.overlap) {
							System.out.print(String.format("%d->%d ", j.instruction_address, j.instruction_address+j.branch_offset));
						}
						System.out.println();
					}
				}

			}
		}

		System.out.println();
		System.out.println("All offsets:");
		for (BranchSourceAndOffset i : branchoffsets_all)
				System.out.print(i.branch_offset + ", ");

		System.out.println();
		System.out.println("Total number of instructions: " + countTotal);		
		System.out.println("Number of branches: " + countBranches);		
	}

	public static boolean Overlap(int a1, int a2, int b1, int b2) {
		// Make sure a1<a2 and b1<b2
		if (a2 < a1) {
			int tmp = a1;
			a1 = a2;
			a2 = tmp;
		}
		if (b2 < b1) {
			int tmp = b1;
			b1 = b2;
			b2 = tmp;
		}
		// Check if the start of a overlaps with the whole range of b
		return (a1>=b1 && a1<=b2);
	}

	public static void AnalyseClass(JavaClass c) {
		HashMap<Method, List<BranchSourceAndOffset>> branchoffsets_for_class = new HashMap<Method, List<BranchSourceAndOffset>>();
		branchoffsets.put(c, branchoffsets_for_class);

		Method[] methods = c.getMethods();
		for (Method m : methods) {
			List<BranchSourceAndOffset> branchoffsets_for_method = new ArrayList<BranchSourceAndOffset>();
			branchoffsets_for_class.put(m, branchoffsets_for_method);

			Code code = m.getCode();
			if (code == null)
				continue;
			InstructionList instructionList = new InstructionList(code.getCode());
			instructionList.setPositions();
			for (InstructionHandle handle : instructionList.getInstructionHandles())
			{
				Instruction instruction = handle.getInstruction(); 
				switch (instruction.getOpcode())
				{
					// 2 byte branch
					case Constants.GOTO:
					case Constants.JSR:
					case Constants.IF_ICMPEQ:
					case Constants.IF_ICMPNE:
					case Constants.IF_ICMPLT:
					case Constants.IF_ICMPGE:
					case Constants.IF_ICMPGT:
					case Constants.IF_ICMPLE:
					case Constants.IFEQ:
					case Constants.IFNE:
					case Constants.IFLT:
					case Constants.IFGE:
					case Constants.IFGT:
					case Constants.IFLE:
					case Constants.IF_ACMPEQ:
					case Constants.IF_ACMPNE:
					case Constants.IFNULL:
					case Constants.IFNONNULL:
					// 4 byte branch
					case Constants.GOTO_W:
					case Constants.JSR_W:
						BranchInstruction branch = (BranchInstruction)instruction;
						branchoffsets_for_method.add(new BranchSourceAndOffset(handle.getPosition(), branch.getIndex()));
						countBranches++;
					break;
					// case Constants.TABLESWITCH:
					// case Constants.LOOKUPSWITCH:
					// break;
					default:
					break;
				}
				countTotal++;
			}
		}
	}
}