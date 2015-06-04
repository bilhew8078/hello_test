# hello_test
Demonstration of dispmanx_offline issue with 1920x1080 video playback and multiple dispmanx layers.
Move the directory "hello_test" into the "hello_pi" directory. Run make per the other examples.
Run "./hello_test.bin grinder.h264" and note the horizontal line that starts creeping into the image.
The program initializes 2 - 1920x1080 dispmanx layers in front of the video layer, and 2 - 1920x1080 dispmanx layers
behind the video layer.  The alpha of all dispmanx layers is set to 0 - transparent.   
"config.txt" settings:  dispmanx_offline=1   gpu_mem=384
Display resolution is 1920x1080 (1080p)
