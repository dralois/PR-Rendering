from ..Utils.Importer import GetPaths, SetPaths
from ..Utils.Logger import LogPerformance
from ..Utils import FullPath, FileDir

from typing import List, Dict
from os import chdir
import multiprocessing as mp
import json
import time

# Render manager, handles multithreaded rendering
class RenderManager(object):

    def __init__(self):
        self.__maxWorkers = mp.cpu_count()
        self.__workers : List[(RenderProcess, mp.JoinableQueue, mp.Event)]
        self.__workers = [None for i in range(self.__maxWorkers)]
        for i in range(self.__maxWorkers):
            self.__CreateProcess(i)

    def __del__(self):
        for i in range(self.__maxWorkers):
            self.__RemoveProcess(i)

    # Process renderfile multithreaded
    def ProcessRenderfile(self, renderfile):
        waitQueues = []
        # Parse string to json
        scenes = json.loads(renderfile)
        # Enqueue scenes
        for i in range(len(scenes)):
            # If no fast worker for scene available
            if i >= (self.__maxWorkers - 1):
                # Get slower, general purpose worker
                _,queue,_ = self.__workers[len(self.__workers)]
                # Enqueue renderfile and block until completed
                queue.put(scenes[i], True)
                queue.join()
            else:
                # Otherwise get fast worker
                _,queue,_ = self.__workers[i]
                # Enqueue renderfile and continue
                queue.put(scenes[i], True)
                waitQueues.append(queue)
        # Wait until fast workers are done
        [waitFor.join() for waitFor in waitQueues]

    # Remove and reload all process
    def UnloadProcesses(self):
        startReload = time.clock()
        for i in range(self.__maxWorkers):
            if self.__RemoveProcess(i):
                self.__CreateProcess(i)
        LogPerformance(startReload, "Process reloading")

    # Create & start a new process
    def __CreateProcess(self, index):
        # Only create if allowed and empty
        if index < self.__maxWorkers:
            if self.__workers[index] is None:
                # Create synchronization objects
                fileQueue = mp.JoinableQueue(1)
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
        self.__fileQueue : mp.JoinableQueue
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
        chdir(FullPath(f"{FileDir(__file__)}\\..\\"))
        # Process file queue
        while True:
            try:
                # Check for work
                data = self.__fileQueue.get(True, 0.1)
                startBuild = time.clock()
                # Create or update scene
                if self.__scene is None:
                    self.__scene = CreateFromJSON(data)
                else:
                    self.__scene = UpdateFromJSON(data, self.__scene)
                LogPerformance(startBuild, "Process build")
                # Process render queue
                startRender = time.clock()
                while self.__scene.RenderQueueRemaining() > 0:
                    self.__scene.RenderQueueProcessNext()
                # Signal rendering complete to main thread
                LogPerformance(startRender, "Process render")
                self.__fileQueue.task_done()
            except mp.queues.Empty:
                # Check for close signal
                if self.__closeEvent.wait(0.1):
                    exit()
