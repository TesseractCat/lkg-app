# LKG App Template

This is an example raylib project designed to facilitate performant, realtime rendering on the Raspberry Pi for the Looking Glass Portrait. 

It also includes a scene system (see scene.h) to quickly create new content for the LKG.

## Building

Please use Raspberry Pi OS Lite and the provided boot config files. Make sure your user has permission to use the GPU.

Please use my fork of raylib, which adds OpenGL ES 3 support for the Pi (for instancing), and enables JPG image support by default: https://github.com/TesseractCat/raylib.

### Installation

1. Build Raylib for the Raspberry Pi 4 (DRM\*) as per https://github.com/raysan5/raylib/wiki/Working-on-Raspberry-Pi
    - Install dependencies (see linked wiki page)
    - Navigate to the raylib `src` directory.
    - Run `sudo make PLATFORM=PLATFORM_DRM`
    - Run `sudo make install`
        - `sudo make install` may expect libraylib.a to be in the parent directory. However make places it into the src directory, so you might have to move libraylib.a up one directory.
2. Run `mkdir build` and `./build.sh`.
3. Update `display.cfg` with your device specific values (taken from the `visual.json` file on the embedded usb drive). Note that for the portrait I find the a viewCone of 50 gives a better experience.

\* Note that DRM means 'Direct Rendering Manager', not 'Digital Rights Management'.
