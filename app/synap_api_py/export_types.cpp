#include "synap/types.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <sstream>

namespace py = pybind11;

using namespace std;
using namespace synaptics::synap;

static void export_types(py::module_& m)
{
    auto types = m.def_submodule("types", "SyNAP types");

    /* Dim2D */
    py::class_<Dim2d>(types, "Dim2d")
    .def_readwrite("x", &Dim2d::x)
    .def_readwrite("y", &Dim2d::y)
    .def(
        "__repr__",
        [](const Dim2d &dim) {
            std::ostringstream oss;
            oss << "Dim2d(x=" << dim.x << ", y=" << dim.y << ")";
            return oss.str();
        }
    )
    ;

    /* DataType */
    py::enum_<DataType>(types, "DataType")
    .value("invalid", DataType::invalid)
    .value("byte", DataType::byte)
    .value("int8", DataType::int8)
    .value("uint8", DataType::uint8)
    .value("int16", DataType::int16)
    .value("uint16", DataType::uint16)
    .value("int32", DataType::int32)
    .value("uint32", DataType::uint32)
    .value("float16", DataType::float16)
    .value("float32", DataType::float32)
    .def(
        "np_type",
        [](const DataType& dtype) {
            switch(dtype) {
                case DataType::byte:    return py::dtype::of<char>();
                case DataType::int8:    return py::dtype::of<int8_t>();
                case DataType::uint8:   return py::dtype::of<uint8_t>();
                case DataType::int16:   return py::dtype::of<int16_t>();
                case DataType::uint16:  return py::dtype::of<uint16_t>();
                case DataType::int32:   return py::dtype::of<int32_t>();
                case DataType::uint32:  return py::dtype::of<uint32_t>();
                case DataType::float16: return py::dtype("float16");
                case DataType::float32: return py::dtype::of<float>();
                default: throw std::invalid_argument("Invalid DataType");
            }
        },
        "Get corresponding NumPy dtype"
    )
    ;

    /* Layout */
    py::enum_<Layout>(types, "Layout")
    .value("none", Layout::none)
    .value("nchw", Layout::nchw)
    .value("nhwc", Layout::nhwc)
    ;

    /* Rect */
    py::class_<Rect>(types, "Rect")
    .def(py::init<>())
    .def("empty", &Rect::empty, "Check if Rect is empty")
    .def_readwrite("origin", &Rect::origin)
    .def_readwrite("size", &Rect::size)
    .def(
        "__repr__",
        [](const Rect &rect) {
            std::ostringstream oss;
            oss << "Rect(origin=(" << rect.origin.x << ", " << rect.origin.y << "), size=(" << rect.size.x << ", " << rect.size.y << "))";
            return oss.str();
        }
    )
    ;

    /* Shape */
    py::class_<Shape>(types, "Shape")
    .def(
        py::init([](py::iterable shape){
            std::vector<int32_t> shape_vec;
            for (auto s: shape) {
                shape_vec.push_back(s.cast<int32_t>());
            }
            return Shape(shape_vec.begin(), shape_vec.end());
        }),
        py::arg("shape")
    )
    .def(
        "__getitem__",
        [](const Shape& self, size_t index) -> int32_t {
            if (index >= self.size()) {
                throw std::out_of_range("Shape index out of range");
            }
            return self[index];
        }
    )
    .def(
        "__iter__",
        [](const Shape& self) -> py::iterator {
            return py::make_iterator(self.begin(), self.end());
        }
    )
    .def(
        "__repr__",
        [](const Shape& self) {
            std::ostringstream oss;
            oss << "Shape(";
            py::tuple result(self.size());
            for (size_t i = 0; i < self.size(); ++i) {
                oss << self[i];
                if (i != self.size() - 1) {
                    oss << ", ";
                }
            }
            oss << ")";
            return oss.str();
        }
    )
    .def("item_count", &Shape::item_count, "Number of elements in a tensor with this shape")
    .def("valid", &Shape::valid, "If shape is valid or not")
    ;
}