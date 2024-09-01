#include "engine.hpp"
#include <iostream>
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
		std::cout << "\033[31m" << "[ERROR]: " << "\033[0m" << error.what() << std::endl;
	}
	s3DTerminateRenderEngine();
	return 0;
}