package java.io;

public abstract class InputStream
{

	public void close()
	{
	}

	public int available()
	{
		return 0;
	}

	public void mark(int readlimit)
	{
	}

	public boolean markSupported()
	{
		return false;
	}

	public void reset()
	{
	}

	public void skip(int n)
	{
		for (; n > 0; n--)
		{
			read();
		}
	}

	abstract public int read();

	public int read(byte[] b)
	{
		return read(b, 0, b.length);
	}

	public int read(byte[] b, int off, int len)
	{
		int i = 0;
		for (; i < len; i++)
		{
			b[off + i] = (byte) read();
		}
		return i;
	}
}
