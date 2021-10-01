import os
import sys

# Capable of muting stdout / stderr
class StdMute(object):

    def __init__(self):
        self.nullStream = open(os.devnull, "w")

    def __enter__(self):
        sys.stdout = self.nullStream
        sys.stderr = self.nullStream
        return self

    def __exit__(self, *args):
        sys.stdout = sys.__stdout__
        sys.stderr = sys.__stderr__

# Capable of fully muting a stream
class FullMute(object):

    def __init__(self, stream=None):
        self.orgStream = stream or sys.stdout
        self.orgStreamfd = self.orgStream.fileno()
        self.cpyStreamfd = None
        self.nullstream = open(os.devnull, "w")

    # Start redirection of stream to null device
    def __enter__(self):
        self.cpyStreamfd = os.dup(self.orgStreamfd)
        os.dup2(self.nullstream.fileno(), self.orgStreamfd)
        return self

    # Restore stream
    def __exit__(self, *args):
        os.dup2(self.cpyStreamfd, self.orgStreamfd)
        os.close(self.cpyStreamfd)
