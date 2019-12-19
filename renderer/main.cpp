#define NOMINMAX
#include <windows.h>
#include <stdint.h>
#include <math.h>

#define global_variable static

global_variable int globalRunning;
global_variable BITMAPINFO globalBitmapInfo;
global_variable VOID *globalBitmapMemory;
global_variable int globalBitmapWidth;
global_variable int globalBitmapHeight;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef int32_t b32;

#define PI32 3.14159265f
#define PI64 3.14159265358979323846

/* SCRATCH A PIXEL */
#include <algorithm>
#include "geometry.h"
#include "cow.h"
#define internal static

enum FitResolutionGate {
	kFill = 0,
	kOverscan
};


struct Screen
{
    f32 right;
    f32 left;
    f32 top;
    f32 bottom;
};

Screen computeScreenCoordinates(f32 filmApertureWidth, f32 filmApertureHeight,
                                u32 imageWidth, u32 imageHeight, FitResolutionGate fitFilm,
                                f32 nearClippingPLane, f32 focalLength)
{
    Screen screen = {};
	f32 filmAspectRatio = filmApertureWidth / filmApertureHeight;
	f32 deviceAspectRatio = imageWidth / (f32)imageHeight;

	screen.top = ((filmApertureHeight * inchToMm / 2) / focalLength) * nearClippingPLane;
	screen.right = ((filmApertureWidth * inchToMm / 2) / focalLength) * nearClippingPLane;

	// field of view (horizontal)
	// f32 fov = 2.0f * 180.0f / PI32 * atan((filmApertureWidth * inchToMm / 2.0f) / focalLength);

	f32 xscale = 1;
	f32 yscale = 1;

	switch (fitFilm) {
		default:
		case kFill:
			if (filmAspectRatio > deviceAspectRatio) {
				xscale = deviceAspectRatio / filmAspectRatio;
			}
			else {
				yscale = filmAspectRatio / deviceAspectRatio;
			}
			break;
		case kOverscan:
			if (filmAspectRatio > deviceAspectRatio) {
				yscale = filmAspectRatio / deviceAspectRatio;
			}
			else {
				xscale = deviceAspectRatio / filmAspectRatio;
			}
			break;
	}

	screen.right *= xscale;
	screen.top *= yscale;

	screen.bottom = -screen.top;
	screen.left = -screen.right;

    return screen;
}

//[comment]
// Compute vertex raster screen coordinates.
// Vertices are defined in world space. They are then converted to camera space,
// then to NDC space (in the range [-1,1]) and then to raster space.
// The z-coordinates of the vertex in raster space is set with the z-coordinate
// of the vertex in camera space.
//[/comment]
void convertToRaster(
	Vec3 &vertexWorld,
	const Matrix44 &worldToCamera,
	const float &l,
	const float &r,
	const float &t,
	const float &b,
	float nearClippingPlane,
	const uint32_t &imageWidth,
	const uint32_t &imageHeight,
	Vec3 &vertexRaster)
{
	Vec3 vertexCamera = multVecMatrix(worldToCamera, vertexWorld);

	// convert to screen space
	Vec2 vertexScreen;
	vertexScreen.x = nearClippingPlane * vertexCamera.x / -vertexCamera.z;
	vertexScreen.y = nearClippingPlane * vertexCamera.y / -vertexCamera.z;

	// now convert point from screen space to NDC space (in range [-1,1])
	Vec2 vertexNDC;
	vertexNDC.x = 2 * vertexScreen.x / (r - l) - (r + l) / (r - l);
	vertexNDC.y = 2 * vertexScreen.y / (t - b) - (t + b) / (t - b);

	// convert to raster space
	vertexRaster.x = (vertexNDC.x + 1) / 2 * imageWidth;
	// in raster space y is down so invert direction
	vertexRaster.y = (1 - vertexNDC.y) / 2 * imageHeight;
	vertexRaster.z = -vertexCamera.z;
}

int renderCow(int bytesPerPixel, u32 width, u32 height)
{
    u32 imageWidth = width;
    u32 imageHeight = height;
    const uint32_t ntris = 3156;
    const float nearClippingPLane = 1;
    const float farClippingPLane = 1000;
    float focalLength = 20; // in mm

// 35mm Full Aperture in inches
    float filmApertureWidth = 0.980f;
    float filmApertureHeight = 0.735f;

    const Matrix44 worldToCamera = {
        0.707107f, -0.331295f, 0.624695f, 0.0f,
        0.0f, 0.883452f, 0.468521f, 0.0f,
        -0.707107f, -0.331295f, 0.624695f, 0.0f,
        -1.63871f, -5.747777f, -40.400412f, 1.0f
    };

	Matrix44 cameraToWorld = worldToCamera.inverse();

	Screen screen = computeScreenCoordinates(filmApertureWidth, filmApertureHeight,
                                             imageWidth, imageHeight, kOverscan,
                                             nearClippingPLane, focalLength);
    f32 t = screen.top;
    f32 b = screen.bottom;
    f32 l = screen.left;
    f32 r = screen.right;

	// define the frame-buffer and the depth-buffer. Initialize depth buffer
	// to far clipping plane.
	Vec3u *frameBuffer = new Vec3u[imageWidth * imageHeight];

	for (uint32_t i = 0; i < imageWidth * imageHeight; ++i) {
		frameBuffer[i] = {(u32)floorf(255 * 0.235294f), (u32)floorf(255 * 0.67451f), (u32)floorf(255 * 0.843137f)};
	}

	float *depthBuffer = new float[imageWidth * imageHeight];

	for (uint32_t i = 0; i < imageWidth * imageHeight; ++i) {
		depthBuffer[i] = farClippingPLane;
	}

	// [comment]
	// Outer loop
	// [/comment]
	for (uint32_t i = 0; i < ntris; ++i) {
		Vec3 v0 = vertices[nvertices[i * 3]];
		Vec3 v1 = vertices[nvertices[i * 3 + 1]];
		Vec3 v2 = vertices[nvertices[i * 3 + 2]];

		// [comment]
		// Convert the vertices of the triangle to raster space
		// [/comment]
		Vec3 v0Raster, v1Raster, v2Raster;
		convertToRaster(v0, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v0Raster);
		convertToRaster(v1, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v1Raster);
		convertToRaster(v2, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v2Raster);

		// [comment]
		// Precompute reciprocal of vertex z-coordinate
		// [/comment]
		v0Raster.z = 1 / v0Raster.z,
		v1Raster.z = 1 / v1Raster.z,
		v2Raster.z = 1 / v2Raster.z;

		// [comment]
		// Prepare vertex attributes. Divide them by their vertex z-coordinate
		// (though we use a multiplication here because v.z = 1 / v.z)
		// [/comment]
		Vec2 st0 = cow_st[stindices[i * 3]];
		Vec2 st1 = cow_st[stindices[i * 3 + 1]];
		Vec2 st2 = cow_st[stindices[i * 3 + 2]];

		st0 *= v0Raster.z;
        st1 *= v1Raster.z;
        st2 *= v2Raster.z;

		float xmin = std::min(std::min(v0Raster.x, v1Raster.x), v2Raster.x);
		float ymin = std::min(std::min(v0Raster.y, v1Raster.y), v2Raster.y);
		float xmax = std::max(std::max(v0Raster.x, v1Raster.x), v2Raster.x);
		float ymax = std::max(std::max(v0Raster.y, v1Raster.y), v2Raster.y);

		// the triangle is out of screen
		if (xmin > imageWidth - 1 || xmax < 0 || ymin > imageHeight - 1 || ymax < 0) {
			continue;
		}

		// be careful xmin/xmax/ymin/ymax can be negative. Don't cast to uint32_t
		uint32_t x0 = std::max(int32_t(0), (int32_t)(std::floor(xmin)));
		uint32_t x1 = std::min(int32_t(imageWidth) - 1, (int32_t)(std::floor(xmax)));
		uint32_t y0 = std::max(int32_t(0), (int32_t)(std::floor(ymin)));
		uint32_t y1 = std::min(int32_t(imageHeight) - 1, (int32_t)(std::floor(ymax)));

		float area = edgeFunction(v0Raster, v1Raster, v2Raster);

		// [comment]
		// Inner loop
		// [/comment]
		for (uint32_t y = y0; y <= y1; ++y) {
			for (uint32_t x = x0; x <= x1; ++x) {
				Vec3 pixelSample = {x + 0.5f, y + 0.5f, 0.0f};
				float w0 = edgeFunction(v1Raster, v2Raster, pixelSample);
				float w1 = edgeFunction(v2Raster, v0Raster, pixelSample);
				float w2 = edgeFunction(v0Raster, v1Raster, pixelSample);
				if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
					w0 /= area;
					w1 /= area;
					w2 /= area;
					float oneOverZ = v0Raster.z * w0 + v1Raster.z * w1 + v2Raster.z * w2;
					float z = 1 / oneOverZ;
					// [comment]
					// Depth-buffer test
					// [/comment]
					if (z < depthBuffer[y * imageWidth + x]) {
						depthBuffer[y * imageWidth + x] = z;

						Vec2 st = st0 * w0 + st1 * w1 + st2 * w2;
						st *= z;

						// [comment]
						// If you need to compute the actual position of the shaded
						// point in camera space. Proceed like with the other vertex attribute.
						// Divide the point coordinates by the vertex z-coordinate then
						// interpolate using barycentric coordinates and finally multiply
						// by sample depth.
						// [/comment]
						Vec3 v0Cam = multVecMatrix(worldToCamera, v0);
						Vec3 v1Cam = multVecMatrix(worldToCamera, v1);
						Vec3 v2Cam = multVecMatrix(worldToCamera, v2);

						float px = (v0Cam.x/-v0Cam.z) * w0 + (v1Cam.x/-v1Cam.z) * w1 + (v2Cam.x/-v2Cam.z) * w2;
						float py = (v0Cam.y/-v0Cam.z) * w0 + (v1Cam.y/-v1Cam.z) * w1 + (v2Cam.y/-v2Cam.z) * w2;

						Vec3 pt = {px * z, py * z, -z}; // pt is in camera space

						// [comment]
						// Compute the face normal which is used for a simple facing ratio.
						// Keep in mind that we are doing all calculation in camera space.
						// Thus the view direction can be computed as the point on the object
						// in camera space minus Vec3(0), the position of the camera in camera
						// space.
						// [/comment]
						Vec3 n = crossProduct(v1Cam - v0Cam, v2Cam - v0Cam);
						n = normalize(n);
						Vec3 viewDirection = -pt;
						viewDirection = normalize(viewDirection);

						float nDotView = std::max(0.f, dotProduct(n, viewDirection));

						// [comment]
						// The final color is the result of the facing ratio multiplied by the
						// checkerboard pattern.
						// [/comment]
						const int M = 10;
						float checker = (f32)((fmod(st.x * M, 1.0f) > 0.5f) ^ (fmod(st.y * M, 1.0f) < 0.5f));
						float c = 0.3f * (1.0f - checker) + 0.7f * checker;
						nDotView *= c;
						frameBuffer[y * imageWidth + x].x = (unsigned char)(nDotView * 255);
						frameBuffer[y * imageWidth + x].y = (unsigned char)(nDotView * 255);
						frameBuffer[y * imageWidth + x].z = (unsigned char)(nDotView * 255);
					}
				}
			}
		}
	}

    u8 *row = (u8 *)globalBitmapMemory;
    int pitch = imageWidth * bytesPerPixel;
    for (u32 y = 0; y < imageHeight; ++y)
    {
        u8 *pixel = row;
        for (u32 x = 0; x < imageWidth; ++x)
        {
            Vec3u v = frameBuffer[y * imageWidth + x];

            *pixel = (u8)v.z;
            ++pixel;
            *pixel = (u8)v.y;
            ++pixel;
            *pixel = (u8)v.x;
            ++pixel;
            *pixel = 0;
            ++pixel;
        }
        row += pitch;
    }

	delete [] frameBuffer;
	delete [] depthBuffer;

	return 0;
}
/* SCRATCH A PIXEL */

internal void win32ResizeDIBSection(int width, int height)
{
    if (globalBitmapMemory)
    {
        VirtualFree(globalBitmapMemory, 0, MEM_RELEASE);
    }

    globalBitmapWidth = width;
    globalBitmapHeight = height;

    globalBitmapInfo.bmiHeader.biSize = sizeof(globalBitmapInfo.bmiHeader);
    globalBitmapInfo.bmiHeader.biWidth = globalBitmapWidth;
    // NOTE(cjh): Negative height is a top-down bitmap
    globalBitmapInfo.bmiHeader.biHeight = -globalBitmapHeight;
    globalBitmapInfo.bmiHeader.biPlanes = 1;
    globalBitmapInfo.bmiHeader.biBitCount = 32;
    globalBitmapInfo.bmiHeader.biCompression = BI_RGB;

    int bytesPerPixel = 4;
    int bitmapMemorySize = globalBitmapWidth * globalBitmapHeight * bytesPerPixel;
    globalBitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void win32UpdateWindow(HDC deviceContext, RECT *windowRect, int x, int y, int width, int height)
{
    (void)x;
    (void)y;
    (void)width;
    (void)height;

    int windowWidth = windowRect->right - windowRect->left;
    int windowHeight = windowRect->bottom - windowRect->top;

    StretchDIBits(deviceContext,
                  /* x, y, width, height, */
                  /* x, y, width, height, */
                  0, 0, globalBitmapWidth, globalBitmapHeight,
                  0, 0, windowWidth, windowHeight, globalBitmapMemory,
                  &globalBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK win32WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
        case WM_SIZE:
        {
            RECT clientRect;
            GetClientRect(windowHandle, &clientRect);

            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            win32ResizeDIBSection(width, height);
            break;
        }
        case WM_DESTROY:
            globalRunning = false;
            break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(windowHandle, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;

            RECT clientRect;
            GetClientRect(windowHandle, &clientRect);
            win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
            EndPaint(windowHandle, &paint);
            break;
        }
        default:
            result = DefWindowProc(windowHandle, message, wParam, lParam);
            break;
    }

    return result;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow)
{
    (void)prevInstance;
    (void)cmdLine;
    (void)cmdShow;

    LARGE_INTEGER countsPerSecond;
    QueryPerformanceFrequency(&countsPerSecond);

    WNDCLASS windowClass = {0};
    windowClass.lpfnWndProc = win32WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "RendererClass";

    RegisterClass(&windowClass);

    HWND windowHandle = CreateWindowEx(0, windowClass.lpszClassName, "Renderer", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                       0, 0, instance, 0);
    if (windowHandle)
    {
        LARGE_INTEGER elapsedMicroseconds = {0};
        globalRunning = 1;
        while (globalRunning)
        {
            LARGE_INTEGER startTime;
            QueryPerformanceCounter(&startTime);

            MSG message = {};
            BOOL messageResult = GetMessageA(&message, 0, 0, 0);
            if (messageResult > 0)
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                break;
            }

            renderCow(4, globalBitmapWidth, globalBitmapHeight);

            HDC deviceContext = GetDC(windowHandle);
            RECT clientRect;
            GetClientRect(windowHandle, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            win32UpdateWindow(deviceContext, &clientRect, 0, 0, width, height);
            ReleaseDC(windowHandle, deviceContext);

            LARGE_INTEGER endTime;
            QueryPerformanceCounter(&endTime);

            elapsedMicroseconds.QuadPart += (endTime.QuadPart - startTime.QuadPart) * 1000000 /
                countsPerSecond.QuadPart / 100;

            if (elapsedMicroseconds.QuadPart > 16)
            {
                elapsedMicroseconds.QuadPart = 0;
            }
        }
    }
    else
    {
        // TODO(cjh): logging
    }

    return 0;
}
