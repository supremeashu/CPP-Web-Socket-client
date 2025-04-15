
WebSocket C++ Client
A command-line WebSocket client implementation in C++ with SSL support. This client can connect to WebSocket servers, send messages, and display received messages.
Features
•	Connect to secure WebSocket (WSS) servers
•	Send text messages to the server
•	Receive and display messages from the server
•	Simple command-line interface
•	Handle connection errors and disconnections gracefully
Requirements
•	C++17 compatible compiler
•	Boost libraries (Beast, ASIO)
•	OpenSSL
•	GN build system

•	Ninja build tool
Building the Project
Install Dependencies
For Ubuntu/Debian:
sudo apt-get update sudo apt-get install build-essential libboost-all-dev libssl-dev 
For macOS (using Homebrew):
brew install boost openssl 
Install GN and Ninja
Follow the instructions at https://gn.googlesource.com/gn/ to install GN.
For Ninja:
sudo apt-get install ninja-build # Ubuntu/Debian brew install ninja # macOS 
Build Instructions
•	Generate the build files for debug:
gn gen out/Debug --args-file=args_debug.gn 
•	Generate the build files for release:
gn gen out/Release --args-

file=args_release.gn 
•	Build the debug version:
ninja -C out/Debug 
•	Build the release version:
ninja -C out/Release 
Usage
Run the built executable:
./out/Debug/websocket_client # Debug build ./out/Release/websocket_client # Release build 
Commands
•	Enter a WebSocket server URL or press Enter to use the default (wss://echo.websocket.events/.ws)
•	Type a message and press Enter to send it to the server
•	/disconnect - Disconnect from the current server
•	/quit - Exit the application
Supported WebSocket Servers

This client has been tested with:
•	wss://echo.websocket.events/.ws
•	wss://piehost.com/websocket-tester
•	wss://websocket.org/tools/websocket-echo-server
Implementation Details
The client uses:
•	Boost.Beast for WebSocket protocol implementation
•	Boost.ASIO for networking
•	OpenSSL for secure connections
•	Multiple threads for handling connections and user input
