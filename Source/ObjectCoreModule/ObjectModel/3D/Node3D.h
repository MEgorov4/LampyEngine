#pragma once
#include "../Node.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"

class Node3D : public Node
{

protected:
    glm::vec3 m_position;  
    glm::vec3 m_rotation; 
    glm::vec3 m_scale;    

public:
    Node3D(Node* parentNode = nullptr);
   
    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& rotation);
    void setScale(const glm::vec3& scale);
    
    glm::vec3 getPosition() const;
    glm::vec3 getRotation() const;
    glm::vec3 getScale() const;
   
    glm::vec3 getGlobalPosition();
    glm::vec3 getGlobalRotation();
    glm::vec3 getGlobalScale();

    glm::mat4 getLocalTransform();  
    glm::mat4 getGlobalTransform(); 
};

