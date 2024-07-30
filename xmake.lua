target("binary_tools")
    set_kind("$(kind)")
    set_languages("c++17")

    add_files("src/*.cpp")
    add_headerfiles("include/(**.h)")

    add_includedirs("include", {public = true})
