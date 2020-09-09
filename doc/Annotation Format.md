# Overview
The annotation output files (_cvs format_) contains one annotation each line. Every image has its own annotation file named _labels\_xxxxxx.csv_. Each file has a header line with a description of each column. The information contained is seperated by semicolons.

# Format

Information | *Bounding Box* | *Object* | *Position* | *Rotation* | *Scale* | *Intrinsics* | *Other*
:---------- | :------------: | :------: | :--------: | :--------: | :-----: | :----------: | :-----:
**Content** | x; y; w; h | objectClass; objectName; objectInstanceID | x; y; z | w; x; y; z | x; y; z; | fx; fy; ox; oy | learn3D
