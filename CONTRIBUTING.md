## For potential contributors:

I'm happy about your interest to contribute! Here are some things you might want to know
before creating your first pull request.

Imgsurf is meant to be a simple API that allows reading/writing lossless image formats.
The main area of contributions is the writing/loading of different formats. Each image
format gets its own file, e.g. `imgsurf_format_qoi.c`, which simply contains `loadQOI`
and `writeQOI`. This is because these two functions tend to be rather big on their own.

Where possible, please provide error checking for things like an unexpected end-of-file
or malformed image data, according to the specificaton.

If you use `fread/fwrite`, please check for its successful completion like so:

```c
if((elements = fread(&byte, 1, 1, file)) != 1)
{
    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read diffs QOI_OP_LUMA.\033[0m\n");
}
```

The ANSI codes are optional, but please output some kind of error and don't return
early. This makes it easier to look at the output and see multiple failure points at
once, if there are any.

If you do use the ANSI codes, please always make sure to terminate them correctly, that
is to say, return to normal output with `\033[0m`.

I'd really appreciate it if you took a look at the code style, which, while maybe
unconventional, I'd like to remain consistent throughout the codebase.

The most important thing is the column limit of `88` characters - please don't create
huge one-liners. If it's longer than existing code, it's probably over.
