## State:

- Currently in development. Finished features:
    - save from RAM to new file in specified format
    - save from RAM to pointer (encode without writing to a file)
    - .qoi read/write in RGBA, BGRA, RGB or BGR

## Planned Features:

- Support for more major lossless image formats, in priority order:
    - .bmp
    - .webp
    - .png
    - .avif
    - .jxl

- Support for the following additional operations:
    - convert formats between each other (really just a wrapper around load/write)
