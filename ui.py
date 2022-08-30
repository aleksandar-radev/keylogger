import tkinter as tk
from datetime import datetime as date
import os
import glob
import re

MINUTES_IN_YEAR = 525949.2
MINUTES_IN_MONTH = 43829.1
MINUTES_IN_DAY = 1440
MINUTES_IN_HOUR = 60
class Application(tk.Frame):
    def __init__(self, logging, master=None):
        super().__init__(master)
        self.master = master
        self.logging = logging
        self.grid()
        self.create_widgets()

    def create_widgets(self):
        self.on_off_button = tk.Button(self)
        if self.logging:
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
        self.logging = not self.logging
        if self.logging:
            self.configure(bg='green')
            self.on_off_button['text'] = 'ON'
        else:
            self.configure(bg='red')
            self.on_off_button['text'] = 'OFF'
