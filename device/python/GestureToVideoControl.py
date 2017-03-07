import traceback
import Defines as CONST

class GestureToVideoControl():
    def __init__(self):
        pass

    def update(self, in_gesture):
        if in_gesture == CONST.GESTURE_WAVE or in_gesture == CONST.GESTURE_FIST:
            out_play = False
            out_stop = True
            out_next_section = False
            out_prev_section = False
            out_volume_increase = False
            out_volume_decrease = False
            return (out_play, out_stop, out_next_section, out_prev_section, out_volume_increase, out_volume_decrease)

        elif in_gesture == CONST.GESTURE_TWO_FINGERS_PINCH_OPEN:
            out_play = True
            out_stop = False
            out_next_section = False
            out_prev_section = False
            out_volume_increase = False
            out_volume_decrease = False
            return (out_play, out_stop, out_next_section, out_prev_section, out_volume_increase, out_volume_decrease)

        elif in_gesture == CONST.GESTURE_SWIPE_LEFT:
            out_play = False
            out_stop = False
            out_next_section = False
            out_prev_section = True
            out_volume_increase = False
            out_volume_decrease = False
            return (out_play, out_stop, out_next_section, out_prev_section, out_volume_increase, out_volume_decrease)

        elif in_gesture == CONST.GESTURE_SWIPE_RIGHT:
            out_play = False
            out_stop = False
            out_next_section = True
            out_prev_section = False
            out_volume_increase = False
            out_volume_decrease = False
            return (out_play, out_stop, out_next_section, out_prev_section, out_volume_increase, out_volume_decrease)

        elif in_gesture == CONST.GESTURE_THUMB_UP:
            out_play = False
            out_stop = False
            out_next_section = False
            out_prev_section = False
            out_volume_increase = True
            out_volume_decrease = False
            return (out_play, out_stop, out_next_section, out_prev_section, out_volume_increase, out_volume_decrease)

        elif in_gesture == CONST.GESTURE_THUMB_DOWN:
            out_play = False
            out_stop = False
            out_next_section = False
            out_prev_section = False
            out_volume_increase = False
            out_volume_decrease = True
            return (out_play, out_stop, out_next_section, out_prev_section, out_volume_increase, out_volume_decrease)

        elif in_gesture == CONST.GESTURE_DIRTY:
            out_play = False
            out_stop = False
            out_next_section = False
            out_prev_section = False
            out_volume_increase = False
            out_volume_decrease = False
            return (out_play, out_stop, out_next_section, out_prev_section, out_volume_increase, out_volume_decrease)

        return None
