add_rules("mode.debug", "mode.release")

target("binary_tools")
    set_kind("headeronly")
    set_languages("c++17")

    add_headerfiles("include/(**.hpp)")

    add_includedirs("include", {public = true})
