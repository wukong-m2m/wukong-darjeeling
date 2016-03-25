import traceback
import colorsys
import Defines as CONST

class GestureToColorControl():
    def __init__(self):
        self.hue = 0.0

    def update(self, in_gesture):
        increase_unit = 30.0
        max_value = 360.0

        if in_gesture == CONST.GESTURE_THUMB_UP:
            if 0 <= self.hue <= (max_value-increase_unit):
                self.hue += increase_unit
                out_red, out_green, out_blue = map(lambda x: int(x * 255), colorsys.hsv_to_rgb(self.hue/max_value,1,1))
                return (out_red, out_green, out_blue)

        elif in_gesture == CONST.GESTURE_THUMB_DOWN:
            if increase_unit <= self.hue <= max_value:
                self.hue -= increase_unit
                out_red, out_green, out_blue = map(lambda x: int(x * 255), colorsys.hsv_to_rgb(self.hue/max_value,1,1))

                return (out_red, out_green, out_blue)

        return None