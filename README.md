# Walnut Chat

Walnut Chat is a simple client/server text chat app built with [Walnut](https://github.com/StudioCherno/Walnut) and the [Walnut-Networking](https://github.com/StudioCherno/Walnut-Networking) module. The server currently runs on both Windows (GUI/headless) and Linux (headless only), and the client is Windows only at this stage (Linux support coming soon).

This was built as a demonstration of networking in C++ for a video on my YouTube channel. **There is no security** so be careful! Definitely don't run this as root on your server/computer and there certainly isn't any message encryption. [Watch the video here.](https://youtu.be/jS9rBienEFQ)


![WalnutExample](https://hazelengine.com/images/WalnutChat.jpg)
_<center>Walnut Chat Client</center>_

## Building
### Windows
Running `scripts/Setup.bat` will generate both `Walnut-Chat.sln` and `Walnut-Chat-Headless.sln` solution files for Visual Studio 2022. The headless variant will only include the server, running in the headless config (no GUI console app), and the `Walnut-Chat` solution can be used to build GUI versions of the client and/or server.

### Linux (tested on Ubuntu 22)
Run `scripts/Setup.sh` to generate make files for the headless server project. You can then call `make` in the root directory of the repository to build.