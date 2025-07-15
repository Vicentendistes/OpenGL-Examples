#ifndef _SHADER_H_
#define _SHADER_H_

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <glad/glad.h>

class Shader {
public:
  Shader(const std::string &vertexPath, const std::string &fragmentPath);
  Shader(const std::string &vertexPath, const std::string &geometryPath,
         const std::string &fragmentPath);
  Shader(Shader &&);
  Shader &operator=(Shader &&);
  ~Shader();

  void use();
  void set(const std::string &name, float value);
  void set(const std::string &name, int value);
  void set(const std::string &name, unsigned int value);
  void set(const std::string &name, bool value);
  void set(const std::string &name, const glm::mat4 &value);
  void set(const std::string &name, const glm::vec2 &value);
  void set(const std::string &name, const glm::vec3 &value);

  unsigned int getSubroutine(int type, const std::string &name);
  void setSubroutine(int type, const std::vector<unsigned int> &value);

private:
  int get(const std::string &name);

protected:
  unsigned int id_;
  Shader();
};

#ifdef GL_COMPUTE_SHADER
class ComputeShader: public Shader {
public:
  unsigned int global[3] = {1, 1, 1};
  ComputeShader(const std::string& path);
  void execute();
  void release();
};
#endif
#endif // _SHADER_H_
