NToken Core 0.12.2
=====================

This is the official reference wallet for NToken digital currency and comprises the backbone of the NToken peer-to-peer network. You can [download NToken Core](https://www.ntoken.org/downloads/) or [build it yourself](#building) using the guides below.

Running
---------------------
The following are some helpful notes on how to run NToken on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/ntoken-qt` (GUI) or
- `bin/ntokend` (headless)

### Windows

Unpack the files into a directory, and then run ntoken-qt.exe.

### OS X

Drag NToken-Qt to your applications folder, and then run NToken-Qt.

### Need Help?

* See the [NToken documentation](https://ntoken.atlassian.net/wiki/display/DOC)
for help and more information.
* Ask for help on [#ntoken](http://webchat.freenode.net?channels=ntoken) on Freenode. If you don't have an IRC client use [webchat here](http://webchat.freenode.net?channels=ntoken).
* Ask for help on the [NTokenTalk](https://ntokentalk.org/) forums.

Building
---------------------
The following are developer notes on how to build NToken Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [OS X Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [OpenBSD Build Notes](build-openbsd.md)

Development
---------------------
The NToken Core repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Multiwallet Qt Development](multiwallet-qt.md)
- [Release Process](release-process.md)
- Source Code Documentation ***TODO***
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [Unit Tests](unit-tests.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)

License
---------------------
Distributed under the [MIT software license](http://www.opensource.org/licenses/mit-license.php).
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](https://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.
