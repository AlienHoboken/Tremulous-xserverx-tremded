XserverX Tremded
==================
Many of the XserverX community mods require a customized tremded which supplies support for MySQL, among other items.

Extra Dependencies
-----------------------
The tremded depends on MySQL/MySQL development libraries. Ensure these are installed before attempting to build.

Building
-----------------------
You can build the tremded the same way you build normally.

Make sure that the `BUILD_SERVER` flag is set in the Makefile before building.

As normal there are shell scripts for building on Windows and Mac OSX. Windows requires MingW be used.

Running
-----------------------
The tremded can be run the same as any other tremulous dedicated server, eg:
`./tremded.x86 +set dedicated 2 +exec server.cfg +set net_port 30721 +set net_ip 127.0.0.1 +set fs_game "base"`

Branches
-----------------------
The development branch contains upcoming features that have not been deemed stable for use in the server yet.

Contributing
-----------------------
If you wish to contribute, please fork the branch you wish to work on, make your changes, and submit a pull request for review.

Also, please report all bugs you encounter!
