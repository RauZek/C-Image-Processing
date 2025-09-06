# C Image Processing

This project implements basic image processing filters in **C (C99)** for grayscale images.  
It demonstrates convolution-based filtering, edge detection, and median filtering using **command-line arguments** and `getopt()` for input parsing.

- All implementation is inside `filters/`.
- Only one file containing the `main()` function is contained in this directory.
- No extra `.c` files outside of the workplace are needed.

with `getopt` the following arguments are parsed:
-i : Path to input file;
-o : Path to output file;
-w : Image width (in pixels);
-s : Stride (bytes per row in memory);
-h : Image height (in pixels).

The filters are applied in this order:

# 1. Sharpening Filter
Enhances image details using a high-pass kernel:

<pre> ```0   -0.5   0
-0.5  3   -0.5
0   -0.5   0 ``` </pre>

# 2. Edge Detection (Sobel Operator)
Applies Sobel kernels in x and y directions:

<pre> ```kx = 
-1  0  1
-2  0  1
-1  0  1

ky =
-1 -2 -1
 0  0  0
 1  2  1 ``` </pre>

where the gradient magnitude is computed as: pixel = sqrt(magx^2 + magy^2)

# 3. Median Filter 
- Uses a 3x3 neighborhood window.
- Each pixel is replaced by the median value of its neighbors.
- Removes the salt-and-pepper noise while also preserving the edges.
  
