# HTTPS Webserver for MS-DOS
This is a minimal HTTPS capable webserver for MS-DOS.

It is not in real working condition, connections only succeed when EtherDFS is running in the background and I have no idea why...

HTTP code was taken from https://github.com/starnight/MicroHttpServer
TLS is provided by https://github.com/Mbed-TLS/mbedtls
TCP/IP stack is https://www.watt-32.net/
INI file reading is provided by https://github.com/rxi/ini

## Configuration
See `HTTPDOS.INI` for an example configuration.

## Self signed certificate
You need to provide matching TLS/SSL certificates in order to run the server.
Self signed certificates can be generated using `genkeys.sh`.

`genkeys.sh`, `cert.conf` and `csr.conf` were created following https://devopscube.com/create-self-signed-certificates-openssl/

## NOISE.SYS
A random number generator can be obtained here: https://github.com/robrwo/noise.sys

## License
Micro HTTP Server's code uses the BSD license, so httpDOS uses the same license. See LICENSE.md file.
