# Introduction #

`HttpSvr` is an HTTP Server library for Arduino and Ethernet Shield. The current version runs on Arduino Mega.

# Installation #

1) Download the sources from svn (follow the instructions in the "Source" section).

2) Create a new directory named "`HttpSvr`" under "sketchbook/libraries" on your computer

3) Copy the entire content of "trunk" as downloaded in step 1) in the "`sketchbook/libraries/HttpSvr`" directory created in step 2).

# Run the example #

The library comes with an example that shows how to use `HttpSvr`.
To run the example, follow these steps:

1) Go to directory "`trunk/extras`"

2) Open file "`HttpMega.png`" and assemble the test bed circuit as shown in figure.

2) Uncompress "`SD-Card.tar.gz`" and copy its content on an micro-SD card. Then put the micro-SD card into the slot on Ethernet shield.

3) Open the Arduino IDE, then open the sketch that you should find in "`File/Sketchbook/libraries/HttpSvr/HttpMega`".

4) Modify the values of variables `HTTPMEGA_MAC_ADDRESS` and `HTTPMEGA_STATIC_IP` according to your board and network configuration.

5) Connect your Ethernet shield to a router and reset the board

6) From your PC, open your favorite browser and type the IP address (the one specified in `HTTPMEGA_STATIC_IP`) in the address bar. After a while, you should see the welcome page on your browser.

7) Play with available pages

8) Read the sources and hack it as desired. There are many comments!