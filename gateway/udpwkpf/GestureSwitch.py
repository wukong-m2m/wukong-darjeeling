import pywinauto
import traceback
from vlcclient import VLCClient
import Defines as CONST
import time

class GestureSwitch():
    def __init__(self):
        self.mode = -1
        self.mode_avail_list = [CONST.MODE_VIDEO, CONST.MODE_HANGOUT, CONST.MODE_1ST_FLOOR]#, CONST.MODE_3RD_FLOOR, CONST.MODE_KITCHEN]

        self.vlc_window_name = r".*VLC"
        try:
            pywinauto.findwindows.find_window(title_re=self.vlc_window_name)
            self.vlc = VLCClient("::1")
            self.vlc.connect()
            self.vlc_status = self.vlc.status().split("\r\n")[-1]
        except Exception as e:
            print "Please start VLC player: vlc Desktop\playlist.xspf --loop --extraintf telnet --telnet-password admin"
            print "Remember to add vlc to PATH and Configure the hotkeys in the preference"
            exit()

        self.hangout_window_name = r"Google Hangout"
        try:
            pywinauto.findwindows.find_window(title_re=self.hangout_window_name)
        except Exception as e:
            print "Please start Hangout app"
            exit()

        self.web_window_name = r"Brightness of 1st-Floor"
        try:
            pywinauto.findwindows.find_window(title_re=self.web_window_name)
        except Exception as e:
            print "Please start Webpage"
            exit()

        self.update_time = time.time()

    def update_bool(self, in_change_mode, in_gesture):
        print "GestureSwitch update_bool mode is ", in_change_mode, "gesture is", in_gesture
        if self.update_time + 0.2 > time.time():
            return None

        if in_change_mode:
            self.mode = (self.mode + 1) % len(self.mode_avail_list)
            return self.update(self.mode, CONST.GESTURE_DIRTY)
        else:
            return self.update(self.mode, in_gesture)

    def update(self, in_change_mode, in_gesture):
        if in_change_mode == -1: return None
        print "GestureSwitch mode is ", in_change_mode, "gesture is", in_gesture
        self.mode = in_change_mode
        self.update_time = time.time()
        try:
            if in_change_mode == CONST.MODE_VIDEO:
                out_mode1gesture = in_gesture
                out_mode2gesture = CONST.GESTURE_DIRTY
                out_mode3gesture = CONST.GESTURE_DIRTY
                out_mode4gesture = CONST.GESTURE_DIRTY
                out_mode5gesture = CONST.GESTURE_DIRTY
                pywinauto.win32functions.SetForegroundWindow(pywinauto.findwindows.find_window(title_re=self.vlc_window_name))

            elif in_change_mode == CONST.MODE_HANGOUT:
                if "pause" not in self.vlc.status().split("\r\n")[-1]:
                    self.vlc.pause()
                out_mode1gesture = CONST.GESTURE_DIRTY
                out_mode2gesture = in_gesture
                out_mode3gesture = CONST.GESTURE_DIRTY
                out_mode4gesture = CONST.GESTURE_DIRTY
                out_mode5gesture = CONST.GESTURE_DIRTY
                try:
                    w = pywinauto.findwindows.find_window(title_re=self.hangout_window_name)
                except Exception as e:
                    w = pywinauto.findwindows.find_window(title_re="Incoming video call")
                pywinauto.win32functions.SetForegroundWindow(w)

            elif in_change_mode == CONST.MODE_1ST_FLOOR:
                if "pause" not in self.vlc.status().split("\r\n")[-1]:
                    self.vlc.pause()
                out_mode1gesture = CONST.GESTURE_DIRTY
                out_mode2gesture = CONST.GESTURE_DIRTY
                out_mode3gesture = in_gesture
                out_mode4gesture = CONST.GESTURE_DIRTY
                out_mode5gesture = CONST.GESTURE_DIRTY
                pywinauto.win32functions.SetForegroundWindow(pywinauto.findwindows.find_window(title_re=self.web_window_name))

            elif in_change_mode == CONST.MODE_3RD_FLOOR:
                if "pause" not in self.vlc.status().split("\r\n")[-1]:
                    self.vlc.pause()
                out_mode1gesture = CONST.GESTURE_DIRTY
                out_mode2gesture = CONST.GESTURE_DIRTY
                out_mode3gesture = CONST.GESTURE_DIRTY
                out_mode4gesture = in_gesture
                out_mode5gesture = CONST.GESTURE_DIRTY
                pywinauto.win32functions.SetForegroundWindow(pywinauto.findwindows.find_window(title_re=self.web_window_name))

            elif in_change_mode == CONST.MODE_KITCHEN:
                if "pause" not in self.vlc.status().split("\r\n")[-1]:
                    self.vlc.pause()
                out_mode1gesture = CONST.GESTURE_DIRTY
                out_mode2gesture = CONST.GESTURE_DIRTY
                out_mode3gesture = CONST.GESTURE_DIRTY
                out_mode4gesture = CONST.GESTURE_DIRTY
                out_mode5gesture = in_gesture
                pywinauto.win32functions.SetForegroundWindow(pywinauto.findwindows.find_window(title_re=self.web_window_name))

        except Exception as e:
            print repr(e)
            print traceback.format_exc()
            return None

        return (out_mode1gesture, out_mode2gesture, out_mode3gesture, out_mode4gesture, out_mode5gesture)