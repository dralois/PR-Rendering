# Overview
The annotation output file (_cvs format_) contains one annotation each line. The information contained is seperated by semicolons.

# Format
**All content is concatenated by semicolons.**

Information | Content
--- | ---
Image Number | xxxxxx
Bounding Box | x; y; w; h
Object | name; meshID; objectID
Position | x; y; z
Rotation | w; x; y; z
Intrinsics | [fx; fy; ox; oy]
