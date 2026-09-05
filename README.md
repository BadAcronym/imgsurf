## State:

- Currently in development. Finished features:
    - save from RAM to new file in specified format
    - save from RAM to pointer (encode without writing to a file)
    - .qoi read/write in RGBA, BGRA, RGB or BGR

- The PNG decoder is in-progress.

## Planned Features:

- Support for additional lossless image formats, in priority order:
    - .bmp
    - .webp
    - .avif
    - .jxl

- Support for the following additional operations:
    - convert formats between each other (really just a wrapper around load/write)
