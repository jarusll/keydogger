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
end

# always start for debug
set args debug
