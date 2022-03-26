# ******** Build stage *********
FROM alpine:3.14 as build-stage

# Add dependencies
RUN apk add --no-cache gcc-arm-none-eabi newlib-arm-none-eabi ninja cmake bash git

# Copy files and create folders
COPY . /home
RUN mkdir -p /home/build/Debug /home/build/Release
RUN mkdir -p /dist/Debug/plain /dist/Debug/dev /dist/Debug/debug /dist/Release/

# Set working directory
WORKDIR /home/build/Debug

# Running build commands
RUN cmake -DDEV_SWITCH=OFF -DDEBUG_SWITCH=OFF -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=OFF -DCMAKE_BUILD_PLATFORM:STRING=Device -DFIRMWARE_TYPE=Main -G Ninja ../../
RUN ninja -j8 all 
RUN cp Cypherock-*.* /dist/Debug/plain/

RUN cmake -DDEV_SWITCH=ON -DDEBUG_SWITCH=ON -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=OFF -DCMAKE_BUILD_PLATFORM:STRING=Device -DFIRMWARE_TYPE=Main -G Ninja ../../
RUN ninja all 
RUN cp Cypherock-*.* /dist/Debug/dev/

RUN cmake -DDEV_SWITCH=OFF -DDEBUG_SWITCH=OFF -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=OFF -DCMAKE_BUILD_PLATFORM:STRING=Device -DFIRMWARE_TYPE=Main -G Ninja ../../
RUN ninja all 
RUN cp Cypherock-*.* /dist/Release/

RUN cmake -DDEV_SWITCH=OFF -DDEBUG_SWITCH=OFF -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=OFF -DCMAKE_BUILD_PLATFORM:STRING=Device -DFIRMWARE_TYPE=Initial -G Ninja ../../
RUN ninja -j8 all
RUN cp Cypherock-*.* /dist/Debug/plain/

RUN cmake -DDEV_SWITCH=OFF -DDEBUG_SWITCH=OFF -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=OFF -DCMAKE_BUILD_PLATFORM:STRING=Device -DFIRMWARE_TYPE=Initial -G Ninja ../../
RUN ninja all
RUN cp Cypherock-*.* /dist/Release/

# ******** Release stage *********
# Additional stage is to reduce the size on disk for the image
FROM alpine:3.14 as release-stage

# External volume to store build files
VOLUME /out

# Temporarily store build files
RUN mkdir -p /dist
COPY --from=build-stage /dist/ /dist
