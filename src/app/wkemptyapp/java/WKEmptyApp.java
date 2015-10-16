import javax.wukong.wkpf.*;
import javax.wukong.virtualwuclasses.*;

public class WKEmptyApp {
    // Empty application to build an WuKong VM without any app.
    public static void main (String[] args) {
        int node_id = WKPF.getMyNodeId(); System.out.println("Java get node id: "+(node_id >> 24 & 0xff) + "." + (node_id >> 16 & 0xff) + "." + (node_id >> 8 & 0xff) + "." + (node_id & 0xff));

        WKPF.appLoadInitLinkTableAndComponentMap();
        createVirtualWuObjects();
        WKPF.appInitCreateLocalObjectAndInitValues();

        while(true){
            VirtualWuObject wuobject = WKPF.select();
            if (wuobject != null) {
                wuobject.update();
            }
        }
    }

    private static void createVirtualWuObjects() {
    }
}