from ..Utils.ShaderUtils import *

from plotoptix import NpOptiX
from plotoptix.materials import m_flat
from plotoptix.enums import ChannelDepth, ChannelOrder, Camera

import numpy as np
np.seterr(invalid='ignore')

import math
import threading
import random

class Raytracer(object):

    def __init__(self):
        # Necessary to only process one frame at a time
        self.renderDone = threading.Event()
        # Create framework & start raytracing
        self.rt = NpOptiX(on_rt_accum_done=self.render_done, start_now=True)

    def render_done(self, framework):
        self.renderDone.set()

    def close(self):
        # Done raytracing
        self.rt.close()

    def vertex_hitmap(self, mesh, frames, intr_c, max_verts, randomize_verts):
        # Used for target data texture
        size = math.ceil(math.sqrt(len(mesh.vertices)))

        # Setup raytracing
        self.rt.resize(size, size)
        self.rt.set_param(min_accumulation_step=1, max_accumulation_frames=1)

        # Build ray targets texture
        targets = np.full((size * size, 4), -1.0, dtype=np.float32)
        targets[:mesh.vertices.shape[0],:3] = mesh.vertices[:,:3]
        targets = targets.reshape(size, size, 4)
        self.rt.set_texture_2d("target", targets)

        # Create camera & mesh
        self.rt.setup_material("flat", m_flat)
        self.rt.setup_camera("cam", cam_type=Camera.CustomProjXYZ, textures=["target"])
        self.rt.set_current_camera("cam")
        self.rt.set_mesh("mesh", mesh.vertices, mesh.faces, mat="flat")

        # Stores unobstructed pixels
        vertexPixels = {fr : np.full((len(mesh.vertices), 3), np.NaN) for fr in frames}

        # Create compute shader
        with ShaderManager("VertexHitmap.comp") as compute:

            # Mesh buffer (constant): struct{uvec4, vec4[3]}
            faces = np.int32(np.hstack((mesh.faces, np.zeros((mesh.faces.shape[0], 1), dtype=np.int64))))
            triangles = np.float32(np.dstack((mesh.triangles, np.ones((mesh.triangles.shape[0], mesh.triangles.shape[1], 1), dtype=np.float64))))
            meshdata = np.concatenate((faces.view("float32"), triangles.reshape(-1, 12)), axis=1)
            compute.SetData(2, meshdata)

            # Target buffer (constant): uvec4
            targetdata = targets.reshape((-1, 4))
            compute.SetData(3, targetdata)

            # Raytrace all frames
            for frame, vals in frames.items():
                # Load current frame values & update camera position
                _, rgb, _, grads, extr, eye_pos = vals
                self.rt.update_camera("cam", eye=eye_pos)
                self.rt.refresh_scene()

                # One frame at a time
                self.renderDone.wait()

                # Load hit targets (fid: hit face indices, pid: hit vertex index)
                fid = self.rt._geo_id[:,:,1].reshape(self.rt._height * self.rt._width)
                pid = self.rt._geo_id[:,:,0].reshape(self.rt._height * self.rt._width)
                pid &= 0xC0000000
                pid >>= 30

                # Hit targets buffer: struct{uint, uint}
                hitData = np.vstack((fid, pid)).transpose()
                compute.SetData(1, hitData)

                # Camera variables: mat4, vec2, vec2, vec2
                camVars = np.concatenate((np.float32(extr.transpose().flatten()), np.array(intr_c, dtype=np.float32)))
                compute.SetData(4, camVars)

                # Image data: uvec4 - RGB + gradient (0/1)
                imageData = np.uint32(np.dstack((rgb, np.uint8(grads)))).reshape(-1, 4)
                compute.SetData(5, imageData)

                # Output buffer: uvec2
                output = np.full((len(mesh.vertices), 4), -1, dtype=np.int32)
                compute.SetData(6, output)

                # Calculate in shader
                compute.StartCompute(math.ceil(hitData.shape[0] / 128))
                result = compute.GetData(6)[:,:3]
                vertexPixels[frame] = np.where(result >= 0.0, result, result * np.nan)

                # Next frame
                self.renderDone.clear()

        # Select random subset of max 200.000 verts
        vert_idx = ([i for i in range(min(len(mesh.vertices), max_verts))] if not randomize_verts else
            random.sample(range(min(len(mesh.vertices), max_verts)), min(len(mesh.vertices), max_verts)))
        selection = np.array([np.take(pixels, vert_idx, axis=0) for _, pixels in vertexPixels.items()]).transpose(1, 0, 2)

        # Return vertices
        return selection

    def radiance_reproject(self, mesh, frames, intr_c, exposure):
        # Used for target data texture
        size = math.ceil(math.sqrt(len(mesh.vertices)))

        # Setup raytracing
        self.rt.resize(size, size)
        self.rt.set_param(min_accumulation_step=1, max_accumulation_frames=1)

        # Build ray targets texture
        targets = np.full((size * size, 4), -1.0, dtype=np.float32)
        targets[:mesh.vertices.shape[0],:3] = mesh.vertices[:,:3]
        targets = targets.reshape(size, size, 4)
        self.rt.set_texture_2d("target", targets)

        # Create camera & mesh
        self.rt.setup_material("flat", m_flat)
        self.rt.setup_camera("cam", cam_type=Camera.CustomProjXYZ, textures=["target"])
        self.rt.set_current_camera("cam")
        self.rt.set_mesh("mesh", mesh.vertices, mesh.faces, mat="flat")

        # Stores weighted radiance samples
        vertexRadiance = {fr : np.full((len(mesh.vertices), 3), np.NaN) for fr in frames}

        # Create compute shader
        with ShaderManager("RadianceReproject.comp") as compute:

            # Mesh buffer (constant): struct{uvec4, vec4, vec4[3]}
            faces = np.int32(np.hstack((mesh.faces, np.zeros((mesh.faces.shape[0], 1), dtype=np.int64))))
            triangles = np.float32(np.dstack((mesh.triangles, np.ones((mesh.triangles.shape[0], mesh.triangles.shape[1], 1), dtype=np.float64))))
            normals = np.float32(np.hstack((mesh.face_normals, np.zeros((mesh.faces.shape[0], 1), dtype=np.float64))))
            meshdata = np.concatenate((faces.view("float32"), normals, triangles.reshape(-1, 12)), axis=1)
            compute.SetData(2, meshdata)

            # Target buffer (constant): uvec4
            targetdata = targets.reshape((-1, 4))
            compute.SetData(3, targetdata)

            # Raytrace all frames again
            for frame, vals in frames.items():
                # Load current frame values & update camera position
                org, _, _, _, extr, eye_pos = vals
                self.rt.update_camera("cam", eye=eye_pos)
                self.rt.refresh_scene()

                # Calculate optical axis & load exposure
                eye = np.float32(np.append(eye_pos, 1.0))
                oj = np.float32(np.matmul(np.linalg.inv(extr), np.array([0.0, 0.0, -1.0, 0.0])))
                ex = np.float32(np.append(exposure[int("".join(filter(str.isdigit, frame)))], 0.0))

                # One frame at a time
                self.renderDone.wait()

                # Load hit targets (fid: hit face indices, pid: hit vertex index)
                fid = self.rt._geo_id[:,:,1].reshape(self.rt._height * self.rt._width)
                pid = self.rt._geo_id[:,:,0].reshape(self.rt._height * self.rt._width)
                pid &= 0xC0000000
                pid >>= 30

                # Hit targets buffer: struct{uint, uint}
                hitData = np.vstack((fid, pid)).transpose()
                compute.SetData(1, hitData)

                # Camera variables: mat4, vec2, vec2, vec2, padding, vec4, vec4, vec4
                camVars = np.concatenate((np.float32(extr.transpose().flatten()),
                    np.array(intr_c, dtype=np.float32), np.zeros((2), dtype=np.float32), eye, oj, ex))
                compute.SetData(4, camVars)

                # Original image data: uvec4
                orgData = np.uint32(np.dstack((org, np.zeros((org.shape[0], org.shape[1], 1), dtype=np.uint8)))).reshape(-1, 4)
                compute.SetData(5, orgData)

                # Output buffer: vec4
                output = np.full((len(mesh.vertices), 4), -1, dtype=np.float32)
                compute.SetData(6, output)

                # Calculate in shader
                compute.StartCompute(math.ceil(hitData.shape[0] / 128))
                result = compute.GetData(6)
                vertexRadiance[frame] = np.where(result >= 0.0, result, result * np.nan)

                # Next frame
                self.renderDone.clear()

        # Generate median vertex color samples
        samples = np.array([vertices for _, vertices in vertexRadiance.items()]).transpose((1, 0, 2))
        weightedsum = np.nansum(samples[:,:,:3] * samples[:,:,3:4], axis=1)
        weightsum = np.nansum(samples[:,:,3:4], axis=1)
        samples = weightedsum / weightsum
        samples = np.where(np.isnan(samples), np.zeros_like(samples), samples / 255.0)

        # Return radiance samples
        return samples

    def render_panorama(self, mesh, frames, samples):
        # Update raytracing mesh
        self.rt.set_mesh("mesh", mesh.vertices, mesh.faces, mat="flat", c=samples)

        # Determine the best camera position of the frames (to avoid being stuck in geometry)
        center = mesh.bounding_box.centroid
        eye_points = np.array([eye for _, _, _, _, _, eye in frames.values()])
        best_candidate = eye_points[np.linalg.norm(np.abs(eye_points - center), ord=2, axis=1).argmin()]
        cam_target = best_candidate + np.array([0, -1, 0])

        # Setup raytracer for 360* panorama capture
        self.rt.resize(7768, 3884)
        self.rt.set_param(min_accumulation_step=100, max_accumulation_frames=300)
        self.rt.setup_camera("pano", cam_type=Camera.Panoramic, eye=best_candidate, target=cam_target, up=[0, 0, 1])
        self.rt.set_current_camera("pano")
        self.rt.refresh_scene()

        # Wait for convergence
        self.renderDone.clear()
        self.renderDone.wait()

        # Get HDR panorama (color + depth)
        pan_hdr = self.rt.get_rt_output(ChannelDepth.Bps32, ChannelOrder.BGR)
        pan_depth = np.ma.array(self.rt._hit_pos[:,:,3:4], mask=self.rt._hit_pos[:,:,3:4]>1e+9)

        # Return rendered hdr / depth panorama
        return pan_hdr, pan_depth, best_candidate
