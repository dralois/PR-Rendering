# Overview
The annotation output files (_cvs format_) contains one annotation each line. Every image has its own annotation file named _labels\_xxxxxx.csv_. Each file has a header line with a description of each column. The information contained is seperated by semicolons.

# Format

Information | *Bounding Box* | *Object* | *Position* | *Rotation* | *Intrinsics*
:--------- | :----------: | :----: | :------: | :------: | :--------:
**Content** | x; y; w; h | objectName; meshID; objectID | x; y; z | w; x; y; z | fx; fy; ox; oy
