/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>

#include "driver/gpio.h"

#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    gpio_num_t id;
} machine_pin_obj_t;

STATIC const machine_pin_obj_t machine_pin_obj[] = {
    {{&machine_pin_type}, GPIO_NUM_0},
    {{&machine_pin_type}, GPIO_NUM_1},
    {{&machine_pin_type}, GPIO_NUM_2},
    {{&machine_pin_type}, GPIO_NUM_3},
    {{&machine_pin_type}, GPIO_NUM_4},
    {{&machine_pin_type}, GPIO_NUM_5},
    {{&machine_pin_type}, GPIO_NUM_6},
    {{&machine_pin_type}, GPIO_NUM_7},
    {{&machine_pin_type}, GPIO_NUM_8},
    {{&machine_pin_type}, GPIO_NUM_9},
    {{&machine_pin_type}, GPIO_NUM_10},
    {{&machine_pin_type}, GPIO_NUM_11},
    {{&machine_pin_type}, GPIO_NUM_12},
    {{&machine_pin_type}, GPIO_NUM_13},
    {{&machine_pin_type}, GPIO_NUM_14},
    {{&machine_pin_type}, GPIO_NUM_15},
    {{&machine_pin_type}, GPIO_NUM_16},
    {{&machine_pin_type}, GPIO_NUM_17},
    {{&machine_pin_type}, GPIO_NUM_18},
    {{&machine_pin_type}, GPIO_NUM_19},
    {{NULL}, -1},
    {{&machine_pin_type}, GPIO_NUM_21},
    {{&machine_pin_type}, GPIO_NUM_22},
    {{&machine_pin_type}, GPIO_NUM_23},
    {{NULL}, -1},
    {{&machine_pin_type}, GPIO_NUM_25},
    {{&machine_pin_type}, GPIO_NUM_26},
    {{&machine_pin_type}, GPIO_NUM_27},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{NULL}, -1},
    {{&machine_pin_type}, GPIO_NUM_32},
    {{&machine_pin_type}, GPIO_NUM_33},
    {{&machine_pin_type}, GPIO_NUM_34},
    {{&machine_pin_type}, GPIO_NUM_35},
    {{&machine_pin_type}, GPIO_NUM_36},
    {{&machine_pin_type}, GPIO_NUM_37},
    {{&machine_pin_type}, GPIO_NUM_38},
    {{&machine_pin_type}, GPIO_NUM_39},
};

gpio_num_t machine_pin_get_id(mp_obj_t pin_in) {
    if (mp_obj_get_type(pin_in) != &machine_pin_type) {
        mp_raise_ValueError("expecting a pin");
    }
    machine_pin_obj_t *self = pin_in;
    return self->id;
}

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "Pin(%u)", self->id);
}

// pin.init(mode, pull=None, *, value)
STATIC mp_obj_t machine_pin_obj_init_helper(const machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_pull, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // configure the pin for gpio
    gpio_pad_select_gpio(self->id);

    // set initial value (do this before configuring mode/pull)
    if (args[ARG_value].u_obj != MP_OBJ_NULL) {
        gpio_set_level(self->id, mp_obj_is_true(args[ARG_value].u_obj));
    }

    // configure mode
    if (args[ARG_mode].u_obj != mp_const_none) {
        gpio_set_direction(self->id, mp_obj_get_int(args[ARG_mode].u_obj));
    }

    // configure pull
    if (args[ARG_pull].u_obj != mp_const_none) {
        gpio_set_pull_mode(self->id, mp_obj_get_int(args[ARG_pull].u_obj));
    }

    return mp_const_none;
}

// constructor(id, ...)
STATIC mp_obj_t machine_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get the wanted pin object
    int wanted_pin = mp_obj_get_int(args[0]);
    const machine_pin_obj_t *self = NULL;
    if (0 <= wanted_pin && wanted_pin < MP_ARRAY_SIZE(machine_pin_obj)) {
        self = (machine_pin_obj_t*)&machine_pin_obj[wanted_pin];
    }
    if (self == NULL || self->base.type == NULL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid pin"));
    }

    if (n_args > 1 || n_kw > 0) {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

// fast method for getting/setting pin value
STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;
    if (n_args == 0) {
        // get pin
        return MP_OBJ_NEW_SMALL_INT(gpio_get_level(self->id));
    } else {
        // set pin
        gpio_set_level(self->id, mp_obj_is_true(args[0]));
        return mp_const_none;
    }
}

// pin.init(mode, pull)
STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

// pin.value([value])
STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPIO_MODE_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPIO_MODE_INPUT_OUTPUT) },
    { MP_ROM_QSTR(MP_QSTR_OPEN_DRAIN), MP_ROM_INT(GPIO_MODE_INPUT_OUTPUT_OD) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP), MP_ROM_INT(GPIO_PULLUP_ONLY) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(GPIO_PULLDOWN_ONLY) },
};

STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

const mp_obj_type_t machine_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .print = machine_pin_print,
    .make_new = machine_pin_make_new,
    .call = machine_pin_call,
    .locals_dict = (mp_obj_t)&machine_pin_locals_dict,
};