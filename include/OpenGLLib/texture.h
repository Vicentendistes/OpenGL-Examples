#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <string>
#include <vector>

class Texture {
public:
  Texture(const std::string &path);
  Texture(const std::vector<unsigned char> &data, int width, int height, int nrChannels);
  Texture(Texture &&);
  Texture &operator=(Texture &&);
  ~Texture();

  void bind();

private:
  void build(const unsigned char* data);
  unsigned int id_;
  int width_, height_, nrChannels_;
};

#endif // _TEXTURE_H_
