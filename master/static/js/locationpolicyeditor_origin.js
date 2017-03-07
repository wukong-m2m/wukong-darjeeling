function LPNode(parentNode,data){
    this.parentNode = parentNode;
    this.level = (parentNode ? parentNode.level + 1 : 0)
    if (this.level > 10){
        console.log('Too deep for '+data)
        return;
    }
    this.parse(data)
}
LPNode.prototype = {
    parse:function(data){
        console.log(data)
        if (data.indexOf('&')==-1 && data.indexOf('|')==-1){
            this.nodes = [[data]]
            return;
        }
        this.nodes = []
        //split by op
        var andNodes = data.split('&')
        for (var i in andNodes){
            var orNodes = andNodes[i].split('|')
            var orList = []
            if (orNodes.length==1){
                orList =[ new LPNode(this,andNodes[i]) ]
            }
            else{
                for (var j in orNodes){
                    orList.push(new LPNode(this,orNodes[j]))
                }
            }
            this.nodes.push(orList)
        }
    },
    toString:function(){
        var or = []
        for (var i in this.nodes){
            or.push(this.nodes[i].join('|'))
        }
        return '['+(or.join('|'))+']'
    }
}
function LocationPolicyEditor(policy){
    this.setPolicy(policy)
}
LocationPolicyEditor.prototype = {
    setPolicy:function(policy){
        this.policy = new LPNode(null,policy);
    },
    render:function(){
        console.log(''+this.policy)
    }
}