# Python bindings with Pybind11

- [Pybind11 Repo](https://github.com/pybind/pybind11)
- [Pybind11 Documentation](https://pybind11.readthedocs.io/en/stable/)

All resources required to generete the python module are inside the directory `/pyRORPO`.

## Usage

Building target `pyRORPO` will generate a dynamic library. This library can be imported as a python module :

``` python
>>> import pyRORPO
>>> print(pyRORPO.__doc__)
FIXME: insert real documentation
```

## Guide

PyRORPO documentation: https://rorpo.readthedocs.io/en/latest/

All bindings are defined using the following c++ files :

- `pyRORPO/pyRORPO.cpp` : The python module is defined inside `pyRORPO/pyRORPO.cpp`.

    To add methods to the module from templated functions, pybind11 requires to specify the type. To cover all kind of input types two levels of macros are used:
    - `BINDING_<Method Name>(type)` : Creates the python binding for a given method. It is defined with a specific type.
    - `BINDINGS_OF_TYPE(type)` : For a given type, adds all bindings using that type.

- `pyRORPO/bind_*.cpp` : Defines the c++ functions that are binded using `BINDING_<Method Name>(type)` macros as well as the macros themselves.

- `pyRORPO/bindingUtils.hpp` : Usefull functions to create bindings (e.g. processing of pybind11 types).
