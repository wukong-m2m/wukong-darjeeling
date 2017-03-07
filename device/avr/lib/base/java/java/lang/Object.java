package java.lang;

public class Object
{

	public native String toString();

	public boolean equals(Object obj)
	{
		return (this == obj);
	}

	public native int hashCode();
	
}
