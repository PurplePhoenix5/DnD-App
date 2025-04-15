      
# Wähle ein passendes Basis-Image. Ubuntu oder Debian sind oft gut.
# Man könnte auch ein Image nehmen, das schon Entwicklungs-Tools hat.
FROM ubuntu:22.04

# Setze Argumente für nicht-interaktive Installation
ARG DEBIAN_FRONTEND=noninteractive

# Installiere grundlegende Tools, C++ Compiler, CMake, Git, Netzwerk-Tools
# UND Node.js/npm für das Frontend UND die Markdown-Bibliothek (cmark)
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    cmake \
    make \
    git \
    curl \
    wget \
    # Für Netzwerkprogrammierung im Backend (SSL für HTTPS, falls nötig)
    libssl-dev \
    # CMark Bibliothek (Entwicklungsversion mit Headern)
    libcmark-dev \
    # Node.js und npm (Beispiel für Node 18.x, passe Version ggf. an)
    ca-certificates \
    gnupg \
    && mkdir -p /etc/apt/keyrings \
    && curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
    && NODE_MAJOR=18 \
    && echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_$NODE_MAJOR.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list \
    && apt-get update \
    && apt-get install nodejs -y \
    # Aufräumen
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Setze das Arbeitsverzeichnis (optional, aber oft praktisch)
WORKDIR /workspace

# Standardmäßig als nicht-root User arbeiten (gut für Sicherheit/Berechtigungen)
# Erstellt einen User 'vscode' - VS Code Remote Containers sucht danach
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Wechsle zum neuen User
USER $USERNAME

# Optional: Standard-Shell auf bash setzen (ist meist eh Standard)
# RUN sudo chsh --shell /bin/bash vscode