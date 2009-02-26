#ifndef _OGL_SHADER_
#define _OGL_SHADER_

#include <string>
#include <map>
#include <OpenGL/gl.h>

class Shader {

friend class XML_ShaderParser;
private:

	GLhandleARB _programObj;
	GLcharARB *_vert;
	GLcharARB *_frag;
	bool _loaded;

	static std::map<std::string, Shader> Shaders;

public:

	static Shader* get(const std::string& name) {
		if(Shaders.count(name) > 0) {
			return &Shaders[name];			
		}
		return NULL;
	}
	
	Shader() : _programObj(NULL), _loaded(false) {}
	Shader(const std::string& vert, const std::string& frag);
	~Shader();

	void load();
	void init();
	void enable();
	void setFloat(const char* name, float);

	static void disable();
};

#include "cseries.h"
#include "XML_ElementParser.h"

XML_ElementParser *Shader_GetParser();

#endif
