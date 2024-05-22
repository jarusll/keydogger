# compile & reload bin
define reload
    # compile
    shell make dev
    # reset executable
    file keydogger
    # clear cache
    directory
    # redraw screen
    refresh
    # show source
    list
    # reload init file
    source .gdbinit
end

# always start for debug
set args debug

define iw
    info watchpoints
end

define ib
    info breakpoints
end

define il
    info args
    info locals
end
