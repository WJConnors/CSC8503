#pragma once
#include "CollisionDetection.h"
#include "Debug.h"

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree;

		template<class T>
		struct QuadTreeEntry {
			Vector3 pos;
			Vector3 size;
			T object;

			QuadTreeEntry(T obj, Vector3 pos, Vector3 size) {
				object		= obj;
				this->pos	= pos;
				this->size	= size;
			}
		};

		template<class T>
		class QuadTreeNode	{
		public:
			typedef std::function<void(std::list<QuadTreeEntry<T>>&)> QuadTreeFunc;
		protected:
			friend class QuadTree<T>;

			QuadTreeNode() {}

			QuadTreeNode(Vector2 pos, Vector2 size) {
				children		= nullptr;
				this->position	= pos;
				this->size		= size;
			}

			~QuadTreeNode() {
				delete[] children;
			}

			void Insert(const T& object, const Vector3& objectPos, const Vector3& objectSize, int depthLeft, int maxSize) {
				// Check if the object's AABB intersects this node's AABB
				if (!CollisionDetection::AABBTest(objectPos, Vector3(position.x, 0, position.y), objectSize, Vector3(size.x, 1000.0f, size.y))) {
					return; // Object doesn't intersect this node
				}

				if (children) { // Node is not a leaf, pass the object to its children
					for (int i = 0; i < 4; ++i) {
						children[i].Insert(object, objectPos, objectSize, depthLeft - 1, maxSize);
					}
				}
				else { // Node is a leaf, handle insertion here
					contents.push_back(QuadTreeEntry<T>(object, objectPos, objectSize));

					// Check if we need to split the node
					if ((int)contents.size() > maxSize && depthLeft > 0) {
						if (!children) {
							Split();

							// Reinsert the contents into the children
							for (const auto& i : contents) {
								for (int j = 0; j < 4; ++j) {
									children[j].Insert(i.object, i.pos, i.size, depthLeft - 1, maxSize);
								}
							}

							// Clear the contents of the current node
							contents.clear();
						}
					}
				}
			}

			void Split() {
				// Calculate half the size of the current node
				Vector2 halfSize = size / 2.0f;

				// Allocate memory for the child nodes
				children = new QuadTreeNode<T>[4];

				// Create the child nodes, each occupying a quadrant of the current node
				children[0] = QuadTreeNode<T>(position + Vector2(-halfSize.x, halfSize.y), halfSize); // Top-left
				children[1] = QuadTreeNode<T>(position + Vector2(halfSize.x, halfSize.y), halfSize);  // Top-right
				children[2] = QuadTreeNode<T>(position + Vector2(-halfSize.x, -halfSize.y), halfSize); // Bottom-left
				children[3] = QuadTreeNode<T>(position + Vector2(halfSize.x, -halfSize.y), halfSize);  // Bottom-right
			}

			void DebugDraw() {
			}

			void OperateOnContents(QuadTreeFunc& func) {
				if (children) {
					// Recursively call OperateOnContents on each child node
					for (int i = 0; i < 4; ++i) {
						children[i].OperateOnContents(func);
					}
				}
				else {
					// If this is a leaf node and it contains data, apply the function
					if (!contents.empty()) {
						func(contents);
					}
				}
			}

		protected:
			std::list< QuadTreeEntry<T> >	contents;

			Vector2 position;
			Vector2 size;

			QuadTreeNode<T>* children;
		};
	}
}


namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree
		{
		public:
			QuadTree(Vector2 size, int maxDepth = 6, int maxSize = 5){
				root = QuadTreeNode<T>(Vector2(), size);
				this->maxDepth	= maxDepth;
				this->maxSize	= maxSize;
			}
			~QuadTree() {
			}

			void Insert(const T& object, const Vector3& pos, const Vector3& size) {
				root.Insert(object, pos, size, maxDepth, maxSize);
			}

			void DebugDraw() {
				root.DebugDraw();
			}

			void OperateOnContents(typename QuadTreeNode<T>::QuadTreeFunc  func) {
				root.OperateOnContents(func);
			}

		protected:
			QuadTreeNode<T> root;
			int maxDepth;
			int maxSize;
		};
	}
}