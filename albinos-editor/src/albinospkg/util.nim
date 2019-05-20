{.compile: "util.c".}
import ../ext/replxx

proc utf8str_codepoint_len*(s: cstring; utf8len: cint): cint {.importc.}
proc context_len*(prefix: cstring): cint {.importc.}

func `+`*(p: cstring, offset: int): type(p) {.inline.} =
   ## Pointer increment
   {.emit: "`result` = `p` + `offset`;".}

func `+`*(p: ptr ReplxxColor, offset: int): type(p) {.inline.} =
   ## Pointer increment
   {.emit: "`result` = `p` + `offset`;".}

proc strncmp*(s1: cstring, s2: cstring, n: csize): cint
   {.header: "<string.h>", importc: "strncmp".}
