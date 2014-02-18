import javax.wukong.wkpf.*;
import javax.wukong.virtualwuclasses.*;

public class WKEmptyApp {
    // Empty application to build an WuKong VM without any app.
    public static void main (String[] args) {
        System.out.println("My node id: " + WKPF.getMyNodeId());

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