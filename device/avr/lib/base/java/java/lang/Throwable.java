package java.lang;

public class Throwable
{

	String message;
	Throwable cause;

	public Throwable()
	{
	};

	public Throwable(String message)
	{
		this.message = message;
	}

	public Throwable(Throwable cause)
	{
		this.cause = cause;
	}

	public Throwable(String message, Throwable cause)
	{
		this.message = message;
		this.cause = cause;
	}

	public String getMessage()
	{
		return message;
	}

}