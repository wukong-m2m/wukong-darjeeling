package java.io;

public abstract class OutputStream
{

	public void close()
	{
	}

	public void write(byte[] b)
	{
		write(b, 0, b.length);
	}

	public void write(byte[] b, int off, int len)
	{
		len += off;
		for (; off < len; off++)
		{
			write(b[off]);
		}
	}

	public void flush() throws IOException
	{
	}

	abstract public void write(int b);
}
