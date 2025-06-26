#pragma once
#include "PotConv.h"
#include "ft2build.h"

#include "freetype/freetype.h"
#include "opencv2/opencv.hpp"
#include <string>

class DrawStringFT
{
private:
    FT_Library library{ nullptr };
    FT_Face face{ nullptr };
    FT_GlyphSlot slot{ nullptr };
    //FT_Matrix matrix{nullptr};
    FT_Vector pen{ 0, 0 };
    int fontsize_ = 0;

public:
    DrawStringFT()
    {
        FT_Init_FreeType(&library);
    }

    ~DrawStringFT()
    {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    void openFont(const std::string& fontname, int fontsize)
    {
        if (FT_New_Face(library, fontname.c_str(), 0, &face))
        {
            fprintf(stderr, "Cannot open font file: %s \n", fontname.c_str());
        }
        FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        FT_Set_Pixel_Sizes(face, fontsize, 0);
        slot = face->glyph;
        fontsize_ = fontsize;
    }

    //获取一个单通道，黑色背景的字符串图像，其中像素值表示透明度，用于下一步
    cv::Mat getStringMat(const std::string& text)
    {
        pen.x = 0;
        pen.y = 0;
        int width = 0;
        auto text1 = PotConv::conv(text, "utf-8", "utf-16le");

        cv::Mat temp = cv::Mat::zeros(fontsize_ * 2, fontsize_ * text1.size(), CV_8UC1);    //上下留出余量
        int y0 = fontsize_ / 2;
        for (int n = 0; n < text1.size(); n += 2)
        {
            FT_Set_Transform(face, nullptr, &pen);
            wchar_t v = *(wchar_t*)&text1[n];
            FT_Load_Char(face, v, FT_LOAD_RENDER);
            draw_bitmap(&slot->bitmap, slot->bitmap_left, fontsize_ * 1 - slot->bitmap_top + fontsize_ / 2, temp);
            pen.x += slot->advance.x;
            pen.y += slot->advance.y;
            width = slot->bitmap.width + slot->bitmap_left;
        }

        cv::Rect nonzero = cv::boundingRect(temp);
        temp = temp(nonzero).clone();
        return temp;
    }

    //在mat上绘制字符串
    //x, y 左上角坐标
    //color 颜色
    //fusion 融合方式，0为覆盖，1为融合
    //back 背景色，0为不使用背景色，255为纯黑
    //注意：如果mat是单通道图像，则color只使用第一个通道的值
    void drawString(const std::string& text, cv::Mat& mat, int x, int y, cv::Vec3b color, int fusion = 0, unsigned char back = 0)
    {
        auto temp = getStringMat(text);
        float gray = color[0] / 255.0;
        for (int i = -2; i < temp.cols + 2; i++)
        {
            for (int j = -2; j < temp.rows + 2; j++)
            {
                int x_mat = i + x;
                int y_mat = j + y;
                uint8_t alpha = 0;
                if (i < 0 || j < 0 || i >= temp.cols || j >= temp.rows)
                {
                    alpha = 0;
                }
                else
                {
                    alpha = temp.at<uint8_t>(j, i);
                }
                if (x_mat < 0 || y_mat < 0 || x_mat >= mat.cols || y_mat >= mat.rows)
                {
                    continue;
                }
                if (mat.channels() == 1)
                {
                    auto& a = mat.at<uint8_t>(y_mat, x_mat);
                    if (back)
                    {
                        a = a * (1 - back / 255.0);
                    }
                    if (fusion == 0)
                    {
                        a = alpha * gray;
                    }
                    else if (fusion == 1)
                    {
                        a = (255 - alpha) * a / 255 + alpha * gray;
                    }
                }
                else if (mat.channels() == 3)
                {
                    auto& a = mat.at<cv::Vec3b>(y_mat, x_mat);
                    if (back)
                    {
                        a = a * (1 - back / 255.0);
                    }
                    for (int c = 0; c < mat.channels(); c++)
                    {
                        uint8_t v = alpha * color[c] / 255;

                        if (fusion == 0)
                        {
                            a[c] = v;
                        }
                        else if (fusion == 1)
                        {
                            a[c] = (255 - alpha) * a[c] / 255 + v;
                        }
                    }
                }
            }
        }
    }

private:
    void draw_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, cv::Mat& image)
    {
        FT_Int x_max = x + bitmap->width;
        FT_Int y_max = y + bitmap->rows;

        image.forEach<uint8_t>([&](uint8_t& a, const int* position)
            {
                if (position[1] >= x && position[1] < x_max && position[0] >= y && position[0] < y_max)
                {
                    int p = position[1] - x;
                    int q = position[0] - y;
                    a = bitmap->buffer[q * bitmap->width + p];
                }
            });
    }
};
