// vim: ts=4 sw=4
function Number()
{
	Block.prototype.init.call(this);	
	this.type='number';
	this.addSignal( new Signal('value'));
	this.addAction( new Action('Add'));
	this.addAction( new Action('Sub'));
}

Number.prototype = new Block();
Number.prototype.constructor = Number;
Block.register('number',Number);
