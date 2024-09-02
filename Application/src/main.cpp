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
		FILE* file;
		fopen_s(&file, "D:\\visualDEV\\Somnium3D\\Application\\src\\errorlog.txt", "a");
		fprintf_s(file, "\n-----------------------------------------------\n");
		fprintf_s(file, error.what());
		fclose(file);
	}
	s3DTerminateRenderEngine();
	return 0;
}