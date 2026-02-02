## State:

- Currently working on the loader. Working formats:
    - .qoi, loaded in RGBA, BGRA, RGB or BGR

## Planned Features:

- Support for more major lossless image formats, in priority order:
    - .qoi
    - .bmp
    - .webp
    - .png
    - .avif
    - .jxl

- Support for the following additional operations:
    - save from RAM to file in specified format
    - convert formats between each other (really just a wrapper around load/write)
