import argparse
import subprocess
import json
import sys

from DatasetMerger import MergeSets, EnumerateSet

def RunGenerator(exe, config, out, maxtime, repeat):
    # Determine generator output dir
    finalDir = json.load(open(config, "r"))["final_path"]
    # Repeat generation multiple times
    for i in range(repeat):
        try:
            # Generate new dataset
            subprocess.run([exe, config], stdout=sys.stdout, stderr=sys.stderr, timeout=(maxtime * 3600.0), check=True)
            # Merge dataset into specified output dir
            print(f"Done generating {i + 1} / {repeat} times")
            MergeSets(out, finalDir)
        except subprocess.TimeoutExpired:
            # Timeouts are acceptable
            print(sys.exc_info())
            continue
        except subprocess.CalledProcessError:
            # Errors during generation may be problematic
            print(sys.exc_info())
            break

def cleanFunc(args):
    print(f'Cleaning {args["directory"]}')
    EnumerateSet(args["directory"])
def mergeFunc(args):
    print(f'Merging {args["other"]} into {args["directory"]}')
    MergeSets(args["directory"], args["other"])
def genFunc(args):
    print(f'Generating {args["repeat"]} times ({args["executable"]} -{args["config"]}), timeout {args["timeout"]}h, into folder {args["directory"]}')
    RunGenerator(args["executable"], args["config"], args["directory"], args["timeout"], args["repeat"])

if __name__ == "__main__":

    parse = argparse.ArgumentParser(description="Merges two datasets into one and removes invalid data points")
    subparse = parse.add_subparsers(title="command", required=True)

    clean = subparse.add_parser("clean", help="clean dataset")
    merge = subparse.add_parser("merge", help="merge datasets")
    gen = subparse.add_parser("generate", help="generate dataset")

    clean.add_argument("-d", "--directory", required=True, type=str,
        help="path to a directory containing a dataset")
    clean.set_defaults(func=cleanFunc)

    merge.add_argument("-d", "--directory", required=True, type=str,
        help="path to a directory containing a dataset")
    merge.add_argument("-o", "--other", type=str, required=True,
        help="path to a directory containing a dataset to merge")
    merge.set_defaults(func=mergeFunc)

    gen.add_argument("-d", "--directory", required=True, type=str,
        help="path to a directory containing a dataset")
    gen.add_argument("-e", "--executable", type=str, required=True,
        help="path to the generator executable")
    gen.add_argument("-c", "--config", type=str, required=True,
        help="path to a config file for generating")
    gen.add_argument("-t", "--timeout", type=float, default=1.0,
        help="generation process timeout in hours")
    gen.add_argument("-r", "--repeat", type=int, default=1,
        help="generation process repeat count")
    gen.set_defaults(func=genFunc)

    args = parse.parse_args()
    args.func(vars(args))
