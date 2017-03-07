import javax.wukong.wknode.WuNodeLeds;

public class WuNodeTest
{
	public static void main(String args[])
	{
		System.out.println("Hello, world!");
		while (true) {
			System.out.println("On!");
			WuNodeLeds.setLed(0, true);
			Thread.sleep(500);
			System.out.println("Off!");			
			WuNodeLeds.setLed(0, false);
			Thread.sleep(500);
		}
	}
}
