import traceback
import Defines as CONST

class GestureToHangoutControl():
    def __init__(self):
        pass

    def update(self, in_gesture):
        if in_gesture == CONST.GESTURE_TWO_FINGERS_PINCH_OPEN:
            out_answer = True
            out_decline = False
            return (out_answer, out_decline)

        elif in_gesture == CONST.GESTURE_WAVE:
            out_answer = False
            out_decline = True
            return (out_answer, out_decline)

        elif in_gesture == CONST.GESTURE_DIRTY:
            out_answer = False
            out_decline = False
            return (out_answer, out_decline)

        return None