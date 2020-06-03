#pragma once

#include "IOWrapper/Output3DWrapper.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

class OutputCapture : public dso::IOWrap::Output3DWrapper {
public:
 
  virtual void publishCamPose(dso::FrameShell *frame, dso::CalibHessian *HCalib) override;

  pybind11::array trajectories() const;
private:
  std::vector<dso::Vec3f> m_traj;
};
