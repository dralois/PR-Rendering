# PRRendering HDR Light Source Estimator

## Required Libraries & Setup

Although this is a python module, there are some required libraries, some of which need to be built manually. These libraries should work on both Linux and Windows, but the module was only tested on Windows.

### Available via pip:
The required packages and dependencies are available to install via a requirements file:
```
pip install -r requirements.txt
```

### Manually built:
1) [Compute Shader for Python](https://github.com/dralois/python-compute-shader)
    - Comes with cmake support, requires OpenGL & Python Dev
    - Simply build and copy to site-packages
2) [PyCeres (Ceres for Python)](https://github.com/Edwinem/ceres_python_bindings)
    - Comes with cmake support, requires a sparse capable library to function
    - For Windows, Eigen is easiest and tested
    - Minilog is enough & Schur specializations need to be turned on
    - The provided _custom\_cpp\_cost\_functions.cpp_ (in _PyCeres/_ folder) needs to be copied to _ceres\_python\_bindings/python\_bindings/_
    - Follow the official _"Build Alongside Ceres"_ build instructions
