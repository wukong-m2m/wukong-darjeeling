import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.io.*;
import java.util.*;

public abstract class DeviceTreeNode extends DefaultMutableTreeNode {
	public abstract int getClientId();
}
