from multiprocessing.queues import JoinableQueue
from ..Utils.Importer import GetPaths, SetPaths
from ..Utils import FullPath, FileDir

from typing import List
from os import chdir

import multiprocessing as mp
import json

# Raised if unload fails
class UnloadException(Exception):
    pass

# Queue supporting joining with timeout
class TimeoutQueue(JoinableQueue):

    # Returns false if join failed
    def join_with_timeout(self, timeout):
        with self._cond:
            if not self._unfinished_tasks._semlock._is_zero():
                return self._cond.wait(timeout)

# Render manager, handles multithreaded rendering
class RenderManager(object):

    def __init__(self, workerThreads):
        self.__maxWorkers = workerThreads
        self.__workers : List[(RenderProcess, TimeoutQueue, mp.Event)]
        self.__workers = [None for i in range(self.__maxWorkers)]
        for i in range(self.__maxWorkers):
            self.__CreateProcess(i)

    def __del__(self):
        for i in range(self.__maxWorkers):
            self.__RemoveProcess(i)

    # Process renderfile multithreaded
    def ProcessRenderfile(self, renderfile, timeout, thread):
        assert thread < self.__maxWorkers
        # Parse string to json
        scene = json.loads(renderfile)
        # Enqueue scene
        _,queue,_ = self.__workers[thread]
        # Enqueue renderfile and block until completed
        queue.put(scene, True)
        # Make sure rendering doesn't get stuck
        try:
            if not queue.join_with_timeout(timeout):
                # If rendering is stuck, reload process
                self.UnloadProcess(thread)
                # Try again
                self.ProcessRenderfile(renderfile, timeout, thread)
        except Exception as e:
            print(f"Unexpected error: {e}")

    # Remove and reload all process
    def UnloadProcess(self, thread):
        assert thread < self.__maxWorkers
        # Reload process if possible
        if self.__RemoveProcess(thread):
            if self.__CreateProcess(thread):
                return True
        else:
            raise UnloadException

    # Create & start a new process
    def __CreateProcess(self, index):
        # Only create if allowed and empty
        if index < self.__maxWorkers:
            if self.__workers[index] is None:
                # Create synchronization objects
                fileQueue = TimeoutQueue(maxsize=1, ctx=mp.get_context())
                closeEvent = mp.Event()
                # Create, start & save render process
                renderPrc = RenderProcess(fileQueue, closeEvent, GetPaths())
                self.__workers.insert(index, (renderPrc, fileQueue, closeEvent))
                renderPrc.start()
                return True
        # Creation failed
        return False

    # Remove & close existing process
    def __RemoveProcess(self, index):
        # Only terminate if allowed
        if index < self.__maxWorkers:
            if self.__workers[index] is not None:
                # Get process and close signal
                renderPrc,queue,closeEvent = self.__workers[index]
                # Set signal, see if it terminates
                closeEvent.set()
                renderPrc.join(0.5)
                # Kill process if necessary
                if renderPrc.is_alive():
                    renderPrc.terminate()
                    queue.close()
                else:
                    renderPrc.close()
                    queue.close()
                # Cleanup & reset
                del renderPrc, queue, closeEvent
                self.__workers[index] = None
                return True
        # Removal failed
        return False

# Single render process
class RenderProcess(mp.Process):

    def __init__(self, fileQueue, closeEvent, importPaths):
        super().__init__(target=self.RenderLoop, args=importPaths, daemon=True)
        self.__fileQueue : TimeoutQueue
        self.__fileQueue = fileQueue
        self.__closeEvent : mp.Event
        self.__closeEvent = closeEvent
        self.__scene = None

    def RenderLoop(self, *paths):
        # Setup for multiprocessing
        SetPaths(paths[0], paths[1])
        # Scene imports
        from .SceneManager import CreateFromJSON, UpdateFromJSON, Scene
        # Change working dir in process
        chdir(FullPath(f"{FileDir(__file__)}/../"))
        # Process file queue
        while True:
            try:
                # Check for work
                data = self.__fileQueue.get(True, 0.1)
                # Create or update scene
                if self.__scene is None:
                    self.__scene = CreateFromJSON(data)
                else:
                    self.__scene = UpdateFromJSON(data, self.__scene)
                # Process render queue
                while self.__scene.RenderQueueRemaining() > 0:
                    self.__scene.RenderQueueProcessNext()
                # Signal rendering complete to main thread
                self.__fileQueue.task_done()
            except mp.queues.Empty:
                # Check for close signal
                if self.__closeEvent.wait(0.1):
                    exit()
