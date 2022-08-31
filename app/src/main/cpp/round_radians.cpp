#include "round_radians.h"

float round_radians(float radians) {
  radians = radians + (float) M_PI;

  if (radians < M_PI / 6) {
    return M_PI / 6 - M_PI;
  } else if (radians < M_PI / 4) {
    return M_PI / 4 - M_PI;
  } else if(radians < M_PI / 3) {
    return M_PI / 3 - M_PI;
  } else if(radians < M_PI / 2) {
    return M_PI / 2 - M_PI;
  } else if(radians < M_PI) {
    return 0.f;
  } else if(radians < 3 * M_PI / 2) {
    return 3 * M_PI / 2 - M_PI;
  } else if(radians < 2 * M_PI - M_PI) {
    return 2 * M_PI - M_PI;
  }

  return 0.f;
}