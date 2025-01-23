#include <memory>
#include <vector>
#include "synap/detector.hpp"
#include "synap/tensor.hpp"
#include "synap/network.hpp"
#include "synap/types.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

using namespace std;
using namespace synaptics::synap;


PYBIND11_MAKE_OPAQUE(vector<Detector::Result::Item>);

static void export_postprocessor(py::module_& m)
{
    auto postprocessor = m.def_submodule("postprocessor", "SyNAP postprocessor");

    /* Detector */
    py::class_<Detector>(postprocessor, "Detector")
    .def(
        py::init<float, int, bool, float, bool>(),
        py::arg("score_threshold") = 0.5,
        py::arg("n_max") = 0,
        py::arg("nms") = true,
        py::arg("iou_threshold") = 0.5,
        py::arg("iou_with_min") = false
    )
    .def(
        "process",
        &Detector::process,
        py::arg("outputs"),
        py::arg("assigned_rect"),
        "Perform detection on network outputs")
    ;

    /* Detector::Result */
    py::class_<Detector::Result>(postprocessor, "DetectorResult")
    .def(py::init<>())
    .def_readonly("success", &Detector::Result::success)
    .def_readonly("items", &Detector::Result::items)
    ;

    /* Detector::Result::Item */
    py::class_<Detector::Result::Item>(postprocessor, "DetectorResultItem")
    .def(py::init<>())
    .def_readonly("class_index", &Detector::Result::Item::class_index)
    .def_readonly("confidence", &Detector::Result::Item::confidence)
    .def_readonly("bounding_box", &Detector::Result::Item::bounding_box)
    .def_readonly("landmarks", &Detector::Result::Item::landmarks)
    ;

    /* Detector::Result::Items */
    py::class_<vector<Detector::Result::Item>>(postprocessor, "DetectorResultItems")
    .def(py::init<>())
    .def("__getitem__", [](vector<Detector::Result::Item>& self, size_t index)
        {return &self[index];}, py::return_value_policy::reference, "get item by index")
    .def("__len__", [](vector<Detector::Result::Item>& self)
        {return self.size();}, "get items size")
    .def(
        "__iter__",
        [](vector<Detector::Result::Item>& self) -> py::iterator {
            return py::make_iterator(self.begin(), self.end());
        },
        "Iterate over tensors"
    )
    ;

}
