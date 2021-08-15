# LKG App Template

This is an example raylib project designed to facilitate performant, realtime rendering on the Raspberry Pi for the Looking Glass Portrait. 

It also includes a scene system (see scene.h) to quickly create new content for the LKG.

## Building

Please use Raspberry Pi OS Lite and the provided boot config files. Make sure your user has permission to use the GPU.

Please use my fork of raylib, which adds OpenGL ES 3 support for the Pi (for instancing), and enables JPG image support by default: https://github.com/TesseractCat/raylib.
