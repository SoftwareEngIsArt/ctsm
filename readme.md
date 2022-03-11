## Compile Time State Machines (CTSM)

CTSM is a C++20 header-only library used to create state machines. CTSM was created primarily as a library used to
develop deterministic AI for game development, however it is not limited to game AI.

## Overview

CTSM is designed to provide a simple API, all that is needed from the user is to create state functions and define a
behavior.

```cpp
struct my_data; // Data passed to the states

// States of the behavior
ctsm::state_t initial(my_data &);
ctsm::state_t intermediate_1(my_data &);
ctsm::state_t intermediate_2(const my_data &);

// Behavior type
using my_behavior = ctsm::behavior<initial, intermediate_1, intermediate_2>;
```

Note that a state function must always return `ctsm::state_t`.

`ctsm::state_t` does not have any public (other than copy & move) constructors. In order to obtain an instance
of `ctsm::state_t`, the `ctsm::state<State>` template constant is provided. It will generate a unique `ctsm::state_t`
instance for the specified state.

```cpp
ctsm::state_t initial_state = ctsm::state<initial>;
```

After a behavior type is defined, any number of instances of this behavior may be created. Behaviors are
default-constructible, trivially-copyable & trivially-destructible types.

```cpp
auto behavior = my_behavior();
```

The default constructor will always select the first state from the template pack. In order to select a different
initial state, the parametrized constructor `ctsm::behavior(ctsm::state_t)` must be used.

```cpp
auto other_behavior = my_behavior(ctsm::state<intermediate_1>);
```

To execute a state, simply invoke the behavior instance with the arguments that should be passed to the state function.
Note that every state function of the behavior must be invocable with the passed arguments.

```cpp
my_data data;
behavior(data);
```

Note that if a state function returns an unrecognized state (that is, state which is not part of this behavior), an
instance of `ctsm::bad_state_exception` will be thrown.

The invocation operator (`operator()`) of `ctsm::behavior` returns an instance of `ctsm::state_t` indicating which state
will be executed on next call to `operator()` (the current state). The current state of `ctsm::behavior` can also be
obtained via `ctsm::behavior::state` member function.

```cpp
ctsm::state_t current_state = behavior.state();
```

### States & Shared library symbols

Since `ctsm::state_t` identifiers are generated at compile-time, they may cause linking issues when attempting to use
states across shared object boundaries (for example, MSVC does not export symbols by default).

In order to solve this, `ctsm::state_t` values for the affected states must be explicitly instantiated (and tagged
with `__declspec(dllexport)` on MSVC) inside a single translation unit.

To do this, you must instantiate the `ctsm::state_t::generator<State>` class template for the target state.

```cpp
// my_dll_source.cpp

// State function that needs to cross dll boundaries.
ctsm::state_t my_state_func();

template class __declspec(dllexport) ctsm::state_t::generator<my_state_func>;
```

### Future plans

Currently, a behavior can only execute one state at a time and must wait for the state to complete execution.

In order to allow async execution of states, C++20 coroutine support will be added.


