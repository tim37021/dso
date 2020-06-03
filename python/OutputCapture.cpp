#include "OutputCapture.h"
#include "util/FrameShell.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

void OutputCapture::publishCamPose(dso::FrameShell *frame,
                                     dso::CalibHessian *HCalib)
{
  if(frame->poseValid) {
    dso::Vec3f pos = frame->camToWorld.translation().cast<float>();
    m_traj.push_back(pos);
  }
}

py::array OutputCapture::trajectories() const {
  auto arr = py::array_t<float>(
                                {(int)m_traj.size(), 3},
                                {3 * sizeof(float), sizeof(float)},
                                reinterpret_cast<const float *>(m_traj.data())
                                );

  return arr;
}
