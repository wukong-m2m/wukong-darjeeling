import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.io.*;
import java.util.*;

public class LogTextArea extends TextArea {
	public LogTextArea() {
		super();
		this.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));
	}
}
