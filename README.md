# omong

i don't know what i'm doing

### todo

 - finish iterator
 - deletion

### notes

 - btree indexing won't construct properly if KeyType isn't default constructible

### static_cast vs. reinterpret_cast?

https://stackoverflow.com/questions/45310822/mmap-and-c-strict-aliasing-rules
 - going to go with reinterpret_cast for now
 - but it looks like indexes won't be portable ):
