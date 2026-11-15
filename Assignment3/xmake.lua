-- Can also be configured independently from inside the Assignment3 directory.
if os.scriptdir() == os.projectdir() then
    set_project("COMP3271_Assignment3")
    set_languages("cxx20")
    add_rules("mode.release", "mode.debug")
    add_requires("argparse")
    add_requires("spdlog", { configs = { std_format = true } })
end

target("assignment3")
    set_kind("binary")
    add_packages("argparse", "spdlog")
    add_files(path.join(os.scriptdir(), "*.cpp"))
    add_headerfiles(path.join(os.scriptdir(), "*.hpp"))
    set_rundir(path.directory(os.scriptdir()))
target_end()
