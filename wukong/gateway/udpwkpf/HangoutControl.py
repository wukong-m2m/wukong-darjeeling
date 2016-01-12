import traceback
import time
import pywinauto
import cv2
import numpy as np
from matplotlib import pyplot as plt
from twisted.internet import reactor

class HangoutControl():
    def __init__(self):
        self.template_incoming = cv2.imread('ss_incoming_call_2.png')
        self.template_ans = cv2.imread('ss_incoming_call_answer.png')
        self.template_dln = cv2.imread('ss_incoming_call_decline.png')
        self.template_join = cv2.imread('ss_incoming_call_join.png')
        self.template_stop = cv2.imread('ss_stop_call.png')
        self.meth = "cv2.TM_CCOEFF_NORMED"

    def update(self, in_answer, in_decline):
        if not (in_answer or in_decline): return
        app = pywinauto.application.Application()
        try:
            w_handle = pywinauto.findwindows.find_window(title_re=r'Hangout Video Call')
        except Exception as e:
            w_handle = None

        if w_handle is not None: # There is existing call
            if in_decline: # Decline
                try:
                    window = app.window_(handle=w_handle)
                    window.Close()
                    # w = window.GetProperties()['ClientRects'][0].width()
                    # h = window.GetProperties()['ClientRects'][0].height()
                    # window.ClickInput(coords=(w/2,h/2))
                    # img = np.array(window.CaptureAsImage().convert("RGB"))
                    # res, top_left, bottom_right = self.match_template(img, self.template_stop, self.meth)
                    # if res is None:
                    #     print "Cannot find Stop buttons within the Hangout Video Call window"
                    #     return
                    # w, h = self.get_pic_shape(self.template_stop)
                    # window.ClickInput(coords=(top_left[0] + w/2, top_left[1] + h/2))
                    return

                except Exception as e:
                    print repr(e), traceback.format_exc()

            else:
                print "User needs to decline existing call first and then answers another one."
                return

        else: # No existing call. Detect if there is incoming call.
            try:
                w_handle = pywinauto.findwindows.find_window(title_re=r'Google Hangout')
            except Exception as e:
                try:
                    w_handle = pywinauto.findwindows.find_window(title_re=r'Incoming video call')
                except Exception as e:
                    print "No incoming call nor in any call"
                    print repr(e), traceback.format_exc()
                    return

            try:
                window = app.window_(handle=w_handle)
                window.ClickInput(coords=(20,10))
                if in_answer:  # Answer
                    img = np.array(window.CaptureAsImage().convert("RGB"))
                    res, top_left, bottom_right = self.match_template(img, self.template_incoming, self.meth)
                    if res is None:
                        print "No incoming call found"
                        return

                    w, h = self.get_pic_shape(self.template_incoming)
                    window.ClickInput(coords=(top_left[0] + w/6, top_left[1] + h/2))

                    reactor.callLater(3, self.join)
                    print "Hangout Call Answered"
                    return

                elif in_decline:  # Decline
                    for count in xrange(4):
                        time.sleep(1)
                        img = np.array(window.CaptureAsImage().convert("RGB"))
                        res, top_left, bottom_right = self.match_template(img, self.template_dln, self.meth)
                        if res is not None: break
                    else:
                        print "No incoming call found or It is muted."
                        return

                    w, h = self.get_pic_shape(self.template_dln)
                    window.ClickInput(coords=(top_left[0] + w/2, top_left[1] + h/2))
                    return

            except Exception as e:
                print repr(e), traceback.format_exc()

    def join(self):
        try:
            app_hvc = pywinauto.application.Application()
            w_handle_hvc = pywinauto.findwindows.find_window(title_re=r'Hangout Video Call')
            w_hvc = app_hvc.window_(handle=w_handle_hvc)
            w = w_hvc.GetProperties()['ClientRects'][0].width()
            h = w_hvc.GetProperties()['ClientRects'][0].height()
            w_hvc.ClickInput(coords=(w/2, h/2))
        except Exception as e:
            print "Cannot find started Hangout Video Call window"
            return

        img = np.array(w_hvc.CaptureAsImage().convert("RGB"))
        res, top_left, bottom_right = self.match_template(img, self.template_join, self.meth)
        if res is None:
            print "Hangout Call Answered. No need to push join button."
            return

        w, h = self.get_pic_shape(self.template_join)
        w_hvc.ClickInput(coords=(top_left[0] + w/2, top_left[1] + h/2))

    def get_pic_shape(self, img):
        sh = img.shape
        if len(sh) == 3: h, w, ch = sh
        else: h, w = sh
        return w, h

    def match_template(self, img, template, meth):
        w, h = self.get_pic_shape(template)
        res = cv2.matchTemplate(img,template,eval(meth))
        min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)
        top_left = max_loc
        bottom_right = (top_left[0] + w, top_left[1] + h)

        if max_val < 0.8:
            return (None, None, None)

        return res, top_left, bottom_right