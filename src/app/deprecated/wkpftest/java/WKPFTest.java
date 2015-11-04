import javax.wukong.wkpf.WKPF;
import javax.wukong.wkpf.VirtualWuObject;
import javax.wukong.virtualwuclasses.GENERATEDWKPF;
import javax.wukong.virtualwuclasses.GENERATEDVirtualThresholdWuObject;
import javax.wukong.virtualwuclasses.VirtualThresholdWuObject;

public class WKPFTest {
	// private static int passedCount=0;
	// private static int failedCount=0;

	// private static class VirtualTestWuClass extends VirtualWuObject {
	// 	public static final byte[] properties = new byte[] {
	// 		WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_READWRITE,
	// 		WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READWRITE
	// 	};
	// 	public void update() {
	// 		System.out.println("WKPFUPDATE(VirtualTestWuClass): noop");
	// 	}
	// }

	// public static void assertEqual(int value, int expected, String message) {
	// 	if (value == expected) {
	// 		System.out.println("OK: " + message);
	// 		passedCount++;
	// 	} else {
	// 		System.out.println("----------->FAIL: " + message);
	// 		System.out.println("Expected: " + expected + " Got: " + value);
	// 		failedCount++;
	// 	}
	// }
	// public static void assertEqualBoolean(boolean value, boolean expected, String message) {
	// 	if (value == expected) {
	// 		System.out.println("OK: " + message);
	// 		passedCount++;
	// 	} else {
	// 		System.out.println("----------->FAIL: " + message);
	// 		failedCount++;
	// 	}
	// }
	// public static void assertEqualObject(Object value, Object expected, String message) {
	// 	if (value == expected) {
	// 		System.out.println("OK: " + message);
	// 		passedCount++;
	// 	} else {
	// 		System.out.println("----------->FAIL: " + message);
	// 		failedCount++;
	// 	}
	// }

	public static void main(String[] args) {
		System.out.println("WuKong WuClass Framework test");

		// byte[] linkDefinitions = {
		//     // Note: Component instance id and wuclass id are little endian
		//     // Note: using WKPF constants now, but this should be generated as literal bytes by the WuML->Java compiler.
		//     // Connect input controller to threshold
		//         (byte)0, (byte)0, (byte)0, (byte)1, (byte)0, (byte)2, (byte)1, (byte)0,
		//         (byte)1, (byte)0, (byte)3, (byte)2, (byte)0, (byte)1, (byte)42, (byte)0
		// };
		// WKPF.loadLinkDefinitions(linkDefinitions);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "setting link definitions");

		// Object[] componentInstanceToWuObjectAddrMap = {
		// 			new byte[]{ 1, 0x10 }, // The test wuclass
		// 			new byte[]{ 1, 0x11 }, // The threshold
		// 			new byte[]{ 1, 0x12 },
		// 			new byte[]{ 2, 0x10 }
		// 			};
		// WKPF.loadComponentToWuObjectAddrMap(componentInstanceToWuObjectAddrMap);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "setting component-node map");

		// WKPF.registerWuClass((short)0x42, VirtualTestWuClass.properties);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering VirtualTestWuClass as id 0x42.");

		// WKPF.registerWuClass((short)0x43, VirtualTestWuClass.properties);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "... and as id 0x43.");

		// VirtualWuObject wuclassInstanceA = new VirtualTestWuClass();
		// WKPF.createWuObject((short)0x42, (byte)0x10, wuclassInstanceA);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for wuclass instance A at port 0x10.");

		// VirtualWuObject wuclassInstanceB = new VirtualTestWuClass();
		// WKPF.createWuObject((short)0x42, (byte)0x10, wuclassInstanceB);
		// assertEqual(WKPF.getErrorCode(), WKPF.ERR_PORT_IN_USE, "Creating another wuobject for wuclass instance B at port 0x10, should fail.");

		// WKPF.destroyWuObject((byte)0x10);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Removing wuclass instance A and wuobject at port 0x10.");

		// WKPF.createWuObject((short)0x42, (byte)0x10, wuclassInstanceB);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating another wuobject for wuclass instance B at port 0x10, now it should work.");

		// WKPF.setPropertyShort(wuclassInstanceB, (byte)0, (short)123);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 0 for wuclass instance B to 123.");

		// assertEqual(WKPF.getPropertyShort(wuclassInstanceB, (byte)0), 123, "Getting value for property 0 for wuclass instance B, should be 123.");

		// WKPF.setPropertyShort(wuclassInstanceA, (byte)0, (short)123);
		// assertEqual(WKPF.getErrorCode(), WKPF.ERR_WUOBJECT_NOT_FOUND, "Setting property 0 for wuclass instance A to 123, should fail (A was deleted).");

		// WKPF.setPropertyShort(wuclassInstanceB, (byte)1, (short)123);
		// assertEqual(WKPF.getErrorCode(), WKPF.ERR_WRONG_DATATYPE, "Setting property 1 for wuclass instance B to 123, should fail (prop 1 is a boolean).");

		// WKPF.setPropertyBoolean(wuclassInstanceB, (byte)1, true);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 for wuclass instance B to true.");
		// assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceB, (byte)1), true, "Getting value for property 1 for wuclass instance B, should be true.");

		// WKPF.setPropertyBoolean(wuclassInstanceB, (byte)1, false);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 for wuclass instance B to false.");
		// assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceB, (byte)1), false, "Getting value for property 1 for wuclass instance B, should be false.");

		// VirtualWuObject wuclassInstanceThreshold = new VirtualThresholdWuObject();

		// // A native threshold is already in the VM, but here we're registering a virtual object.
		// // This works, but should never happen in a real application.
		// WKPF.registerWuClass(GENERATEDWKPF.WUCLASS_THRESHOLD, GENERATEDVirtualThresholdWuObject.properties);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering VirtualThresholdWuObject.");

		// WKPF.createWuObject((short)GENERATEDWKPF.WUCLASS_THRESHOLD, (byte)0x11, wuclassInstanceThreshold);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for virtual Threshold wuclass at port 0x11.");

		// WKPF.setPropertyShort(wuclassInstanceThreshold, GENERATEDWKPF.PROPERTY_THRESHOLD_OPERATOR, GENERATEDWKPF.ENUM_THRESHOLD_OPERATOR_GT);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup properties: operator=>");
		// WKPF.setPropertyShort(wuclassInstanceThreshold, GENERATEDWKPF.PROPERTY_THRESHOLD_THRESHOLD, (short)1000);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup properties: threshold=1000");
		// WKPF.setPropertyShort(wuclassInstanceThreshold, GENERATEDWKPF.PROPERTY_THRESHOLD_VALUE, (short)1200);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup properties: value=1200");

		// wuclassInstanceThreshold.update();
		// assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceThreshold, GENERATEDWKPF.PROPERTY_THRESHOLD_OUTPUT), true, "Getting output of virtual threshold wuclass, should be true.");

		// WKPF.setPropertyShort(wuclassInstanceThreshold, GENERATEDWKPF.PROPERTY_THRESHOLD_VALUE, (short)800);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "setting: value=800");

		// wuclassInstanceThreshold.update();
		// assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceThreshold, GENERATEDWKPF.PROPERTY_THRESHOLD_OUTPUT), false, "Getting output of virtual threshold wuclass, should be false.");

		// // Test application:
		// // Component 0: VirtualTestWuClass on node 1, port 0x10
		// // Component 1: Threshold          on node 1, port 0x11
		// // Component 2: VirtualTestWuClass on node 1, port 0x12
		// // Component 3: VirtualTestWuClass on node 2, port 0x10

		// // Links component 0, property 0 -> component 1, property 2 (short property in test wuclass to threshold.value)
		// //       component 1, property 3 -> component 2, property 1 (threshold.output to boolean property in test wuclass)

		// assertEqualBoolean(WKPF.isLocalComponent((short)0), true, "Component 0 is local");
		// assertEqual(WKPF.getPortNumberForComponent((short)0), 0x10, "Component 0 is on port 0x10");
		// assertEqualBoolean(WKPF.isLocalComponent((short)1), true, "Component 1 is local");
		// assertEqual(WKPF.getPortNumberForComponent((short)1), 0x11, "Component 1 is on port 0x11");
		// assertEqualBoolean(WKPF.isLocalComponent((short)2), true, "Component 2 is local");
		// assertEqual(WKPF.getPortNumberForComponent((short)2), 0x12, "Component 2 is on port 0x12");
		// assertEqualBoolean(WKPF.isLocalComponent((short)3), false, "Component 3 is not local");
		// assertEqual(WKPF.getPortNumberForComponent((short)3), 0x10, "Component 3 is on port 0x10");


		// // Update the threshold object through the initialisation functions (access by component id instead of java instance)
		// WKPF.setPropertyShort((short)1, GENERATEDWKPF.PROPERTY_THRESHOLD_VALUE, (short)1200);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Set threshold value=1200");
		// wuclassInstanceThreshold.update();
		// assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceThreshold, GENERATEDWKPF.PROPERTY_THRESHOLD_OUTPUT), true, "Getting output of virtual threshold wuclass, should be true.");

		// WKPF.setPropertyShort((short)3, (byte)0, (short)1200);
		// assertEqual(WKPF.getErrorCode(), WKPF.ERR_REMOTE_PROPERTY_FROM_JAVASET_NOT_SUPPORTED, "Can't set properties for component 2 since it's not local.");

		// assertEqual(WKPF.getMyNodeId(), 1, "My node id is 1");

		// // Test property propagation: set the value on component 0, should propagate through
		// // to the threshold, and then from the threshold's output to component 2's boolean property.
		// // First create component 2.
		// WKPF.createWuObject((short)0x42, (byte)0x12, wuclassInstanceB);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Test property propagation: First creating wuobject for component 2.");

		// WKPF.setPropertyShort((short)0, (byte)0, (short)798);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Set component 0, property 0 to 798.");

		// WKPF.select().update();
		// assertEqual(WKPF.getPropertyShort(wuclassInstanceThreshold, (byte)2), 798, "Propagated 798 value to threshold.");
		// assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceThreshold, (byte)3), false, "Threshold output is now false.");
		// WKPF.select().update();
		// assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceB, (byte)1), false, "Threshold output propagated to component 2, port 1.");

		// WKPF.setPropertyShort((short)0, (byte)0, (short)1500);
		// assertEqual(WKPF.getErrorCode(), WKPF.OK, "Set component 0, property 0 to 1500.");

		// WKPF.select().update();
		// assertEqual(WKPF.getPropertyShort(wuclassInstanceThreshold, (byte)2), 1500, "Propagated 1500 value to threshold.");
		// WKPF.select().update();
		// assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceThreshold, (byte)3), true, "Threshold output is now true.");
		// WKPF.select().update();
		// assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceB, (byte)1), true, "Threshold output propagated to component 2, port 1.");

		// System.out.println("WuKong WuClass Framework test - done. Passed:" + passedCount + " Failed:" + failedCount);
		// // while (true) {} // Need loop to prevent it from exiting the program
	}
}
