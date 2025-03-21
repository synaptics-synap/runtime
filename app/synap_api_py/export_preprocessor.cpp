#include <string>
#include "synap/input_data.hpp"
#include "synap/preprocessor.hpp"
#include "synap/tensor.hpp"
#include "synap/types.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

using namespace std;
using namespace synaptics::synap;

class PreprocessorWrapper {
public:
    PreprocessorWrapper(const PreprocessorWrapper&) = delete;
    PreprocessorWrapper(PreprocessorWrapper&&) = delete;
    PreprocessorWrapper& operator=(const PreprocessorWrapper&) = delete;
    PreprocessorWrapper& operator=(PreprocessorWrapper&&) = delete;

    PreprocessorWrapper()
    :
    _preproc()
    {}

    Rect assign(Tensors& inputs, const InputData& input_data, size_t start_index = 0) const
    {
        Rect assigned_rect;
        if (input_data.empty()) {
            throw std::invalid_argument("Invalid input data");
        }
        if (!_preproc.assign(inputs, input_data, start_index, &assigned_rect)) {
            throw std::runtime_error("Error while preprocessing data");
        }
        return assigned_rect;
    }

    Rect assign(Tensors& inputs, const std::string& filename, size_t start_index = 0) const
    {
        InputData input_data(filename);
        if (input_data.empty()) {
            std::ostringstream err;
            err << "Invalid input image: " << filename;
            throw std::invalid_argument(err.str());
        }
        return assign(inputs, input_data, start_index);
    }

    Rect assign(Tensors& inputs, const uint8_t* buffer, size_t buffer_size, Shape shape, Layout layout, size_t start_index = 0) const
    {
        InputData input_data(buffer, buffer_size, InputType::raw, shape, layout);
        return assign(inputs, input_data, start_index);
    }

private:
    Preprocessor _preproc;
};

static void export_preprocessor(py::module_& m)
{
    auto preprocessor = m.def_submodule("preprocessor", "SyNAP preprocessor");

    /* InputData */
    py::class_<InputData>(preprocessor, "InputData")
    .def(py::init<const string &>(), "load input data from file")
    .def("empty", &InputData::empty, "check if data present or not")
    .def("data", &InputData::data, py::return_value_policy::reference, "get pointer to data")
    .def("size", &InputData::size, "get data size in bytes")
    ;

    /* Preprocessor */
    py::class_<PreprocessorWrapper>(preprocessor, "Preprocessor")
    .def(py::init<>())
    .def(
        "assign",
        static_cast<Rect (PreprocessorWrapper::*)(Tensors&, const InputData&, size_t) const>(&PreprocessorWrapper::assign),
        py::arg("inputs"),
        py::arg("input_data"),
        py::arg("input_index") = 0,
        "Write input data to network inputs"
    )
    .def(
        "assign",
        static_cast<Rect (PreprocessorWrapper::*)(Tensors&, const std::string&, size_t) const>(&PreprocessorWrapper::assign),
        py::arg("inputs"),
        py::arg("filename"),
        py::arg("input_index") = 0,
        "Write image data to network inputs"
    )
    .def(
        "assign",
        [](const PreprocessorWrapper& self, Tensors& inputs, py::array_t<uint8_t> data, Shape shape, Layout layout, size_t input_index) -> Rect {
            py::buffer_info info = data.request();
            const uint8_t* buffer = static_cast<const uint8_t*>(info.ptr);
            size_t buffer_size = info.size;
            return self.assign(inputs, buffer, buffer_size, shape, layout, input_index);
        },
        py::arg("inputs"),
        py::arg("data"),
        py::arg("shape"),
        py::arg("layout"),
        py::arg("input_index") = 0,
        "Write raw data to network inputs"
    )
    ;
}