// vim: ts=4 sw=4
function LightActuator()
{
	Block.prototype.init.call(this);	
	this.type='Light_Actuator';
	this.addAction( new Action('on_off'));
}

LightActuator.prototype = new Block();
LightActuator.prototype.constructor = LightActuator;

Block.register('Light_Actuator',LightActuator);
