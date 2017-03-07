// vim: ts=4 sw=4
function LightSensor()
{
	Block.prototype.init.call(this);	
	this.type='Light_Sensor';
	this.addSignal( new Signal('current_value'));
	this.addAction( new Action('refresh_rate'));
}

LightSensor.prototype = new Block();
LightSensor.prototype.constructor = LightSensor;

LightSensor.prototype.getCurrent=function() {
	// Get data from the master here
	return 0;
}
LightSensor.prototype.getRefreshRate=function() {
	// Get data from the master here
	return 0;
}

Block.register('Light_Sensor',LightSensor);
