# hello_test
Demonstration of dispmanx_offline issue with 1920x1080 video playback and multiple dispmanx layers.
Clone the "hello_test" repository into the "hello_pi" directory. CD to "hello_test" and make per the other examples.
Run "./hello_test.bin grinder.h264" and note the horizontal tear line that starts creeping into the image at the top
of the frame after about 20-30 seconds.
The program initializes 2 - 1920x1080 dispmanx layers in front of the video layer, and 2 - 1920x1080 dispmanx layers
behind the video layer.  The alpha of all dispmanx layers is set to 0 - transparent.   
"config.txt" settings:  dispmanx_offline=1   gpu_mem=384
Display resolution is 1920x1080 (1080p)
In my real world application the tear doesn't always appear at the top of the frame. I have seen this in many other 
videos, however the "grinder.h264" makes it very easy to see.  The "hello_video" player is set to loop.
I am using a Pi 2 B+.  I am suspecting some issue with the vertical sync and compositing with Dispmanx layers.
