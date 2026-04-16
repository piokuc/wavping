#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <chrono>
#include <string>
#include <thread>

#include "miniaudio.h"

namespace {

struct EngineGuard {
    ma_engine engine{};
    bool initialized = false;

    ~EngineGuard() {
        if (initialized) {
            ma_engine_uninit(&engine);
        }
    }
};

struct SoundGuard {
    ma_sound sound{};
    bool initialized = false;

    ~SoundGuard() {
        if (initialized) {
            ma_sound_uninit(&sound);
        }
    }
};

#if defined(_WIN32)
bool filesystem_path_from_python(PyObject* value, std::wstring* out)
{
    PyObject* fs_path = PyOS_FSPath(value);
    if (fs_path == nullptr) {
        return false;
    }

    PyObject* unicode_path = fs_path;
    if (PyBytes_Check(fs_path)) {
        unicode_path = PyUnicode_FromEncodedObject(fs_path, Py_FileSystemDefaultEncoding, "surrogatepass");
        Py_DECREF(fs_path);
        if (unicode_path == nullptr) {
            return false;
        }
    }

    Py_ssize_t path_length = 0;
    wchar_t* wide_path = PyUnicode_AsWideCharString(unicode_path, &path_length);
    Py_DECREF(unicode_path);
    if (wide_path == nullptr) {
        return false;
    }

    out->assign(wide_path, static_cast<size_t>(path_length));
    PyMem_Free(wide_path);
    return true;
}

ma_result play_file_blocking(const std::wstring& path, ma_uint32 poll_interval_ms)
{
    EngineGuard engine;
    SoundGuard sound;

    ma_result result = ma_engine_init(nullptr, &engine.engine);
    if (result != MA_SUCCESS) {
        return result;
    }
    engine.initialized = true;

    result = ma_sound_init_from_file_w(&engine.engine, path.c_str(), 0, nullptr, nullptr, &sound.sound);
    if (result != MA_SUCCESS) {
        return result;
    }
    sound.initialized = true;

    result = ma_sound_start(&sound.sound);
    if (result != MA_SUCCESS) {
        return result;
    }

    while (!ma_sound_at_end(&sound.sound)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
    }

    return MA_SUCCESS;
}
#else
bool filesystem_path_from_python(PyObject* value, std::string* out)
{
    PyObject* fs_path = PyOS_FSPath(value);
    if (fs_path == nullptr) {
        return false;
    }

    PyObject* bytes_path = fs_path;
    if (!PyBytes_Check(fs_path)) {
        bytes_path = PyUnicode_EncodeFSDefault(fs_path);
        Py_DECREF(fs_path);
        if (bytes_path == nullptr) {
            return false;
        }
    }

    char* raw_path = PyBytes_AsString(bytes_path);
    if (raw_path == nullptr) {
        Py_DECREF(bytes_path);
        return false;
    }

    out->assign(raw_path, static_cast<size_t>(PyBytes_GET_SIZE(bytes_path)));
    Py_DECREF(bytes_path);
    return true;
}

ma_result play_file_blocking(const std::string& path, ma_uint32 poll_interval_ms)
{
    EngineGuard engine;
    SoundGuard sound;

    ma_result result = ma_engine_init(nullptr, &engine.engine);
    if (result != MA_SUCCESS) {
        return result;
    }
    engine.initialized = true;

    result = ma_sound_init_from_file(&engine.engine, path.c_str(), 0, nullptr, nullptr, &sound.sound);
    if (result != MA_SUCCESS) {
        return result;
    }
    sound.initialized = true;

    result = ma_sound_start(&sound.sound);
    if (result != MA_SUCCESS) {
        return result;
    }

    while (!ma_sound_at_end(&sound.sound)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
    }

    return MA_SUCCESS;
}
#endif

PyObject* py_play(PyObject*, PyObject* args, PyObject* kwargs)
{
    static const char* keyword_names[] = {"path", "poll_interval_ms", nullptr};

    PyObject* path_obj = nullptr;
    unsigned int poll_interval_ms = 10;
    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "O|I:play",
            const_cast<char**>(keyword_names),
            &path_obj,
            &poll_interval_ms)) {
        return nullptr;
    }

    if (poll_interval_ms == 0) {
        PyErr_SetString(PyExc_ValueError, "poll_interval_ms must be greater than zero");
        return nullptr;
    }

#if defined(_WIN32)
    std::wstring path;
#else
    std::string path;
#endif

    if (!filesystem_path_from_python(path_obj, &path)) {
        return nullptr;
    }

    ma_result result = MA_SUCCESS;
    Py_BEGIN_ALLOW_THREADS
    result = play_file_blocking(path, static_cast<ma_uint32>(poll_interval_ms));
    Py_END_ALLOW_THREADS

    if (result != MA_SUCCESS) {
        PyErr_Format(
            PyExc_RuntimeError,
            "wavping.play failed for %R: %s (%d)",
            path_obj,
            ma_result_description(result),
            static_cast<int>(result));
        return nullptr;
    }

    Py_RETURN_NONE;
}

PyMethodDef module_methods[] = {
    {"play", reinterpret_cast<PyCFunction>(py_play), METH_VARARGS | METH_KEYWORDS, PyDoc_STR("Play a WAV file and block until it finishes.")},
    {nullptr, nullptr, 0, nullptr},
};

PyModuleDef module_definition = {
    PyModuleDef_HEAD_INIT,
    "_native",
    "Native wavping module.",
    -1,
    module_methods,
};

}  // namespace

PyMODINIT_FUNC PyInit__native(void)
{
    return PyModule_Create(&module_definition);
}
