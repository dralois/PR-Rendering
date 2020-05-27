import sys

def ImportBPY():
    org_sys = list(sys.path)
    import bpy
    bpy_sys = list(sys.path)
    return org_sys, bpy_sys
