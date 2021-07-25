#ifndef Octree_h
#define Octree_h

#include <vector>
#include <iostream>
#include <algorithm>
#include "Vec3.h"
#include "Point.h"

class Octree
{
public:
	struct BaseNode
	{
		Vec3 origin; //Center of this Node
		float halfSize; //Half width/height/depth of this Node

		BaseNode() : origin(NULL), halfSize(NULL) { }
		BaseNode(const Vec3& origin, const float halfSize) : origin(origin), halfSize(halfSize) { }

		virtual ~BaseNode() {}
	};

	//Holds only one Point and pointers to 8 children nodes
	struct InnerNode : BaseNode
	{
		BaseNode *children[8];
		InnerNode *parent;
		Point *data;

		InnerNode(const Vec3& origin, const float halfSize)
		{
			this->origin = origin;
			this->halfSize = halfSize;
			this->data = NULL;
			parent = NULL;

			for (int i = 0; i < 8; ++i)
			{
				children[i] = NULL;
			}

		}

		~InnerNode()
		{
			for (int i = 0; i < 8; ++i)
			{
				if (children[i] != NULL)
					delete children[i];
			}
		}

		//Recursive branch clear from last node
		void RemoveChildNode(BaseNode* node)
		{
			for (int i = 0; i < 8; ++i)
			{
				if (children[i] == node)
				{
					delete children[i];
					children[i] = NULL;
				}
			}

			//If this node has no children or data, remove it from the parent
			if (this->IsDivided())
			{
				if (data == NULL)
				{
					if (this->parent != NULL)
						this->parent->RemoveChildNode(this);
				}
			}
		}

		void RemoveData()
		{
			data = NULL;
		}

		int GetNodeIndexContainingPoint(const Vec3 &point) const
		{
			int oct = 0;
			if (point.x() >= origin.x()) oct |= 4;
			if (point.y() >= origin.y()) oct |= 2;
			if (point.z() >= origin.z()) oct |= 1;
			return oct;
		}

		//Node is divided if it has any child nodes
		bool IsDivided() const
		{
			for (int i = 0; i < 8; ++i)
			{
				if (children[i] != NULL)
					return false;
			}
			return true;
		}
	};

	//Holds points at the bottom of the tree
	struct LeafNode : BaseNode
	{
		InnerNode *parent;
		std::vector<Point*> data;

		LeafNode(const Vec3& origin, const float halfSize)
		{
			this->origin = origin;
			this->halfSize = halfSize;
			parent = NULL;
		}

		~LeafNode() { }

		void Insert(Point* point)
		{
			data.push_back(point);
		}

		//Remove a point from the vector 
		void RemoveData(Point* point)
		{
			data.erase(std::remove(data.begin(), data.end(), point), data.end());

			//delete this node from parent if its empty
			if (data.size() == 0)
			{
				parent->RemoveChildNode(this);
			}
		}

		// Loop through the data vector and return the points that are inside this box
		void GetPointsInBox(const Vec3 &bmin, const Vec3 &bmax, std::vector<Vec3> &results)
		{
			for (Point* point : data)
			{
				const Vec3 rPoint = point->GetPosition();

				if (rPoint.x() > bmax.x()) continue;
				if (rPoint.y() > bmax.y()) continue;
				if (rPoint.z() > bmax.z()) continue;
				if (rPoint.x() < bmin.x()) continue;
				if (rPoint.y() < bmin.y()) continue;
				if (rPoint.z() < bmin.z()) continue;

				results.push_back(point->GetPosition());
			}
		}
	};

private:
	InnerNode *rootNode;
	int maxDepth = 1; //it will resize if out-of-scope point is added

	//Create tree from the given vector
	void SimpleInsert(std::vector<Point*> &points)
	{
		for (Point *points : points)
		{
			this->Insert(points);
		}
	}

	//Resizes the tree outwards towards the point
	void Grow(const Vec3 &point)
	{
		//if any point is out of scope of the tree, it has to grow
		Vec3 newOrigin = Vec3(rootNode->origin.x() + (point.x() > rootNode->origin.x() ? rootNode->halfSize : -rootNode->halfSize),
			rootNode->origin.y() + (point.y() > rootNode->origin.y() ? rootNode->halfSize : -rootNode->halfSize),
			rootNode->origin.z() + (point.z() > rootNode->origin.z() ? rootNode->halfSize : -rootNode->halfSize));

		InnerNode *newRoot = new InnerNode(newOrigin, rootNode->halfSize * 2);

		const int index = newRoot->GetNodeIndexContainingPoint(rootNode->origin);
		newRoot->children[index] = rootNode;
		rootNode->parent = newRoot;
		rootNode = newRoot;
		maxDepth++;
	}

	bool IsPointOutOfScope(const Vec3 &pos)
	{
		Vec3 cmax = rootNode->origin + rootNode->halfSize;
		Vec3 cmin = rootNode->origin - rootNode->halfSize;

		if (pos.x() > cmax.x() &&
			pos.y() > cmax.y() &&
			pos.z() > cmax.z() &&
			pos.x() < cmin.x() &&
			pos.y() < cmin.y() &&
			pos.z() < cmin.z())
		{
			return true;
		}
		return false;
	}

	void RecursiveInsert(Point* point, InnerNode* currentNode, const int currentDepth = 0)
	{
		// if max depth reached
		// new node is created if it didn't exist before
		// point is assigned to that node
		if (currentDepth >= maxDepth)
		{
			int index = currentNode->GetNodeIndexContainingPoint(point->GetPosition());

			if (currentNode->children[index] == NULL)
			{
				currentNode->children[index] = new LeafNode((const Vec3(
					currentNode->origin.x() + currentNode->halfSize * (index & 4 ? 0.5 : -0.5),
					currentNode->origin.y() + currentNode->halfSize * (index & 2 ? 0.5 : -0.5),
					currentNode->origin.z() + currentNode->halfSize * (index & 1 ? 0.5 : -0.5))), currentNode->halfSize*0.5);

				static_cast<LeafNode*>(currentNode->children[index])->parent = currentNode;
			}

			static_cast<LeafNode*>(currentNode->children[index])->Insert(point);
			return;
		}

		// if current node is divided
		// data is either assigned or passed down whle creating new nodes
		// it happens until both new and previous point are in separate nodes
		// or until max depth was reached

		if (currentNode->IsDivided())
		{
			if (currentNode->data == NULL)
			{
				currentNode->data = point;
				return;
			}
			else
			{
				Point* oldData = currentNode->data;
				currentNode->data = NULL;

				int index = currentNode->GetNodeIndexContainingPoint(oldData->GetPosition());

				if (currentNode->children[index] == NULL)
				{
					currentNode->children[index] = new InnerNode((const Vec3(
						currentNode->origin.x() + currentNode->halfSize * (index & 4 ? 0.5 : -0.5),
						currentNode->origin.y() + currentNode->halfSize * (index & 2 ? 0.5 : -0.5),
						currentNode->origin.z() + currentNode->halfSize * (index & 1 ? 0.5 : -0.5))), currentNode->halfSize*0.5);

					static_cast<InnerNode*>(currentNode->children[index])->parent = currentNode;
				}


				int otherIndex = currentNode->GetNodeIndexContainingPoint(point->GetPosition());

				if (currentNode->children[otherIndex] == NULL)
				{
					currentNode->children[otherIndex] = new InnerNode((const Vec3(
						currentNode->origin.x() + currentNode->halfSize * (otherIndex & 4 ? 0.5 : -0.5),
						currentNode->origin.y() + currentNode->halfSize * (otherIndex & 2 ? 0.5 : -0.5),
						currentNode->origin.z() + currentNode->halfSize * (otherIndex & 1 ? 0.5 : -0.5))), currentNode->halfSize*0.5);

					static_cast<InnerNode*>(currentNode->children[otherIndex])->parent = currentNode;
				}

				this->RecursiveInsert(oldData, static_cast<InnerNode*>(currentNode->children[index]), currentDepth + 1);
				this->RecursiveInsert(point, static_cast<InnerNode*>(currentNode->children[otherIndex]), currentDepth + 1);
			}
		}
		else
		{
			const int index = currentNode->GetNodeIndexContainingPoint(point->GetPosition());

			if (currentNode->children[index] == NULL)
			{
				currentNode->children[index] = new InnerNode((const Vec3(
					currentNode->origin.x() + currentNode->halfSize * (index & 4 ? 0.5 : -0.5),
					currentNode->origin.y() + currentNode->halfSize * (index & 2 ? 0.5 : -0.5),
					currentNode->origin.z() + currentNode->halfSize * (index & 1 ? 0.5 : -0.5))), currentNode->halfSize*0.5);


				static_cast<InnerNode*>(currentNode->children[index])->parent = currentNode;
			}
			this->RecursiveInsert(point, static_cast<InnerNode*>(currentNode->children[index]), currentDepth + 1);
		}
	}

	// Go down the tree to find the node holding a point
	void RecursiveGetNode(Point* point, BaseNode *&returnNode, InnerNode *currentNode = NULL, const int currentDepth = 0)
	{
		if (currentDepth >= maxDepth + 1)
		{
			returnNode = currentNode;
			return;
		}

		if (currentNode->IsDivided())
		{
			if (currentNode->data != NULL)
			{
				if (currentNode->data == point)
				{
					returnNode = currentNode;
					return;
				}
			}
		}
		else
		{
			const int index = currentNode->GetNodeIndexContainingPoint(point->GetPosition());

			this->RecursiveGetNode(point, returnNode, static_cast<InnerNode*>(currentNode->children[index]), currentDepth + 1);
		}
	}

	// Go down the tree finding all points inside a given box
	void RecursiveGetPointsInsideBox(const Vec3 &bmin, const Vec3 &bmax, std::vector<Vec3> &results, int currentDepth = 0, InnerNode *currentNode = NULL)
	{
		if (currentNode == NULL)
		{
			currentNode = rootNode;
		}

		if (currentDepth >= maxDepth)
		{
			for (int i = 0; i < 8; ++i)
			{
				if (currentNode->children[i] != NULL)
				{
					Vec3 cmax = currentNode->children[i]->origin + currentNode->children[i]->halfSize;
					Vec3 cmin = currentNode->children[i]->origin - currentNode->children[i]->halfSize;

					if (cmax.x() < bmin.x() || cmax.y() < bmin.y() || cmax.z() < bmin.z()) continue;
					if (cmin.x() > bmax.x() || cmin.y() > bmax.y() || cmin.z() > bmax.z()) continue;

					static_cast<LeafNode*>(currentNode->children[i])->GetPointsInBox(bmin, bmax, results);
				}
			}
			return;
		}

		if (currentNode->IsDivided())
		{
			if (currentNode->data != NULL)
			{
				const Vec3& p = currentNode->data->GetPosition();
				if (p.x() > bmax.x() || p.y() > bmax.y() || p.z() > bmax.z()) return;
				if (p.x() < bmin.x() || p.y() < bmin.y() || p.z() < bmin.z()) return;
				results.push_back(currentNode->data->GetPosition());
			}
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				if (currentNode->children[i] != NULL)
				{
					Vec3 cmax = currentNode->children[i]->origin + currentNode->children[i]->halfSize;
					Vec3 cmin = currentNode->children[i]->origin - currentNode->children[i]->halfSize;

					if (cmax.x() < bmin.x() || cmax.y() < bmin.y() || cmax.z() < bmin.z()) continue;
					if (cmin.x() > bmax.x() || cmin.y() > bmax.y() || cmin.z() > bmax.z()) continue;

					this->RecursiveGetPointsInsideBox(bmin, bmax, results, currentDepth + 1, static_cast<InnerNode*>(currentNode->children[i]));
				}
			}
		}
	}

public:
	Octree(const Vec3& origin, const float halfSize, const int maxDepth) : maxDepth(maxDepth)
	{
		//Create Root Node
		rootNode = new InnerNode(origin, halfSize);
	}

	Octree(const Vec3& origin, const float halfSize, const int maxDepth, std::vector<Point*> &points) : maxDepth(maxDepth)
	{
		//Create Root Node
		rootNode = new InnerNode(origin, halfSize);

		SimpleInsert(points);
	}

	~Octree() { delete rootNode; }

	void Insert(Point* point)
	{
		//check if point is outside of the tree scope
		if (IsPointOutOfScope(point->GetPosition()))
		{
			Grow(point->GetPosition());
			//Try inserting again
			Insert(point);
			return;
		}

		//Start recursive insert with rootNode
		RecursiveInsert(point, rootNode);
	}

	void Remove(Point* point)
	{
		//find node with this point, start search at rootNode
		BaseNode* node;
		RecursiveGetNode(point, node, rootNode);

		if (static_cast<LeafNode*>(node)->data.size() > 0)
		{
			//true leaf node
			static_cast<LeafNode*>(node)->RemoveData(point);
			return;
		}

		if (static_cast<InnerNode*>(node)->IsDivided())
		{
			static_cast<InnerNode*>(node)->RemoveData();
			//recursive delete until parent is null or data isn't null
			static_cast<InnerNode*>(node)->parent->RemoveChildNode(node);
		}
	}

	// Get all points inside box of given size
	void GetPointsInsideBox(const Vec3 &bmin, const Vec3 &bmax, std::vector<Vec3> &results)
	{
		RecursiveGetPointsInsideBox(bmin, bmax, results, 0, rootNode);
	}
};

#endif

