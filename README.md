# tmd-eic-ana

[![Build and Test](https://github.com/Gregtom3/tmd-eic-ana/actions/workflows/ci.yml/badge.svg)](https://github.com/Gregtom3/tmd-eic-ana/actions/workflows/ci.yml)

Analysis repo for TMD's at EIC using epic-analysis

---

## 1. Install yaml-cpp locally

```bash
# Clone and build yaml-cpp
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
mkdir build && cd build

cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local -DYAML_BUILD_SHARED_LIBS=ON
make
make install
```

If it does not already, point your `LD_LIBRARY_PATH` variable to the appropriate directory

```bash
export LD_LIBRARY_PATH=$HOME/.local/lib64:$LD_LIBRARY_PATH
```

---

## 2. Build the project

```bash
make
```

---
