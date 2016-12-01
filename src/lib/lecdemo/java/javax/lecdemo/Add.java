package javax.lecdemo;

public class Add {
    public static int add(int a, int b) {
        return a+b;
    }

    public static int x;
    public static int add_x(int a) {
        return a+x;
    }

    public static long xl;
    public static long add_xl(long a) {
        return a+xl;
    }

    public static int add_using_adder(int a, int b) {
        Adder adder = new Adder(a);
        return adder.add(b);
    }

    public static void printPlus1000000(int a) {
        System.out.println("The sum is " + a + 1000000);
    }
}