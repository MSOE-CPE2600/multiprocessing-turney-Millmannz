# System Programming Lab 11 Multiprocessing
## Overview
In my implementation I used the provided mandel.c file and added the multiprocessing and multi image creating functionality. I created two new fucntions inside the program: The first is `extract_prefix()` which is a simple helper fucntion to determine the beginning of the file name either provided by the user or the default in order to ease the naming of our newly created files. The second is `create_image()` which takes the steps to make the image and store it and keeps it in its own fucntion to avoid repeated code.
## Results
<img width="1535" height="983" alt="image" src="https://github.com/user-attachments/assets/ee5d8189-847c-4742-b791-f5133b9228b7" />

### The speed of the image processing time decreases at a diminishing rate as the # of processes increases. The overall processing time decreased from 83s to 9.8s so nearly 8.5x performance.
