import java.awt.Component;
import javax.swing.Icon;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;

public class TestTree {
    private static void createAndShowUI() {
        JFrame frame = new JFrame();
        final JTree tree = new JTree(buildDemoModel());

        tree.setCellRenderer(new DefaultTreeCellRenderer() {
            private Icon loadIcon = UIManager.getIcon("InternalFrame.maximizeIcon");
            private Icon saveIcon = UIManager.getIcon("InternalFrame.closeIcon");
            @Override
            public Component getTreeCellRendererComponent(JTree tree,
                    Object value, boolean selected, boolean expanded,
                    boolean isLeaf, int row, boolean focused) {
                Component c = super.getTreeCellRendererComponent(tree, value,
                        selected, expanded, isLeaf, row, focused);
                if (selected)
                    setIcon(loadIcon);
                else
                    setIcon(saveIcon);
                return c;
            }
        });
        tree.setVisibleRowCount(10);
        frame.add(new JScrollPane(tree));

        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLocationByPlatform(true);
        frame.pack();
        frame.setVisible(true);
    }

    private static DefaultTreeModel buildDemoModel() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Root");

        root.add(new DefaultMutableTreeNode("A"));
        root.add(new DefaultMutableTreeNode("B"));
        root.add(new DefaultMutableTreeNode("C"));

        return new DefaultTreeModel(root);
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                createAndShowUI();
            }
        });
    }
}