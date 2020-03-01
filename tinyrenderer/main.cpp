#include "math.h"
#include <chrono>

#include "tgaimage.h"
#include "tgaimage.cpp"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void swap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    if (dy > dx)
    {
        swap(&x0, &y0);
        swap(&x1, &y1);
        steep = true;
    }

    if (x0 > x1)
    {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    f32 derror = abs(dy / (f32)dx);
    f32 error = 0;
    int yinc = y1 > y0 ? 1 : -1;
    int y = y0;
    for (int x = x0; x < x1; ++x)
    {
        if (steep)
        {
            image.set(y, x, color);
        }
        else
        {
            image.set(x, y, color);
        }
        error += derror;

        if (error > 0.5f)
        {
            y += yinc;
            error -= 1.0f;

        }

    }
}

int main()
{
    int width = 100;
    int height = 100;

    TGAImage image(width, height, TGAImage::RGB);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i)
    {
        // line(0, 0, width - 1, height -1, image, white);
        // line(0, height - 1, width - 1, 0, image, red);
        line(13, 20, 80, 40, image, white);
        line(20, 13, 40, 80, image, red);
        line(80, 40, 13, 20, image, red);

    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << ms << " ms" << "\n";

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("build\\output.tga");

    return 0;
}

