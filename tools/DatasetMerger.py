import argparse
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

if __name__ == "__main__":

    parse = argparse.ArgumentParser(description="Merges two datasets into one and removes invalid data points")
    parse.add_argument("-o", "--original", required=True, type=str,
        help="path to original dataset, must be a folder")
    parse.add_argument("-m", "--merge", required=True, type=str,
        help="path to the dataset to be merged, must be a folder")
    args = vars(parse.parse_args())

    requiredFolders = ["annotations", "depth", "rgb", "segs"]

    # Open both data sets
    with os.scandir(args["original"]) as org:
        with os.scandir(args["merge"]) as mrg:

            # Parse contained folders into lists
            orgEntries = [entry for entry in org]
            mrgEntries = [entry for entry in mrg]

            # All folders of the data set structure must exist
            if (not all(elem.path in requiredFolders for elem in orgEntries) or
                not all(elem.path in requiredFolders for elem in mrgEntries)):
                os.error("Either original or dataset to merge is not a valid dataset!")

            # Find last entry number in original
            lastEntry = [int(s) for s in re.findall(r"\d+", max(
                os.scandir(os.path.join(args["original"], "rgb")), key=
                    lambda x : [int(s) for s in re.findall(r"\d+", x.name)][0]).name)][0]

            # Clean the data set to be merged
            with os.scandir(os.path.join(args["merge"], "annotations")) as (annotations
                ), os.scandir(os.path.join(args["merge"], "depth")) as (depth
                ), os.scandir(os.path.join(args["merge"], "rgb")) as (rgb
                ), os.scandir(os.path.join(args["merge"], "segs")) as segs:

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

                # Copy to original set with new names
                [os.rename(entry.path, os.path.join(args["original"], "annotations", entry.Renamed(lastEntry, min(annotationFiles).num))) for entry in annotationFiles]
                [os.rename(entry.path, os.path.join(args["original"], "depth", entry.Renamed(lastEntry, min(depthFiles).num))) for entry in depthFiles]
                [os.rename(entry.path, os.path.join(args["original"], "rgb", entry.Renamed(lastEntry, min(rgbFiles).num))) for entry in rgbFiles]
                [os.rename(entry.path, os.path.join(args["original"], "segs", entry.Renamed(lastEntry, min(segsFiles).num))) for entry in segsFiles]
