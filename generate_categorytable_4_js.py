##
## create the wuClassCategoaryTableRaw in static/js/datamodel.js
##
data = """
Software	Logic	Threshold
Software	Logic	And_Gate
Software	Logic	Or_Gate
Software	Logic	Xor_Gate
Software	Logic	Not_Gate
Software	Logic	Equal
Software	Condition	If_Short
Software	Condition	If_Boolean
Software	Condition	Condition_Selector_Boolean
Software	Condition	Condition_Selector_Short
Software	Calculation	Math_Op
Software	Special Usage	Server
Software	Special Usage	Multiplexer
Software	Special Usage	Virtual_Slider
Software	Special Usage	Controller
Software	Special Usage	User_Aware
Software	Special Usage	Plugin
		
		
		
Sensor	Digital Input	PIR_Sensor
Sensor	Digital Input	Binary_Sensor
Sensor	Digital Input	Magnetic_Sensor
Sensor	Digital Input	Button
Sensor	Digital Input	Touch_Sensor
Sensor	Fast Digital I/O	Ultrasound_Sensor
Sensor	Analog Input	Light_Sensor
Sensor	Analog Input	Slider
Sensor	Analog Input	Ir_Sensor
Sensor	Analog Input	Microphone_Sensor
Sensor	Analog Input	Pressure_Sensor_0
Sensor	Analog Input	Temperature_Sensor
Sensor	Analog Input	Sound_Sensor
Sensor	I2C	Temperature_Humidity_Sensor
Sensor	Unit Test	Binary_TestSensor
Sensor	Unit Test	Integer_TestSensor
Sensor	UART	User
Sensor	UART	Gesture
Sensor	Others	Gh_Sensor
		
		
Actuator	Digital Output	Light_Actuator
Actuator	Digital Output	LED
Actuator	Digital Output	Fan
Actuator	Digital Output	Relay
Actuator	Analog Output	RGBLED
Actuator	Analog Output	Dimmer
Actuator	Analog Output	Mist
Actuator	PWM	Buzzer
Actuator	PWM	Sound
Actuator	PWM	MOSFET_LED
Actuator	I2C	Grove_LCD
Actuator	UART	Grove_MP3
Actuator	UART	Gesture_MP3"""

lines = data.split('\n')
table = {}
for line in lines:
    cols = filter(lambda x:x.strip(),line.split('\t'))
    if len(cols) != 3: continue
    if table.has_key(cols[0]):
        if table[cols[0]].has_key(cols[1]):
            table[cols[0]][cols[1]].append(cols[2])
        else:
            table[cols[0]][cols[1]] = [cols[2]]
    else:
        table[cols[0]] = {cols[1]:[cols[2]]}
import json
print 'var wuClassCategoaryTableRaw =',json.dumps(table)