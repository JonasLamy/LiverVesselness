#include "pyRORPO.hpp"

#include "bind_RORPO.hpp"
#include "bind_RORPO_multiscale.hpp"
#include "bind_RPO.hpp"

using namespace pyRORPO;

#define BINDINGS_OF_TYPE(type) \
    RORPO_MULTISCALE_BINDING(type); \
    RORPO_BINDING(type); \
    RPO_BINDING(type); \

PYBIND11_MODULE(pyRORPO, m) {
    m.doc() = "pybind11 RORPO plugin"; // optional module docstring

    BINDINGS_OF_TYPE(int8_t);
    BINDINGS_OF_TYPE(int16_t);
    BINDINGS_OF_TYPE(int32_t);
    BINDINGS_OF_TYPE(int64_t);

    BINDINGS_OF_TYPE(uint8_t);
    BINDINGS_OF_TYPE(uint16_t);
    BINDINGS_OF_TYPE(uint32_t);
    BINDINGS_OF_TYPE(uint64_t);

    BINDINGS_OF_TYPE(float);
    BINDINGS_OF_TYPE(double );
    BINDINGS_OF_TYPE(long double);
}