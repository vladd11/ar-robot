#ifndef AR_SHOP_ARCODE_CXX_API_HPP
#define AR_SHOP_ARCODE_CXX_API_HPP

#include "arcore_c_api.h"

namespace Ar {
  class Pose {
  private:
    const ArSession *mSession;
    ArPose *impl;

  public:
    inline Pose(const ArSession *session,
                const float *pose_raw) {
      ArPose_create(session, pose_raw, &impl);
      mSession = session;
    }

    inline ~Pose() {
      ArPose_destroy(impl);
    }

    inline void getPoseRaw(float *out_pose_raw_7) {
      ArPose_getPoseRaw(mSession, impl, out_pose_raw_7);
    }

    inline void getMatrix(float *out_matrix_col_major_4x4) {
      ArPose_getMatrix(mSession, impl, out_matrix_col_major_4x4);
    }

    inline ArPose *operator*() const {
      return impl;
    }
  };
}

#endif