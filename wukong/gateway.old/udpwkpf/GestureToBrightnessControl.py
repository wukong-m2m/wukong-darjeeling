import traceback
import Defines as CONST

class GestureToBrightnessControl():
    def __init__(self):
        self.brightness = 0

    def update(self, in_gesture):
        increase_unit = 32

        if in_gesture == CONST.GESTURE_THUMB_UP:
            if 0 <= self.brightness <= (256-increase_unit):
                self.brightness += increase_unit
                out_brightness = self.brightness - 1
                return out_brightness

        elif in_gesture == CONST.GESTURE_THUMB_DOWN:
            if increase_unit <= self.brightness <= 256:
                self.brightness -= increase_unit
                if self.brightness > 0:
                    out_brightness = self.brightness - 1
                else:
                    out_brightness = self.brightness
                return out_brightness
        return None
