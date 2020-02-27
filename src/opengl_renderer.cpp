#include "opengl_renderer.h"


OpenglRenderer::OpenglRenderer(int  window_width, int window_height)
{
    window_height_ = window_height;
    window_width_ = window_width;

    initFrameBuffer();

    glEnable(GL_DEPTH_TEST);
}

OpenglObj::OpenglObj OpenglRenderer::loadObj(std::string path){
    std::ifstream in(path, ios::in);
    if (!in)
    {
        std::cerr << "Cannot open " << path << std::endl; exit(1);
    }

    OpenglObj obj;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.substr(0,2) == "v ")
        {
            std::istringstream s(line.substr(2));
            glm::vec4 v; s >> v.x; s >> v.y; s >> v.z; v.w = 1.0f;
            obj.vertices_.push_back(v);
        }
        else if (line.substr(0,2) == "f ")
        {
            std::istringstream s(line.substr(2));
            GLushort a,b,c;
            s >> a; s >> b; s >> c;
            a--; b--; c--;
            obj.elements_.push_back(a); obj.elements_.push_back(b); obj.elements_.push_back(c);
        }
    }

    return obj;
}


void OpenglRenderer::render_scene(const std::string path, const std::vector<Eigen::Isometry3f> poses)
{
    OpenglObj scene = loadObj(path);
    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, scene.vertices_.size() * sizeof(glm::vec4), &scene.vertices_[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, scene.elements_.size() * sizeof(unsigned int),
                 &scene.elements_[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)offsetof(Vertex, Normal));

    glBindVertexArray(0);

    glClearColor(m_background[0],m_background[1],m_background[2],0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    for(auto &m : model_jobs) drawVAOModel(m.first->VAO,m.first->idxData.size(),m.second);
    for(auto &m : bbox_jobs) drawVAOBoundingBox(m.first->VAO,m.second);
    for(auto &m : coord_jobs) drawVAOCoordinateSystem(m.first->VAO,m.second);

    render_rect =  (rect.width==0) ? Rect(0,0,getWidth(),getHeight()) : rect;
    copy_rect.width = render_rect.width;
    copy_rect.height = render_rect.height;

    // Read back into color and depth maps from each corresponding buffer
    glPixelStorei(GL_PACK_ALIGNMENT, (m_color.step & 3) ? 1 : 4);
    glPixelStorei(GL_PACK_ROW_LENGTH,m_color.step/m_color.elemSize());
    glReadPixels(render_rect.x,render_rect.y,render_rect.width,render_rect.height,GL_BGR,GL_UNSIGNED_BYTE,m_color.data);
    glPixelStorei(GL_PACK_ALIGNMENT,4);
    glPixelStorei(GL_PACK_ROW_LENGTH,m_depth.step/m_depth.elemSize());
    glReadPixels(render_rect.x,render_rect.y,render_rect.width,render_rect.height,GL_DEPTH_COMPONENT,GL_FLOAT,m_depth.data);
    convertZBufferToDepth(m_depth);

    model_jobs.clear();
    bbox_jobs.clear();
    coord_jobs.clear();
    doneCurrent();

}

void OpenglRenderer::initFrameBuffer() {
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    GLuint FramebufferName = 2;
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    // The texture we're going to render to
    GLuint renderedTexture;
    glGenTextures(1, &renderedTexture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    std::cout << "frame buffer " << renderedTexture << std::endl;

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width_, window_height_, 0, GL_RGB, GL_FLOAT, 0);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.ptr());

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // The depth buffer
    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width_, window_height_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return;
    // Render to our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
}
