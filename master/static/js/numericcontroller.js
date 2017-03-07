// vim: ts=4 sw=4
function NumericController()
{
	Block.prototype.init.call(this);	
	this.type='Numeric_Controller';
	this.addSignal( new Signal('output'));
}

NumericController.prototype = new Block();
NumericController.prototype.constructor = NumericController;
Block.register('Numeric_Controller',NumericController);
