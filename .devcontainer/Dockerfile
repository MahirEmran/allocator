FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build tools matching the GitHub Actions ubuntu-latest runner
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        doxygen \
        python3 \
        python3-pip \
        git \
    && pip3 install --break-system-packages gcovr cpplint \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
