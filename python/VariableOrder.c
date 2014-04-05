/*
 * VariableOrder.c
 *
 *  Created on: Feb 12, 2014
 *      Author: dejan
 */

#include "VariableOrder.h"
#include "Variable.h"

#include <structmember.h>
#include <malloc.h>

/** Default variable database */
static variable_order_simple_t* default_var_order = 0;

variable_order_simple_t* VariableOrder_get_default_order(void) {
  if (!default_var_order) {
    default_var_order = (variable_order_simple_t*) variable_order_simple_ops.variable_order_ops.new();
  }
  return default_var_order;
}

static PyObject*
VariableOrder_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

static int
VariableOrder_init(VariableOrder* self, PyObject* args);

static void
VariableOrder_dealloc(VariableOrder* self);

static PyObject*
VariableOrder_str(PyObject* self);

static PyObject*
VariableOrder_repr(PyObject* self);

static PyObject*
VariableOrder_push(PyObject* self, PyObject* args);

static PyObject*
VariableOrder_pop(PyObject* self);

static PyObject*
VariableOrder_set(PyObject* self, PyObject* args);

PyMethodDef VariableOrder_methods[] = {
    {"push", (PyCFunction)VariableOrder_push, METH_VARARGS, "Add a variable to the top of the order"},
    {"pop", (PyCFunction)VariableOrder_pop, METH_NOARGS, "Remove the top variable from the order"},
    {"set", (PyCFunction)VariableOrder_set, METH_VARARGS, "Set the order to the given list of variables"},
    {NULL}  /* Sentinel */
};

PyTypeObject VariableOrderType = {
    PyObject_HEAD_INIT(NULL)
    0,                            /*ob_size*/
    "polypy.VariableOrder",       /*tp_name*/
    sizeof(VariableOrder),        /*tp_basicsize*/
    0,                            /*tp_itemsize*/
    (destructor)VariableOrder_dealloc, /*tp_dealloc*/
    0,                            /*tp_print*/
    0,                            /*tp_getattr*/
    0,                            /*tp_setattr*/
    0,                            /*tp_compare*/
    VariableOrder_repr,           /*tp_repr*/
    0,                            /*tp_as_number*/
    0,                            /*tp_as_sequence*/
    0,                            /*tp_as_mapping*/
    0,                            /*tp_hash */
    0,                            /*tp_call*/
    VariableOrder_str,                 /*tp_str*/
    0,                            /*tp_getattro*/
    0,                            /*tp_setattro*/
    0,                            /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES, /*tp_flags*/
    "VariableOrder objects",      /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    VariableOrder_methods,             /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    (initproc)VariableOrder_init,      /* tp_init */
    0,                            /* tp_alloc */
    VariableOrder_new,                 /* tp_new */
};

PyObject*
VariableOrder_create(variable_order_simple_t* var_order) {
  VariableOrder *self = (VariableOrder*)VariableOrderType.tp_alloc(&VariableOrderType, 0);
  if (self != NULL) {
    self->var_order = var_order;
  }
  return (PyObject *)self;
}

static PyObject*
VariableOrder_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  return VariableOrder_create(0);
}

static PyObject*
VariableOrder_set(PyObject* self, PyObject* args) {

  VariableOrder* order = (VariableOrder*) self;
  if (PyTuple_Check(args) && PyTuple_Size(args) == 1) {
    PyObject* list = PyTuple_GetItem(args, 0);
    if (PyList_Check(list)) {
      int i;
      for (i = 0; i < PyList_Size(list); ++ i) {
        if (!PyVariable_CHECK(PyList_GetItem(list, i))) {
          Py_INCREF(Py_NotImplemented);
          return Py_NotImplemented;
        }
      }
      // Clear the current order
      variable_order_simple_ops.clear(order->var_order);
      // Fill the order
      for (i = 0; i < PyList_Size(list); ++ i) {
        variable_t var = ((Variable*) PyList_GetItem(list, i))->x;
        if (!variable_order_simple_ops.contains(order->var_order, var)) {
          variable_order_simple_ops.push(order->var_order, var);
        }
      }
    } else {
      Py_INCREF(Py_NotImplemented);
      return Py_NotImplemented;
    }
  } else {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  Py_RETURN_NONE;
}

static int
VariableOrder_init(VariableOrder* self, PyObject* args)
{
  if (PyTuple_Check(args) && PyTuple_Size(args) == 1) {
    PyObject* list = PyTuple_GetItem(args, 0);
    if (PyList_Check(list)) {
      int i;
      for (i = 0; i < PyList_Size(list); ++ i) {
        if (!PyVariable_CHECK(PyList_GetItem(list, i))) {
          return -1;
        }
      }
      self->var_order = (variable_order_simple_t*) variable_order_simple_ops.variable_order_ops.new();
      for (i = 0; i < PyList_Size(list); ++ i) {
        variable_t var = ((Variable*) PyList_GetItem(list, i))->x;
        variable_order_simple_ops.push(self->var_order, var);
      }
    } else {
      return -1;
    }
  } else {
    return -1;
  }
  return 0;
}

static void
VariableOrder_dealloc(VariableOrder* self)
{
  if (self->var_order) {
    variable_order_simple_ops.variable_order_ops.detach((variable_order_t*) self->var_order);
  }
  self->ob_type->tp_free((PyObject*)self);
}

static PyObject*
VariableOrder_str(PyObject* self) {
  VariableOrder* var_order = (VariableOrder*) self;
  char* var_order_str = variable_order_simple_ops.to_string(var_order->var_order, Variable_get_default_db());
  PyObject* str = PyString_FromString(var_order_str);
  free(var_order_str);
  return str;
}

static PyObject*
VariableOrder_repr(PyObject* self) {
  VariableOrder* var_order = (VariableOrder*) self;
  char* var_order_str = variable_order_simple_ops.to_string(var_order->var_order, Variable_get_default_db());
  char* var_order_repr = malloc(strlen(var_order_str) + strlen(VariableOrderType.tp_name) + 3);
  sprintf(var_order_repr, "%s(%s)", VariableOrderType.tp_name, var_order_str);
  PyObject* str = PyString_FromString(var_order_repr);
  free(var_order_repr);
  free(var_order_str);
  return str;
}

static PyObject*
VariableOrder_push(PyObject* self, PyObject* args) {
  int error = 0;
  if (PyTuple_Check(args) && PyTuple_Size(args) == 1) {
    PyObject* variable = PyTuple_GetItem(args, 0);
    if (PyVariable_CHECK(variable)) {
      variable_order_simple_t* var_order = ((VariableOrder*) self)->var_order;
      variable_t x = ((Variable*) variable)->x;
      if (!variable_order_simple_ops.contains(var_order, x)) {
        variable_order_simple_ops.push(var_order, x);
      }
    } else {
      error = 1;
    }
  } else {
    error = 1;
  }
  if (error) {
    PyErr_SetString(PyExc_BaseException, "Only variables can be pushed");
  }

  Py_RETURN_NONE;
}

static PyObject*
VariableOrder_pop(PyObject* self) {
  variable_order_simple_t* var_order = ((VariableOrder*) self)->var_order;
  if (variable_order_simple_ops.size(var_order) > 0) {
    variable_order_simple_ops.pop(var_order);
  }
  Py_RETURN_NONE;
}
