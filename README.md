# Octree-cpp

A fairly simple implementation of the Octree. There is a lot more to be done with it, but I'm happy with it performance-wise at least.

There are some optimizations and features:
- Growth - the tree grows if an out-of-scope point is added
- Insertion/Removal - A specific point can be added or removed from the tree. Nodes are only created when required, each child is not split into 8 children, but only if inserted point will need to occupy it.
- Inner/Leaf nodes - Inner nodes store only one point, Leaf nodes are created when maxDepth is reached and store all remaining points
- Only center and half size of the node is stored per node, instead of storing bounds to save some memory.

It would be nice to implement that in the future:
- Shrinking - when point is removed and a branch colapses, and if root node has only one child, it should become the new root node.
- Stack/Queue of used nodes - when nodes are removed they are deleted, but for a more dynamic scene when there is a large number of objects moving, it might be better to store unused nodes and reuse them when necessary instead of creating new ones.


References and useful stuff:
- (this one is about quadtrees, but the post is great and can easily be extended to 3D) https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
- https://github.com/brandonpelfrey/SimpleOctree/blob/master/Octree.h
- https://doc.magnum.graphics/magnum/examples-octree.html
- http://nomis80.org/code/octree.html
- https://www.youtube.com/watch?v=m0guE7804to
- https://github.com/mcserep/NetOctree
- https://www.gamedev.net/articles/programming/general-and-gameplay-programming/introduction-to-octrees-r3529/
- https://www.geeksforgeeks.org/octree-insertion-and-searching/
- https://imkaywu.github.io/blog/2015/11/voxel/
- https://github.com/gametutorials/tutorials/blob/master/OpenGL/Octree/Octree.cpp
- https://www.piko3d.net/tutorials/space-partitioning-tutorial-piko3ds-dynamic-octree/
- https://gameprogrammingpatterns.com/spatial-partition.html
