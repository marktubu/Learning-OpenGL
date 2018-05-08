//
//  camera.h
//  ShaderTest
//
//  Created by apple on 2018/5/8.
//  Copyright © 2018年 tubu. All rights reserved.
//

#ifndef camera_h
#define camera_h

#include <glm/glm.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float FOV = 45.0f;

class camera{
public:
    glm::vec3 Position;
    glm::vec3 Direction;
    glm::vec3 UP;
    glm::vec3 Right;
    
    const glm::vec3 WorldUP = glm::vec3(0,1,0);
    
    float Pitch;            //俯仰角(上下点头)绕x轴旋转
    float Yaw;              //偏航角(左右转向)绕y轴旋转
    float Roll;             //翻滚角(左右翻滚)绕z轴旋转(本例中暂不使用)
    
    float Fov = 60;         //field of view
    float Near = 0.1f;      //near clip panel
    float Far = 1000;       //far clip panel
    
    float MovementSpeed = 0.5f;
    float MouseSensitivity = 0.1f;
    
    camera(glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f), glm::vec3 direction = glm::vec3(0,0,-1), glm::vec3 up = glm::vec3(0,1,0))
    {
        Position = position;
        Direction = direction;
        Right = glm::cross(up, Direction);
        UP = glm::cross(Direction, Right);
    }
    
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Direction, UP);
    }
    
    glm::mat4 GetProjectionMatrix(float aspect)
    {
        return glm::perspective(Fov, aspect, Near, Far);
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        float xoff = MouseSensitivity * xoffset;
        float yoff = MouseSensitivity * yoffset;
        
        Yaw += xoff;
        Pitch += yoff;
        
        if(constrainPitch)
        {
            if(Pitch > 89.0f)
                Pitch = 89.0f;
            if(Pitch < -89.0f)
                Pitch = -89.0f;
        }
        
        updateCameraVectors();
    }
    
    void ProcessMouseScroll(float yoffset)
    {
        if(Fov >= 1.0f && Fov <= 45.0f)
            Fov -= yoffset;
        if(Fov <= 1.0f)
            Fov = 1.0f;
        if(Fov >= 45.0f)
            Fov = 45.0f;
    }
    
    void ProcessKeyboard(Camera_Movement direction, float deltatime)
    {
        float velocity = MovementSpeed * deltatime;
        if(direction == FORWARD)
            Position += Direction * velocity;
        if(direction == BACKWARD)
            Position -= Direction * velocity;
        if(direction == LEFT)
            Position -= Right * velocity;
        if(direction == RIGHT)
            Position += Right * velocity;
    }
    
private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Direction = glm::normalize(front);
        Right = glm::normalize(glm::cross(WorldUP, Direction));
        UP = glm::normalize(glm::cross(Right, Direction));      //cross:右手螺旋定则?
    }
};

#endif /* camera_h */
