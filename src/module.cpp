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
