package javax.lecdemo;

public class Adder {
    private int amount;

    public Adder(int amount) {
        this.amount = amount;
    }

    public int add(int x) {
        return x + amount;
    }
}
