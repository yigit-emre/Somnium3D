#pragma once

#ifndef S3D_RENDER_ENGINE_EXPORT
#define S3D_API __declspec(dllimport)
#else
#define S3D_API __declspec(dllexport)
#endif // !S3D_RENDER_ENGINE_EXPORT