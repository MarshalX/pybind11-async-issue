## pybind11 async issue

### Required prerequisites
- [x] Read the doc and used [test_async.py](https://github.com/pybind/pybind11/blob/master/tests/test_async.cpp) file of pybind11 repository. 
- [x] Search the issue tracker and discussions.
- [x] Asked in [discussion](https://github.com/pybind/pybind11/discussions/3593) and in [gitter](https://gitter.im/pybind/Lobby?at=61d7f74f5dc6213cd4ccc93b).

### Problem description

`await` statement doesn't work properly (blocks for forever) with `asyncio.Future` that was returned to Python without result and the result was set in another thread after returning the future.

### Reproducible example code

Repository with project that can reproduce the issue: https://github.com/MarshalX/pybind11-async-issue

main.py
```python3
import asyncio

import async_issue


# it's a trick
async def wrap(future):
    on_done_event = asyncio.Event()

    def _done_callback(_):
        on_done_event.set()

    future.add_done_callback(_done_callback)
    await asyncio.wait_for(on_done_event.wait(), timeout=10)

    return future.result()


async def main():
    r = await async_issue.test()
    # works fine
    print('Awaited result of future without thread', r)

    r = await wrap(async_issue.thread_test())
    # works fine (with big delay)
    print('Wrapped future with add_done_callback trick', r)
    r = await async_issue.thread_test()
    # blocked forever. This print doesn't appear
    print('Awaited result of future without wrapping', r)


if __name__ == '__main__':
    loop = asyncio.get_event_loop().run_until_complete(main())

```

module.cpp
```cpp
#include <chrono>
#include <thread>

#include <pybind11/pybind11.h>

namespace py = pybind11;

static py::object test() {
  py::object loop = py::module_::import("asyncio.events").attr("get_event_loop")();
  py::object f = loop.attr("create_future")();

  f.attr("set_result")(5);

  return py::object{f};
}

static void thread_task(py::object future)
{
  int delay_in_sec = 1;
  std::this_thread::sleep_for(std::chrono::milliseconds(delay_in_sec * 1000));

  py::gil_scoped_acquire acquire;
  future.attr("set_result")(5);
  py::gil_scoped_release release;
}

static py::object thread_test() {
  py::object loop = py::module_::import("asyncio.events").attr("get_event_loop")();
  py::object f = loop.attr("create_future")();

  std::thread thread(thread_task, py::object{f});

  py::gil_scoped_release release;
  thread.detach();

  return py::object{f};
}

PYBIND11_MODULE(async_issue, m) {
  m.def("test", &test);
  m.def("thread_test", &thread_test);
}
```

#### Actual output of main.py
```
Awaited result of future without thread 5
Wrapped future with add_done_callback trick 5
```

#### Expected output of main.py
```
Awaited result of future without thread 5
Wrapped future with add_done_callback trick 5
Awaited result of future without wrapping 5
```
