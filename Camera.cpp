#include "Camera.hpp"


inline bool inRange(float a, float b, float x) {   
    return a < x && x < b;
}

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) : cameraPosition(cameraPosition), cameraTarget(cameraTarget), cameraUpDirection(cameraUp) {
        this->cameraFrontDirection = glm::vec3(0.0f, 0.0f, -1.0f);
        this->cameraRightDirection = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(this->cameraPosition, this->cameraPosition + this->cameraFrontDirection, this->cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 newPos{};

        switch (direction) {
        case gps::MOVE_FORWARD:
            newPos = this->cameraPosition + this->cameraFrontDirection * speed;
            break;
        case gps::MOVE_BACKWARD:
            newPos = this->cameraPosition - this->cameraFrontDirection * speed;
            break;
        case gps::MOVE_RIGHT:
            newPos = this->cameraPosition + this->cameraRightDirection * speed;
            break;
        case gps::MOVE_LEFT:
            newPos = this->cameraPosition - this->cameraRightDirection * speed;
            break;
        case gps::MOVE_UP:
            newPos = this->cameraPosition + glm::vec3(0, 1, 0) * speed;
            break;
        case gps::MOVE_DOWN:
            newPos = this->cameraPosition - glm::vec3(0, 1, 0) * speed;
            break;
        }
    
        if (verifyPos(newPos))
            this->cameraPosition = newPos;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        this->cameraFrontDirection = glm::normalize(front);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }

    void Camera::printPosition() {
        glm::vec3 p = this->cameraPosition;
        printf("x=%f  y=%f  z=%f\n", p.x, p.y, p.z);
    }

    void Camera::setPosition(glm::vec3 newPos) {
        this->cameraPosition = newPos;
    }

    glm::vec3 Camera::getPosition()
    {
        return this->cameraPosition;
    }

    void Camera::resetPosition()
    {
        this->cameraPosition = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    bool Camera::verifyPos(glm::vec3 newPos)
    {
        float x, y, z;
        x = newPos.x;
        y = newPos.y;
        z = newPos.z;

        if (!gps::insideScene<float>(x, y, z))
            return false;
        
        for (auto& boundarie : gps::boundaries) {
            if (inRange(boundarie.xMin, boundarie.xMax, x) && inRange(boundarie.yMin, boundarie.yMax, y) && inRange(boundarie.zMin, boundarie.zMax, z)) {
                return false;
            }
        }
        
        return true;
    }
}