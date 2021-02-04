/**
 * @file graphics.cpp
 *
 * 画像描画関連のプログラムを集めたファイル．
 */

#include <array>
#include "graphics.hpp"

void RGBResv8BitPerColorPixelWriter::Write(Vector2D<int> pos, const PixelColor& c) {
  auto p = PixelAt(pos);
  p[0] = c.r;
  p[1] = c.g;
  p[2] = c.b;
}

void BGRResv8BitPerColorPixelWriter::Write(Vector2D<int> pos, const PixelColor& c) {
  auto p = PixelAt(pos);
  p[0] = c.b;
  p[1] = c.g;
  p[2] = c.r;
}

void DrawRectangle(PixelWriter& writer, const Vector2D<int>& pos,
                   const Vector2D<int>& size, const PixelColor& c) {
  for (int dx = 0; dx < size.x; ++dx) {
    writer.Write(pos + Vector2D<int>{dx, 0}, c);
    writer.Write(pos + Vector2D<int>{dx, size.y - 1}, c);
  }
  for (int dy = 1; dy < size.y - 1; ++dy) {
    writer.Write(pos + Vector2D<int>{0, dy}, c);
    writer.Write(pos + Vector2D<int>{size.x - 1, dy}, c);
  }
}

void FillRectangle(PixelWriter& writer, const Vector2D<int>& pos,
                   const Vector2D<int>& size, const PixelColor& c) {
  for (int dy = 0; dy < size.y; ++dy) {
    for (int dx = 0; dx < size.x; ++dx) {
      writer.Write(pos + Vector2D<int>{dx, dy}, c);
    }
  }
}

DesktopBgImage* desktop_bg_image;

void DrawCustomBackground(PixelWriter& writer) {
  const auto width = writer.Width();
  const auto height = writer.Height();
  const auto inner_width = width;
  const auto inner_height = height - 50;
  if(desktop_bg_image == nullptr) {
    FillRectangle(writer,
        {0, 0},
        {inner_width, inner_height},
        kDesktopBGColor);
  } else {
    for(int y = 0; y < inner_height; ++y) {
      for(int x = 0; x < inner_width; ++x) {
        PixelColor& c = desktop_bg_image->sample(x, y, inner_width, inner_height);
        writer.Write(Vector2D<int>{x, y}, c);
      }
    }
  }
}

void DrawDesktop(PixelWriter& writer) {
  const auto width = writer.Width();
  const auto height = writer.Height();
  DrawCustomBackground(writer);
  FillRectangle(writer,
                {0, height - 50},
                {width, 50},
                {1, 8, 17});
  FillRectangle(writer,
                {0, height - 50},
                {width / 5, 50},
                {80, 80, 80});
  DrawRectangle(writer,
                {10, height - 40},
                {30, 30},
                {160, 160, 160});
}

FrameBufferConfig screen_config;
PixelWriter* screen_writer;

Vector2D<int> ScreenSize() {
  return {
    static_cast<int>(screen_config.horizontal_resolution),
    static_cast<int>(screen_config.vertical_resolution)
  };
}

namespace {
  char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
  char bg_image_buf[sizeof(DesktopBgImage)];
}

void InitializeGraphics(const FrameBufferConfig& screen_config) {
  ::screen_config = screen_config;

  desktop_bg_image = nullptr;

  switch (screen_config.pixel_format) {
    case kPixelRGBResv8BitPerColor:
      ::screen_writer = new(pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{screen_config};
      break;
    case kPixelBGRResv8BitPerColor:
      ::screen_writer = new(pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{screen_config};
      break;
    default:
      exit(1);
  }

  DrawDesktop(*screen_writer);
}

bool InitializeDesktopBgImage(PixelWriter* writer, int width, int height, const char* data) {
    if(width > DesktopBgImage::MAX_BACKGROUND_WIDTH || height > DesktopBgImage::MAX_BACKGROUND_HEIGHT) {
        return false;
    }

    ::desktop_bg_image = new(bg_image_buf) DesktopBgImage(width, height, data);
    DrawDesktop(*writer);
    return true;
}

bool FinalizeDesktopBgImage(PixelWriter* writer) {
    if(desktop_bg_image == nullptr) {
        return false;
    }

    desktop_bg_image->~DesktopBgImage();
    desktop_bg_image = nullptr;

    DrawDesktop(*writer);
    return true;
}
