#pragma once 


#include "../../Abstract/ITexture.h"
namespace RenderModule::OpenGL
{
	class OpenGLTexture : public ITexture
	{
	public:
		OpenGLTexture(const std::shared_ptr<ResourceModule::RTexture>& texture);
		~OpenGLTexture() override;
		
		void bind() const override;
		void unbind() const override;

		TextureHandle getTextureID() override { return {m_textureID}; }
	private:
		unsigned int m_textureID;
	};
}