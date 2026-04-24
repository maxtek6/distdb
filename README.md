# distdb

Optimized key-value storage for static content.

## About

The goal of this project is to create a solution for compiling static 
assets into a single archive, where they can be read and served with 
minimal overhead in both storage and processing. It is named after the
`dist` folder, which is the default output directory for `vite`. While 
other solutions exist, they differ in their design or target use.

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
of using hyperpage would be that existing archives can be modified.

+ Legacy formats: Other formats such as Web Archive(Apple), MAFF(Mozilla), 
or Compiled HTML(Microsoft), are no longer being actively maintained.