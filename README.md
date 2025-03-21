# SyNAP Runtime
SyNAP runtime libraries and utilities

## Build SyNAP Framework
### Setup Astra Cross-compilation Toolchain
#### **Step 1: Download toolchain**
Clone the Astra cross-compilation toolchain for your board from the [SDK releases]():
```bash
wget https://github.com/synaptics-astra/sdk/releases/download/v1.5.0/sl1680-poky-glibc-x86_64-astra-media-cortexa73-sl1680-toolchain-4.0.17.sh
```

#### **Step 2: Install toolchain**
Run the toolchain setup script
```bash
chmod 755 poky-glibc-x86_64-astra-media-cortexa73-sl1680-toolchain-4.0.17.sh
./poky-glibc-x86_64-astra-media-cortexa73-sl1680-toolchain-4.0.17.sh
```

#### **(Optional) Step 3: Alias toolchain activation**
For convenience, set up an alias to activate the toolchain environment
```bash
alias toolchain='source /opt/poky/4.0.17/environment-setup-cortexa73-poky-linux'
```

> [!TIP]
> To check if the cross-compilation environment is active, use the following command in your Ubuntu terminal:
> ```
> echo $CC
> ```
> You should see an output similar to:
> ```
> aarch64-poky-linux-gcc -mcpu=cortex-a73 -march=armv8-a+crc -mbranch-protection=standard -fstack-protector-strong -O2 -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=/opt/poky/4.0.17/sysroots/cortexa73-poky-linux

### Build Process
#### **Step 1: Clone the repository**
```bash
git clone https://github.com/synaptics-synap/runtime.git
cd runtime
```

#### **Step 2: Create a build directory**
```bash
mkdir -p build
cd build
```

#### **Step 3: Configure and build**
Using `Ninja`:
```bash
cmake .. -G Ninja
ninja
```

Using `Unix Makefiles`:
```bash
cmake .. -G "Unix Makefiles"
make
```

### Build Outputs
#### **1. SyNAP runtime library (libsynap.so)**

This shared library object is generated at `build/lib/synapnb/libsynapnb.so`. 
<br>To apply updates to the runtime, replace the existing system library on your board at `/usr/lib/libsynapnb.so` with the new one. This ensures that any applications using the SyNAP runtime will link to the updated version.

#### **2. Application binaries**

Each compiled application binary will be placed in its respective folder at `build/app/<app name>/`.
<br>To deploy an application, copy the generated binary to the system binary directory `/usr/bin/` on your board.

> [!WARNING]
> It is highly recommended to make backups before replacing library files and/or binaries in `/usr/lib/` and `/usr/bin/`
