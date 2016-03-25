import traceback
import pywinauto

class VideoControl():
    def __init__(self):
        self.pause_only = "^{SPACE}"
        self.play_only = "+{SPACE}"
        self.next = "%n"
        self.prev = "%p"
        self.vol_inc = "^{UP}"
        self.vol_dec = "^w"

    def update(self, in_play, in_stop, in_next_section, in_prev_section, in_volume_increase, in_volume_decrease):
        if not (in_play or in_stop or in_next_section or in_prev_section or in_volume_increase or in_volume_decrease): return
        app = pywinauto.application.Application()
        try:
            w_handle = pywinauto.findwindows.find_window(title_re=r".*VLC")
        except Exception as e:
            print "No video is playing"
            print repr(e), traceback.format_exc()
            return

        try:
            window = app.window_(handle=w_handle)
            keys = None

            if in_stop:
                keys = self.pause_only

            elif in_play:
                keys = self.play_only

            elif in_next_section:
                keys = self.next

            elif in_prev_section:
                keys = self.prev

            elif in_volume_increase:
                keys = self.vol_inc

            elif in_volume_decrease:
                keys = self.vol_dec

            if keys is not None:
                print keys
                window.TypeKeys(keys)

        except Exception as e:
            print "Unknown VLC error"
            print repr(e), traceback.format_exc()

        app = None
        window = None