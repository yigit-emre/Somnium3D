#include "engine.hpp"
#include <stdexcept>



int main() 
{
	try
	{
		AppWindowCreateInfo windowCreateInfo{};
		s3DInitRenderEngine(windowCreateInfo, true);
	}
	catch (const std::runtime_error& error)
	{
		s3DExceptionHandle(error.what());
	}
	s3DTerminateRenderEngine();
	return 0;
}