import javax.wukong.wkpf.*;
import javax.wukong.virtualwuclasses.*;

public class WKDeploy {
    /* Component names to indexes:
    {%- for component in changesets.components %}
    {{ component.type }} => {{ component.index }}
    {%- endfor %}
    */

    public static void main (String[] args) {
        // Application: {{ name }}
        System.out.println("My node id: " + WKPF.getMyNodeId());

        // Register virtual WuClasses (just register all for now, maybe change this per node later)
        {%- for component in changesets.components %}
            {% for wuobject in component.instances %}
                {% if wuobject.wuclass().virtual %}
        WKPF.registerWuClass(GENERATEDWKPF.{{ wuobject.wuclass().wuclassdef()|wuclassconstname }}, {{ wuobject.wuclass().wuclassdef()|wuclassgenclassname }}.properties);
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
                {% if wuobject.wuclass().virtual %}
        if (WKPF.isLocalComponent((short){{ component.index }})) {
            VirtualWuObject wuclassInstance{{ wuobject.wuclass().wuclassdef()|wuclassname }} = new {{ wuobject.wuclass().wuclassdef()|wuclassvirtualclassname }}();
            WKPF.createWuObject((short)GENERATEDWKPF.{{ wuobject.wuclass().wuclassdef()|wuclassconstname }}, WKPF.getPortNumberForComponent((short){{ component.index }}), wuclassInstance{{ wuobject.wuclass().wuclassdef()|wuclassname }});
        }
                {% endif %}
            {% endfor %}
        {%- endfor %}
    }
}
