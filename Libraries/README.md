![HyTech logo](https://hytechracing.gatech.edu/images/hytech_logo_small.png)

# Libraries Organization
This folder contains all the libraries our team's code uses. There are symlinks also in this folder so the Teensyduino IDE is able to compile the libraries, meaning that any new library should also get a new **relative path** [symlink created](https://unix.stackexchange.com/a/120728). Finally, all external libraries are installed as [branch specified submodules](https://stackoverflow.com/a/18799234) and specified in the `.gitmodules` file in the top level directory.
## External
`External` is for code not written by HyTech Racing. All external libraries are stored as [git submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules) and can be cloned and updated from the command line using `git submodule update --init --recursive` and `git submodule update --recursive --remote`. All libraries here should be submodules referencing an external repository.
## HyTech_Racing
`HyTech_Racing` is only for the libraries we have written and maintain inhouse. Do not put a library created by someone else in this folder.
