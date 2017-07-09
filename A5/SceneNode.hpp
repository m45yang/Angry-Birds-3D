#pragma once

#include "Material.hpp"

#include <glm/glm.hpp>

#include <list>
#include <stack>
#include <string>
#include <iostream>
#include <vector>

enum class NodeType {
    SceneNode,
    GeometryNode,
    JointNode,
    PhysicsNode
};

class SceneNode {
public:
    SceneNode(const std::string & name);

    SceneNode(const SceneNode & other);

    virtual ~SceneNode();

    static int totalSceneNodes();

    const glm::mat4& get_transform() const;
    const glm::mat4& get_inverse() const;

    void set_transform(const glm::mat4& m);

    void add_child(SceneNode* child);
    void add_parent(SceneNode* parent);

    void remove_child(SceneNode* child);

    //-- Transformations:
    void rotate(char axis, float angle);
    virtual void scale(const glm::vec3& amount);
    virtual void translate(const glm::vec3& amount);


    friend std::ostream & operator << (std::ostream & os, const SceneNode & node);

    bool isSelected;

    // Transformations
    glm::mat4 trans;
    glm::mat4 invtrans;

    std::list<SceneNode*> children;

    NodeType m_nodeType;
    std::string m_name;
    unsigned int m_nodeId;

    // A list of all SceneNodes indexed by m_NodeId
    static std::vector<unsigned int> geometryNodesToJoints;

    // The GeometryNodes that are selected.
    static std::vector<bool> selectedGeometryNodes;

private:
    // The number of SceneNode instances.
    static unsigned int nodeInstanceCount;
};
