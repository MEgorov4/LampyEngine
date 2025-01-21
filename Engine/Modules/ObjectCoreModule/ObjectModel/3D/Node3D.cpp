#include "Node3D.h"

Node3D::Node3D(Node* parentNode)
    : Node(parentNode), m_position(0.f), m_rotation(0.f), m_scale(1.f)
{
}

void Node3D::setPosition(const glm::vec3& position)
{
    m_position = position;
}

void Node3D::setRotation(const glm::vec3& rotation)
{
    m_rotation = rotation;
}

void Node3D::setScale(const glm::vec3& scale)
{
    m_scale = scale;
}

glm::vec3 Node3D::getPosition() const
{
    return m_position;
}

glm::vec3 Node3D::getRotation() const
{
    return m_rotation;
}

glm::vec3 Node3D::getScale() const
{
    return m_scale;
}

glm::mat4 Node3D::getLocalTransform()
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
    glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
    glm::mat4 rotation = rotationZ * rotationY * rotationX;
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);

    return translation * rotation * scale;
}

glm::mat4 Node3D::getGlobalTransform()
{
    glm::mat4 localTransform = getLocalTransform();

    if (m_parentNode)
    {
        if (Node3D* parentNode3D = dynamic_cast<Node3D*>(m_parentNode))
        {
            return parentNode3D->getGlobalTransform() * localTransform;
        }
    }

    return localTransform;
}

glm::vec3 Node3D::getGlobalPosition()
{
    glm::mat4 globalTransform = getGlobalTransform();
    return glm::vec3(globalTransform[3]);
}

glm::vec3 Node3D::getGlobalRotation()
{
    if (m_parentNode)
    {
        if (Node3D* parentNode3D = dynamic_cast<Node3D*>(m_parentNode))
        {
            return parentNode3D->getGlobalRotation() + m_rotation;
        }
    }
    return m_rotation;
}

glm::vec3 Node3D::getGlobalScale()
{
    if (m_parentNode)
    {
        if (Node3D* parentNode3D = dynamic_cast<Node3D*>(m_parentNode))
        {
            return parentNode3D->getGlobalScale() * m_scale;
        }
    }
    return m_scale;
}

