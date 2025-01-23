#include <pybind11/pybind11.h>

#include "export_preprocessor.cpp"
#include "export_postprocessor.cpp"
#include "export_tensor.cpp"
#include "export_types.cpp"

namespace py = pybind11;

using namespace synaptics::synap;

PYBIND11_MODULE(synap, m)
{
    m.doc() = "SyNAP Python API";

    export_preprocessor(m);
    export_postprocessor(m);
    export_tensors(m);
    export_types(m);
}
