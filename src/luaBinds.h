#pragma once
#include <string.h>
#include <math.h>
#include <time.h>
#include "../lua/src/lua.h"
#include "../lua/src/lauxlib.h"
#include "../lua/src/lualib.h"

#include "globals.h"
#include "util.h"
#include "notes.h"

// Lua util
int luaB_get_target(lua_State *L, int pnum) {
    int target = luaL_optinteger(L, pnum, 1);
    target -= LUA_INDEX; // Lua indices start at 1 but C indices start at 0
    if (target < 0) {
        printf("lua warn: Lua indices start at 1. Got: %d\n", target);
        target = 0;
    }
    target = target > OSC_COUNT ? OSC_COUNT : target;
    return target;
}

// LUA BINDS
int luaB_debug(lua_State *L) {
    const char *msg = luaL_checkstring(L, 1);
    debug("lua: %s\n", msg);
    return 0;
}

int luaB_enable(lua_State *L) {
    int target = luaB_get_target(L, 1);
    _synth[target].enabled = 1;
    debug("lua: activate(%d)\n", target);
    return 0;
}

int luaB_disable(lua_State *L) {
    int target = luaB_get_target(L, 1);
    _synth[target].enabled = 0;
    debug("lua: deactivate(%d)\n", target);
    return 0;
}

int luaB_freq(lua_State *L) {
    float val = luaL_checknumber(L, 1);
    int target = luaB_get_target(L, 2); 
    _synth[target].freq = val;
    _synth[target].env_pos = 0;
    debug("lua: freq(%f, %d)\n", val, target);
    return 0;
}
int luaB_note(lua_State *L) {
    int target = luaB_get_target(L, 2);

    if (lua_type(L, 1) == LUA_TSTRING) {
        const char *note = luaL_checkstring(L, 1);
        for (int i = 0; i < sizeof(notes) / sizeof(Note); i++) {
            if (strcmp(notes[i].name, note) == 0) {
                _synth[target].freq = notes[i].freq;
                _synth[target].env_pos = 0;
                debug("lua: note: %s, freq: %f, target: %d\n", note, notes[i].freq, target);
                return 0;
            }
        }
    } 
    else if (lua_type(L, 1) == LUA_TNUMBER) {
        int note = luaL_checkinteger(L, 1);
        float freq = 440.0f * powf(2.0f, (note - 69) / 12.0f);
        _synth[target].freq = freq;
        _synth[target].env_pos = 0;
        debug("lua: note: %d, freq: %f, target: %d\n", note, freq, target);
        return 0;
    }

    luaL_error(L, "Invalid argument for note. Expected string or number.");
    return 0;
}
int luaB_detune(lua_State *L) {
    float val = luaL_checknumber(L, 1); 
    int target = luaB_get_target(L, 2);
    _synth[target].detune = val;
    debug("lua: detune(%f, %d)\n", val, target);
    return 0;
}
int luaB_amp(lua_State *L) {
    float val = luaL_checknumber(L, 1);
    int target = luaB_get_target(L, 2);
    _synth[target].amp = val;
    debug("lua: amp(%f, %d)\n", val, target);
    return 0;
}
int luaB_wave(lua_State *L) {
    int val = luaL_checkinteger(L, 1); 
    int target = luaB_get_target(L, 2);
    _synth[target].wave = val;
    debug("lua: wave(%d, %d)\n", val, target);
    return 0;
}
int luaB_solo(lua_State *L) {
    int target = luaB_get_target(L, 1);
    for (int i = 0; i < OSC_COUNT; i++) {
        _synth[i].enabled = 0;
    }
    _synth[target].enabled = 1;
    debug("lua: solo(%d)\n", target);
    return 0;
}
int luaB_pan(lua_State *L) {
    float val = luaL_checknumber(L, 1);
    int target = luaB_get_target(L, 2);
    _synth[target].pan = val;
    debug("lua: pan(%f, %d)\n", val, target);
    return 0;
}
int luaB_env(lua_State *L) {
    // Input is in ticks, convert to seconds
    float attack = luaL_checknumber(L, 1) / 128.0f;
    float sustain = luaL_checknumber(L, 2) / 128.0f;
    float release = luaL_checknumber(L, 3) / 128.0f;
    int target = luaB_get_target(L, 4);
    _synth[target].env[0] = attack;
    _synth[target].env[1] = sustain;
    _synth[target].env[2] = release;
    _synth[target].env_enabled = 1;
    debug("lua: env(%f, %f, %f, %d)\n", attack, sustain, release, target);
    return 0;
}
int luaB_lowpass(lua_State *L){
    float cutoff = luaL_checknumber(L, 1);
    float resonance = luaL_optnumber(L, 2, 1.0f);
    int target = luaB_get_target(L, 3);
    _synth[target].lp_cutoff = cutoff;
    _synth[target].lp_resonance = resonance;
    _synth[target].lp_enabled = 1;
    debug("lua: lowpass(%f, %f, %d)\n", cutoff, resonance, target);
    return 0;
}
int luaB_highpass(lua_State *L){
    float cutoff = luaL_checknumber(L, 1);
    float resonance = luaL_optnumber(L, 2, 1.0f);
    int target = luaB_get_target(L, 3);
    _synth[target].hp_cutoff = cutoff;
    _synth[target].hp_resonance = resonance;
    _synth[target].hp_enabled = 1;
    debug("lua: highpass(%f, %f, %d)\n", cutoff, resonance, target);
    return 0;
}
int luaB_speed(lua_State *L) {
    float val = luaL_checknumber(L, 1);
    if (val <= 0) {
        printf("lua warn: speed must be greater than 0\n");
        val = 0.001f;
    }
    if (val > 1) {
        printf("lua warn: speed must be less than or equal to 1\n");
        val = 1.0f;
    }
    _sys.speed = val;
    debug("lua: speed(%f)\n", val);
    return 0;
}
int luaB_bus_lowpass(lua_State *L) {
    float cutoff = luaL_checknumber(L, 1);
    float resonance = luaL_checknumber(L, 2);
    _bus.lp_cutoff = cutoff;
    _bus.lp_resonance = resonance;
    debug("lua: bus_lowpass(%f, %f)\n", cutoff, resonance);
    return 0;
}
int luaB_bus_amp(lua_State *L) {
    float val = luaL_checknumber(L, 1);
    if (val < 0) {
        printf("lua warn: bus amp must be greater than or equal to 0\n");
        val = 0.0f;
    }
    _bus.amp = val;
    debug("lua: bus_amp(%f)\n", val);
    return 0;
}
// Memory functions
int luaB_mem_set(lua_State *L) {
    float val = luaL_checknumber(L, 1);
    int index = luaL_optinteger(L, 2, 1);
    index -= LUA_INDEX;
    _sys.memory[index] = val;
    debug("lua: mem_set(%d, %f)\n", index, val);
    return 0;
}
int luaB_mem_get(lua_State *L) {
    int index = luaL_checkinteger(L, 1);
    index -= LUA_INDEX;
    if (floor(_sys.memory[index]) == _sys.memory[index]) {
        lua_pushinteger(L, _sys.memory[index]);
        debug("lua: mem_get(%d)\n", index);
        return 1;
    }
    float val = _sys.memory[index];
    lua_pushnumber(L, val);
    debug("lua: mem_get(%d)\n", index);
    return 1;
}
void luaB_binds(lua_State *L) {
    lua_register(L, "dbg", luaB_debug);  // Using dbg which is shorter and not a reserved name
    lua_register(L, "enable", luaB_enable);
    lua_register(L, "disable", luaB_disable);
    lua_register(L, "freq", luaB_freq);
    lua_register(L, "note", luaB_note);
    lua_register(L, "detune", luaB_detune);
    lua_register(L, "amp", luaB_amp);
    lua_register(L, "wave", luaB_wave);
    lua_register(L, "solo", luaB_solo);
    lua_register(L, "pan", luaB_pan);
    lua_register(L, "env", luaB_env);
    lua_register(L, "lowpass", luaB_lowpass);
    lua_register(L, "highpass", luaB_highpass);
    lua_register(L, "speed", luaB_speed);
    lua_register(L, "bus_lowpass", luaB_bus_lowpass);
    lua_register(L, "bus_amp", luaB_bus_amp);
    lua_register(L, "mem_set", luaB_mem_set);
    lua_register(L, "mem_get", luaB_mem_get);
}

void luaB_run() { 
    float seconds = (float)_sys.sample_num / (float)SAMPLE_RATE;
    int tick = floor(seconds * _sys.speed * 128);
    if (tick <= _sys.tick_num) return;

    struct timespec ts;
    if (_sys.output_mode == 1) {
        debug("\nlua start\n");
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
            perror("clock_gettime");
            return;
        }
    }

    _sys.tick_num = tick;
    debug("tick: %d\n", tick);
    debug("seconds: %f\n", seconds);
    // Init a new lua lua_State
    lua_State *L = luaL_newstate();
    luaB_binds(L);
    luaL_openlibs(L);  // Load standard libraries

    // Pass system variables to Lua
    lua_pushnumber(L, seconds);
    lua_setglobal(L, "seconds");
    lua_pushnumber(L, tick);
    lua_setglobal(L, "tick");
    lua_pushstring(L, &_sys.keypress);
    lua_setglobal(L, "keypress");

    if (luaL_dofile(L, _sys.filepath) != LUA_OK) {
        fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // remove error message from stack
    }

    lua_close(L);

    if (_sys.output_mode == 1) {
        struct timespec ts2;
        if (clock_gettime(CLOCK_REALTIME, &ts2) == -1) {
            perror("clock_gettime");
            return;
        }
        // Calculate the time difference in nano seconds
        long diff = ts2.tv_nsec - ts.tv_nsec;
        // Log in microseconds
        debug("lua time: %dµs \n", diff / 1000);
    }

}
