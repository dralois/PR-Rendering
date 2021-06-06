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
    mesh.visual.vertex_colors[index][:3] = color
