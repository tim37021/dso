#include "DirectSparseOdometry.h"
#include "FullSystem/FullSystem.h"
#include "log.h"
#include "util/ImageAndExposure.h"
#include <Eigen/Dense>

#include "IOWrapper/Output3DWrapper.h"
#ifdef HAS_PANGOLIN
#include "IOWrapper/Pangolin/PangolinDSOViewer.h"
#endif

namespace py = pybind11;

DirectSparseOdometry::DirectSparseOdometry(bool gui)
  : m_viewer(nullptr)
{
  m_fullSystem = new dso::FullSystem();
  m_fullSystem->linearizeOperation = true;

  if(gui) {
#ifdef HAS_PANGOLIN
    m_viewer =
        new dso::IOWrap::PangolinDSOViewer(dso::wG[0], dso::hG[0], false);
    m_fullSystem->outputWrapper.push_back(m_viewer);
#else
    LOG("This distro does not has built-in GUI");
#endif

  }
}

DirectSparseOdometry::~DirectSparseOdometry() {
  for(auto wrapper: m_fullSystem->outputWrapper) {
    wrapper->join();
    delete wrapper;
  }
  delete m_fullSystem;
}

void DirectSparseOdometry::addActiveFrame(py::array frame, int idx) {
  auto frame_type = frame.dtype();

  uint32_t w, h;

  if(frame.ndim() == 2 || (frame.ndim() == 3 && frame.shape()[2] == 1)) {
    w = frame.shape()[1];
    h = frame.shape()[0];
  } else {
    LOG_ERROR("Dimension error! (Must be (H, W) or (H, W, 1))");
    return;
  }

  if(frame_type.is(py::dtype::of<float>())) {
    dso::ImageAndExposure *image = new dso::ImageAndExposure(w, h);
    // Assume we don't have image infomation
    fprintf(stderr, "dim = %d %d", w, h);
    image->exposure_time = 1.f;
    image->timestamp = 0.0;
    image->image = new float[w * h];
    memcpy(image->image, frame.request().ptr, w*h*sizeof(float));
    m_fullSystem->addActiveFrame(image, idx);
    delete image;
  } else {
    LOG_ERROR("Type error, must be float32");
  }
}

void DirectSparseOdometry::waitForMapping() const {
  m_fullSystem->blockUntilMappingIsFinished();
}

void DirectSparseOdometry::setGammaFunction(py::array gamma) {

  // TODO
  auto info = gamma.request();

  delete [] m_gamma;
  m_gamma = new float[256];
  if (gamma.dtype().is(py::dtype::of<float>())) {
    float *g = reinterpret_cast<float *>(info.ptr);
    std::copy(g, g+256, m_gamma);
  } else if (gamma.dtype().is(py::dtype::of<double>())) {
    double *g = reinterpret_cast<double *>(info.ptr);
    std::copy(g, g+256, m_gamma);
  } else if (gamma.dtype().is(py::dtype::of<int>())) {
    int *g = reinterpret_cast<int *>(info.ptr);
    std::copy(g, g+256, m_gamma);
  } else {
    LOG_ERROR("Unsupported numpy array type")
  }

  m_fullSystem->setGammaFunction(m_gamma);
}

void DirectSparseOdometry::clearGammaFunction() {
  // TODO
  // remember to fix setGammaFunction
  LOG_ERROR("FullSystem doesn't support reseting gamma function yet");
}

void DirectSparseOdometry::setCalibration(py::array K, int w, int h) {
  if(K.dtype().is(py::dtype::of<float>())) {
    if(K.ndim() == 2 && K.shape()[0] == 3 && K.shape()[1] == 3) {
      auto info = K.request();
      Eigen::Matrix3f eK = Eigen::Map<Eigen::Matrix<float, 3, 3, Eigen::RowMajor> >((float *)info.ptr);
      //memcpy(&eK, info.ptr, sizeof(eK));
      std::cout<<eK;
      dso::setGlobalCalib(w, h, eK);
      LOG("Calibration matrix changed");
    } else {
      LOG_ERROR("Dimension error");
    }
  } else {
    LOG_ERROR("Types of K matrix are unsupported yet")
  }
}

void DirectSparseOdometry::run() const {
#ifdef HAS_PANGOLIN
    m_viewer->run();
#endif
}
