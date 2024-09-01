#include "engine.hpp"


int main() 
{
	AppWindowCreateInfo windowCreateInfo{};
	s3DInitRenderEngine(windowCreateInfo, true);

	s3DTerminateRenderEngine();
	return 0;
}