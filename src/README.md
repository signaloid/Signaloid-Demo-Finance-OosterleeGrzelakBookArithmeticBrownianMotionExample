# Source code:

## main.c
Implementation of the main functionality of the application.

## kernel.c/h
Mode-dispatch calculation kernels. `calculateOutputUxHw` runs the UxHw-mode
kernel (a single distributional path). `calculateOutputMonteCarlo` runs the
Monte Carlo kernel (one independent path per iteration).

## arithmetic-brownian-motion-uxhw.c/h
Implementation of the Arithmetic Brownian Motion algorithm, as well as put and
call option payoff calculation, using UxHw distributional arithmetic.

## arithmetic-brownian-motion-monte-carlo.c/h
Implementation of the Arithmetic Brownian Motion algorithm, as well as put and
call option payoff calculation, for the native Monte Carlo build (one sample
per iteration).

## utilities.c/h
These contain utility methods for parsing, setting, and reporting
the usage of demo-specific command-line arguments of C/C++ demo applications.
These methods call similar methods from `common.c` for handling
command-line arguments common to all of our C/C++ demo applications.

## common.c/h
These contain utility methods for parsing, setting, and reporting
the usage of command-line arguments common to all of our C/C++ demo applications,
as well as other methods that we commonly use across our
C/C++ demo applications, e.g., standard methods for I/O handling. These
source files are symlinks to the original files contained in the repository
[Signaloid-Demo-CommonUtilityRoutines](https://github.com/signaloid/Signaloid-Demo-CommonUtilityRoutines)
which is included as a submodule in `submodules/common`.

## uxhw.c/h
These contain methods that implement the probabilistic versions of the methods
in the UxHw API (e.g., `UxHwDoubleGaussDist`) and uses the GNU Scientific Library (GSL)
random number generators to achieve that. This allows building our C/C++ demo applications
natively (i.e., on conventional architectures) and running native Monte Carlo evaluations
of our C/C++ demo applications without modifying the source code.
These source files are symlinks to the original files and are contained in the repository
[Signaloid-Demo-UxHwCompatibilityForNativeExecution](https://github.com/signaloid/Signaloid-Demo-UxHwCompatibilityForNativeExecution)
which is included as a submodule in `submodules/compat`.

## config.mk
Signaloid cores use this file to identify the source codes they will use when
building the C/C++ demo application.

# To Build Natively on Non-Signaloid Platforms

From the repository root, on both macOS and Linux:
```
make local-build
```

This builds the `demo-native-mc` executable at the repository root, using the
top-level `Makefile` (see `../Makefile`), which compiles the sources listed
above together with the UxHw compatibility shim in `uxhw.c`. See the
[root README's Prerequisites section](../README.md#prerequisites) for the
dependencies this build requires.
