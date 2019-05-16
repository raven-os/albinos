{.compile: "util.c"}

proc utf8str_codepoint_len*(s: cstring; utf8len: cint): cint {.importc.}
proc context_len*(prefix: cstring): cint {.importc}