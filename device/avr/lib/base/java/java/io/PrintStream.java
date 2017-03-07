/**
 * 
 */
package java.io;

/**
 * @author Michael Maaser
 *
 */
public class PrintStream extends OutputStream {

	private OutputStream underlying;

	/**
	 * 
	 */
	public PrintStream(OutputStream os) {
		this.underlying = os;
	}

	/* (non-Javadoc)
	 * @see java.io.OutputStream#write(int)
	 */
	public void write(int b) {
		if (underlying != null) {
			underlying.write(b);
		}
	}

	public void print(String string) {
		write(string.getBytes());
	}

	public void println(String string) {
		write(string.getBytes());
		write((int)'\r');
		write((int)'\n');
	}

	public void println() {
		write((int)'\r');
		write((int)'\n');
	}	
}
