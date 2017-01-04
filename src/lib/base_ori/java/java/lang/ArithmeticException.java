package java.lang;

/**
 * Thrown when an exceptional arithmetic condition has occurred. For example, 
 * an integer "divide by zero" throws an instance of this class.
 * 
 * @author Niels Brouwers
 */
public class ArithmeticException extends RuntimeException
{
	/**
	 * Constructs an <code>ArrayStoreException</code> with no detail message.
	 */
	public ArithmeticException()
	{
		super();
	}

	/**
	 * Constructs an <code>ArithmeticException</code> with the specified detail
	 * message.
	 * 
	 * @param message the detail message.
	 */
	public ArithmeticException(String message)
	{
		super(message);
	}
}
