#include "pointMass.h"

pointMass::pointMass(int id, glm::vec3 position, glm::vec3 speed, float mass) :
    position(position),
    mass(mass),
    speed(speed),
    id(id){
}
void pointMass::update(glm::vec3* dynamicPos){
    this->position += this->speed;
    dynamicPos[this->id] = this->position;

}