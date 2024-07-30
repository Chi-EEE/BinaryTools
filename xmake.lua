add_rules("mode.debug", "mode.release")

target("binary_tools")
    set_kind("$(kind)")
    set_languages("c++17")

    add_files("src/*.cpp")
    add_headerfiles("include/(**.h)")

    add_includedirs("include", {public = true})

    if is_plat("windows") and is_kind("shared") then
        add_rules("utils.symbols.export_all", {export_classes = true})
    end

