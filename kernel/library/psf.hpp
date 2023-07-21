#ifndef __LIBRARY__PSF_HPP_
#define __LIBRARY__PSF_HPP_

#include <library/types.hpp>

struct _PSF_FONT {
  u32 magic;
  u32 version;
  u32 headersize;
  u32 flags;
  u32 numglyph;
  u32 bytesperglyph;
  u32 height;
  u32 width;
} __attribute__((packed));

#endif