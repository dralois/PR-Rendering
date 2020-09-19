import os
import re

class NumberedEntry(object):

    def __init__(self, entry : os.DirEntry):
        self.path = entry.path
        self.name = entry.name
        self.num = [int(s) for s in re.findall(r"\d+", entry.name)][0]

    def __hash__(self):
        return hash(self.num)

    def __eq__(self, cmp):
        return self.num == cmp.num

    def __le__(self, cmp):
        return self.num <= cmp.num

    def __lt__(self, cmp):
        return self.num < cmp.num

    def Renamed(self, lastIndex, minIndex):
        newIndex = lastIndex + (self.num - minIndex + 1)
        return re.sub(r"\d+", "{0:06d}".format(newIndex), self.name)

requiredFolders = set(["annotations", "depth", "rgb", "segs"])

# Remove invalid data points from a dataset
def CleanSet(dir):
    # Open directory
    with os.scandir(os.path.normpath(dir)) as clean:

        # Parse contained folders into lists
        cleanEntries = set([entry.name for entry in clean])

        # All folders of the data set structure must exist
        if not requiredFolders <= cleanEntries:
            print(f"Clean: Directory {dir} is not a valid dataset!")
            return

        # Clean the data set to be merged
        with os.scandir(os.path.join(dir, "annotations")) as (annotations
            ), os.scandir(os.path.join(dir, "depth")) as (depth
            ), os.scandir(os.path.join(dir, "rgb")) as (rgb
            ), os.scandir(os.path.join(dir, "segs")) as segs:

            # Store data point numbers contained in all folders
            annotationFiles = set([NumberedEntry(entry) for entry in annotations])
            depthFiles = set([NumberedEntry(entry) for entry in depth])
            rgbFiles = set([NumberedEntry(entry) for entry in rgb])
            segsFiles = set([NumberedEntry(entry) for entry in segs])

            # Build intersection
            validPoints = annotationFiles & depthFiles & rgbFiles & segsFiles

            # Reduce to invalid data points
            annotationRemove = annotationFiles - validPoints
            depthRemove = depthFiles - validPoints
            rgbRemove = rgbFiles - validPoints
            segsRemove = segsFiles - validPoints
            annotationFiles -= annotationRemove
            depthFiles -= depthRemove
            rgbFiles -= rgbRemove
            segsFiles -= segsRemove

            # Remove all invalid data points from all folders
            [print(entry.path) for entry in annotationRemove]
            [os.remove(entry.path) for entry in annotationRemove]
            [print(entry.path) for entry in depthRemove]
            [os.remove(entry.path) for entry in depthRemove]
            [print(entry.path) for entry in rgbRemove]
            [os.remove(entry.path) for entry in rgbRemove]
            [print(entry.path) for entry in segsRemove]
            [os.remove(entry.path) for entry in segsRemove]

            # Return valid data
            return annotationFiles, depthFiles, rgbFiles, segsFiles

# Merge two datasets into one
def MergeSets(dir, merge):
    # Open directory
    with os.scandir(os.path.normpath(dir)) as org:

        # Parse contained folders into lists
        orgEntries = set([entry.name for entry in org])

        # All folders of the data set structure must exist
        if not requiredFolders <= orgEntries:
            print(f"Merge: Directory {dir} is not a valid dataset!")
            return

        # Find last entry number in original
        lastEntry = [int(s) for s in re.findall(r"\d+", max(
            os.scandir(os.path.join(dir, "rgb")), key=
                lambda x : [int(s) for s in re.findall(r"\d+", x.name)][0]).name)][0]

        # Clean merging dataset
        annotationFiles, depthFiles, rgbFiles, segsFiles = CleanSet(merge)

        # Copy to original set with new names
        [os.rename(entry.path, os.path.join(dir, "annotations", entry.Renamed(lastEntry, min(annotationFiles).num))) for entry in annotationFiles]
        [os.rename(entry.path, os.path.join(dir, "depth", entry.Renamed(lastEntry, min(depthFiles).num))) for entry in depthFiles]
        [os.rename(entry.path, os.path.join(dir, "rgb", entry.Renamed(lastEntry, min(rgbFiles).num))) for entry in rgbFiles]
        [os.rename(entry.path, os.path.join(dir, "segs", entry.Renamed(lastEntry, min(segsFiles).num))) for entry in segsFiles]
