#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <cstdio>
#include "log.h"
#include "DirectSparseOdometry.h"
#include "util/MinimalImage.h"
#include "util/Undistort.h"


namespace py = pybind11;

py::array build_numpy2d(float *arr, int w, int h) {

  py::capsule free_when_done(arr, [](void *f) {
    fprintf(stderr, "%p", f);
    delete[](float *) f;
  });

  return py::array_t<float>({h, w},     // shape
                            {w * 4, 4}, // C-style contiguous strides for double
                            arr,        // the data pointer
                            free_when_done);
}

static void setCalibration(py::array K, int w, int h) {
  if (K.dtype().is(py::dtype::of<float>())) {
    if (K.ndim() == 2 && K.shape()[0] == 3 && K.shape()[1] == 3) {
      auto info = K.request();
      Eigen::Matrix3f eK =
          Eigen::Map<Eigen::Matrix<float, 3, 3, Eigen::RowMajor>>(
              (float *)info.ptr);
      // memcpy(&eK, info.ptr, sizeof(eK));
      std::cout << eK;
      dso::setGlobalCalib(w, h, eK);
      LOG("Calibration matrix changed");
    } else {
      LOG_ERROR("Dimension error");
    }
  } else {
    LOG_ERROR("Types of K matrix are unsupported yet")
  }
}

PYBIND11_MODULE(dso_python, m) {

  dso::setting_photometricCalibration = 0;
  dso::setting_affineOptModeA =
      -1; //-1: fix. >=0: optimize (with prior, if > 0).
  dso::setting_affineOptModeB =
      -1; //-1: fix. >=0: optimize (with prior, if > 0).
  dso::setting_minGradHistAdd = 3;

  dso::benchmarkSetting_width = 424;
  dso::benchmarkSetting_height = 320;

  dso::setting_desiredImmatureDensity = 150;
  dso::setting_desiredPointDensity = 200;
  dso::setting_minFrames = 4;
  dso::setting_maxFrames = 6;
  dso::setting_maxOptIterations = 4;
  dso::setting_minOptIterations = 1;
  dso::disableAllDisplay = true;

  m.doc() = "pybind11 example plugin"; // optional module docstring

  m.def("setCalibration", &setCalibration, "Set calib globally");

  py::class_<DirectSparseOdometry>(m, "DirectSparseOdometry")
      .def(py::init<bool>())
      .def("addActiveFrame", &DirectSparseOdometry::addActiveFrame)
      .def("setGammaFunction", &DirectSparseOdometry::setGammaFunction)
      .def("clearGammaFunction", &DirectSparseOdometry::clearGammaFunction)
    //.def("setCalibration", &DirectSparseOdometry::setCalibration)
      .def("waitForMapping", &DirectSparseOdometry::waitForMapping)
      .def("run", [](DirectSparseOdometry &instance) {
		      py::gil_scoped_release release;
		      instance.run();
		      //py::gil_scoped_acquire acquire;
      })
      .def_property_readonly("initFailed", &DirectSparseOdometry::initFailed)
      .def_property_readonly("hasGamma", &DirectSparseOdometry::hasGamma)
      .def_property_readonly("isLost", &DirectSparseOdometry::isLost);

  py::class_<dso::Undistort>(m, "Undistort")
    .def("getUndistorterForFile", &dso::Undistort::getUndistorterForFile)
    .def_property_readonly("K", [](dso::Undistort &instance) {
      auto k = instance.getK();
      // Create a Python object that will free the allocated
      // memory when destroyed:
      auto foo = new float [9];
      for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
          foo[i*3+j] = static_cast<float>(k(i, j));
      return build_numpy2d(foo, 3, 3);
    })
    .def("undistort", [] (dso::Undistort &instance, py::array frame) {
      if(frame.dtype().is(py::dtype::of<uint8_t>())) {
        LOG("Yo");
        // TODO check dimensions
        dso::MinimalImageB img(frame.shape()[1], frame.shape()[0], (unsigned char *)frame.request().ptr);
        // TODO move undistort to header file
        auto ret = instance.undistort<unsigned char>(&img, 1.0f, 0.0f);
        return build_numpy2d(ret->image, ret->w, ret->h);
      }
      return py::array();
    })
    .def_property_readonly("width", [] (dso::Undistort &instance) {
      return instance.getSize()[0];
    })
    .def_property_readonly("height", [](dso::Undistort &instance) {
      return instance.getSize()[1];
    });

}
