// vim: ts=4 sw=4
function FlowList()
{
	this.list=[];
}

FlowList.prototype.addFlow=function() {
	var flow={};
	var d = new Date();
	flow.name=d.toUTCString;
	flow.nodes = [];
	flow.links = [];
	this.list.push(flow);
}

FlowList.prototype.delFlow=function(idx) {
	this.list.splice(idx,1);
}

FlowList.prototype.getFlowText=function() {
	return this.list;
}
FlowList.prototype.addClick=function(idx)
{
	var self=this;
	$('#flowlist_'+idx).click(function() {
		FBP_loadScenario(self.list[idx].nodes, self.list[idx].links);
		self.now = idx;
	});
}

FlowList.prototype.update=function(nodes,lines) {
	this.list[this.now].nodes = nodes;
	this.list[this.now].lines = lines;
	this.list[this.now].undolist.push(JSON.stringify(nodes, lines));
}

FlowList.prototype.undo=function() {
	var str = this.list[this.now].undolist.pop();
	if (str == null) return;
	var meta = JSON.parse(str);
	FBP_refreshContent(meta.nodes,meta.lines);
}

FlowList.prototype.fillList=function(div) {
	div.empty();
	div.append('<table id=flowlisttab></table>');
	for(i=0;i<this.list.length;i++) {
		$('#flowlisttab').append('<tr><td id=flowlist_'+i+'>'+this.list[i].name+'</td></td>');
		this.addClick(i);
	}
}
