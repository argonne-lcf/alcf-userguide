# Python on Aurora

## Framework Modules

Frameworks on Aurora can be loaded into a users environment by loading the `frameworks` module as follows. The conda environment loaded with this module makes available TensorFlow, Horovod, and Pytorch with Intel extensions and optimizations. The following commands can be used both from an interactive session on a terminal and on a batch job script.

Note that the framework modules may load a different oneAPI than the default module.  The frameworks are updated on approximately a quarterly cadence at the moment.

```
module use /soft/modulefiles
module load frameworks
```
These pre-built `conda` environments come with GPU-supported builds of PyTorch
and TensorFlow. Both of these frameworks have
`Horovod` support for multi-node calculations. Many other commonly used Python
modules are available through these modules.

For more information on pytorch and tensorflow please see their respective 
pages: 

- [PyTorch](https://pytorch.org/)
- [TensorFlow](https://www.tensorflow.org/)

From a login node we can do the following commands to list the available 
modules:

```
module load /soft/modulefiles/
module avail
```
This shows a list of avilable modules including the frameworks module. There 
are many frameworks modules available. The latest 
frameworks release could be used using:

```
$ module load frameworks/2023.12.15.001

The following have been reloaded with a version change:
  1) gcc/11.2.0 => gcc/12.2.0     2) intel_compute_runtime/release/agama-devel-551 => intel_compute_runtime/release/stable-736.25

$ which python3
/soft/datascience/aurora_nre_models_frameworks-2024.0/bin/python3

$ which python
/soft/datascience/aurora_nre_models_frameworks-2024.0/bin/python
```
At the time of writing this module contains Python 3.9.18.
Future modules will
contain updated versions of Python, PyTorch, TensorFlow, etc.

While the shared Anaconda environment encapsulated in the module contains many 
of the most commonly used Python libraries for our users, you may still 
encounter a scenario in which you need to extend the functionality of the 
environment (i.e. install additional packages)

You can use a virtual environment to extend/modify an existing frameworks 
module. 

## Virtual environments via `venv`

Creating your own (empty) virtual Python environment in a directory that is 
writable to you is straightforward:

```
python3 -m venv /path/to/new/virtual/environment
```

This creates a new folder that is fairly lightweight folder (<20 MB) with its 
own Python interpreter where you can install whatever packages you'd like. 
First, you must activate the virtual environment to make this Python 
interpreter the default interpreter in your shell session.  By default, this environment
will not have access to the framework packages but instead will be empty.

You activate the new environment whenever you want to start using it via 
running the activate script in that folder:

```
source /path/to/new/virtual/environment/bin/activate
```

In many cases, you do not want an empty virtual environment, but instead want 
to start from the `conda` base environment's installed packages, only adding 
and/or changing a few modules.

To extend the base Anaconda environment with `venv` (e.g. `my_env` in the current 
directory) and inherit the base enviroment packages, one can use the 
`--system-site-packages` flag:

```
module use /soft/modulefiles/
module load frameworks/2023.12.15.001
python3 -m venv --system-site-packages my_env
source my_env/bin/activate

# Install additional packages here
```
You can always retroactively change the `--system-site-packages` flag state for 
this virtual environment by editing `my_env/pyvenv.cfg` and changing the value 
of the line `include-system-site-packages = false`.

To install a different version of a package that is already installed in the 
base environment, you can use:

```
pip install --ignore-installed ... # or -I
```
The shared base environment is not writable, so it is impossible to remove or 
uninstall packages from it. The packages installed with the above `pip` command 
should shadow those installed in the base environment.

## Using `pip install --user` (not recommended)

With the conda environment setup, one can install common Python modules using 
`pip install --users <module-name>` which will install packages in 
`$PYTHONUSERBASE/lib/pythonX.Y/site-packages`. The `$PYTHONUSERBASE` 
environment variable is automatically set when you load the base `conda` 
module, and is equal to `/home/$USER/.local/aurora/frameworks/2023.12.15.001`

Note, Python modules installed this way that contain command line binaries will 
not have those binaries automatically added to the shell's `$PATH`. To manually 
add the path:

```
export PATH=$PYTHONUSERBASE/bin:$PATH
```
Be sure to remove this location from `$PATH` if you deactivate the base 
Anaconda environment or unload the module.

Cloning the Anaconda environment, or using `venv` are both more flexible and 
transparent when compared to `--user` installs.



