import logging
import time
from threading import Semaphore

__logger = None

__performance = {}
__logThreadSem = Semaphore()

__mapping = {   "debug" : logging.DEBUG,
                "info" : logging.INFO,
                "warning" : logging.WARNING,
                "error" : logging.ERROR,
                "critical" : logging.CRITICAL}

__reversed = {  logging.DEBUG : "debug",
                logging.INFO : "warning",
                logging.WARNING : "warning",
                logging.ERROR : "error",
                logging.CRITICAL : "critical"}

# Get global logger
def GetLogger():
    global __logger

    if not __logger:
        __logger = logging.getLogger(__name__)
        # Logs to console in a nice format
        handler = logging.StreamHandler()
        handler.setFormatter(logging.Formatter("[Wrapper] %(processName)s : %(message)s"))
        __logger.addHandler(handler)
        __logger.setLevel(logging.INFO)

    return __logger

# Set global logging level (string)
def SetLevel(level):
    __logger.setLevel(__mapping.get(level, logging.INFO))

# Get global logging level (string)
def GetLevel():
    return __reversed.get(__logger.level, "error")

# Convenience performance measure function
def LogPerformance(what, thread):
    global __performance, __logThreadSem
    # Log duration or start timer
    if thread in __performance:
        if what in __performance[thread]:
            __logThreadSem.acquire()
            GetLogger().critical("*************************************************************")
            GetLogger().critical(f"[Thread {thread}] {what} duration: {time.clock() - __performance[thread][what]}")
            GetLogger().critical("*************************************************************")
            del __performance[thread][what]
            __logThreadSem.release()
        else:
            __performance[thread][what] = time.clock()
    else:
        __performance[thread] = {}
