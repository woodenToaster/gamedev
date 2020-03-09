#include "math.h"
#include <algorithm>
#include <chrono>

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include "model.cpp"
#include "tgaimage.cpp"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

void swap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void line(int x0, int y0, int x1, int y1, TGAImage &image, const TGAColor &color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = (int)(y0*(1.-t) + y1*t);
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

void line(Vec2i v0, Vec2i v1, TGAImage &image, const TGAColor &color)
{
    line(v0.x, v0.y, v1.x, v1.y, image, color);
}

void triangle_outline(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color) {
    line(v0, v1, image, color);
    line(v1, v2, image, color);
    line(v2, v0, image, color);
}

void filled_triangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, const TGAColor &color)
{
    int height = image.get_height();
    int width = image.get_width();

    Vec2i sorted[3] = {v0, v1, v2};

    for (int i = 0; i < 3; ++i)
    {
        for (int j = i + 1; j < 3; ++j)
        {

            if (sorted[j].y < sorted[i].y)
            {
                Vec2i tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }

    // draw bounding box
    // int xMin = std::min(std::min(v0.x, v1.x), v2.x);
    // int xMax = std::max(std::max(v0.x, v1.x), v2.x);
    // int yMin = std::min(std::min(v0.y, v1.y), v2.y);
    // int yMax = std::max(std::max(v0.y, v1.y), v2.y);
    // line(xMin, yMin, xMax, yMin, image, color);
    // line(xMin, yMax, xMax, yMax, image, color);
    // line(xMin, yMin, xMin, yMax, image, color);
    // line(xMax, yMin, xMax, yMax, image, color);
    
    // line sorted[0] to sorted[1]
    Vec2i first = sorted[0];
    Vec2i second = sorted[1];
    Vec2i third = sorted[2];
    
    f32 slope1 = (second.y - first.y) / (f32)(second.x - first.x);
    f32 b1 = first.y - (slope1 * first.x);
    f32 slope2 = (third.y - second.y) / (f32)(third.x - second.x);
    f32 b2 = second.y - (slope2 * second.x);
    f32 slope3 = (first.y - third.y) / (f32)(first.x - third.x);
    f32 b3 = third.y - (slope3 * third.x);

    for (int y = yMin; y <= yMax; ++y)
    {
        for (int x = xMin; x <= xMax; ++x)
        {
            int result1 = slope1 * x + b1;
            int result2 = slope2 * x + b2;
            int result3 = slope3 * x + b3;
            
            if ()
            {
                image.set(x, y, color);
            }
        }
    }
}

#if 0
void line(int x0, int y0, int x1, int y1, TGAImage &image, const TGAColor &color)
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
#endif

int main()
{
    int width = 200;
    int height = 200;

    // Model model("obj\\african_head.obj");
    TGAImage image(width, height, TGAImage::RGB);

#if 0
    for (int i = 0; i < model.nfaces(); ++i)
    {
        std::vector<int> face = model.face(i);

        for (int j = 0; j < face.size(); ++j)
        {
            Vec3f v0 = model.vert(face[j]);
            Vec3f v1 = model.vert(face[(j + 1) % face.size()]);
            int x0 = (int)(((v0.x + 1.0f) / 2.0f) * width);
            int y0 = (int)(((v0.y + 1.0f) / 2.0f) * height);
            int x1 = (int)(((v1.x + 1.0f) / 2.0f) * width);
            int y1 = (int)(((v1.y + 1.0f) / 2.0f) * height);
            line(x0, y0, x1, y1, image, white);
        }
    }
#endif

    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    triangle_outline(t0[0], t0[1], t0[2], image, red);
    triangle_outline(t1[0], t1[1], t1[2], image, white);
    triangle_outline(t2[0], t2[1], t2[2], image, green);

    filled_triangle(t0[0], t0[1], t0[2], image, red);
    filled_triangle(t1[0], t1[1], t1[2], image, white);
    filled_triangle(t2[0], t2[1], t2[2], image, green);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("build\\output.tga");

    return 0;
}