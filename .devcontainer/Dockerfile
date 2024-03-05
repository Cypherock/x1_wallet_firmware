FROM cypherock/x1-firmware-builder:v1.0.0

ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && mkdir -p /home/$USERNAME && chown -R $USER_UID:$USER_GID /home/$USERNAME

RUN apt update -y && apt install g++ libsdl2-dev gdb clang-tidy-15 clang-format-15 clang-tools-15 -y

RUN ln -s clang-check-15 /usr/bin/clang-check
RUN ln -s clang-format-15 /usr/bin/clang-format
RUN ln -s git-clang-format-15 /usr/bin/git-clang-format
RUN ln -s clang-tidy-15 /usr/bin/clang-tidy
RUN ln -s run-clang-tidy-15 /usr/bin/run-clang-tidy

USER $USERNAME