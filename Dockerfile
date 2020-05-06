FROM ubuntu:focal

RUN apt-get update && \
    apt-get install -y \
        cmake \
        curl \
        gcc \
        g++ \
    && rm -rf /var/lib/apt/lists/*

ARG cmake_version=3.12.3
WORKDIR /opt/
RUN curl -L https://github.com/Kitware/CMake/releases/download/v${cmake_version}/cmake-${cmake_version}.tar.gz -o /tmp/cmake-${cmake_version}.tar.gz && \
    tar xf /tmp/cmake-${cmake_version}.tar.gz -C /opt/ && \
    rm -f /tmp/cmake-${cmake_version}.tar.gz
WORKDIR /opt/cmake-${cmake_version}/build
RUN mkdir release && \
    cmake .. && \
    make -j$(nproc) && \
    make install


RUN apt-get update && \
    apt-get install -y \
        curl \
        gcc \
        g++ \
        xz-utils \
        python \
    && rm -rf /var/lib/apt/lists/*

ARG llvm_version=6.0.0
WORKDIR /opt/llvm
RUN curl -L http://releases.llvm.org/${llvm_version}/llvm-${llvm_version}.src.tar.xz -o /tmp/llvm-${llvm_version}.src.tar.xz && \
    tar xf /tmp/llvm-${llvm_version}.src.tar.xz -C /opt/ && \
    rm -f /tmp/llvm-${llvm_version}.src.tar.xz
WORKDIR /opt/llvm-${llvm_version}.src/build
RUN mkdir release && \
    [ "$(cmake --version | head -1)" = "cmake version ${cmake_version}" ] && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_TARGETS_TO_BUILD=X86 \
        -DBUILD_SHARED_LIBS=1 \
        -DLLVM_ENABLE_RTTI=1 \
        .. && \
    make -j$(nproc) && \
    make install

RUN apt-get update && \
    apt-get install -y \
        gcc \
        curl \
        libexpat-dev \
        pkg-config \
        zlib1g-dev \
        x11proto-dev \
        libx11-dev \
        libxext-dev \
        xcb-proto \
        libxcb-dri2-0-dev \
        libx11-xcb-dev \
        libxcb-xfixes0-dev \
        python \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && \
    apt-get install -y \
        libtool-bin \
	meson \
	ninja-build \
	python3-pip \
	libx11-dev \
	libxext-dev \
	x11proto-core-dev \
	x11proto-gl-dev \
	libglew-dev \
	freeglut3-dev \
	bison \
        flex \
	python-mako \
	python-lxml \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install mako

RUN apt-get update && \
    apt-get install -y \
        libdrm-dev \
    && rm -rf /var/lib/apt/lists/*

ARG mesa_version=20.0.6
RUN curl -L https://mesa.freedesktop.org/archive/mesa-${mesa_version}.tar.xz -o /tmp/mesa-${mesa_version}.tar.xz && \
    tar xf /tmp/mesa-${mesa_version}.tar.xz -C /opt/ && \
    rm -f /tmp/mesa-${mesa_version}.tar.xz
WORKDIR /opt/mesa-${mesa_version}/build
RUN mkdir release && \
    meson \
        --buildtype=release \
	-Dglx=disabled \
	-Dvulkan-drivers= \
	-Ddri-drivers= \
	-Dgallium-drivers=swrast,swr \
	-Dplatforms= \
	-Dosmesa=gallium \
	-Dgallium-omx=disabled \
	build .. \
    && \
    ninja -C build && \
    meson install -C build

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y \
        cmake \
	build-essential \
	gcc \
        git \
        zlib1g-dev \
        pkg-config \
	gdb \
    && rm -rf /var/lib/apt/lists/*

#RUN apt-get update && \
#    apt-get install -y \
#        mesa \
#    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /.swr/jitcache && chmod 777 -R /.swr

WORKDIR /app
COPY Makefile /app/
COPY src /app/src
ENV PKG_CONFIG_PATH=/app/contrib/lib/pkgconfig
RUN ls -lahR /app/ && make

CMD []
