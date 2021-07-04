import trimesh
import trimesh.visual

def load_mesh(path, format) -> trimesh.Trimesh:
    mesh : trimesh.Trimesh
    mesh = trimesh.load_mesh(path, format)
    setattr(mesh, "debugcolor", trimesh.visual.ColorVisuals(mesh, None, (0,0,0)))
    return mesh

def store_mesh(path, mesh: trimesh.Trimesh):
    temp = mesh.visual.copy()
    mesh.visual = getattr(mesh, "debugcolor")
    mesh.export(path, "glb")
    mesh.visual = temp
    setattr(mesh, "debugcolor", trimesh.visual.ColorVisuals(mesh, None, (0,0,0)))

def set_vertex_color(mesh: trimesh.Trimesh, index, color):
    getattr(mesh, "debugcolor").vertex_colors[index][:3] = color
