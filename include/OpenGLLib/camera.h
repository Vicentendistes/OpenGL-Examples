#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // para glm::mat4 en la view()

class Camera {
public:
  glm::vec3 position;
  float distance;
  float yaw;
  float pitch;
  float sensitivity = 0.1f;  // typo corregido y valor práctico
  float zoom        = 45.f;
  float speed       = 1.f;

  Camera(glm::vec3 position = {0.f,0.f,3.f},
         float distance    = 3.f,
         float yaw         = -90.f,
         float pitch       = 0.f,
         glm::vec3 up      = {0.f,1.f,0});
  Camera(Camera &&)            = default;
  Camera(const Camera &)       = default;
  Camera &operator=(Camera &&) = default;
  Camera &operator=(const Camera &) = default;
  ~Camera() = default;

  void update(float deltaTime = 0);
  glm::mat4 view();

  void key_callback(float xoffset, float yoffset);
  void cursor_position_callback(float xoffset, float yoffset);
  void scroll_callback(float yoffset);

private:
  glm::vec3 up_;
  glm::vec3 front_;
  glm::vec3 right_;
  glm::vec3 worldUp_;
  glm::vec3 direction_;

  void updateVectors();
};

#endif // _CAMERA_H_
