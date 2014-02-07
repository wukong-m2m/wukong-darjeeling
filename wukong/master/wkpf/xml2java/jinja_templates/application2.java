import javax.wukong.wkpf.*;
import javax.wukong.virtualwuclasses.*;

public class WKDeploy {
    /* Component names to indexes:
    {%- for component in changesets.components %}
    {{ component.type }} => {{ component.deployid }}
    {%- endfor %}
    */

    public static void main (String[] args) {
        // Application: {{ name }}
        System.out.println("My node id: " + WKPF.getMyNodeId());

        // Register virtual WuClasses (just register all for now, maybe change this per node later)
        {%- for component in changesets.components %}
            {% for wuobject in component.instances %}
                {% if wuobject.virtual %}
        WKPF.registerWuClass((short){{ wuobject.wuclassdef|wuclassid }}, {{ wuobject.wuclassdef|wuclassgenclassname }}.properties);
                {% endif %}
            {% endfor %}
        {%- endfor %}


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
        //all WuClasses from the same group has the same instanceIndex and wuclass
        {%- for component in changesets.components %}
            {% for wuobject in component.instances %}
                // If wuclass is not on nodes and has to create a virtual wuobject
                {% if wuobject.virtual %}
        if (WKPF.isLocalComponent((short){{ component.deployid }})) {
            VirtualWuObject wuclassInstance{{ wuobject.wuclassdef|wuclassname }} = new {{ wuobject.wuclassdef|wuclassvirtualclassname }}();
            WKPF.createWuObject((short){{ wuobject.wuclassdef|wuclassid }}, WKPF.getPortNumberForComponent((short){{ component.deployid }}), wuclassInstance{{ wuobject.wuclassdef|wuclassname }});
        }
                {% endif %}
            {% endfor %}
        {%- endfor %}
    }
}
