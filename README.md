# traverse.cpp

Algebraic data structure traversal and folding in C++.

## Rationale

I was following some opengl tutorial that shows how to coordonate serveral elements of a hierarchical structure to render a character on screen, and I thought to myself: ["I can do better"](https://github.com/de-passage/opengl-stuffs/blob/main/examples/hierarchy.cpp). The original code was using a giant class representing the character and a collection of private functions calling each other to render the whole thing and handle motion. The thing was completely unreadable, a perfect example of how simple code can fail to convey intent. 

I then quickly abstracted the whole thing into a compile-time library defining arbitrary hierarchies of items, and wrote a way to select individual elements and to traverse the resulting tree. This repo is the latter part, generalized to work on anything with minimum effort from the user.

The basic idea is that any C++ object is intrinsically either a single atomic[^1] entity, or a collection of said atoms. Traversal, the act of recursively exploring a collection and applying some action to the atoms, always makes sense: we typîcally use standard algorithms for collections of homogeneous values, and the visitor design pattern for aggregates. This library tries to offer a unified way to traverse and fold[^2] C++ objects.

[^1]: As in indivisible, nothing to do with concurrency. 
[^2]: Collect all traversed values into a single result.

## Usage

The example subfolder contains everything you need to understand the library.

The [traverse.cpp](https://github.com/de-passage/traverse.cpp/blob/main/examples/traverse.cpp) file shows how to use the traverse() function with standard types. 

The [composite.cpp](https://github.com/de-passage/traverse.cpp/blob/main/examples/composite.cpp) example shows how to use the library to print tag hierarchies into HTML and markdown.

Finally, [customization_points.cpp](https://github.com/de-passage/traverse.cpp/blob/main/examples/customization_points.cpp) explains various ways to extend your types to support traversal.
