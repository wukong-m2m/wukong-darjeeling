/*
 * WuKong
 * Location Policy Editor
 * 2015, July 1
 * Yeh, Hsin Yuan
 */

/* Sample
~range(10,0,0,0) | farthest(0,0,0) & ~id(1)
*/

function LocationPolicyGroup(editor,parent,policy_string){
    this.editor = editor
    this.parent = parent
    if (parent) parent.nextgroup = this

    this.deepth = parent ? parent.deepth+1 : 0
    
    this.funcname = 'id'
    this.not = false
    this.parameters = []
    this.op = this.deepth==0 ? '' : 'AND'

    this.width = 0
    this.top = 0
    this.left = 0

    this.setPolicy(policy_string)
}
LocationPolicyGroup.prototype = {
    setPolicy:function(policy){
        this.policy = policy
        if (!policy) return;
        var pat = new RegExp('(\\~?)(\\w+)\\((.*?)\\)')
        var groups = policy.match(pat)
        if (!groups) return           
        this.not = groups[1]=='~'
        this.funcname = groups[2]
        this.parameters = groups[3].split(',')
        //figure out if we have next group
        policy = policy.substring(groups[0].length).trim()
        if (!policy) return;
        var logic = policy.substr(0,1)
        if (!(logic=='|' || logic=='&')){
            return;
        }
        //move to next
        policy = policy.substring(1).trim()
        this.nextgroup = new LocationPolicyGroup(this.editor,this)
        this.nextgroup.setPolicy(policy)
        this.nextgroup.op = logic=='|' ? 'OR' : 'AND'
    },
    getPolicy:function(){
        var pieces= []
        if (this.op) pieces.push((this.op=='AND' ? '& ' : '| '))
        if (this.not) pieces.push('~')
        pieces.push((this.funcname||'<not-selected>')+'('+this.parameters.join(',')+')')
        return pieces.join('')+(this.nextgroup ? ' '+this.nextgroup.getPolicy() : '')
    },    
    render:function(){
        var tags = ['<div class="LocationPolicyGroup" deepth="'+this.deepth+'">']
        tags.push('<div class="LocationPolicyGroupButtons"><select name="op"'+(this.deepth==0 ? ' disabled' : '')+'>')
        if (this.deepth > 0){
            var ops = ['AND','OR']
            for (var i=0;i<2;i++){
                var op = ops[i]
                tags.push('<option value="'+op+'"'+(this.op==op ? ' selected':'')+'>'+op+'</option>')
            }
        }
        tags.push('</select></div>')
        tags.push('<div class="LocationPolicyGroupButtons"><select name="not">')
        tags.push('<option value=""'+(this.not ? '' : ' selected')+'></option>')
        tags.push('<option value="not"'+(this.not ? ' selected':'')+'>NOT</option>')
        tags.push('</select></div>')
        var funcnameSel = ['<select name="funcname" class="LocationPolicyFuncnameSel">']
        var names = ['id','use','range','closest','farthest']
        for (var i=0,name;name=names[i];i++){
            name = name.trim()
            var sel = name==this.funcname ? ' selected' : ''
            funcnameSel.push('<option value="'+name+'"'+sel+'>'+name+'</option>')
        }
        funcnameSel.push('</select>')
        tags.push('<div class="LocationPolicyGroupFuncname">'+funcnameSel.join('')+'</div>')
        var value = this.parameters.length ? this.parameters.join(',') : ''
        tags.push('<div class="LocationPolicyGroupParam"><input style="width:100%;height:30px" value="'+value+'">')
        tags.push('</div>') //LocationPolicyGroupParam
        tags.push('<div class="LocationPolicyGroupDelete">⊗</div>')
        tags.push('</div>') //LocationPolicyGroup
        return tags.join('')+(this.nextgroup ? this.nextgroup.render() : '')
    },
    resetDeepth:function(deepth){
        this.deepth = this.parent ? this.parent.deepth + 1 : 0
        if (this.nextgroup) this.nextgroup.resetDeepth()
    },
    harakiri:function(){
        if (this.nextgroup){
            if (this.parent){
                this.parent.nextgroup = this.nextgroup
                this.nextgroup.parent = this.parent
            }
            else{
                this.editor.firstGroup = this.nextgroup
                this.nextgroup.parent = null
            }
            this.nextgroup.resetDeepth()
        }
        else{
            if (this.parent){
                this.parent.nextgroup = null
            }
            else{
                this.editor.firstGroup = null;
            }
        }
    },
    getLastGroup:function(){
        if (this.nextgroup) return this.nextgroup.getLastGroup()
        else return this
    }
}
function LocationPolicyEditor(div,contentDiv){
    this.div = div
    this.contentDiv = contentDiv
    this.firstGroup = new LocationPolicyGroup(this,null,null)
}
LocationPolicyEditor.prototype={
    setPolicy:function(policy_string){
        //var policy_string = '~range(10,0,0,0) | farthest(0,0,0) & ~id(1)'
        this.firstGroup.setPolicy(policy_string)
    },
    updatePolicy:function(){
        var ele = this.div.querySelector('.LocationPolicyEditorPolicy')
        var policy = this.firstGroup ? this.firstGroup.getPolicy() : ''
        if (ele.innerText != policy){
            ele.innerText = policy
            if (this.policyChangeListener) {
                this.policyChangeListener(policy)
            }
        }
    },
    render:function(){
        var tags = ['<div class="LocationPolicyEditorPolicy"></div>']
        if (this.firstGroup) tags.push(this.firstGroup.render())
        tags.push('<div class="LocationPolicyEditorAdd"><button class="blue">Add Policy Item</button></div>')
        this.contentDiv.innerHTML = tags.join('')
        // set the layout
        var viewportSize = getViewportSize()
        this.div.style.width = '500px'
        this.div.style.height = (viewportSize.height*0.8)+'px'
        this.div.style.left = (viewportSize.width-500)/2+'px'
        this.div.style.top = (viewportSize.height*0.1)+'px'
        this.div.style.display='inline-block'
        // hookup the buttons
        var self = this
        /* add item */
        this.div.querySelector('.LocationPolicyEditorAdd').onclick = function(evt){
            var parentGroup =  self.firstGroup ? self.firstGroup.getLastGroup() : null;
            var group = new LocationPolicyGroup(self,parentGroup,null)
            if (!self.firstGroup) self.firstGroup = group;
            self.render()
        }
        /* delete item */
        var getPolicyGroup = function(evt){
            var target = evt.currentTarget
            var ele = target.parentNode
            while (ele){
                if (ele.className=='LocationPolicyGroup'){
                    var deepth = ele.getAttribute('deepth')
                    if (deepth==0) return self.firstGroup
                    var group = self.firstGroup
                    while (group){
                        group = group.nextgroup
                        if (group.deepth==deepth) return group
                    }
                    return group
                    break;
                }
                ele = ele.parentNode
            }
        }
        var buttons = this.div.querySelectorAll('.LocationPolicyGroupDelete')
        for (var i=0,button;button=buttons[i];i++){
            button.onclick=function(evt){
                var policyGroup = getPolicyGroup(evt)
                policyGroup.harakiri()
                self.render()                
            }
        }
        /* user interactive */
        var selects = this.div.querySelectorAll('.LocationPolicyGroup select')
        for (var i=0,select;select=selects[i];i++){
            select.onchange = function(evt){
                var policyGroup = getPolicyGroup(evt)
                var name = this.getAttribute('name')
                var value = this.options[this.selectedIndex].value
                
                switch(name){
                    case 'funcname':
                        policyGroup.funcname = value
                        break
                    case 'not':
                        policyGroup.not = (value == '') ? false : true
                        console.log([value,policyGroup.not])
                        break;
                    case 'op':
                        policyGroup.op = value
                }
                self.updatePolicy()
            }
        }
        var inputs = this.div.querySelectorAll('.LocationPolicyGroup input')
        for (var i=0,input;input=inputs[i];i++){
            input.onkeyup = function(evt){
                var policyGroup = getPolicyGroup(evt)
                var value = this.value
                policyGroup.parameters = value.split(',')                
                self.updatePolicy()
            }
        }
        /* set policy string */
        this.updatePolicy()
    }
}