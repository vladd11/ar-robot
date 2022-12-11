#ifndef AR_SHOP_ARCODE_CXX_API_HPP
#define AR_SHOP_ARCODE_CXX_API_HPP

#include "arcore_c_api.h"

namespace Ar {
  class Pose {
  public:
    ArPose *impl;

    inline Pose(const ArSession *session,
                const float *pose_raw) {
      ArPose_create(session, pose_raw, &impl);
    }

    inline ~Pose() {
      ArPose_destroy(impl);
    }

    inline void getPoseRaw(const ArSession *session,
                           float *out_pose_raw_7) {
      ArPose_getPoseRaw(session, impl, out_pose_raw_7);
    }

    inline void getMatrix(const ArSession *session,
                          float *out_matrix_col_major_4x4) {
      ArPose_getMatrix(session, impl, out_matrix_col_major_4x4);
    }

    ArPose *operator*() const {
      return impl;
    }
  };
}

#endif