import numpy as np

import trimesh
import trimesh.visual

def load_mesh(path, format) -> trimesh.Trimesh:
    mesh : trimesh.Trimesh
    mesh = trimesh.load_mesh(path, format)
    mesh.visual = trimesh.visual.ColorVisuals(mesh, None, (0,0,0))
    return mesh

def store_mesh(path, mesh: trimesh.Trimesh):
    mesh.export(path, "glb")
    mesh.visual = trimesh.visual.ColorVisuals(mesh, None, (0,0,0))

def set_vertex_color(mesh: trimesh.Trimesh, index, color):
    mesh.visual.vertex_colors[index] = max_to_white(color)

def set_vertex_colors(mesh: trimesh.Trimesh, colors):
    vector_func = np.vectorize(max_to_white, otypes=[np.uint8], signature="(n)->(4)")
    mesh.visual = trimesh.visual.ColorVisuals(mesh, None, vector_func(colors))

def max_to_white(col):
    if np.max(col) > np.iinfo(np.uint8).max:
        return np.full((4), 255, dtype=np.uint8)
    else:
        return np.append(col, 255)
