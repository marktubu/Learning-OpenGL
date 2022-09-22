
#include <glm/glm.hpp>

#include "Component.h"

class Transform : public Component {
public:
    glm::vec3 Position() { return position; }
    glm::vec3 Rotation() { return rotation; }
    glm::vec3 Scale() { return scale; }

    void SetPosition(glm::vec3 pos) { position = pos; }
    void SetRotation(glm::vec3 rot) { rotation = rot; }
    void SetScale(glm::vec3 scale) { scale = scale; }

private:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};