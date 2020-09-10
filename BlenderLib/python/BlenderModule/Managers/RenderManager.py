from multiprocessing.queues import JoinableQueue
from ..Utils.Importer import GetPaths, SetPaths
from ..Utils import FullPath, FileDir

from typing import List
from os import chdir
from sys import exc_info

import multiprocessing as mp
import json

# Queue supporting joining with timeout
class TimeoutQueue(JoinableQueue):

    # Returns false if join failed
    def join_with_timeout(self, timeout):
        with self._cond:
            if not self._unfinished_tasks._semlock._is_zero():
                return self._cond.wait(timeout)

# Render manager, handles multithreaded rendering
class RenderManager(object):

    # Raised if process can't be removed
    class RemoveException(Exception):
        pass

    # Raised if process can't be created
    class CreateException(Exception):
        pass

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
        # Prevent exceptions (crashes C++!)
        try:
            _,queue,_ = self.__workers[thread]
            # Enqueue renderfile and block until completed
            queue.put(scene, True)
            # Make sure rendering doesn't get stuck
            if not queue.join_with_timeout(timeout):
                # If rendering is stuck, reload process
                self.UnloadProcess(thread)
                # Try again
                self.ProcessRenderfile(renderfile, timeout, thread)
        except Exception as ex:
            # Debug output on crash
            exInfo = exc_info()
            print(f"Unexpected {exInfo[0]}: {exInfo[2]} ({ex})")

    # Remove and reload all process
    def UnloadProcess(self, thread):
        assert thread < self.__maxWorkers
        # Reload process if possible
        if self.__RemoveProcess(thread):
            if self.__CreateProcess(thread):
                return True

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
        raise RenderManager.CreateException

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
                    renderPrc.kill()
                else:
                    renderPrc.close()
                # Remove queue
                queue.close()
                queue.join_thread()
                # Cleanup & reset
                del renderPrc, queue, closeEvent
                self.__workers[index] = None
                return True
        # Removal failed
        raise RenderManager.RemoveException

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
        from .SceneManager import CreateFromJSON, UpdateFromJSON
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
                    break
