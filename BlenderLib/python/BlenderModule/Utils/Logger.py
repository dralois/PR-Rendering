import logging

__logger = None

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
def get_logger():
    global __logger

    if not __logger:
        __logger = logging.getLogger(__name__)
        # Logs to console in a nice format
        handler = logging.StreamHandler()
        handler.setFormatter(logging.Formatter("[Wrapper] %(threadName)s : %(message)s"))
        __logger.addHandler(handler)
        __logger.setLevel(logging.INFO)

    return __logger

# Set global logging level (string)
def set_logger_level(level):
    __logger.setLevel(__mapping.get(level, logging.INFO))

# Get global logging level (string)
def get_logger_level():
    return __reversed.get(__logger.level, "error")
