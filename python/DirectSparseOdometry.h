#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <FullSystem/FullSystem.h>

class OutputCapture;

// forward declaration
namespace dso {
  class FullSystem;

  namespace IOWrap {
    class PangolinDSOViewer;
  }
} // namespace dso

class DirectSparseOdometry {

public:
  DirectSparseOdometry(bool gui);
  ~DirectSparseOdometry();

  void addActiveFrame(pybind11::array frame, int idx);
  void waitForMapping() const;

  bool initFailed() const { return m_fullSystem->initFailed; }

  bool hasGamma() const { return static_cast<bool>(m_gamma); }

  bool isLost() const { return m_fullSystem->isLost; }
  void run() const;

  void setGammaFunction(pybind11::array gamma);
  void clearGammaFunction();

  void setCalibration(pybind11::array K, int w, int h);

  pybind11::array trajectories() const;

private:
  dso::FullSystem *m_fullSystem;
  float *m_gamma = nullptr;
  dso::IOWrap::PangolinDSOViewer *m_viewer;
  OutputCapture *m_capture;

};
