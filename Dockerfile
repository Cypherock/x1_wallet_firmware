# ******** Build stage *********
FROM alpine:3.14 as build-stage

# Add dependencies
RUN apk add --no-cache gcc-arm-none-eabi newlib-arm-none-eabi ninja cmake bash git python3 py-pip protoc
RUN apk add --no-cache g++

# Copy files and create folders
COPY . /home
RUN mkdir -p /home/build/Debug /home/build/Release
RUN mkdir -p /dist/Debug/plain /dist/Debug/dev /dist/Debug/debug /dist/Release/
RUN mkdir -p /pcache

ENV PYTHONUNBUFFERED=1
RUN apk add --update --no-cache python3 && ln -sf python3 /usr/bin/python
RUN python3 -m ensurepip
RUN pip3 install --no-cache-dir ecdsa==0.16.1
RUN pip3 install --no-cache-dir wheel
RUN pip3 install --no-cache-dir --upgrade pip setuptools
RUN pip3 install --no-cache-dir protobuf grpcio-tools==1.47.0

# Set working directory
WORKDIR /home/build/Debug
