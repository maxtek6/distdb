# distdb

Optimized storage for static web content.

## About

The goal of this project is to create a solution for compiling static 
assets into a single archive, where they can be read and served with 
minimal overhead in both storage and processing. It is named after the
`dist` folder, which is the default output directory for `vite`. While 
other solutions exist, they differ in their design or target use.

## Usage

`distdb` relies on a portable format that is both language and platform 
agnostic.

### Building

An archive can be built using the command-line utility `distdbc`. It 
requires an input directory and an output filename:

```bash
distdbc /path/to/content -o archive.dist
```

### Reading

The reader provides applications with a zero-copy interface using a 
single heap allocation and two system calls(`open()` and `mmap()`) 
during initialization. This dramatically reduces the memory and 
processing overhead, as the kernel will page resources in and out 
of physical memory as needed.

Archive resources can be loaded using the C interface:

```c
struct distdb_archive *archive
distdb_open("/my/archive/path", &archive);

distdb_resource resource;
distdb_find(archive, "/index.html", &resource);

printf("found resource of type %s and length %zu\n", resource->mime_type, resource->length);

distdb_close(db);
```

## Comparison

Given that `distdb` targets the common, albeit specific, use case of 
read-only asset loading, it is only appropriate to compare it to other 
solutions based on this criteria. Other solutions include:

+ Directory access: Many applications and web servers simply point to 
a directory on the filesystem and load assets as they are needed. This 
is fine in most cases, as they often have caching mechanisms to prevent 
excessive system call overhead. The main drawback to this approach is 
simplicity and performance become competing goals for developers, as 
developers must maintain a caching mechanism. This is also unpleasant 
for distribution.

+ WARC/WACZ: Used by the Wayback Machine and other services. While it 
is a widely recognized standard with a well defined specification, the 
intended use of this format is for web crawling. As a a result, these 
archives capture metadata and other information that would not be useful
to asset loading.

+ Hyperpage: Also developed my Maxtek Consulting, it uses SQLite as a 
container and stores blobs in a database, indexed by path. While this 
solution also provides a single file optimized for reads, it does not 
fully strip out the storage and processing overhead. The main advantage 
of using hyperpage would be that existing archives can be modified. A 
direct comparison shows distdb outperforming hyperpage by a factor of 
10 in both speed and storage.

+ Legacy formats: Other formats such as Web Archive(Apple), MAFF(Mozilla), 
or Compiled HTML(Microsoft), are not actively maintained.