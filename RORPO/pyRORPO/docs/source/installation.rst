************
Installation
************

Python package (to-be-available)
================================
Install the package (or add it to your
``requirements.txt`` file):

.. code:: console

    $ pip install rorpo


Via a shared library
====================
Build the project with cmake, then run the command
``make pyRORPO`` to generate ``pyRORPO.*.so`` file.

.. code:: console

	$ mkdir build
	$ cd build
	$ cmake ..
	$ make pyRORPO

The generated shared library is in pyRORPO directory.

.. code:: console

	$ ls pyRORPO/pyRORPO.*.so


.. note::
	Move ``pyRORPO.*.so`` to working directory of your
	project or insert the path of the directory
	containing this library to ``sys.path`` to import
	the module

.. code:: python

	import sys
	PATH_TO_SHARED_LIBRARY_DIR = "RORPO/build/pyRORPO/"
	sys.path.append(PATH_TO_SHARED_LIBRARY_DIR)

Then import the module

.. code:: python

	import pyRORPO