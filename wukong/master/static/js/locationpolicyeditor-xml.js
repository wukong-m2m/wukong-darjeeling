/*
 * WuKong
 * Location Policy Editor - XML version
 * 2015, June 23
 * Yeh, Hsin Yuan
 * Capability:
 *      - create the editor GUI by xml
 *      - generate the location policy string
 *
 * Note: (HY: 2015-July-01)
 *  This version is stopped developing, due to
 *  simple combination is enough at this moment.
 */

/* Sample
~range(10,0,0,0) | farthest(0,0,0) & ~id(1)
<or>
  <or>
    <and>
      <not>
        <function>
          <ITEM>range</ITEM>
          <ITEM>10</ITEM>
          <ITEM>0</ITEM>
          <ITEM>0</ITEM>
          <ITEM>0</ITEM>
        </function>
      </not>
    </and>
    <or>
      <and>
        <function>
          <ITEM>farthest</ITEM>
          <ITEM>0</ITEM>
          <ITEM>0</ITEM>
          <ITEM>0</ITEM>
        </function>
        <and>
          <not>
            <function>
              <ITEM>id</ITEM>
              <ITEM>1</ITEM>
            </function>
          </not>
        </and>
      </and>
    </or>
  </or>
</or>
*/

function LocationPolicyGroup(editor,parent,xmlEle){
    this.editor = editor
    this.parent = parent
    this.funcname = ''
    this.not = false
    this.width = 0
    this.top = 0
    this.left = 0
    this.children = []
    this.setXml(xmlEle)
}
LocationPolicyGroup.prototype = {
    render:function(){
        if (this.tagName=='FUNCTION'){

            var p = this
            while (p=p.parent){
                if ( p.tagName=='NOT' ) continue
                if (p.children.length==1) continue
                p.width += 350
                p.left += 30;
            }

            var tags = ['<div class="LocationPolicyGroup">']
            tags.push('<div class="LocationPolicyGroupButtons"><select><option></option><option>AND</option><option>OR</option><option>NOT</option></select></div>')
            var funcnameSel = ['<select class="LocationPolicyFuncnameSel">']
            var names = [' ','id','use','range','closest','farest']
            for (var i=0,name;name=names[i];i++){
                name = name.trim()
                var sel = name==this.funcname ? ' selected' : ''
                funcnameSel.push('<option value="'+name+'"'+sel+'>'+name+'</option>')
            }
            funcnameSel.push('</select>')
            tags.push('<div class="LocationPolicyGroupFuncname">'+funcnameSel.join('')+'</div>')
            tags.push('<div class="LocationPolicyGroupParam"><input style="width:100%;height:30px">')
            tags.push('</div>') //LocationPolicyGroupParam
            tags.push('</div>') //LocationPolicyGroup
            return tags.join('')
        }
        else{
            //for AND and OR
            if (this.children.length==1) {
                if (this.parent) this.parent.width += 30
                return this.children[0].render()
            }
            var tags = []
            for (var i=0,child;child =this.children[i];i++){
                var tag = child.render()
                if (tag) tags.push(tag)
            }
            var prefix = []
            var addEncloseTag = this.tagName != 'NOT'
            if (addEncloseTag){
                prefix.push('<div class="LocationPolicyGroupButtons"><select><option></option><option>AND</option><option>OR</option><option>NOT</option></select></div>')

                var style = []
                if (this.tagName=='OR'){
                    var top = 0;
                    var p = this
                    var divHeight = 30
                    while (p=p.parent){
                        if (p.tagName=='OR') top+=divHeight
                    }
                    style.push( 'top:'+top+'px' )
                }
                /*
                else if (this.tagName=='AND'){
                    var divWidth = 300;
                    var width = divWidth * this.children.length
                    style.push( 'width:'+width+'px' )
                }
                */
                // add width
                style.push( 'width:'+this.width+'px' )
                style.push( 'left:'+this.left+'px' )
                // add z-index; for inner div to receive mouse event
                var p = this
                var z = 0
                while (p = p.parent){
                    z += 1
                }
                if (z) style.push('z-index:'+z)
                prefix.push('<div style="'+style.join(';')+'" class="LocationPolicy'+this.tagName+'">')

                tags.unshift(prefix.join(''))
                tags.push('</div>')
            }

            return tags.join('\n\n')
        }
    },
    setXml:function(xmlEle){
        this.xmlEle = xmlEle
        this.tagName = ''
        if (!xmlEle) return
        var tagName = this.xmlEle.tagName
        this.tagName = tagName
        switch(tagName){
            case 'AND':
            case 'NOT':
                this.not = true
            case 'OR':
                var children = this.xmlEle.children;
                for (var i=0,child;child=children[i];i++){
                    var childGroup = new LocationPolicyGroup(this.editor,this,child)
                    this.children.push(childGroup)
                }
                console.log('or #'+this.children.length)
                break;
            case 'FUNCTION':
                break
            case 'ITEM':
                break;
            default:
                console.log('Unknown XML Tag:'+tagName)
        }
    }
}
function LocationPolicyEditor(div,contentDiv){
    this.div = div
    this.contentDiv = contentDiv
    this.firstGroup = new LocationPolicyGroup(this,null,null)
}
LocationPolicyEditor.prototype={
    loadXml:function(){
        var xml = '<or>  <or>    <and>      <not>        <function>          <ITEM>range</ITEM>          <ITEM>10</ITEM>          <ITEM>0</ITEM>          <ITEM>0</ITEM>          <ITEM>0</ITEM>        </function>      </not>    </and>    <or>      <and>        <function>          <ITEM>farthest</ITEM>          <ITEM>0</ITEM>          <ITEM>0</ITEM>          <ITEM>0</ITEM>        </function>        <and>          <not>            <function>              <ITEM>id</ITEM>              <ITEM>1</ITEM>            </function>          </not>        </and>      </and>    </or>  </or></or>'
        var root = document.getElementById('locationpolicyxml')
        root.innerHTML = xml
        this.xmlroot = root
        this.firstGroup.setXml(root.firstChild)
    },
    render:function(){
        var tags = []
        tags.push(this.firstGroup.render())
        this.contentDiv.innerHTML = tags.join('')
        // set the layout
        var viewportSize = getViewportSize()
        this.div.style.width = (viewportSize.width*0.8)+'px'
        this.div.style.height = (viewportSize.height*0.8)+'px'
        this.div.style.left = (viewportSize.width*0.1)+'px'
        this.div.style.top = (viewportSize.height*0.1)+'px'
        this.div.style.display='inline-block'
    }
}