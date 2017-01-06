/**
 * 
 */
package javax.darjeeling;

import java.io.OutputStream;

/**
 * @author Michael Maaser
 *
 */
public class ConsoleOutputStream extends OutputStream {

	/* (non-Javadoc)
	 * @see java.io.OutputStream#write(int)
	 */
	public void write(int b) {
		Darjeeling.print(""+(char)b);
	}

	public void write(byte[] b)
	{
		Darjeeling.print(new String(b));
	}
}
