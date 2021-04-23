import trimesh

def load_mesh(path, format) -> trimesh.Trimesh:
    return trimesh.load_mesh(path, format)
