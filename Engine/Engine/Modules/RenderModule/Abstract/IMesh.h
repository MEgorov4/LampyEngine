#pragma once 

#include <memory>
#include "../../ResourceModule/Mesh.h"
namespace RenderModule
{
	class IMesh
	{
	public:
		IMesh(const std::shared_ptr<ResourceModule::RMesh>& mesh) {}
		virtual ~IMesh() = default;

		virtual void bind() const = 0;
		virtual void draw() const = 0;
		virtual void drawIndexed(int instanceCount) const = 0;
		virtual void unbind() const = 0;
	};
	
}