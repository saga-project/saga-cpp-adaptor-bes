/*
	cacerts.c

	Store CA certificates in memory for optimizations and/or stand-alone
	clients.

gSOAP XML Web services tools
Copyright (C) 2000-2008, Robert van Engelen, Genivia Inc., All Rights Reserved.
This part of the software is released under one of the following licenses:
GPL, the gSOAP public license, or Genivia's license for commercial use.
--------------------------------------------------------------------------------
gSOAP public license.

The contents of this file are subject to the gSOAP Public License Version 1.3
(the "License"); you may not use this file except in compliance with the
License. You may obtain a copy of the License at
http://www.cs.fsu.edu/~engelen/soaplicense.html
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.

The Initial Developer of the Original Code is Robert A. van Engelen.
Copyright (C) 2000-2008, Robert van Engelen, Genivia Inc., All Rights Reserved.
--------------------------------------------------------------------------------
GPL license.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org

This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
--------------------------------------------------------------------------------
A commercial use license is available from Genivia, Inc., contact@genivia.com
--------------------------------------------------------------------------------
## Part of the source for this file is provided by Mozilla:
## http://lxr.mozilla.org/seamonkey/source/security/nss/lib/ckfw/builtins/certdata.txt
## This file was converted to PEM format with tools provided by OpenSSL.
## 
## The contents of this file are subject to the Mozilla Public License Version
## 1.1 (the "License"); you may not use this file except in compliance with
## the License. You may obtain a copy of the License at
## http://www.mozilla.org/MPL/
## 
## Software distributed under the License is distributed on an "AS IS" basis,
## WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
## for the specific language governing rights and limitations under the
## License.
## 
## The Original Code is the Netscape security libraries.
## 
## The Initial Developer of the Original Code is Netscape Communications
## Corporation.  Portions created by Netscape are Copyright (C) 1994-2000
## Netscape Communications Corporation.  All Rights Reserved.
## 
## Contributor(s):
## 
## Alternatively, the contents of this file may be used under the terms of the
## GNU General Public License Version 2 or later (the "GPL"), in which case
## the provisions of the GPL are applicable instead of those above.  If you
## wish to allow use of your version of this file only under the terms of the
## GPL and not to allow others to use your version of this file under the MPL,
## indicate your decision by deleting the provisions above and replace them
## with the notice and other provisions required by the GPL.  If you do not
## delete the provisions above, a recipient may use your version of this file
## under either the MPL or the GPL.
*/

#include "cacerts.h"

#ifdef __cplusplus
extern "C" {
#endif

int soap_ssl_client_cacerts(struct soap *soap)
{ extern const char cacert_pem_data[];
  const char *data = cacert_pem_data;
  const char *next = data;
  int err = SOAP_OK;
  int max = 0;
  unsigned char *der;
  char *buf = NULL;
  X509_STORE *store;
  if ((err = soap_ssl_client_context(soap, SOAP_SSL_DEFAULT, NULL, NULL, NULL, NULL, NULL)))
    return err;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Populating certificate chain\n"));
  store = SSL_CTX_get_cert_store(soap->ctx);
  for (;;)
  { X509 *cert = NULL;
    int len;
    data = strstr(next, "-----BEGIN CERTIFICATE-----");
    if (!data)
      break;
    data += 27;
    next = strstr(data, "-----END CERTIFICATE-----");
    if (!next)
      break;
    len = (next - data + 3) / 4 * 3;
    if (len > max)
    { if (buf)
        SOAP_FREE(soap, buf);
      buf = (char*)SOAP_MALLOC(soap, len);
      max = len;
    }
    soap_base642s(soap, data, buf, (size_t)len, NULL);
    der = (unsigned char*)buf;
    if (!d2i_X509(&cert, &der, len))
      break;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Adding certificate %d bytes\n", len));
    if (X509_STORE_add_cert(store, cert) != 1)
    { char line[80];
      sprintf(line, "At position %lu", (unsigned long)(data - cacert_pem_data));
      err = soap_set_receiver_error(soap, line, "SSL add chain certificate failed in soap_ssl_client_cacerts()", SOAP_SSL_ERROR);
      break;
    }
    data = next + 25;
  }
  if (buf)
    SOAP_FREE(soap, buf);
  return err;
}

const char cacert_pem_data[] = "\
Verisign/RSA Secure Server CA\
=============================\
\
MD5 Fingerprint=74:7B:82:03:43:F0:00:9E:6B:B3:EC:47:BF:85:A5:93\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            02:ad:66:7e:4e:45:fe:5e:57:6f:3c:98:19:5e:dd:c0\
        Signature Algorithm: md2WithRSAEncryption\
        Issuer: C=US, O=RSA Data Security, Inc., OU=Secure Server Certification Authority\
        Validity\
            Not Before: Nov  9 00:00:00 1994 GMT\
            Not After : Jan  7 23:59:59 2010 GMT\
        Subject: C=US, O=RSA Data Security, Inc., OU=Secure Server Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1000 bit)\
                Modulus (1000 bit):\
                    00:92:ce:7a:c1:ae:83:3e:5a:aa:89:83:57:ac:25:\
                    01:76:0c:ad:ae:8e:2c:37:ce:eb:35:78:64:54:03:\
                    e5:84:40:51:c9:bf:8f:08:e2:8a:82:08:d2:16:86:\
                    37:55:e9:b1:21:02:ad:76:68:81:9a:05:a2:4b:c9:\
                    4b:25:66:22:56:6c:88:07:8f:f7:81:59:6d:84:07:\
                    65:70:13:71:76:3e:9b:77:4c:e3:50:89:56:98:48:\
                    b9:1d:a7:29:1a:13:2e:4a:11:59:9c:1e:15:d5:49:\
                    54:2c:73:3a:69:82:b1:97:39:9c:6d:70:67:48:e5:\
                    dd:2d:d6:c8:1e:7b\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: md2WithRSAEncryption\
        65:dd:7e:e1:b2:ec:b0:e2:3a:e0:ec:71:46:9a:19:11:b8:d3:\
        c7:a0:b4:03:40:26:02:3e:09:9c:e1:12:b3:d1:5a:f6:37:a5:\
        b7:61:03:b6:5b:16:69:3b:c6:44:08:0c:88:53:0c:6b:97:49:\
        c7:3e:35:dc:6c:b9:bb:aa:df:5c:bb:3a:2f:93:60:b6:a9:4b:\
        4d:f2:20:f7:cd:5f:7f:64:7b:8e:dc:00:5c:d7:fa:77:ca:39:\
        16:59:6f:0e:ea:d3:b5:83:7f:4d:4d:42:56:76:b4:c9:5f:04:\
        f8:38:f8:eb:d2:5f:75:5f:cd:7b:fc:e5:8e:80:7c:fc:50\
-----BEGIN CERTIFICATE-----\
MIICNDCCAaECEAKtZn5ORf5eV288mBle3cAwDQYJKoZIhvcNAQECBQAwXzELMAkG\
A1UEBhMCVVMxIDAeBgNVBAoTF1JTQSBEYXRhIFNlY3VyaXR5LCBJbmMuMS4wLAYD\
VQQLEyVTZWN1cmUgU2VydmVyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTk0\
MTEwOTAwMDAwMFoXDTEwMDEwNzIzNTk1OVowXzELMAkGA1UEBhMCVVMxIDAeBgNV\
BAoTF1JTQSBEYXRhIFNlY3VyaXR5LCBJbmMuMS4wLAYDVQQLEyVTZWN1cmUgU2Vy\
dmVyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIGbMA0GCSqGSIb3DQEBAQUAA4GJ\
ADCBhQJ+AJLOesGugz5aqomDV6wlAXYMra6OLDfO6zV4ZFQD5YRAUcm/jwjiioII\
0haGN1XpsSECrXZogZoFokvJSyVmIlZsiAeP94FZbYQHZXATcXY+m3dM41CJVphI\
uR2nKRoTLkoRWZweFdVJVCxzOmmCsZc5nG1wZ0jl3S3WyB57AgMBAAEwDQYJKoZI\
hvcNAQECBQADfgBl3X7hsuyw4jrg7HFGmhkRuNPHoLQDQCYCPgmc4RKz0Vr2N6W3\
YQO2WxZpO8ZECAyIUwxrl0nHPjXcbLm7qt9cuzovk2C2qUtN8iD3zV9/ZHuO3ABc\
1/p3yjkWWW8O6tO1g39NTUJWdrTJXwT4OPjr0l91X817/OWOgHz8UA==\
-----END CERTIFICATE-----\
\
ABAecom (sub., Am. Bankers Assn.) Root CA\
=========================================\
\
MD5 Fingerprint=41:B8:07:F7:A8:D1:09:EE:B4:9A:8E:70:4D:FC:1B:78\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            d0:1e:40:90:00:00:46:52:00:00:00:01:00:00:00:04\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, ST=DC, L=Washington, O=ABA.ECOM, INC., CN=ABA.ECOM Root CA/emailAddress=admin@digsigtrust.com\
        Validity\
            Not Before: Jul 12 17:33:53 1999 GMT\
            Not After : Jul  9 17:33:53 2009 GMT\
        Subject: C=US, ST=DC, L=Washington, O=ABA.ECOM, INC., CN=ABA.ECOM Root CA/emailAddress=admin@digsigtrust.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b1:d3:11:e0:79:55:43:07:08:4c:cb:05:42:00:\
                    e2:0d:83:46:3d:e4:93:ba:b6:06:d3:0d:59:bd:3e:\
                    c1:ce:43:67:01:8a:21:a8:ef:bc:cc:d0:a2:cc:b0:\
                    55:96:53:84:66:05:00:da:44:49:80:d8:54:0a:a5:\
                    25:86:94:ed:63:56:ff:70:6c:a3:a1:19:d2:78:be:\
                    68:2a:44:5e:2f:cf:cc:18:5e:47:bc:3a:b1:46:3d:\
                    1e:f0:b9:2c:34:5f:8c:7c:4c:08:29:9d:40:55:eb:\
                    3c:7d:83:de:b5:f0:f7:8a:83:0e:a1:4c:b4:3a:a5:\
                    b3:5f:5a:22:97:ec:19:9b:c1:05:68:fd:e6:b7:a9:\
                    91:94:2c:e4:78:48:24:1a:25:19:3a:eb:95:9c:39:\
                    0a:8a:cf:42:b2:f0:1c:d5:5f:fb:6b:ed:68:56:7b:\
                    39:2c:72:38:b0:ee:93:a9:d3:7b:77:3c:eb:71:03:\
                    a9:38:4a:16:6c:89:2a:ca:da:33:13:79:c2:55:8c:\
                    ed:9c:bb:f2:cb:5b:10:f8:2e:61:35:c6:29:4c:2a:\
                    d0:2a:63:d1:65:59:b4:f8:cd:f9:f4:00:84:b6:57:\
                    42:85:9d:32:a8:f9:2a:54:fb:ff:78:41:bc:bd:71:\
                    28:f4:bb:90:bc:ff:96:34:04:e3:45:9e:a1:46:28:\
                    40:81\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE, pathlen:8\
    Signature Algorithm: sha1WithRSAEncryption\
        04:6f:25:86:e4:e6:96:27:b4:d9:42:c0:d0:c9:00:b1:7f:54:\
        3e:87:b2:6d:24:a9:2f:0a:7e:fd:a4:44:b0:f8:54:07:bd:1b:\
        9d:9d:ca:7b:50:24:7b:11:5b:49:a3:a6:bf:12:74:d5:89:b7:\
        b7:2f:98:64:25:14:b7:61:e9:7f:60:80:6b:d3:64:e8:ab:bd:\
        1a:d6:51:fa:c0:b4:5d:77:1a:7f:64:08:5e:79:c6:05:4c:f1:\
        7a:dd:4d:7d:ce:e6:48:7b:54:d2:61:92:81:d6:1b:d6:00:f0:\
        0e:9e:28:77:a0:4d:88:c7:22:76:19:c3:c7:9e:1b:a6:77:78:\
        f8:5f:9b:56:d1:f0:f2:17:ac:8e:9d:59:e6:1f:fe:57:b6:d9:\
        5e:e1:5d:9f:45:ec:61:68:19:41:e1:b2:20:26:fe:5a:30:76:\
        24:ff:40:72:3c:79:9f:7c:22:48:ab:46:cd:db:b3:86:2c:8f:\
        bf:05:41:d3:c1:e3:14:e3:41:17:26:d0:7c:a7:71:4c:19:e8:\
        4a:0f:72:58:31:7d:ec:60:7a:a3:22:28:bd:19:24:60:3f:3b:\
        87:73:c0:6b:e4:cb:ae:b7:ab:25:43:b2:55:2d:7b:ab:06:0e:\
        75:5d:34:e5:5d:73:6d:9e:b2:75:40:a5:59:c9:4f:31:71:88:\
        d9:88:7f:54\
-----BEGIN CERTIFICATE-----\
MIIDtTCCAp2gAwIBAgIRANAeQJAAAEZSAAAAAQAAAAQwDQYJKoZIhvcNAQEFBQAw\
gYkxCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJEQzETMBEGA1UEBxMKV2FzaGluZ3Rv\
bjEXMBUGA1UEChMOQUJBLkVDT00sIElOQy4xGTAXBgNVBAMTEEFCQS5FQ09NIFJv\
b3QgQ0ExJDAiBgkqhkiG9w0BCQEWFWFkbWluQGRpZ3NpZ3RydXN0LmNvbTAeFw05\
OTA3MTIxNzMzNTNaFw0wOTA3MDkxNzMzNTNaMIGJMQswCQYDVQQGEwJVUzELMAkG\
A1UECBMCREMxEzARBgNVBAcTCldhc2hpbmd0b24xFzAVBgNVBAoTDkFCQS5FQ09N\
LCBJTkMuMRkwFwYDVQQDExBBQkEuRUNPTSBSb290IENBMSQwIgYJKoZIhvcNAQkB\
FhVhZG1pbkBkaWdzaWd0cnVzdC5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAw\
ggEKAoIBAQCx0xHgeVVDBwhMywVCAOINg0Y95JO6tgbTDVm9PsHOQ2cBiiGo77zM\
0KLMsFWWU4RmBQDaREmA2FQKpSWGlO1jVv9wbKOhGdJ4vmgqRF4vz8wYXke8OrFG\
PR7wuSw0X4x8TAgpnUBV6zx9g9618PeKgw6hTLQ6pbNfWiKX7BmbwQVo/ea3qZGU\
LOR4SCQaJRk665WcOQqKz0Ky8BzVX/tr7WhWezkscjiw7pOp03t3POtxA6k4ShZs\
iSrK2jMTecJVjO2cu/LLWxD4LmE1xilMKtAqY9FlWbT4zfn0AIS2V0KFnTKo+SpU\
+/94Qby9cSj0u5C8/5Y0BONFnqFGKECBAgMBAAGjFjAUMBIGA1UdEwEB/wQIMAYB\
Af8CAQgwDQYJKoZIhvcNAQEFBQADggEBAARvJYbk5pYntNlCwNDJALF/VD6Hsm0k\
qS8Kfv2kRLD4VAe9G52dyntQJHsRW0mjpr8SdNWJt7cvmGQlFLdh6X9ggGvTZOir\
vRrWUfrAtF13Gn9kCF55xgVM8XrdTX3O5kh7VNJhkoHWG9YA8A6eKHegTYjHInYZ\
w8eeG6Z3ePhfm1bR8PIXrI6dWeYf/le22V7hXZ9F7GFoGUHhsiAm/lowdiT/QHI8\
eZ98IkirRs3bs4Ysj78FQdPB4xTjQRcm0HyncUwZ6EoPclgxfexgeqMiKL0ZJGA/\
O4dzwGvky663qyVDslUte6sGDnVdNOVdc22esnVApVnJTzFxiNmIf1Q=\
-----END CERTIFICATE-----\
\
Digital Signature Trust Co. Global CA 1\
=======================================\
\
MD5 Fingerprint=25:7A:BA:83:2E:B6:A2:0B:DA:FE:F5:02:0F:08:D7:AD\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 913315222 (0x36701596)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=Digital Signature Trust Co., OU=DSTCA E1\
        Validity\
            Not Before: Dec 10 18:10:23 1998 GMT\
            Not After : Dec 10 18:40:23 2018 GMT\
        Subject: C=US, O=Digital Signature Trust Co., OU=DSTCA E1\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:a0:6c:81:a9:cf:34:1e:24:dd:fe:86:28:cc:de:\
                    83:2f:f9:5e:d4:42:d2:e8:74:60:66:13:98:06:1c:\
                    a9:51:12:69:6f:31:55:b9:49:72:00:08:7e:d3:a5:\
                    62:44:37:24:99:8f:d9:83:48:8f:99:6d:95:13:bb:\
                    43:3b:2e:49:4e:88:37:c1:bb:58:7f:fe:e1:bd:f8:\
                    bb:61:cd:f3:47:c0:99:a6:f1:f3:91:e8:78:7c:00:\
                    cb:61:c9:44:27:71:69:55:4a:7e:49:4d:ed:a2:a3:\
                    be:02:4c:00:ca:02:a8:ee:01:02:31:64:0f:52:2d:\
                    13:74:76:36:b5:7a:b4:2d:71\
                Exponent: 3 (0x3)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 CRL Distribution Points: \
                DirName:/C=US/O=Digital Signature Trust Co./OU=DSTCA E1/CN=CRL1\
\
            X509v3 Private Key Usage Period: \
                Not Before: Dec 10 18:10:23 1998 GMT, Not After: Dec 10 18:10:23 2018 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:6A:79:7E:91:69:46:18:13:0A:02:77:A5:59:5B:60:98:25:0E:A2:F8\
\
            X509v3 Subject Key Identifier: \
                6A:79:7E:91:69:46:18:13:0A:02:77:A5:59:5B:60:98:25:0E:A2:F8\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0\
..V4.0....\
    Signature Algorithm: sha1WithRSAEncryption\
        22:12:d8:7a:1d:dc:81:06:b6:09:65:b2:87:c8:1f:5e:b4:2f:\
        e9:c4:1e:f2:3c:c1:bb:04:90:11:4a:83:4e:7e:93:b9:4d:42:\
        c7:92:26:a0:5c:34:9a:38:72:f8:fd:6b:16:3e:20:ee:82:8b:\
        31:2a:93:36:85:23:88:8a:3c:03:68:d3:c9:09:0f:4d:fc:6c:\
        a4:da:28:72:93:0e:89:80:b0:7d:fe:80:6f:65:6d:18:33:97:\
        8b:c2:6b:89:ee:60:3d:c8:9b:ef:7f:2b:32:62:73:93:cb:3c:\
        e3:7b:e2:76:78:45:bc:a1:93:04:bb:86:9f:3a:5b:43:7a:c3:\
        8a:65\
-----BEGIN CERTIFICATE-----\
MIIDKTCCApKgAwIBAgIENnAVljANBgkqhkiG9w0BAQUFADBGMQswCQYDVQQGEwJV\
UzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMREwDwYDVQQL\
EwhEU1RDQSBFMTAeFw05ODEyMTAxODEwMjNaFw0xODEyMTAxODQwMjNaMEYxCzAJ\
BgNVBAYTAlVTMSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4x\
ETAPBgNVBAsTCERTVENBIEUxMIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQCg\
bIGpzzQeJN3+hijM3oMv+V7UQtLodGBmE5gGHKlREmlvMVW5SXIACH7TpWJENySZ\
j9mDSI+ZbZUTu0M7LklOiDfBu1h//uG9+LthzfNHwJmm8fOR6Hh8AMthyUQncWlV\
Sn5JTe2io74CTADKAqjuAQIxZA9SLRN0dja1erQtcQIBA6OCASQwggEgMBEGCWCG\
SAGG+EIBAQQEAwIABzBoBgNVHR8EYTBfMF2gW6BZpFcwVTELMAkGA1UEBhMCVVMx\
JDAiBgNVBAoTG0RpZ2l0YWwgU2lnbmF0dXJlIFRydXN0IENvLjERMA8GA1UECxMI\
RFNUQ0EgRTExDTALBgNVBAMTBENSTDEwKwYDVR0QBCQwIoAPMTk5ODEyMTAxODEw\
MjNagQ8yMDE4MTIxMDE4MTAyM1owCwYDVR0PBAQDAgEGMB8GA1UdIwQYMBaAFGp5\
fpFpRhgTCgJ3pVlbYJglDqL4MB0GA1UdDgQWBBRqeX6RaUYYEwoCd6VZW2CYJQ6i\
+DAMBgNVHRMEBTADAQH/MBkGCSqGSIb2fQdBAAQMMAobBFY0LjADAgSQMA0GCSqG\
SIb3DQEBBQUAA4GBACIS2Hod3IEGtgllsofIH160L+nEHvI8wbsEkBFKg05+k7lN\
QseSJqBcNJo4cvj9axY+IO6CizEqkzaFI4iKPANo08kJD038bKTaKHKTDomAsH3+\
gG9lbRgzl4vCa4nuYD3Im+9/KzJic5PLPON74nZ4RbyhkwS7hp86W0N6w4pl\
-----END CERTIFICATE-----\
\
Digital Signature Trust Co. Global CA 3\
=======================================\
\
MD5 Fingerprint=93:C2:8E:11:7B:D4:F3:03:19:BD:28:75:13:4A:45:4A\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 913232846 (0x366ed3ce)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=Digital Signature Trust Co., OU=DSTCA E2\
        Validity\
            Not Before: Dec  9 19:17:26 1998 GMT\
            Not After : Dec  9 19:47:26 2018 GMT\
        Subject: C=US, O=Digital Signature Trust Co., OU=DSTCA E2\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:bf:93:8f:17:92:ef:33:13:18:eb:10:7f:4e:16:\
                    bf:ff:06:8f:2a:85:bc:5e:f9:24:a6:24:88:b6:03:\
                    b7:c1:c3:5f:03:5b:d1:6f:ae:7e:42:ea:66:23:b8:\
                    63:83:56:fb:28:2d:e1:38:8b:b4:ee:a8:01:e1:ce:\
                    1c:b6:88:2a:22:46:85:fb:9f:a7:70:a9:47:14:3f:\
                    ce:de:65:f0:a8:71:f7:4f:26:6c:8c:bc:c6:b5:ef:\
                    de:49:27:ff:48:2a:7d:e8:4d:03:cc:c7:b2:52:c6:\
                    17:31:13:3b:b5:4d:db:c8:c4:f6:c3:0f:24:2a:da:\
                    0c:9d:e7:91:5b:80:cd:94:9d\
                Exponent: 3 (0x3)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 CRL Distribution Points: \
                DirName:/C=US/O=Digital Signature Trust Co./OU=DSTCA E2/CN=CRL1\
\
            X509v3 Private Key Usage Period: \
                Not Before: Dec  9 19:17:26 1998 GMT, Not After: Dec  9 19:17:26 2018 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:1E:82:4D:28:65:80:3C:C9:41:6E:AC:35:2E:5A:CB:DE:EE:F8:39:5B\
\
            X509v3 Subject Key Identifier: \
                1E:82:4D:28:65:80:3C:C9:41:6E:AC:35:2E:5A:CB:DE:EE:F8:39:5B\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0\
..V4.0....\
    Signature Algorithm: sha1WithRSAEncryption\
        47:8d:83:ad:62:f2:db:b0:9e:45:22:05:b9:a2:d6:03:0e:38:\
        72:e7:9e:fc:7b:e6:93:b6:9a:a5:a2:94:c8:34:1d:91:d1:c5:\
        d7:f4:0a:25:0f:3d:78:81:9e:0f:b1:67:c4:90:4c:63:dd:5e:\
        a7:e2:ba:9f:f5:f7:4d:a5:31:7b:9c:29:2d:4c:fe:64:3e:ec:\
        b6:53:fe:ea:9b:ed:82:db:74:75:4b:07:79:6e:1e:d8:19:83:\
        73:de:f5:3e:d0:b5:de:e7:4b:68:7d:43:2e:2a:20:e1:7e:a0:\
        78:44:9e:08:f5:98:f9:c7:7f:1b:1b:d6:06:20:02:58:a1:c3:\
        a2:03\
-----BEGIN CERTIFICATE-----\
MIIDKTCCApKgAwIBAgIENm7TzjANBgkqhkiG9w0BAQUFADBGMQswCQYDVQQGEwJV\
UzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMREwDwYDVQQL\
EwhEU1RDQSBFMjAeFw05ODEyMDkxOTE3MjZaFw0xODEyMDkxOTQ3MjZaMEYxCzAJ\
BgNVBAYTAlVTMSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4x\
ETAPBgNVBAsTCERTVENBIEUyMIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQC/\
k48Xku8zExjrEH9OFr//Bo8qhbxe+SSmJIi2A7fBw18DW9Fvrn5C6mYjuGODVvso\
LeE4i7TuqAHhzhy2iCoiRoX7n6dwqUcUP87eZfCocfdPJmyMvMa1795JJ/9IKn3o\
TQPMx7JSxhcxEzu1TdvIxPbDDyQq2gyd55FbgM2UnQIBA6OCASQwggEgMBEGCWCG\
SAGG+EIBAQQEAwIABzBoBgNVHR8EYTBfMF2gW6BZpFcwVTELMAkGA1UEBhMCVVMx\
JDAiBgNVBAoTG0RpZ2l0YWwgU2lnbmF0dXJlIFRydXN0IENvLjERMA8GA1UECxMI\
RFNUQ0EgRTIxDTALBgNVBAMTBENSTDEwKwYDVR0QBCQwIoAPMTk5ODEyMDkxOTE3\
MjZagQ8yMDE4MTIwOTE5MTcyNlowCwYDVR0PBAQDAgEGMB8GA1UdIwQYMBaAFB6C\
TShlgDzJQW6sNS5ay97u+DlbMB0GA1UdDgQWBBQegk0oZYA8yUFurDUuWsve7vg5\
WzAMBgNVHRMEBTADAQH/MBkGCSqGSIb2fQdBAAQMMAobBFY0LjADAgSQMA0GCSqG\
SIb3DQEBBQUAA4GBAEeNg61i8tuwnkUiBbmi1gMOOHLnnvx75pO2mqWilMg0HZHR\
xdf0CiUPPXiBng+xZ8SQTGPdXqfiup/1902lMXucKS1M/mQ+7LZT/uqb7YLbdHVL\
B3luHtgZg3Pe9T7Qtd7nS2h9Qy4qIOF+oHhEngj1mPnHfxsb1gYgAlihw6ID\
-----END CERTIFICATE-----\
\
Digital Signature Trust Co. Global CA 2\
=======================================\
\
MD5 Fingerprint=6C:C9:A7:6E:47:F1:0C:E3:53:3B:78:4C:4D:C2:6A:C5\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            d0:1e:40:8b:00:00:02:7c:00:00:00:02:00:00:00:01\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=us, ST=Utah, L=Salt Lake City, O=Digital Signature Trust Co., OU=DSTCA X1, CN=DST RootCA X1/emailAddress=ca@digsigtrust.com\
        Validity\
            Not Before: Dec  1 18:18:55 1998 GMT\
            Not After : Nov 28 18:18:55 2008 GMT\
        Subject: C=us, ST=Utah, L=Salt Lake City, O=Digital Signature Trust Co., OU=DSTCA X1, CN=DST RootCA X1/emailAddress=ca@digsigtrust.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:d2:c6:26:b6:e7:a5:3d:c1:c4:68:d5:50:6f:53:\
                    c5:6f:49:13:09:b8:af:2c:48:8d:14:6a:a3:17:5f:\
                    5a:f9:d3:2e:75:2f:d8:28:62:d1:93:2f:fc:4d:d4:\
                    ab:87:e5:08:c7:99:e7:92:3f:75:bd:eb:25:b4:15:\
                    c1:9b:19:3d:d2:44:8d:d7:74:20:6d:37:02:8f:69:\
                    93:5b:8a:c4:19:9d:f4:b2:0e:fc:16:6c:b9:b1:05:\
                    92:83:d1:85:2c:60:94:3e:45:55:a0:d9:ab:08:21:\
                    e6:60:e8:3b:74:f2:99:50:51:68:d0:03:2d:b1:80:\
                    be:a3:d8:52:b0:44:cd:43:4a:70:8e:58:85:95:e1:\
                    4e:2c:d6:2d:41:6f:d6:84:e7:c8:98:44:ca:47:db:\
                    2c:24:a5:69:26:cf:6b:b8:27:62:c3:f4:c9:7a:92:\
                    23:ed:13:67:82:ae:45:2e:45:e5:7e:72:3f:85:9d:\
                    94:62:10:e6:3c:91:a1:ad:77:00:e0:15:ec:f3:84:\
                    80:72:7a:8e:6e:60:97:c7:24:59:10:34:83:5b:e1:\
                    a5:a4:69:b6:57:35:1c:78:59:c6:d3:2f:3a:73:67:\
                    ee:94:ca:04:13:05:62:06:70:23:b3:f4:7c:ee:45:\
                    d9:64:0b:5b:49:aa:a4:43:ce:26:c4:44:12:6c:b8:\
                    dd:79\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        a2:37:b2:3f:69:fb:d7:86:79:54:49:31:95:33:2b:f3:d1:09:\
        14:49:62:60:86:a5:b0:11:e2:50:c2:1d:06:57:3e:2d:e8:33:\
        64:be:9b:aa:ad:5f:1b:4d:d4:99:95:a2:8b:9a:c9:62:72:b5:\
        69:ea:d9:58:ab:35:ed:15:a2:43:d6:b6:bc:07:79:65:64:73:\
        7d:d7:79:ca:7b:d5:5a:51:c6:e1:53:04:96:8d:38:cf:a3:17:\
        ac:39:71:6b:01:c3:8b:53:3c:63:e9:ee:79:c0:e4:be:92:32:\
        64:7a:b3:1f:97:94:62:bd:ea:b2:20:15:95:fb:97:f2:78:2f:\
        63:36:40:38:e3:46:0f:1d:dd:ac:95:ca:e7:4b:90:7b:b1:4b:\
        a9:d4:c5:eb:9a:da:aa:d5:a3:94:14:46:8d:2d:1f:f3:3a:d6:\
        93:3a:f6:3e:79:fc:e8:e6:b0:75:ed:ee:3d:c9:70:c7:5d:aa:\
        81:4b:46:25:1c:c7:6c:15:e3:95:4e:0f:aa:32:37:94:0a:17:\
        24:92:13:84:58:d2:63:6f:2b:f7:e6:5b:62:0b:13:17:b0:0d:\
        52:4c:fe:fe:6f:5c:e2:91:6e:1d:fd:a4:62:d7:68:fa:8e:7a:\
        4f:d2:08:da:93:dc:f0:92:11:7a:d0:dc:72:93:0c:73:93:62:\
        85:68:d0:f4\
-----BEGIN CERTIFICATE-----\
MIID2DCCAsACEQDQHkCLAAACfAAAAAIAAAABMA0GCSqGSIb3DQEBBQUAMIGpMQsw\
CQYDVQQGEwJ1czENMAsGA1UECBMEVXRhaDEXMBUGA1UEBxMOU2FsdCBMYWtlIENp\
dHkxJDAiBgNVBAoTG0RpZ2l0YWwgU2lnbmF0dXJlIFRydXN0IENvLjERMA8GA1UE\
CxMIRFNUQ0EgWDExFjAUBgNVBAMTDURTVCBSb290Q0EgWDExITAfBgkqhkiG9w0B\
CQEWEmNhQGRpZ3NpZ3RydXN0LmNvbTAeFw05ODEyMDExODE4NTVaFw0wODExMjgx\
ODE4NTVaMIGpMQswCQYDVQQGEwJ1czENMAsGA1UECBMEVXRhaDEXMBUGA1UEBxMO\
U2FsdCBMYWtlIENpdHkxJDAiBgNVBAoTG0RpZ2l0YWwgU2lnbmF0dXJlIFRydXN0\
IENvLjERMA8GA1UECxMIRFNUQ0EgWDExFjAUBgNVBAMTDURTVCBSb290Q0EgWDEx\
ITAfBgkqhkiG9w0BCQEWEmNhQGRpZ3NpZ3RydXN0LmNvbTCCASIwDQYJKoZIhvcN\
AQEBBQADggEPADCCAQoCggEBANLGJrbnpT3BxGjVUG9TxW9JEwm4ryxIjRRqoxdf\
WvnTLnUv2Chi0ZMv/E3Uq4flCMeZ55I/db3rJbQVwZsZPdJEjdd0IG03Ao9pk1uK\
xBmd9LIO/BZsubEFkoPRhSxglD5FVaDZqwgh5mDoO3TymVBRaNADLbGAvqPYUrBE\
zUNKcI5YhZXhTizWLUFv1oTnyJhEykfbLCSlaSbPa7gnYsP0yXqSI+0TZ4KuRS5F\
5X5yP4WdlGIQ5jyRoa13AOAV7POEgHJ6jm5gl8ckWRA0g1vhpaRptlc1HHhZxtMv\
OnNn7pTKBBMFYgZwI7P0fO5F2WQLW0mqpEPOJsREEmy43XkCAwEAATANBgkqhkiG\
9w0BAQUFAAOCAQEAojeyP2n714Z5VEkxlTMr89EJFEliYIalsBHiUMIdBlc+Legz\
ZL6bqq1fG03UmZWii5rJYnK1aerZWKs17RWiQ9a2vAd5ZWRzfdd5ynvVWlHG4VME\
lo04z6MXrDlxawHDi1M8Y+nuecDkvpIyZHqzH5eUYr3qsiAVlfuX8ngvYzZAOONG\
Dx3drJXK50uQe7FLqdTF65raqtWjlBRGjS0f8zrWkzr2Pnn86Oawde3uPclwx12q\
gUtGJRzHbBXjlU4PqjI3lAoXJJIThFjSY28r9+ZbYgsTF7ANUkz+/m9c4pFuHf2k\
Ytdo+o56T9II2pPc8JIRetDccpMMc5NihWjQ9A==\
-----END CERTIFICATE-----\
\
Digital Signature Trust Co. Global CA 4\
=======================================\
\
MD5 Fingerprint=CD:3B:3D:62:5B:09:B8:09:36:87:9E:12:2F:71:64:BA\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            d0:1e:40:8b:00:00:77:6d:00:00:00:01:00:00:00:04\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=us, ST=Utah, L=Salt Lake City, O=Digital Signature Trust Co., OU=DSTCA X2, CN=DST RootCA X2/emailAddress=ca@digsigtrust.com\
        Validity\
            Not Before: Nov 30 22:46:16 1998 GMT\
            Not After : Nov 27 22:46:16 2008 GMT\
        Subject: C=us, ST=Utah, L=Salt Lake City, O=Digital Signature Trust Co., OU=DSTCA X2, CN=DST RootCA X2/emailAddress=ca@digsigtrust.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:dc:75:f0:8c:c0:75:96:9a:c0:62:1f:26:f7:c4:\
                    e1:9a:ea:e0:56:73:5b:99:cd:01:44:a8:08:b6:d5:\
                    a7:da:1a:04:18:39:92:4a:78:a3:81:c2:f5:77:7a:\
                    50:b4:70:ff:9a:ab:c6:c7:ca:6e:83:4f:42:98:fb:\
                    26:0b:da:dc:6d:d6:a9:99:55:52:67:e9:28:03:92:\
                    dc:e5:b0:05:9a:0f:15:f9:6b:59:72:56:f2:fa:39:\
                    fc:aa:68:ee:0f:1f:10:83:2f:fc:9d:fa:17:96:dd:\
                    82:e3:e6:45:7d:c0:4b:80:44:1f:ed:2c:e0:84:fd:\
                    91:5c:92:54:69:25:e5:62:69:dc:e5:ee:00:52:bd:\
                    33:0b:ad:75:02:85:a7:64:50:2d:c5:19:19:30:c0:\
                    26:db:c9:d3:fd:2e:99:ad:59:b5:0b:4d:d4:41:ae:\
                    85:48:43:59:dc:b7:a8:e2:a2:de:c3:8f:d7:b8:a1:\
                    62:a6:68:50:52:e4:cf:31:a7:94:85:da:9f:46:32:\
                    17:56:e5:f2:eb:66:3d:12:ff:43:db:98:ef:77:cf:\
                    cb:81:8d:34:b1:c6:50:4a:26:d1:e4:3e:41:50:af:\
                    6c:ae:22:34:2e:d5:6b:6e:83:ba:79:b8:76:65:48:\
                    da:09:29:64:63:22:b9:fb:47:76:85:8c:86:44:cb:\
                    09:db\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        b5:36:0e:5d:e1:61:28:5a:11:65:c0:3f:83:03:79:4d:be:28:\
        a6:0b:07:02:52:85:cd:f8:91:d0:10:6c:b5:6a:20:5b:1c:90:\
        d9:30:3c:c6:48:9e:8a:5e:64:f9:a1:71:77:ef:04:27:1f:07:\
        eb:e4:26:f7:73:74:c9:44:18:1a:66:d3:e0:43:af:91:3b:d1:\
        cb:2c:d8:74:54:3a:1c:4d:ca:d4:68:cd:23:7c:1d:10:9e:45:\
        e9:f6:00:6e:a6:cd:19:ff:4f:2c:29:8f:57:4d:c4:77:92:be:\
        e0:4c:09:fb:5d:44:86:66:21:a8:b9:32:a2:56:d5:e9:8c:83:\
        7c:59:3f:c4:f1:0b:e7:9d:ec:9e:bd:9c:18:0e:3e:c2:39:79:\
        28:b7:03:0d:08:cb:c6:e7:d9:01:37:50:10:ec:cc:61:16:40:\
        d4:af:31:74:7b:fc:3f:31:a7:d0:47:73:33:39:1b:cc:4e:6a:\
        d7:49:83:11:06:fe:eb:82:58:33:32:4c:f0:56:ac:1e:9c:2f:\
        56:9a:7b:c1:4a:1c:a5:fd:55:36:ce:fc:96:4d:f4:b0:f0:ec:\
        b7:6c:82:ed:2f:31:99:42:4c:a9:b2:0d:b8:15:5d:f1:df:ba:\
        c9:b5:4a:d4:64:98:b3:26:a9:30:c8:fd:a6:ec:ab:96:21:ad:\
        7f:c2:78:b6\
-----BEGIN CERTIFICATE-----\
MIID2DCCAsACEQDQHkCLAAB3bQAAAAEAAAAEMA0GCSqGSIb3DQEBBQUAMIGpMQsw\
CQYDVQQGEwJ1czENMAsGA1UECBMEVXRhaDEXMBUGA1UEBxMOU2FsdCBMYWtlIENp\
dHkxJDAiBgNVBAoTG0RpZ2l0YWwgU2lnbmF0dXJlIFRydXN0IENvLjERMA8GA1UE\
CxMIRFNUQ0EgWDIxFjAUBgNVBAMTDURTVCBSb290Q0EgWDIxITAfBgkqhkiG9w0B\
CQEWEmNhQGRpZ3NpZ3RydXN0LmNvbTAeFw05ODExMzAyMjQ2MTZaFw0wODExMjcy\
MjQ2MTZaMIGpMQswCQYDVQQGEwJ1czENMAsGA1UECBMEVXRhaDEXMBUGA1UEBxMO\
U2FsdCBMYWtlIENpdHkxJDAiBgNVBAoTG0RpZ2l0YWwgU2lnbmF0dXJlIFRydXN0\
IENvLjERMA8GA1UECxMIRFNUQ0EgWDIxFjAUBgNVBAMTDURTVCBSb290Q0EgWDIx\
ITAfBgkqhkiG9w0BCQEWEmNhQGRpZ3NpZ3RydXN0LmNvbTCCASIwDQYJKoZIhvcN\
AQEBBQADggEPADCCAQoCggEBANx18IzAdZaawGIfJvfE4Zrq4FZzW5nNAUSoCLbV\
p9oaBBg5kkp4o4HC9Xd6ULRw/5qrxsfKboNPQpj7Jgva3G3WqZlVUmfpKAOS3OWw\
BZoPFflrWXJW8vo5/Kpo7g8fEIMv/J36F5bdguPmRX3AS4BEH+0s4IT9kVySVGkl\
5WJp3OXuAFK9MwutdQKFp2RQLcUZGTDAJtvJ0/0uma1ZtQtN1EGuhUhDWdy3qOKi\
3sOP17ihYqZoUFLkzzGnlIXan0YyF1bl8utmPRL/Q9uY73fPy4GNNLHGUEom0eQ+\
QVCvbK4iNC7Va26Dunm4dmVI2gkpZGMiuftHdoWMhkTLCdsCAwEAATANBgkqhkiG\
9w0BAQUFAAOCAQEAtTYOXeFhKFoRZcA/gwN5Tb4opgsHAlKFzfiR0BBstWogWxyQ\
2TA8xkieil5k+aFxd+8EJx8H6+Qm93N0yUQYGmbT4EOvkTvRyyzYdFQ6HE3K1GjN\
I3wdEJ5F6fYAbqbNGf9PLCmPV03Ed5K+4EwJ+11EhmYhqLkyolbV6YyDfFk/xPEL\
553snr2cGA4+wjl5KLcDDQjLxufZATdQEOzMYRZA1K8xdHv8PzGn0EdzMzkbzE5q\
10mDEQb+64JYMzJM8FasHpwvVpp7wUocpf1VNs78lk30sPDst2yC7S8xmUJMqbIN\
uBVd8d+6ybVK1GSYsyapMMj9puyrliGtf8J4tg==\
-----END CERTIFICATE-----\
\
Verisign Class 1 Public Primary Certification Authority\
=======================================================\
\
MD5 Fingerprint=97:60:E8:57:5F:D3:50:47:E5:43:0C:94:36:8A:B0:62\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            cd:ba:7f:56:f0:df:e4:bc:54:fe:22:ac:b3:72:aa:55\
        Signature Algorithm: md2WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 1 Public Primary Certification Authority\
        Validity\
            Not Before: Jan 29 00:00:00 1996 GMT\
            Not After : Aug  1 23:59:59 2028 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=Class 1 Public Primary Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:e5:19:bf:6d:a3:56:61:2d:99:48:71:f6:67:de:\
                    b9:8d:eb:b7:9e:86:80:0a:91:0e:fa:38:25:af:46:\
                    88:82:e5:73:a8:a0:9b:24:5d:0d:1f:cc:65:6e:0c:\
                    b0:d0:56:84:18:87:9a:06:9b:10:a1:73:df:b4:58:\
                    39:6b:6e:c1:f6:15:d5:a8:a8:3f:aa:12:06:8d:31:\
                    ac:7f:b0:34:d7:8f:34:67:88:09:cd:14:11:e2:4e:\
                    45:56:69:1f:78:02:80:da:dc:47:91:29:bb:36:c9:\
                    63:5c:c5:e0:d7:2d:87:7b:a1:b7:32:b0:7b:30:ba:\
                    2a:2f:31:aa:ee:a3:67:da:db\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: md2WithRSAEncryption\
        4c:3f:b8:8b:c6:68:df:ee:43:33:0e:5d:e9:a6:cb:07:84:4d:\
        7a:33:ff:92:1b:f4:36:ad:d8:95:22:36:68:11:6c:7c:42:cc:\
        f3:9c:2e:c4:07:3f:14:b0:0f:4f:ff:90:92:76:f9:e2:bc:4a:\
        e9:8f:cd:a0:80:0a:f7:c5:29:f1:82:22:5d:b8:b1:dd:81:23:\
        a3:7b:25:15:46:30:79:16:f8:ea:05:4b:94:7f:1d:c2:1c:c8:\
        e3:b7:f4:10:40:3c:13:c3:5f:1f:53:e8:48:e4:86:b4:7b:a1:\
        35:b0:7b:25:ba:b8:d3:8e:ab:3f:38:9d:00:34:00:98:f3:d1:\
        71:94\
-----BEGIN CERTIFICATE-----\
MIICPTCCAaYCEQDNun9W8N/kvFT+IqyzcqpVMA0GCSqGSIb3DQEBAgUAMF8xCzAJ\
BgNVBAYTAlVTMRcwFQYDVQQKEw5WZXJpU2lnbiwgSW5jLjE3MDUGA1UECxMuQ2xh\
c3MgMSBQdWJsaWMgUHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTAeFw05\
NjAxMjkwMDAwMDBaFw0yODA4MDEyMzU5NTlaMF8xCzAJBgNVBAYTAlVTMRcwFQYD\
VQQKEw5WZXJpU2lnbiwgSW5jLjE3MDUGA1UECxMuQ2xhc3MgMSBQdWJsaWMgUHJp\
bWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTCBnzANBgkqhkiG9w0BAQEFAAOB\
jQAwgYkCgYEA5Rm/baNWYS2ZSHH2Z965jeu3noaACpEO+jglr0aIguVzqKCbJF0N\
H8xlbgyw0FaEGIeaBpsQoXPftFg5a27B9hXVqKg/qhIGjTGsf7A01480Z4gJzRQR\
4k5FVmkfeAKA2txHkSm7NsljXMXg1y2He6G3MrB7MLoqLzGq7qNn2tsCAwEAATAN\
BgkqhkiG9w0BAQIFAAOBgQBMP7iLxmjf7kMzDl3ppssHhE16M/+SG/Q2rdiVIjZo\
EWx8QszznC7EBz8UsA9P/5CSdvnivErpj82ggAr3xSnxgiJduLHdgSOjeyUVRjB5\
FvjqBUuUfx3CHMjjt/QQQDwTw18fU+hI5Ia0e6E1sHslurjTjqs/OJ0ANACY89Fx\
lA==\
-----END CERTIFICATE-----\
\
Verisign Class 2 Public Primary Certification Authority\
=======================================================\
\
MD5 Fingerprint=B3:9C:25:B1:C3:2E:32:53:80:15:30:9D:4D:02:77:3E\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            2d:1b:fc:4a:17:8d:a3:91:eb:e7:ff:f5:8b:45:be:0b\
        Signature Algorithm: md2WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 2 Public Primary Certification Authority\
        Validity\
            Not Before: Jan 29 00:00:00 1996 GMT\
            Not After : Aug  1 23:59:59 2028 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=Class 2 Public Primary Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:b6:5a:8b:a3:0d:6a:23:83:80:6b:cf:39:87:f4:\
                    21:13:33:06:4c:25:a2:ed:55:12:97:c5:a7:80:b9:\
                    fa:83:c1:20:a0:fa:2f:15:0d:7c:a1:60:6b:7e:79:\
                    2c:fa:06:0f:3a:ae:f6:1b:6f:b1:d2:ff:2f:28:52:\
                    5f:83:7d:4b:c4:7a:b7:f8:66:1f:80:54:fc:b7:c2:\
                    8e:59:4a:14:57:46:d1:9a:93:be:41:91:03:bb:15:\
                    80:93:5c:eb:e7:cc:08:6c:3f:3e:b3:4a:fc:ff:4b:\
                    6c:23:d5:50:82:26:44:19:8e:23:c3:71:ea:19:24:\
                    47:04:9e:75:bf:c8:a6:00:1f\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: md2WithRSAEncryption\
        8a:1b:2b:fa:39:c1:74:d7:5e:d8:19:64:a2:58:4a:2d:37:e0:\
        33:47:0f:ac:ed:f7:aa:db:1e:e4:8b:06:5c:60:27:ca:45:52:\
        ce:16:ef:3f:06:64:e7:94:68:7c:60:33:15:11:69:af:9d:62:\
        8d:a3:03:54:6b:a6:be:e5:ee:05:18:60:04:bf:42:80:fd:d0:\
        a8:a8:1e:01:3b:f7:a3:5c:af:a3:dc:e6:26:80:23:3c:b8:44:\
        74:f7:0a:ae:49:8b:61:78:cc:24:bf:88:8a:a7:0e:ea:73:19:\
        41:fd:4d:03:f0:88:d1:e5:78:8d:a5:2a:4f:f6:97:0d:17:77:\
        ca:d8\
-----BEGIN CERTIFICATE-----\
MIICPDCCAaUCEC0b/EoXjaOR6+f/9YtFvgswDQYJKoZIhvcNAQECBQAwXzELMAkG\
A1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFz\
cyAyIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTk2\
MDEyOTAwMDAwMFoXDTI4MDgwMTIzNTk1OVowXzELMAkGA1UEBhMCVVMxFzAVBgNV\
BAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFzcyAyIFB1YmxpYyBQcmlt\
YXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIGfMA0GCSqGSIb3DQEBAQUAA4GN\
ADCBiQKBgQC2WoujDWojg4BrzzmH9CETMwZMJaLtVRKXxaeAufqDwSCg+i8VDXyh\
YGt+eSz6Bg86rvYbb7HS/y8oUl+DfUvEerf4Zh+AVPy3wo5ZShRXRtGak75BkQO7\
FYCTXOvnzAhsPz6zSvz/S2wj1VCCJkQZjiPDceoZJEcEnnW/yKYAHwIDAQABMA0G\
CSqGSIb3DQEBAgUAA4GBAIobK/o5wXTXXtgZZKJYSi034DNHD6zt96rbHuSLBlxg\
J8pFUs4W7z8GZOeUaHxgMxURaa+dYo2jA1Rrpr7l7gUYYAS/QoD90KioHgE796Nc\
r6Pc5iaAIzy4RHT3Cq5Ji2F4zCS/iIqnDupzGUH9TQPwiNHleI2lKk/2lw0Xd8rY\
-----END CERTIFICATE-----\
\
Verisign Class 3 Public Primary Certification Authority\
=======================================================\
\
MD5 Fingerprint=10:FC:63:5D:F6:26:3E:0D:F3:25:BE:5F:79:CD:67:67\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            70:ba:e4:1d:10:d9:29:34:b6:38:ca:7b:03:cc:ba:bf\
        Signature Algorithm: md2WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 3 Public Primary Certification Authority\
        Validity\
            Not Before: Jan 29 00:00:00 1996 GMT\
            Not After : Aug  1 23:59:59 2028 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=Class 3 Public Primary Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:c9:5c:59:9e:f2:1b:8a:01:14:b4:10:df:04:40:\
                    db:e3:57:af:6a:45:40:8f:84:0c:0b:d1:33:d9:d9:\
                    11:cf:ee:02:58:1f:25:f7:2a:a8:44:05:aa:ec:03:\
                    1f:78:7f:9e:93:b9:9a:00:aa:23:7d:d6:ac:85:a2:\
                    63:45:c7:72:27:cc:f4:4c:c6:75:71:d2:39:ef:4f:\
                    42:f0:75:df:0a:90:c6:8e:20:6f:98:0f:f8:ac:23:\
                    5f:70:29:36:a4:c9:86:e7:b1:9a:20:cb:53:a5:85:\
                    e7:3d:be:7d:9a:fe:24:45:33:dc:76:15:ed:0f:a2:\
                    71:64:4c:65:2e:81:68:45:a7\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: md2WithRSAEncryption\
        bb:4c:12:2b:cf:2c:26:00:4f:14:13:dd:a6:fb:fc:0a:11:84:\
        8c:f3:28:1c:67:92:2f:7c:b6:c5:fa:df:f0:e8:95:bc:1d:8f:\
        6c:2c:a8:51:cc:73:d8:a4:c0:53:f0:4e:d6:26:c0:76:01:57:\
        81:92:5e:21:f1:d1:b1:ff:e7:d0:21:58:cd:69:17:e3:44:1c:\
        9c:19:44:39:89:5c:dc:9c:00:0f:56:8d:02:99:ed:a2:90:45:\
        4c:e4:bb:10:a4:3d:f0:32:03:0e:f1:ce:f8:e8:c9:51:8c:e6:\
        62:9f:e6:9f:c0:7d:b7:72:9c:c9:36:3a:6b:9f:4e:a8:ff:64:\
        0d:64\
-----BEGIN CERTIFICATE-----\
MIICPDCCAaUCEHC65B0Q2Sk0tjjKewPMur8wDQYJKoZIhvcNAQECBQAwXzELMAkG\
A1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFz\
cyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTk2\
MDEyOTAwMDAwMFoXDTI4MDgwMTIzNTk1OVowXzELMAkGA1UEBhMCVVMxFzAVBgNV\
BAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFzcyAzIFB1YmxpYyBQcmlt\
YXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIGfMA0GCSqGSIb3DQEBAQUAA4GN\
ADCBiQKBgQDJXFme8huKARS0EN8EQNvjV69qRUCPhAwL0TPZ2RHP7gJYHyX3KqhE\
BarsAx94f56TuZoAqiN91qyFomNFx3InzPRMxnVx0jnvT0Lwdd8KkMaOIG+YD/is\
I19wKTakyYbnsZogy1Olhec9vn2a/iRFM9x2Fe0PonFkTGUugWhFpwIDAQABMA0G\
CSqGSIb3DQEBAgUAA4GBALtMEivPLCYATxQT3ab7/AoRhIzzKBxnki98tsX63/Do\
lbwdj2wsqFHMc9ikwFPwTtYmwHYBV4GSXiHx0bH/59AhWM1pF+NEHJwZRDmJXNyc\
AA9WjQKZ7aKQRUzkuxCkPfAyAw7xzvjoyVGM5mKf5p/AfbdynMk2OmufTqj/ZA1k\
-----END CERTIFICATE-----\
\
Verisign Class 1 Public Primary Certification Authority - G2\
============================================================\
\
MD5 Fingerprint=DB:23:3D:F9:69:FA:4B:B9:95:80:44:73:5E:7D:41:83\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            4c:c7:ea:aa:98:3e:71:d3:93:10:f8:3d:3a:89:91:92\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 1 Public Primary Certification Authority - G2, OU=(c) 1998 VeriSign, Inc. - For authorized use only, OU=VeriSign Trust Network\
        Validity\
            Not Before: May 18 00:00:00 1998 GMT\
            Not After : Aug  1 23:59:59 2028 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=Class 1 Public Primary Certification Authority - G2, OU=(c) 1998 VeriSign, Inc. - For authorized use only, OU=VeriSign Trust Network\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:aa:d0:ba:be:16:2d:b8:83:d4:ca:d2:0f:bc:76:\
                    31:ca:94:d8:1d:93:8c:56:02:bc:d9:6f:1a:6f:52:\
                    36:6e:75:56:0a:55:d3:df:43:87:21:11:65:8a:7e:\
                    8f:bd:21:de:6b:32:3f:1b:84:34:95:05:9d:41:35:\
                    eb:92:eb:96:dd:aa:59:3f:01:53:6d:99:4f:ed:e5:\
                    e2:2a:5a:90:c1:b9:c4:a6:15:cf:c8:45:eb:a6:5d:\
                    8e:9c:3e:f0:64:24:76:a5:cd:ab:1a:6f:b6:d8:7b:\
                    51:61:6e:a6:7f:87:c8:e2:b7:e5:34:dc:41:88:ea:\
                    09:40:be:73:92:3d:6b:e7:75\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        a9:4f:c3:0d:c7:67:be:2c:cb:d9:a8:cd:2d:75:e7:7e:15:9e:\
        3b:72:eb:7e:eb:5c:2d:09:87:d6:6b:6d:60:7c:e5:ae:c5:90:\
        23:0c:5c:4a:d0:af:b1:5d:f3:c7:b6:0a:db:e0:15:93:0d:dd:\
        03:bc:c7:76:8a:b5:dd:4f:c3:9b:13:75:b8:01:c0:e6:c9:5b:\
        6b:a5:b8:89:dc:ac:a4:dd:72:ed:4e:a1:f7:4f:bc:06:d3:ea:\
        c8:64:74:7b:c2:95:41:9c:65:73:58:f1:90:9a:3c:6a:b1:98:\
        c9:c4:87:bc:cf:45:6d:45:e2:6e:22:3f:fe:bc:0f:31:5c:e8:\
        f2:d9\
-----BEGIN CERTIFICATE-----\
MIIDAjCCAmsCEEzH6qqYPnHTkxD4PTqJkZIwDQYJKoZIhvcNAQEFBQAwgcExCzAJ\
BgNVBAYTAlVTMRcwFQYDVQQKEw5WZXJpU2lnbiwgSW5jLjE8MDoGA1UECxMzQ2xh\
c3MgMSBQdWJsaWMgUHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAtIEcy\
MTowOAYDVQQLEzEoYykgMTk5OCBWZXJpU2lnbiwgSW5jLiAtIEZvciBhdXRob3Jp\
emVkIHVzZSBvbmx5MR8wHQYDVQQLExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMB4X\
DTk4MDUxODAwMDAwMFoXDTI4MDgwMTIzNTk1OVowgcExCzAJBgNVBAYTAlVTMRcw\
FQYDVQQKEw5WZXJpU2lnbiwgSW5jLjE8MDoGA1UECxMzQ2xhc3MgMSBQdWJsaWMg\
UHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAtIEcyMTowOAYDVQQLEzEo\
YykgMTk5OCBWZXJpU2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5\
MR8wHQYDVQQLExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMIGfMA0GCSqGSIb3DQEB\
AQUAA4GNADCBiQKBgQCq0Lq+Fi24g9TK0g+8djHKlNgdk4xWArzZbxpvUjZudVYK\
VdPfQ4chEWWKfo+9Id5rMj8bhDSVBZ1BNeuS65bdqlk/AVNtmU/t5eIqWpDBucSm\
Fc/IReumXY6cPvBkJHalzasab7bYe1FhbqZ/h8jit+U03EGI6glAvnOSPWvndQID\
AQABMA0GCSqGSIb3DQEBBQUAA4GBAKlPww3HZ74sy9mozS11534Vnjty637rXC0J\
h9ZrbWB85a7FkCMMXErQr7Fd88e2CtvgFZMN3QO8x3aKtd1Pw5sTdbgBwObJW2ul\
uIncrKTdcu1OofdPvAbT6shkdHvClUGcZXNY8ZCaPGqxmMnEh7zPRW1F4m4iP/68\
DzFc6PLZ\
-----END CERTIFICATE-----\
\
Verisign Class 2 Public Primary Certification Authority - G2\
============================================================\
\
MD5 Fingerprint=2D:BB:E5:25:D3:D1:65:82:3A:B7:0E:FA:E6:EB:E2:E1\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            b9:2f:60:cc:88:9f:a1:7a:46:09:b8:5b:70:6c:8a:af\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 2 Public Primary Certification Authority - G2, OU=(c) 1998 VeriSign, Inc. - For authorized use only, OU=VeriSign Trust Network\
        Validity\
            Not Before: May 18 00:00:00 1998 GMT\
            Not After : Aug  1 23:59:59 2028 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=Class 2 Public Primary Certification Authority - G2, OU=(c) 1998 VeriSign, Inc. - For authorized use only, OU=VeriSign Trust Network\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:a7:88:01:21:74:2c:e7:1a:03:f0:98:e1:97:3c:\
                    0f:21:08:f1:9c:db:97:e9:9a:fc:c2:04:06:13:be:\
                    5f:52:c8:cc:1e:2c:12:56:2c:b8:01:69:2c:cc:99:\
                    1f:ad:b0:96:ae:79:04:f2:13:39:c1:7b:98:ba:08:\
                    2c:e8:c2:84:13:2c:aa:69:e9:09:f4:c7:a9:02:a4:\
                    42:c2:23:4f:4a:d8:f0:0e:a2:fb:31:6c:c9:e6:6f:\
                    99:27:07:f5:e6:f4:4c:78:9e:6d:eb:46:86:fa:b9:\
                    86:c9:54:f2:b2:c4:af:d4:46:1c:5a:c9:15:30:ff:\
                    0d:6c:f5:2d:0e:6d:ce:7f:77\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        72:2e:f9:7f:d1:f1:71:fb:c4:9e:f6:c5:5e:51:8a:40:98:b8:\
        68:f8:9b:1c:83:d8:e2:9d:bd:ff:ed:a1:e6:66:ea:2f:09:f4:\
        ca:d7:ea:a5:2b:95:f6:24:60:86:4d:44:2e:83:a5:c4:2d:a0:\
        d3:ae:78:69:6f:72:da:6c:ae:08:f0:63:92:37:e6:bb:c4:30:\
        17:ad:77:cc:49:35:aa:cf:d8:8f:d1:be:b7:18:96:47:73:6a:\
        54:22:34:64:2d:b6:16:9b:59:5b:b4:51:59:3a:b3:0b:14:f4:\
        12:df:67:a0:f4:ad:32:64:5e:b1:46:72:27:8c:12:7b:c5:44:\
        b4:ae\
-----BEGIN CERTIFICATE-----\
MIIDAzCCAmwCEQC5L2DMiJ+hekYJuFtwbIqvMA0GCSqGSIb3DQEBBQUAMIHBMQsw\
CQYDVQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xPDA6BgNVBAsTM0Ns\
YXNzIDIgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkgLSBH\
MjE6MDgGA1UECxMxKGMpIDE5OTggVmVyaVNpZ24sIEluYy4gLSBGb3IgYXV0aG9y\
aXplZCB1c2Ugb25seTEfMB0GA1UECxMWVmVyaVNpZ24gVHJ1c3QgTmV0d29yazAe\
Fw05ODA1MTgwMDAwMDBaFw0yODA4MDEyMzU5NTlaMIHBMQswCQYDVQQGEwJVUzEX\
MBUGA1UEChMOVmVyaVNpZ24sIEluYy4xPDA6BgNVBAsTM0NsYXNzIDIgUHVibGlj\
IFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkgLSBHMjE6MDgGA1UECxMx\
KGMpIDE5OTggVmVyaVNpZ24sIEluYy4gLSBGb3IgYXV0aG9yaXplZCB1c2Ugb25s\
eTEfMB0GA1UECxMWVmVyaVNpZ24gVHJ1c3QgTmV0d29yazCBnzANBgkqhkiG9w0B\
AQEFAAOBjQAwgYkCgYEAp4gBIXQs5xoD8JjhlzwPIQjxnNuX6Zr8wgQGE75fUsjM\
HiwSViy4AWkszJkfrbCWrnkE8hM5wXuYuggs6MKEEyyqaekJ9MepAqRCwiNPStjw\
DqL7MWzJ5m+ZJwf15vRMeJ5t60aG+rmGyVTyssSv1EYcWskVMP8NbPUtDm3Of3cC\
AwEAATANBgkqhkiG9w0BAQUFAAOBgQByLvl/0fFx+8Se9sVeUYpAmLho+Jscg9ji\
nb3/7aHmZuovCfTK1+qlK5X2JGCGTUQug6XELaDTrnhpb3LabK4I8GOSN+a7xDAX\
rXfMSTWqz9iP0b63GJZHc2pUIjRkLbYWm1lbtFFZOrMLFPQS32eg9K0yZF6xRnIn\
jBJ7xUS0rg==\
-----END CERTIFICATE-----\
\
GTE CyberTrust Root CA\
======================\
\
MD5 Fingerprint=C4:D7:F0:B2:A3:C5:7D:61:67:F0:04:CD:43:D3:BA:58\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number: 419 (0x1a3)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=US, O=GTE Corporation, CN=GTE CyberTrust Root\
        Validity\
            Not Before: Feb 23 23:01:00 1996 GMT\
            Not After : Feb 23 23:59:00 2006 GMT\
        Subject: C=US, O=GTE Corporation, CN=GTE CyberTrust Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:b8:e6:4f:ba:db:98:7c:71:7c:af:44:b7:d3:0f:\
                    46:d9:64:e5:93:c1:42:8e:c7:ba:49:8d:35:2d:7a:\
                    e7:8b:bd:e5:05:31:59:c6:b1:2f:0a:0c:fb:9f:a7:\
                    3f:a2:09:66:84:56:1e:37:29:1b:87:e9:7e:0c:ca:\
                    9a:9f:a5:7f:f5:15:94:a3:d5:a2:46:82:d8:68:4c:\
                    d1:37:15:06:68:af:bd:f8:b0:b3:f0:29:f5:95:5a:\
                    09:16:61:77:0a:22:25:d4:4f:45:aa:c7:bd:e5:96:\
                    df:f9:d4:a8:8e:42:cc:24:c0:1e:91:27:4a:b5:6d:\
                    06:80:63:39:c4:a2:5e:38:03\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: md5WithRSAEncryption\
        12:b3:75:c6:5f:1d:e1:61:55:80:00:d4:81:4b:7b:31:0f:23:\
        63:e7:3d:f3:03:f9:f4:36:a8:bb:d9:e3:a5:97:4d:ea:2b:29:\
        e0:d6:6a:73:81:e6:c0:89:a3:d3:f1:e0:a5:a5:22:37:9a:63:\
        c2:48:20:b4:db:72:e3:c8:f6:d9:7c:be:b1:af:53:da:14:b4:\
        21:b8:d6:d5:96:e3:fe:4e:0c:59:62:b6:9a:4a:f9:42:dd:8c:\
        6f:81:a9:71:ff:f4:0a:72:6d:6d:44:0e:9d:f3:74:74:a8:d5:\
        34:49:e9:5e:9e:e9:b4:7a:e1:e5:5a:1f:84:30:9c:d3:9f:a5:\
        25:d8\
-----BEGIN CERTIFICATE-----\
MIIB+jCCAWMCAgGjMA0GCSqGSIb3DQEBBAUAMEUxCzAJBgNVBAYTAlVTMRgwFgYD\
VQQKEw9HVEUgQ29ycG9yYXRpb24xHDAaBgNVBAMTE0dURSBDeWJlclRydXN0IFJv\
b3QwHhcNOTYwMjIzMjMwMTAwWhcNMDYwMjIzMjM1OTAwWjBFMQswCQYDVQQGEwJV\
UzEYMBYGA1UEChMPR1RFIENvcnBvcmF0aW9uMRwwGgYDVQQDExNHVEUgQ3liZXJU\
cnVzdCBSb290MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC45k+625h8cXyv\
RLfTD0bZZOWTwUKOx7pJjTUteueLveUFMVnGsS8KDPufpz+iCWaEVh43KRuH6X4M\
ypqfpX/1FZSj1aJGgthoTNE3FQZor734sLPwKfWVWgkWYXcKIiXUT0Wqx73llt/5\
1KiOQswkwB6RJ0q1bQaAYznEol44AwIDAQABMA0GCSqGSIb3DQEBBAUAA4GBABKz\
dcZfHeFhVYAA1IFLezEPI2PnPfMD+fQ2qLvZ46WXTeorKeDWanOB5sCJo9Px4KWl\
IjeaY8JIILTbcuPI9tl8vrGvU9oUtCG41tWW4/5ODFlitppK+ULdjG+BqXH/9Apy\
bW1EDp3zdHSo1TRJ6V6e6bR64eVaH4QwnNOfpSXY\
-----END CERTIFICATE-----\
\
Verisign Class 3 Public Primary Certification Authority - G2\
============================================================\
\
MD5 Fingerprint=A2:33:9B:4C:74:78:73:D4:6C:E7:C1:F3:8D:CB:5C:E9\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            7d:d9:fe:07:cf:a8:1e:b7:10:79:67:fb:a7:89:34:c6\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 3 Public Primary Certification Authority - G2, OU=(c) 1998 VeriSign, Inc. - For authorized use only, OU=VeriSign Trust Network\
        Validity\
            Not Before: May 18 00:00:00 1998 GMT\
            Not After : Aug  1 23:59:59 2028 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=Class 3 Public Primary Certification Authority - G2, OU=(c) 1998 VeriSign, Inc. - For authorized use only, OU=VeriSign Trust Network\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:cc:5e:d1:11:5d:5c:69:d0:ab:d3:b9:6a:4c:99:\
                    1f:59:98:30:8e:16:85:20:46:6d:47:3f:d4:85:20:\
                    84:e1:6d:b3:f8:a4:ed:0c:f1:17:0f:3b:f9:a7:f9:\
                    25:d7:c1:cf:84:63:f2:7c:63:cf:a2:47:f2:c6:5b:\
                    33:8e:64:40:04:68:c1:80:b9:64:1c:45:77:c7:d8:\
                    6e:f5:95:29:3c:50:e8:34:d7:78:1f:a8:ba:6d:43:\
                    91:95:8f:45:57:5e:7e:c5:fb:ca:a4:04:eb:ea:97:\
                    37:54:30:6f:bb:01:47:32:33:cd:dc:57:9b:64:69:\
                    61:f8:9b:1d:1c:89:4f:5c:67\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        51:4d:cd:be:5c:cb:98:19:9c:15:b2:01:39:78:2e:4d:0f:67:\
        70:70:99:c6:10:5a:94:a4:53:4d:54:6d:2b:af:0d:5d:40:8b:\
        64:d3:d7:ee:de:56:61:92:5f:a6:c4:1d:10:61:36:d3:2c:27:\
        3c:e8:29:09:b9:11:64:74:cc:b5:73:9f:1c:48:a9:bc:61:01:\
        ee:e2:17:a6:0c:e3:40:08:3b:0e:e7:eb:44:73:2a:9a:f1:69:\
        92:ef:71:14:c3:39:ac:71:a7:91:09:6f:e4:71:06:b3:ba:59:\
        57:26:79:00:f6:f8:0d:a2:33:30:28:d4:aa:58:a0:9d:9d:69:\
        91:fd\
-----BEGIN CERTIFICATE-----\
MIIDAjCCAmsCEH3Z/gfPqB63EHln+6eJNMYwDQYJKoZIhvcNAQEFBQAwgcExCzAJ\
BgNVBAYTAlVTMRcwFQYDVQQKEw5WZXJpU2lnbiwgSW5jLjE8MDoGA1UECxMzQ2xh\
c3MgMyBQdWJsaWMgUHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAtIEcy\
MTowOAYDVQQLEzEoYykgMTk5OCBWZXJpU2lnbiwgSW5jLiAtIEZvciBhdXRob3Jp\
emVkIHVzZSBvbmx5MR8wHQYDVQQLExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMB4X\
DTk4MDUxODAwMDAwMFoXDTI4MDgwMTIzNTk1OVowgcExCzAJBgNVBAYTAlVTMRcw\
FQYDVQQKEw5WZXJpU2lnbiwgSW5jLjE8MDoGA1UECxMzQ2xhc3MgMyBQdWJsaWMg\
UHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAtIEcyMTowOAYDVQQLEzEo\
YykgMTk5OCBWZXJpU2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5\
MR8wHQYDVQQLExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMIGfMA0GCSqGSIb3DQEB\
AQUAA4GNADCBiQKBgQDMXtERXVxp0KvTuWpMmR9ZmDCOFoUgRm1HP9SFIIThbbP4\
pO0M8RcPO/mn+SXXwc+EY/J8Y8+iR/LGWzOOZEAEaMGAuWQcRXfH2G71lSk8UOg0\
13gfqLptQ5GVj0VXXn7F+8qkBOvqlzdUMG+7AUcyM83cV5tkaWH4mx0ciU9cZwID\
AQABMA0GCSqGSIb3DQEBBQUAA4GBAFFNzb5cy5gZnBWyATl4Lk0PZ3BwmcYQWpSk\
U01UbSuvDV1Ai2TT1+7eVmGSX6bEHRBhNtMsJzzoKQm5EWR0zLVznxxIqbxhAe7i\
F6YM40AIOw7n60RzKprxaZLvcRTDOaxxp5EJb+RxBrO6WVcmeQD2+A2iMzAo1KpY\
oJ2daZH9\
-----END CERTIFICATE-----\
\
Verisign Class 4 Public Primary Certification Authority - G2\
============================================================\
\
MD5 Fingerprint=26:6D:2C:19:98:B6:70:68:38:50:54:19:EC:90:34:60\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            32:88:8e:9a:d2:f5:eb:13:47:f8:7f:c4:20:37:25:f8\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 4 Public Primary Certification Authority - G2, OU=(c) 1998 VeriSign, Inc. - For authorized use only, OU=VeriSign Trust Network\
        Validity\
            Not Before: May 18 00:00:00 1998 GMT\
            Not After : Aug  1 23:59:59 2028 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=Class 4 Public Primary Certification Authority - G2, OU=(c) 1998 VeriSign, Inc. - For authorized use only, OU=VeriSign Trust Network\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:ba:f0:e4:cf:f9:c4:ae:85:54:b9:07:57:f9:8f:\
                    c5:7f:68:11:f8:c4:17:b0:44:dc:e3:30:73:d5:2a:\
                    62:2a:b8:d0:cc:1c:ed:28:5b:7e:bd:6a:dc:b3:91:\
                    24:ca:41:62:3c:fc:02:01:bf:1c:16:31:94:05:97:\
                    76:6e:a2:ad:bd:61:17:6c:4e:30:86:f0:51:37:2a:\
                    50:c7:a8:62:81:dc:5b:4a:aa:c1:a0:b4:6e:eb:2f:\
                    e5:57:c5:b1:2b:40:70:db:5a:4d:a1:8e:1f:bd:03:\
                    1f:d8:03:d4:8f:4c:99:71:bc:e2:82:cc:58:e8:98:\
                    3a:86:d3:86:38:f3:00:29:1f\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        85:8c:12:c1:a7:b9:50:15:7a:cb:3e:ac:b8:43:8a:dc:aa:dd:\
        14:ba:89:81:7e:01:3c:23:71:21:88:2f:82:dc:63:fa:02:45:\
        ac:45:59:d7:2a:58:44:5b:b7:9f:81:3b:92:68:3d:e2:37:24:\
        f5:7b:6c:8f:76:35:96:09:a8:59:9d:b9:ce:23:ab:74:d6:83:\
        fd:32:73:27:d8:69:3e:43:74:f6:ae:c5:89:9a:e7:53:7c:e9:\
        7b:f6:4b:f3:c1:65:83:de:8d:8a:9c:3c:88:8d:39:59:fc:aa:\
        3f:22:8d:a1:c1:66:50:81:72:4c:ed:22:64:4f:4f:ca:80:91:\
        b6:29\
-----BEGIN CERTIFICATE-----\
MIIDAjCCAmsCEDKIjprS9esTR/h/xCA3JfgwDQYJKoZIhvcNAQEFBQAwgcExCzAJ\
BgNVBAYTAlVTMRcwFQYDVQQKEw5WZXJpU2lnbiwgSW5jLjE8MDoGA1UECxMzQ2xh\
c3MgNCBQdWJsaWMgUHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAtIEcy\
MTowOAYDVQQLEzEoYykgMTk5OCBWZXJpU2lnbiwgSW5jLiAtIEZvciBhdXRob3Jp\
emVkIHVzZSBvbmx5MR8wHQYDVQQLExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMB4X\
DTk4MDUxODAwMDAwMFoXDTI4MDgwMTIzNTk1OVowgcExCzAJBgNVBAYTAlVTMRcw\
FQYDVQQKEw5WZXJpU2lnbiwgSW5jLjE8MDoGA1UECxMzQ2xhc3MgNCBQdWJsaWMg\
UHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAtIEcyMTowOAYDVQQLEzEo\
YykgMTk5OCBWZXJpU2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5\
MR8wHQYDVQQLExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMIGfMA0GCSqGSIb3DQEB\
AQUAA4GNADCBiQKBgQC68OTP+cSuhVS5B1f5j8V/aBH4xBewRNzjMHPVKmIquNDM\
HO0oW369atyzkSTKQWI8/AIBvxwWMZQFl3Zuoq29YRdsTjCG8FE3KlDHqGKB3FtK\
qsGgtG7rL+VXxbErQHDbWk2hjh+9Ax/YA9SPTJlxvOKCzFjomDqG04Y48wApHwID\
AQABMA0GCSqGSIb3DQEBBQUAA4GBAIWMEsGnuVAVess+rLhDityq3RS6iYF+ATwj\
cSGIL4LcY/oCRaxFWdcqWERbt5+BO5JoPeI3JPV7bI92NZYJqFmduc4jq3TWg/0y\
cyfYaT5DdPauxYma51N86Xv2S/PBZYPejYqcPIiNOVn8qj8ijaHBZlCBckztImRP\
T8qAkbYp\
-----END CERTIFICATE-----\
\
GlobalSign Root CA\
==================\
\
MD5 Fingerprint=AB:BF:EA:E3:6B:29:A6:CC:A6:78:35:99:EF:AD:2B:80\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            02:00:00:00:00:00:d6:78:b7:94:05\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=BE, O=GlobalSign nv-sa, OU=Root CA, CN=GlobalSign Root CA\
        Validity\
            Not Before: Sep  1 12:00:00 1998 GMT\
            Not After : Jan 28 12:00:00 2014 GMT\
        Subject: C=BE, O=GlobalSign nv-sa, OU=Root CA, CN=GlobalSign Root CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:da:0e:e6:99:8d:ce:a3:e3:4f:8a:7e:fb:f1:8b:\
                    83:25:6b:ea:48:1f:f1:2a:b0:b9:95:11:04:bd:f0:\
                    63:d1:e2:67:66:cf:1c:dd:cf:1b:48:2b:ee:8d:89:\
                    8e:9a:af:29:80:65:ab:e9:c7:2d:12:cb:ab:1c:4c:\
                    70:07:a1:3d:0a:30:cd:15:8d:4f:f8:dd:d4:8c:50:\
                    15:1c:ef:50:ee:c4:2e:f7:fc:e9:52:f2:91:7d:e0:\
                    6d:d5:35:30:8e:5e:43:73:f2:41:e9:d5:6a:e3:b2:\
                    89:3a:56:39:38:6f:06:3c:88:69:5b:2a:4d:c5:a7:\
                    54:b8:6c:89:cc:9b:f9:3c:ca:e5:fd:89:f5:12:3c:\
                    92:78:96:d6:dc:74:6e:93:44:61:d1:8d:c7:46:b2:\
                    75:0e:86:e8:19:8a:d5:6d:6c:d5:78:16:95:a2:e9:\
                    c8:0a:38:eb:f2:24:13:4f:73:54:93:13:85:3a:1b:\
                    bc:1e:34:b5:8b:05:8c:b9:77:8b:b1:db:1f:20:91:\
                    ab:09:53:6e:90:ce:7b:37:74:b9:70:47:91:22:51:\
                    63:16:79:ae:b1:ae:41:26:08:c8:19:2b:d1:46:aa:\
                    48:d6:64:2a:d7:83:34:ff:2c:2a:c1:6c:19:43:4a:\
                    07:85:e7:d3:7c:f6:21:68:ef:ea:f2:52:9f:7f:93:\
                    90:cf\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Subject Key Identifier: \
                60:7B:66:1A:45:0D:97:CA:89:50:2F:7D:04:CD:34:A8:FF:FC:FD:4B\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
    Signature Algorithm: md5WithRSAEncryption\
        ae:aa:9f:fc:b7:d2:cb:1f:5f:39:29:28:18:9e:34:c9:6c:4f:\
        6f:1a:f0:64:a2:70:4a:4f:13:86:9b:60:28:9e:e8:81:49:98:\
        7d:0a:bb:e5:b0:9d:3d:36:db:8f:05:51:ff:09:31:2a:1f:dd:\
        89:77:9e:0f:2e:6c:95:04:ed:86:cb:b4:00:3f:84:02:4d:80:\
        6a:2a:2d:78:0b:ae:6f:2b:a2:83:44:83:1f:cd:50:82:4c:24:\
        af:bd:f7:a5:b4:c8:5a:0f:f4:e7:47:5e:49:8e:37:96:fe:9a:\
        88:05:3a:d9:c0:db:29:87:e6:19:96:47:a7:3a:a6:8c:8b:3c:\
        77:fe:46:63:a7:53:da:21:d1:ac:7e:49:a2:4b:e6:c3:67:59:\
        2f:b3:8a:0e:bb:2c:bd:a9:aa:42:7c:35:c1:d8:7f:d5:a7:31:\
        3a:4e:63:43:39:af:08:b0:61:34:8c:d3:98:a9:43:34:f6:0f:\
        87:29:3b:9d:c2:56:58:98:77:c3:f7:1b:ac:f6:9d:f8:3e:aa:\
        a7:54:45:f0:f5:f9:d5:31:65:fe:6b:58:9c:71:b3:1e:d7:52:\
        ea:32:17:fc:40:60:1d:c9:79:24:b2:f6:6c:fd:a8:66:0e:82:\
        dd:98:cb:da:c2:44:4f:2e:a0:7b:f2:f7:6b:2c:76:11:84:46:\
        8a:78:a3:e3\
-----BEGIN CERTIFICATE-----\
MIIDdTCCAl2gAwIBAgILAgAAAAAA1ni3lAUwDQYJKoZIhvcNAQEEBQAwVzELMAkG\
A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\
b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\
MDBaFw0xNDAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\
YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\
aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\
jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\
xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\
1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\
snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\
U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\
9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIABjAdBgNVHQ4EFgQU\
YHtmGkUNl8qJUC99BM00qP/8/UswDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0B\
AQQFAAOCAQEArqqf/LfSyx9fOSkoGJ40yWxPbxrwZKJwSk8ThptgKJ7ogUmYfQq7\
5bCdPTbbjwVR/wkxKh/diXeeDy5slQTthsu0AD+EAk2AaioteAuubyuig0SDH81Q\
gkwkr733pbTIWg/050deSY43lv6aiAU62cDbKYfmGZZHpzqmjIs8d/5GY6dT2iHR\
rH5Jokvmw2dZL7OKDrssvamqQnw1wdh/1acxOk5jQzmvCLBhNIzTmKlDNPYPhyk7\
ncJWWJh3w/cbrPad+D6qp1RF8PX51TFl/mtYnHGzHtdS6jIX/EBgHcl5JLL2bP2o\
Zg6C3ZjL2sJETy6ge/L3ayx2EYRGinij4w==\
-----END CERTIFICATE-----\
\
ValiCert Class 1 VA\
===================\
\
MD5 Fingerprint=65:58:AB:15:AD:57:6C:1E:A8:A7:B5:69:AC:BF:FF:EB\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: L=ValiCert Validation Network, O=ValiCert, Inc., OU=ValiCert Class 1 Policy Validation Authority, CN=http://www.valicert.com//emailAddress=info@valicert.com\
        Validity\
            Not Before: Jun 25 22:23:48 1999 GMT\
            Not After : Jun 25 22:23:48 2019 GMT\
        Subject: L=ValiCert Validation Network, O=ValiCert, Inc., OU=ValiCert Class 1 Policy Validation Authority, CN=http://www.valicert.com//emailAddress=info@valicert.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:d8:59:82:7a:89:b8:96:ba:a6:2f:68:6f:58:2e:\
                    a7:54:1c:06:6e:f4:ea:8d:48:bc:31:94:17:f0:f3:\
                    4e:bc:b2:b8:35:92:76:b0:d0:a5:a5:01:d7:00:03:\
                    12:22:19:08:f8:ff:11:23:9b:ce:07:f5:bf:69:1a:\
                    26:fe:4e:e9:d1:7f:9d:2c:40:1d:59:68:6e:a6:f8:\
                    58:b0:9d:1a:8f:d3:3f:f1:dc:19:06:81:a8:0e:e0:\
                    3a:dd:c8:53:45:09:06:e6:0f:70:c3:fa:40:a6:0e:\
                    e2:56:05:0f:18:4d:fc:20:82:d1:73:55:74:8d:76:\
                    72:a0:1d:9d:1d:c0:dd:3f:71\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        50:68:3d:49:f4:2c:1c:06:94:df:95:60:7f:96:7b:17:fe:4f:\
        71:ad:64:c8:dd:77:d2:ef:59:55:e8:3f:e8:8e:05:2a:21:f2:\
        07:d2:b5:a7:52:fe:9c:b1:b6:e2:5b:77:17:40:ea:72:d6:23:\
        cb:28:81:32:c3:00:79:18:ec:59:17:89:c9:c6:6a:1e:71:c9:\
        fd:b7:74:a5:25:45:69:c5:48:ab:19:e1:45:8a:25:6b:19:ee:\
        e5:bb:12:f5:7f:f7:a6:8d:51:c3:f0:9d:74:b7:a9:3e:a0:a5:\
        ff:b6:49:03:13:da:22:cc:ed:71:82:2b:99:cf:3a:b7:f5:2d:\
        72:c8\
-----BEGIN CERTIFICATE-----\
MIIC5zCCAlACAQEwDQYJKoZIhvcNAQEFBQAwgbsxJDAiBgNVBAcTG1ZhbGlDZXJ0\
IFZhbGlkYXRpb24gTmV0d29yazEXMBUGA1UEChMOVmFsaUNlcnQsIEluYy4xNTAz\
BgNVBAsTLFZhbGlDZXJ0IENsYXNzIDEgUG9saWN5IFZhbGlkYXRpb24gQXV0aG9y\
aXR5MSEwHwYDVQQDExhodHRwOi8vd3d3LnZhbGljZXJ0LmNvbS8xIDAeBgkqhkiG\
9w0BCQEWEWluZm9AdmFsaWNlcnQuY29tMB4XDTk5MDYyNTIyMjM0OFoXDTE5MDYy\
NTIyMjM0OFowgbsxJDAiBgNVBAcTG1ZhbGlDZXJ0IFZhbGlkYXRpb24gTmV0d29y\
azEXMBUGA1UEChMOVmFsaUNlcnQsIEluYy4xNTAzBgNVBAsTLFZhbGlDZXJ0IENs\
YXNzIDEgUG9saWN5IFZhbGlkYXRpb24gQXV0aG9yaXR5MSEwHwYDVQQDExhodHRw\
Oi8vd3d3LnZhbGljZXJ0LmNvbS8xIDAeBgkqhkiG9w0BCQEWEWluZm9AdmFsaWNl\
cnQuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDYWYJ6ibiWuqYvaG9Y\
LqdUHAZu9OqNSLwxlBfw8068srg1knaw0KWlAdcAAxIiGQj4/xEjm84H9b9pGib+\
TunRf50sQB1ZaG6m+FiwnRqP0z/x3BkGgagO4DrdyFNFCQbmD3DD+kCmDuJWBQ8Y\
TfwggtFzVXSNdnKgHZ0dwN0/cQIDAQABMA0GCSqGSIb3DQEBBQUAA4GBAFBoPUn0\
LBwGlN+VYH+Wexf+T3GtZMjdd9LvWVXoP+iOBSoh8gfStadS/pyxtuJbdxdA6nLW\
I8sogTLDAHkY7FkXicnGah5xyf23dKUlRWnFSKsZ4UWKJWsZ7uW7EvV/96aNUcPw\
nXS3qT6gpf+2SQMT2iLM7XGCK5nPOrf1LXLI\
-----END CERTIFICATE-----\
\
ValiCert Class 2 VA\
===================\
\
MD5 Fingerprint=A9:23:75:9B:BA:49:36:6E:31:C2:DB:F2:E7:66:BA:87\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: L=ValiCert Validation Network, O=ValiCert, Inc., OU=ValiCert Class 2 Policy Validation Authority, CN=http://www.valicert.com//emailAddress=info@valicert.com\
        Validity\
            Not Before: Jun 26 00:19:54 1999 GMT\
            Not After : Jun 26 00:19:54 2019 GMT\
        Subject: L=ValiCert Validation Network, O=ValiCert, Inc., OU=ValiCert Class 2 Policy Validation Authority, CN=http://www.valicert.com//emailAddress=info@valicert.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:ce:3a:71:ca:e5:ab:c8:59:92:55:d7:ab:d8:74:\
                    0e:f9:ee:d9:f6:55:47:59:65:47:0e:05:55:dc:eb:\
                    98:36:3c:5c:53:5d:d3:30:cf:38:ec:bd:41:89:ed:\
                    25:42:09:24:6b:0a:5e:b3:7c:dd:52:2d:4c:e6:d4:\
                    d6:7d:5a:59:a9:65:d4:49:13:2d:24:4d:1c:50:6f:\
                    b5:c1:85:54:3b:fe:71:e4:d3:5c:42:f9:80:e0:91:\
                    1a:0a:5b:39:36:67:f3:3f:55:7c:1b:3f:b4:5f:64:\
                    73:34:e3:b4:12:bf:87:64:f8:da:12:ff:37:27:c1:\
                    b3:43:bb:ef:7b:6e:2e:69:f7\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        3b:7f:50:6f:6f:50:94:99:49:62:38:38:1f:4b:f8:a5:c8:3e:\
        a7:82:81:f6:2b:c7:e8:c5:ce:e8:3a:10:82:cb:18:00:8e:4d:\
        bd:a8:58:7f:a1:79:00:b5:bb:e9:8d:af:41:d9:0f:34:ee:21:\
        81:19:a0:32:49:28:f4:c4:8e:56:d5:52:33:fd:50:d5:7e:99:\
        6c:03:e4:c9:4c:fc:cb:6c:ab:66:b3:4a:21:8c:e5:b5:0c:32:\
        3e:10:b2:cc:6c:a1:dc:9a:98:4c:02:5b:f3:ce:b9:9e:a5:72:\
        0e:4a:b7:3f:3c:e6:16:68:f8:be:ed:74:4c:bc:5b:d5:62:1f:\
        43:dd\
-----BEGIN CERTIFICATE-----\
MIIC5zCCAlACAQEwDQYJKoZIhvcNAQEFBQAwgbsxJDAiBgNVBAcTG1ZhbGlDZXJ0\
IFZhbGlkYXRpb24gTmV0d29yazEXMBUGA1UEChMOVmFsaUNlcnQsIEluYy4xNTAz\
BgNVBAsTLFZhbGlDZXJ0IENsYXNzIDIgUG9saWN5IFZhbGlkYXRpb24gQXV0aG9y\
aXR5MSEwHwYDVQQDExhodHRwOi8vd3d3LnZhbGljZXJ0LmNvbS8xIDAeBgkqhkiG\
9w0BCQEWEWluZm9AdmFsaWNlcnQuY29tMB4XDTk5MDYyNjAwMTk1NFoXDTE5MDYy\
NjAwMTk1NFowgbsxJDAiBgNVBAcTG1ZhbGlDZXJ0IFZhbGlkYXRpb24gTmV0d29y\
azEXMBUGA1UEChMOVmFsaUNlcnQsIEluYy4xNTAzBgNVBAsTLFZhbGlDZXJ0IENs\
YXNzIDIgUG9saWN5IFZhbGlkYXRpb24gQXV0aG9yaXR5MSEwHwYDVQQDExhodHRw\
Oi8vd3d3LnZhbGljZXJ0LmNvbS8xIDAeBgkqhkiG9w0BCQEWEWluZm9AdmFsaWNl\
cnQuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDOOnHK5avIWZJV16vY\
dA757tn2VUdZZUcOBVXc65g2PFxTXdMwzzjsvUGJ7SVCCSRrCl6zfN1SLUzm1NZ9\
WlmpZdRJEy0kTRxQb7XBhVQ7/nHk01xC+YDgkRoKWzk2Z/M/VXwbP7RfZHM047QS\
v4dk+NoS/zcnwbNDu+97bi5p9wIDAQABMA0GCSqGSIb3DQEBBQUAA4GBADt/UG9v\
UJSZSWI4OB9L+KXIPqeCgfYrx+jFzug6EILLGACOTb2oWH+heQC1u+mNr0HZDzTu\
IYEZoDJJKPTEjlbVUjP9UNV+mWwD5MlM/Mtsq2azSiGM5bUMMj4QssxsodyamEwC\
W/POuZ6lcg5Ktz885hZo+L7tdEy8W9ViH0Pd\
-----END CERTIFICATE-----\
\
RSA Root Certificate 1\
======================\
\
MD5 Fingerprint=A2:6F:53:B7:EE:40:DB:4A:68:E7:FA:18:D9:10:4B:72\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: L=ValiCert Validation Network, O=ValiCert, Inc., OU=ValiCert Class 3 Policy Validation Authority, CN=http://www.valicert.com//emailAddress=info@valicert.com\
        Validity\
            Not Before: Jun 26 00:22:33 1999 GMT\
            Not After : Jun 26 00:22:33 2019 GMT\
        Subject: L=ValiCert Validation Network, O=ValiCert, Inc., OU=ValiCert Class 3 Policy Validation Authority, CN=http://www.valicert.com//emailAddress=info@valicert.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:e3:98:51:96:1c:e8:d5:b1:06:81:6a:57:c3:72:\
                    75:93:ab:cf:9e:a6:fc:f3:16:52:d6:2d:4d:9f:35:\
                    44:a8:2e:04:4d:07:49:8a:38:29:f5:77:37:e7:b7:\
                    ab:5d:df:36:71:14:99:8f:dc:c2:92:f1:e7:60:92:\
                    97:ec:d8:48:dc:bf:c1:02:20:c6:24:a4:28:4c:30:\
                    5a:76:6d:b1:5c:f3:dd:de:9e:10:71:a1:88:c7:5b:\
                    9b:41:6d:ca:b0:b8:8e:15:ee:ad:33:2b:cf:47:04:\
                    5c:75:71:0a:98:24:98:29:a7:49:59:a5:dd:f8:b7:\
                    43:62:61:f3:d3:e2:d0:55:3f\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        56:bb:02:58:84:67:08:2c:df:1f:db:7b:49:33:f5:d3:67:9d:\
        f4:b4:0a:10:b3:c9:c5:2c:e2:92:6a:71:78:27:f2:70:83:42:\
        d3:3e:cf:a9:54:f4:f1:d8:92:16:8c:d1:04:cb:4b:ab:c9:9f:\
        45:ae:3c:8a:a9:b0:71:33:5d:c8:c5:57:df:af:a8:35:b3:7f:\
        89:87:e9:e8:25:92:b8:7f:85:7a:ae:d6:bc:1e:37:58:2a:67:\
        c9:91:cf:2a:81:3e:ed:c6:39:df:c0:3e:19:9c:19:cc:13:4d:\
        82:41:b5:8c:de:e0:3d:60:08:20:0f:45:7e:6b:a2:7f:a3:8c:\
        15:ee\
-----BEGIN CERTIFICATE-----\
MIIC5zCCAlACAQEwDQYJKoZIhvcNAQEFBQAwgbsxJDAiBgNVBAcTG1ZhbGlDZXJ0\
IFZhbGlkYXRpb24gTmV0d29yazEXMBUGA1UEChMOVmFsaUNlcnQsIEluYy4xNTAz\
BgNVBAsTLFZhbGlDZXJ0IENsYXNzIDMgUG9saWN5IFZhbGlkYXRpb24gQXV0aG9y\
aXR5MSEwHwYDVQQDExhodHRwOi8vd3d3LnZhbGljZXJ0LmNvbS8xIDAeBgkqhkiG\
9w0BCQEWEWluZm9AdmFsaWNlcnQuY29tMB4XDTk5MDYyNjAwMjIzM1oXDTE5MDYy\
NjAwMjIzM1owgbsxJDAiBgNVBAcTG1ZhbGlDZXJ0IFZhbGlkYXRpb24gTmV0d29y\
azEXMBUGA1UEChMOVmFsaUNlcnQsIEluYy4xNTAzBgNVBAsTLFZhbGlDZXJ0IENs\
YXNzIDMgUG9saWN5IFZhbGlkYXRpb24gQXV0aG9yaXR5MSEwHwYDVQQDExhodHRw\
Oi8vd3d3LnZhbGljZXJ0LmNvbS8xIDAeBgkqhkiG9w0BCQEWEWluZm9AdmFsaWNl\
cnQuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDjmFGWHOjVsQaBalfD\
cnWTq8+epvzzFlLWLU2fNUSoLgRNB0mKOCn1dzfnt6td3zZxFJmP3MKS8edgkpfs\
2Ejcv8ECIMYkpChMMFp2bbFc893enhBxoYjHW5tBbcqwuI4V7q0zK89HBFx1cQqY\
JJgpp0lZpd34t0NiYfPT4tBVPwIDAQABMA0GCSqGSIb3DQEBBQUAA4GBAFa7AliE\
Zwgs3x/be0kz9dNnnfS0ChCzycUs4pJqcXgn8nCDQtM+z6lU9PHYkhaM0QTLS6vJ\
n0WuPIqpsHEzXcjFV9+vqDWzf4mH6eglkrh/hXqu1rweN1gqZ8mRzyqBPu3GOd/A\
PhmcGcwTTYJBtYze4D1gCCAPRX5ron+jjBXu\
-----END CERTIFICATE-----\
\
Verisign Class 1 Public Primary Certification Authority - G3\
============================================================\
\
MD5 Fingerprint=B1:47:BC:18:57:D1:18:A0:78:2D:EC:71:E8:2A:95:73\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            8b:5b:75:56:84:54:85:0b:00:cf:af:38:48:ce:b1:a4\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=VeriSign Trust Network, OU=(c) 1999 VeriSign, Inc. - For authorized use only, CN=VeriSign Class 1 Public Primary Certification Authority - G3\
        Validity\
            Not Before: Oct  1 00:00:00 1999 GMT\
            Not After : Jul 16 23:59:59 2036 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=VeriSign Trust Network, OU=(c) 1999 VeriSign, Inc. - For authorized use only, CN=VeriSign Class 1 Public Primary Certification Authority - G3\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:dd:84:d4:b9:b4:f9:a7:d8:f3:04:78:9c:de:3d:\
                    dc:6c:13:16:d9:7a:dd:24:51:66:c0:c7:26:59:0d:\
                    ac:06:08:c2:94:d1:33:1f:f0:83:35:1f:6e:1b:c8:\
                    de:aa:6e:15:4e:54:27:ef:c4:6d:1a:ec:0b:e3:0e:\
                    f0:44:a5:57:c7:40:58:1e:a3:47:1f:71:ec:60:f6:\
                    6d:94:c8:18:39:ed:fe:42:18:56:df:e4:4c:49:10:\
                    78:4e:01:76:35:63:12:36:dd:66:bc:01:04:36:a3:\
                    55:68:d5:a2:36:09:ac:ab:21:26:54:06:ad:3f:ca:\
                    14:e0:ac:ca:ad:06:1d:95:e2:f8:9d:f1:e0:60:ff:\
                    c2:7f:75:2b:4c:cc:da:fe:87:99:21:ea:ba:fe:3e:\
                    54:d7:d2:59:78:db:3c:6e:cf:a0:13:00:1a:b8:27:\
                    a1:e4:be:67:96:ca:a0:c5:b3:9c:dd:c9:75:9e:eb:\
                    30:9a:5f:a3:cd:d9:ae:78:19:3f:23:e9:5c:db:29:\
                    bd:ad:55:c8:1b:54:8c:63:f6:e8:a6:ea:c7:37:12:\
                    5c:a3:29:1e:02:d9:db:1f:3b:b4:d7:0f:56:47:81:\
                    15:04:4a:af:83:27:d1:c5:58:88:c1:dd:f6:aa:a7:\
                    a3:18:da:68:aa:6d:11:51:e1:bf:65:6b:9f:96:76:\
                    d1:3d\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        ab:66:8d:d7:b3:ba:c7:9a:b6:e6:55:d0:05:f1:9f:31:8d:5a:\
        aa:d9:aa:46:26:0f:71:ed:a5:ad:53:56:62:01:47:2a:44:e9:\
        fe:3f:74:0b:13:9b:b9:f4:4d:1b:b2:d1:5f:b2:b6:d2:88:5c:\
        b3:9f:cd:cb:d4:a7:d9:60:95:84:3a:f8:c1:37:1d:61:ca:e7:\
        b0:c5:e5:91:da:54:a6:ac:31:81:ae:97:de:cd:08:ac:b8:c0:\
        97:80:7f:6e:72:a4:e7:69:13:95:65:1f:c4:93:3c:fd:79:8f:\
        04:d4:3e:4f:ea:f7:9e:ce:cd:67:7c:4f:65:02:ff:91:85:54:\
        73:c7:ff:36:f7:86:2d:ec:d0:5e:4f:ff:11:9f:72:06:d6:b8:\
        1a:f1:4c:0d:26:65:e2:44:80:1e:c7:9f:e3:dd:e8:0a:da:ec:\
        a5:20:80:69:68:a1:4f:7e:e1:6b:cf:07:41:fa:83:8e:bc:38:\
        dd:b0:2e:11:b1:6b:b2:42:cc:9a:bc:f9:48:22:79:4a:19:0f:\
        b2:1c:3e:20:74:d9:6a:c3:be:f2:28:78:13:56:79:4f:6d:50:\
        ea:1b:b0:b5:57:b1:37:66:58:23:f3:dc:0f:df:0a:87:c4:ef:\
        86:05:d5:38:14:60:99:a3:4b:de:06:96:71:2c:f2:db:b6:1f:\
        a4:ef:3f:ee\
-----BEGIN CERTIFICATE-----\
MIIEGjCCAwICEQCLW3VWhFSFCwDPrzhIzrGkMA0GCSqGSIb3DQEBBQUAMIHKMQsw\
CQYDVQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xHzAdBgNVBAsTFlZl\
cmlTaWduIFRydXN0IE5ldHdvcmsxOjA4BgNVBAsTMShjKSAxOTk5IFZlcmlTaWdu\
LCBJbmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNlIG9ubHkxRTBDBgNVBAMTPFZlcmlT\
aWduIENsYXNzIDEgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3Jp\
dHkgLSBHMzAeFw05OTEwMDEwMDAwMDBaFw0zNjA3MTYyMzU5NTlaMIHKMQswCQYD\
VQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xHzAdBgNVBAsTFlZlcmlT\
aWduIFRydXN0IE5ldHdvcmsxOjA4BgNVBAsTMShjKSAxOTk5IFZlcmlTaWduLCBJ\
bmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNlIG9ubHkxRTBDBgNVBAMTPFZlcmlTaWdu\
IENsYXNzIDEgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkg\
LSBHMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAN2E1Lm0+afY8wR4\
nN493GwTFtl63SRRZsDHJlkNrAYIwpTRMx/wgzUfbhvI3qpuFU5UJ+/EbRrsC+MO\
8ESlV8dAWB6jRx9x7GD2bZTIGDnt/kIYVt/kTEkQeE4BdjVjEjbdZrwBBDajVWjV\
ojYJrKshJlQGrT/KFOCsyq0GHZXi+J3x4GD/wn91K0zM2v6HmSHquv4+VNfSWXjb\
PG7PoBMAGrgnoeS+Z5bKoMWznN3JdZ7rMJpfo83ZrngZPyPpXNspva1VyBtUjGP2\
6KbqxzcSXKMpHgLZ2x87tNcPVkeBFQRKr4Mn0cVYiMHd9qqnoxjaaKptEVHhv2Vr\
n5Z20T0CAwEAATANBgkqhkiG9w0BAQUFAAOCAQEAq2aN17O6x5q25lXQBfGfMY1a\
qtmqRiYPce2lrVNWYgFHKkTp/j90CxObufRNG7LRX7K20ohcs5/Ny9Sn2WCVhDr4\
wTcdYcrnsMXlkdpUpqwxga6X3s0IrLjAl4B/bnKk52kTlWUfxJM8/XmPBNQ+T+r3\
ns7NZ3xPZQL/kYVUc8f/NveGLezQXk//EZ9yBta4GvFMDSZl4kSAHsef493oCtrs\
pSCAaWihT37ha88HQfqDjrw43bAuEbFrskLMmrz5SCJ5ShkPshw+IHTZasO+8ih4\
E1Z5T21Q6huwtVexN2ZYI/PcD98Kh8TvhgXVOBRgmaNL3gaWcSzy27YfpO8/7g==\
-----END CERTIFICATE-----\
\
Verisign Class 2 Public Primary Certification Authority - G3\
============================================================\
\
MD5 Fingerprint=F8:BE:C4:63:22:C9:A8:46:74:8B:B8:1D:1E:4A:2B:F6\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            61:70:cb:49:8c:5f:98:45:29:e7:b0:a6:d9:50:5b:7a\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=VeriSign Trust Network, OU=(c) 1999 VeriSign, Inc. - For authorized use only, CN=VeriSign Class 2 Public Primary Certification Authority - G3\
        Validity\
            Not Before: Oct  1 00:00:00 1999 GMT\
            Not After : Jul 16 23:59:59 2036 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=VeriSign Trust Network, OU=(c) 1999 VeriSign, Inc. - For authorized use only, CN=VeriSign Class 2 Public Primary Certification Authority - G3\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:af:0a:0d:c2:d5:2c:db:67:b9:2d:e5:94:27:dd:\
                    a5:be:e0:b0:4d:8f:b3:61:56:3c:d6:7c:c3:f4:cd:\
                    3e:86:cb:a2:88:e2:e1:d8:a4:69:c5:b5:e2:bf:c1:\
                    a6:47:50:5e:46:39:8b:d5:96:ba:b5:6f:14:bf:10:\
                    ce:27:13:9e:05:47:9b:31:7a:13:d8:1f:d9:d3:02:\
                    37:8b:ad:2c:47:f0:8e:81:06:a7:0d:30:0c:eb:f7:\
                    3c:0f:20:1d:dc:72:46:ee:a5:02:c8:5b:c3:c9:56:\
                    69:4c:c5:18:c1:91:7b:0b:d5:13:00:9b:bc:ef:c3:\
                    48:3e:46:60:20:85:2a:d5:90:b6:cd:8b:a0:cc:32:\
                    dd:b7:fd:40:55:b2:50:1c:56:ae:cc:8d:77:4d:c7:\
                    20:4d:a7:31:76:ef:68:92:8a:90:1e:08:81:56:b2:\
                    ad:69:a3:52:d0:cb:1c:c4:23:3d:1f:99:fe:4c:e8:\
                    16:63:8e:c6:08:8e:f6:31:f6:d2:fa:e5:76:dd:b5:\
                    1c:92:a3:49:cd:cd:01:cd:68:cd:a9:69:ba:a3:eb:\
                    1d:0d:9c:a4:20:a6:c1:a0:c5:d1:46:4c:17:6d:d2:\
                    ac:66:3f:96:8c:e0:84:d4:36:ff:22:59:c5:f9:11:\
                    60:a8:5f:04:7d:f2:1a:f6:25:42:61:0f:c4:4a:b8:\
                    3e:89\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        34:26:15:3c:c0:8d:4d:43:49:1d:bd:e9:21:92:d7:66:9c:b7:\
        de:c5:b8:d0:e4:5d:5f:76:22:c0:26:f9:84:3a:3a:f9:8c:b5:\
        fb:ec:60:f1:e8:ce:04:b0:c8:dd:a7:03:8f:30:f3:98:df:a4:\
        e6:a4:31:df:d3:1c:0b:46:dc:72:20:3f:ae:ee:05:3c:a4:33:\
        3f:0b:39:ac:70:78:73:4b:99:2b:df:30:c2:54:b0:a8:3b:55:\
        a1:fe:16:28:cd:42:bd:74:6e:80:db:27:44:a7:ce:44:5d:d4:\
        1b:90:98:0d:1e:42:94:b1:00:2c:04:d0:74:a3:02:05:22:63:\
        63:cd:83:b5:fb:c1:6d:62:6b:69:75:fd:5d:70:41:b9:f5:bf:\
        7c:df:be:c1:32:73:22:21:8b:58:81:7b:15:91:7a:ba:e3:64:\
        48:b0:7f:fb:36:25:da:95:d0:f1:24:14:17:dd:18:80:6b:46:\
        23:39:54:f5:8e:62:09:04:1d:94:90:a6:9b:e6:25:e2:42:45:\
        aa:b8:90:ad:be:08:8f:a9:0b:42:18:94:cf:72:39:e1:b1:43:\
        e0:28:cf:b7:e7:5a:6c:13:6b:49:b3:ff:e3:18:7c:89:8b:33:\
        5d:ac:33:d7:a7:f9:da:3a:55:c9:58:10:f9:aa:ef:5a:b6:cf:\
        4b:4b:df:2a\
-----BEGIN CERTIFICATE-----\
MIIEGTCCAwECEGFwy0mMX5hFKeewptlQW3owDQYJKoZIhvcNAQEFBQAwgcoxCzAJ\
BgNVBAYTAlVTMRcwFQYDVQQKEw5WZXJpU2lnbiwgSW5jLjEfMB0GA1UECxMWVmVy\
aVNpZ24gVHJ1c3QgTmV0d29yazE6MDgGA1UECxMxKGMpIDE5OTkgVmVyaVNpZ24s\
IEluYy4gLSBGb3IgYXV0aG9yaXplZCB1c2Ugb25seTFFMEMGA1UEAxM8VmVyaVNp\
Z24gQ2xhc3MgMiBQdWJsaWMgUHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0\
eSAtIEczMB4XDTk5MTAwMTAwMDAwMFoXDTM2MDcxNjIzNTk1OVowgcoxCzAJBgNV\
BAYTAlVTMRcwFQYDVQQKEw5WZXJpU2lnbiwgSW5jLjEfMB0GA1UECxMWVmVyaVNp\
Z24gVHJ1c3QgTmV0d29yazE6MDgGA1UECxMxKGMpIDE5OTkgVmVyaVNpZ24sIElu\
Yy4gLSBGb3IgYXV0aG9yaXplZCB1c2Ugb25seTFFMEMGA1UEAxM8VmVyaVNpZ24g\
Q2xhc3MgMiBQdWJsaWMgUHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAt\
IEczMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArwoNwtUs22e5LeWU\
J92lvuCwTY+zYVY81nzD9M0+hsuiiOLh2KRpxbXiv8GmR1BeRjmL1Za6tW8UvxDO\
JxOeBUebMXoT2B/Z0wI3i60sR/COgQanDTAM6/c8DyAd3HJG7qUCyFvDyVZpTMUY\
wZF7C9UTAJu878NIPkZgIIUq1ZC2zYugzDLdt/1AVbJQHFauzI13TccgTacxdu9o\
koqQHgiBVrKtaaNS0MscxCM9H5n+TOgWY47GCI72MfbS+uV23bUckqNJzc0BzWjN\
qWm6o+sdDZykIKbBoMXRRkwXbdKsZj+WjOCE1Db/IlnF+RFgqF8EffIa9iVCYQ/E\
Srg+iQIDAQABMA0GCSqGSIb3DQEBBQUAA4IBAQA0JhU8wI1NQ0kdvekhktdmnLfe\
xbjQ5F1fdiLAJvmEOjr5jLX77GDx6M4EsMjdpwOPMPOY36TmpDHf0xwLRtxyID+u\
7gU8pDM/CzmscHhzS5kr3zDCVLCoO1Wh/hYozUK9dG6A2ydEp85EXdQbkJgNHkKU\
sQAsBNB0owIFImNjzYO1+8FtYmtpdf1dcEG59b98377BMnMiIYtYgXsVkXq642RI\
sH/7NiXaldDxJBQX3RiAa0YjOVT1jmIJBB2UkKab5iXiQkWquJCtvgiPqQtCGJTP\
cjnhsUPgKM+351psE2tJs//jGHyJizNdrDPXp/naOlXJWBD5qu9ats9LS98q\
-----END CERTIFICATE-----\
\
Verisign Class 3 Public Primary Certification Authority - G3\
============================================================\
\
MD5 Fingerprint=CD:68:B6:A7:C7:C4:CE:75:E0:1D:4F:57:44:61:92:09\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            9b:7e:06:49:a3:3e:62:b9:d5:ee:90:48:71:29:ef:57\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=VeriSign Trust Network, OU=(c) 1999 VeriSign, Inc. - For authorized use only, CN=VeriSign Class 3 Public Primary Certification Authority - G3\
        Validity\
            Not Before: Oct  1 00:00:00 1999 GMT\
            Not After : Jul 16 23:59:59 2036 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=VeriSign Trust Network, OU=(c) 1999 VeriSign, Inc. - For authorized use only, CN=VeriSign Class 3 Public Primary Certification Authority - G3\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:cb:ba:9c:52:fc:78:1f:1a:1e:6f:1b:37:73:bd:\
                    f8:c9:6b:94:12:30:4f:f0:36:47:f5:d0:91:0a:f5:\
                    17:c8:a5:61:c1:16:40:4d:fb:8a:61:90:e5:76:20:\
                    c1:11:06:7d:ab:2c:6e:a6:f5:11:41:8e:fa:2d:ad:\
                    2a:61:59:a4:67:26:4c:d0:e8:bc:52:5b:70:20:04:\
                    58:d1:7a:c9:a4:69:bc:83:17:64:ad:05:8b:bc:d0:\
                    58:ce:8d:8c:f5:eb:f0:42:49:0b:9d:97:27:67:32:\
                    6e:e1:ae:93:15:1c:70:bc:20:4d:2f:18:de:92:88:\
                    e8:6c:85:57:11:1a:e9:7e:e3:26:11:54:a2:45:96:\
                    55:83:ca:30:89:e8:dc:d8:a3:ed:2a:80:3f:7f:79:\
                    65:57:3e:15:20:66:08:2f:95:93:bf:aa:47:2f:a8:\
                    46:97:f0:12:e2:fe:c2:0a:2b:51:e6:76:e6:b7:46:\
                    b7:e2:0d:a6:cc:a8:c3:4c:59:55:89:e6:e8:53:5c:\
                    1c:ea:9d:f0:62:16:0b:a7:c9:5f:0c:f0:de:c2:76:\
                    ce:af:f7:6a:f2:fa:41:a6:a2:33:14:c9:e5:7a:63:\
                    d3:9e:62:37:d5:85:65:9e:0e:e6:53:24:74:1b:5e:\
                    1d:12:53:5b:c7:2c:e7:83:49:3b:15:ae:8a:68:b9:\
                    57:97\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        11:14:96:c1:ab:92:08:f7:3f:2f:c9:b2:fe:e4:5a:9f:64:de:\
        db:21:4f:86:99:34:76:36:57:dd:d0:15:2f:c5:ad:7f:15:1f:\
        37:62:73:3e:d4:e7:5f:ce:17:03:db:35:fa:2b:db:ae:60:09:\
        5f:1e:5f:8f:6e:bb:0b:3d:ea:5a:13:1e:0c:60:6f:b5:c0:b5:\
        23:22:2e:07:0b:cb:a9:74:cb:47:bb:1d:c1:d7:a5:6b:cc:2f:\
        d2:42:fd:49:dd:a7:89:cf:53:ba:da:00:5a:28:bf:82:df:f8:\
        ba:13:1d:50:86:82:fd:8e:30:8f:29:46:b0:1e:3d:35:da:38:\
        62:16:18:4a:ad:e6:b6:51:6c:de:af:62:eb:01:d0:1e:24:fe:\
        7a:8f:12:1a:12:68:b8:fb:66:99:14:14:45:5c:ae:e7:ae:69:\
        17:81:2b:5a:37:c9:5e:2a:f4:c6:e2:a1:5c:54:9b:a6:54:00:\
        cf:f0:f1:c1:c7:98:30:1a:3b:36:16:db:a3:6e:ea:fd:ad:b2:\
        c2:da:ef:02:47:13:8a:c0:f1:b3:31:ad:4f:1c:e1:4f:9c:af:\
        0f:0c:9d:f7:78:0d:d8:f4:35:56:80:da:b7:6d:17:8f:9d:1e:\
        81:64:e1:fe:c5:45:ba:ad:6b:b9:0a:7a:4e:4f:4b:84:ee:4b:\
        f1:7d:dd:11\
-----BEGIN CERTIFICATE-----\
MIIEGjCCAwICEQCbfgZJoz5iudXukEhxKe9XMA0GCSqGSIb3DQEBBQUAMIHKMQsw\
CQYDVQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xHzAdBgNVBAsTFlZl\
cmlTaWduIFRydXN0IE5ldHdvcmsxOjA4BgNVBAsTMShjKSAxOTk5IFZlcmlTaWdu\
LCBJbmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNlIG9ubHkxRTBDBgNVBAMTPFZlcmlT\
aWduIENsYXNzIDMgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3Jp\
dHkgLSBHMzAeFw05OTEwMDEwMDAwMDBaFw0zNjA3MTYyMzU5NTlaMIHKMQswCQYD\
VQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xHzAdBgNVBAsTFlZlcmlT\
aWduIFRydXN0IE5ldHdvcmsxOjA4BgNVBAsTMShjKSAxOTk5IFZlcmlTaWduLCBJ\
bmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNlIG9ubHkxRTBDBgNVBAMTPFZlcmlTaWdu\
IENsYXNzIDMgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkg\
LSBHMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMu6nFL8eB8aHm8b\
N3O9+MlrlBIwT/A2R/XQkQr1F8ilYcEWQE37imGQ5XYgwREGfassbqb1EUGO+i2t\
KmFZpGcmTNDovFJbcCAEWNF6yaRpvIMXZK0Fi7zQWM6NjPXr8EJJC52XJ2cybuGu\
kxUccLwgTS8Y3pKI6GyFVxEa6X7jJhFUokWWVYPKMIno3Nij7SqAP395ZVc+FSBm\
CC+Vk7+qRy+oRpfwEuL+wgorUeZ25rdGt+INpsyow0xZVYnm6FNcHOqd8GIWC6fJ\
Xwzw3sJ2zq/3avL6QaaiMxTJ5Xpj055iN9WFZZ4O5lMkdBteHRJTW8cs54NJOxWu\
imi5V5cCAwEAATANBgkqhkiG9w0BAQUFAAOCAQEAERSWwauSCPc/L8my/uRan2Te\
2yFPhpk0djZX3dAVL8WtfxUfN2JzPtTnX84XA9s1+ivbrmAJXx5fj267Cz3qWhMe\
DGBvtcC1IyIuBwvLqXTLR7sdwdela8wv0kL9Sd2nic9TutoAWii/gt/4uhMdUIaC\
/Y4wjylGsB49Ndo4YhYYSq3mtlFs3q9i6wHQHiT+eo8SGhJouPtmmRQURVyu565p\
F4ErWjfJXir0xuKhXFSbplQAz/DxwceYMBo7Nhbbo27q/a2ywtrvAkcTisDxszGt\
TxzhT5yvDwyd93gN2PQ1VoDat20Xj50egWTh/sVFuq1ruQp6Tk9LhO5L8X3dEQ==\
-----END CERTIFICATE-----\
\
Verisign Class 4 Public Primary Certification Authority - G3\
============================================================\
\
MD5 Fingerprint=DB:C8:F2:27:2E:B1:EA:6A:29:23:5D:FE:56:3E:33:DF\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number:\
            ec:a0:a7:8b:6e:75:6a:01:cf:c4:7c:cc:2f:94:5e:d7\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=VeriSign Trust Network, OU=(c) 1999 VeriSign, Inc. - For authorized use only, CN=VeriSign Class 4 Public Primary Certification Authority - G3\
        Validity\
            Not Before: Oct  1 00:00:00 1999 GMT\
            Not After : Jul 16 23:59:59 2036 GMT\
        Subject: C=US, O=VeriSign, Inc., OU=VeriSign Trust Network, OU=(c) 1999 VeriSign, Inc. - For authorized use only, CN=VeriSign Class 4 Public Primary Certification Authority - G3\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:ad:cb:a5:11:69:c6:59:ab:f1:8f:b5:19:0f:56:\
                    ce:cc:b5:1f:20:e4:9e:26:25:4b:e0:73:65:89:59:\
                    de:d0:83:e4:f5:0f:b5:bb:ad:f1:7c:e8:21:fc:e4:\
                    e8:0c:ee:7c:45:22:19:76:92:b4:13:b7:20:5b:09:\
                    fa:61:ae:a8:f2:a5:8d:85:c2:2a:d6:de:66:36:d2:\
                    9b:02:f4:a8:92:60:7c:9c:69:b4:8f:24:1e:d0:86:\
                    52:f6:32:9c:41:58:1e:22:bd:cd:45:62:95:08:6e:\
                    d0:66:dd:53:a2:cc:f0:10:dc:54:73:8b:04:a1:46:\
                    33:33:5c:17:40:b9:9e:4d:d3:f3:be:55:83:e8:b1:\
                    89:8e:5a:7c:9a:96:22:90:3b:88:25:f2:d2:53:88:\
                    02:0c:0b:78:f2:e6:37:17:4b:30:46:07:e4:80:6d:\
                    a6:d8:96:2e:e8:2c:f8:11:b3:38:0d:66:a6:9b:ea:\
                    c9:23:5b:db:8e:e2:f3:13:8e:1a:59:2d:aa:02:f0:\
                    ec:a4:87:66:dc:c1:3f:f5:d8:b9:f4:ec:82:c6:d2:\
                    3d:95:1d:e5:c0:4f:84:c9:d9:a3:44:28:06:6a:d7:\
                    45:ac:f0:6b:6a:ef:4e:5f:f8:11:82:1e:38:63:34:\
                    66:50:d4:3e:93:73:fa:30:c3:66:ad:ff:93:2d:97:\
                    ef:03\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: sha1WithRSAEncryption\
        8f:fa:25:6b:4f:5b:e4:a4:4e:27:55:ab:22:15:59:3c:ca:b5:\
        0a:d4:4a:db:ab:dd:a1:5f:53:c5:a0:57:39:c2:ce:47:2b:be:\
        3a:c8:56:bf:c2:d9:27:10:3a:b1:05:3c:c0:77:31:bb:3a:d3:\
        05:7b:6d:9a:1c:30:8c:80:cb:93:93:2a:83:ab:05:51:82:02:\
        00:11:67:6b:f3:88:61:47:5f:03:93:d5:5b:0d:e0:f1:d4:a1:\
        32:35:85:b2:3a:db:b0:82:ab:d1:cb:0a:bc:4f:8c:5b:c5:4b:\
        00:3b:1f:2a:82:a6:7e:36:85:dc:7e:3c:67:00:b5:e4:3b:52:\
        e0:a8:eb:5d:15:f9:c6:6d:f0:ad:1d:0e:85:b7:a9:9a:73:14:\
        5a:5b:8f:41:28:c0:d5:e8:2d:4d:a4:5e:cd:aa:d9:ed:ce:dc:\
        d8:d5:3c:42:1d:17:c1:12:5d:45:38:c3:38:f3:fc:85:2e:83:\
        46:48:b2:d7:20:5f:92:36:8f:e7:79:0f:98:5e:99:e8:f0:d0:\
        a4:bb:f5:53:bd:2a:ce:59:b0:af:6e:7f:6c:bb:d2:1e:00:b0:\
        21:ed:f8:41:62:82:b9:d8:b2:c4:bb:46:50:f3:31:c5:8f:01:\
        a8:74:eb:f5:78:27:da:e7:f7:66:43:f3:9e:83:3e:20:aa:c3:\
        35:60:91:ce\
-----BEGIN CERTIFICATE-----\
MIIEGjCCAwICEQDsoKeLbnVqAc/EfMwvlF7XMA0GCSqGSIb3DQEBBQUAMIHKMQsw\
CQYDVQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xHzAdBgNVBAsTFlZl\
cmlTaWduIFRydXN0IE5ldHdvcmsxOjA4BgNVBAsTMShjKSAxOTk5IFZlcmlTaWdu\
LCBJbmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNlIG9ubHkxRTBDBgNVBAMTPFZlcmlT\
aWduIENsYXNzIDQgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3Jp\
dHkgLSBHMzAeFw05OTEwMDEwMDAwMDBaFw0zNjA3MTYyMzU5NTlaMIHKMQswCQYD\
VQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xHzAdBgNVBAsTFlZlcmlT\
aWduIFRydXN0IE5ldHdvcmsxOjA4BgNVBAsTMShjKSAxOTk5IFZlcmlTaWduLCBJ\
bmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNlIG9ubHkxRTBDBgNVBAMTPFZlcmlTaWdu\
IENsYXNzIDQgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkg\
LSBHMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAK3LpRFpxlmr8Y+1\
GQ9Wzsy1HyDkniYlS+BzZYlZ3tCD5PUPtbut8XzoIfzk6AzufEUiGXaStBO3IFsJ\
+mGuqPKljYXCKtbeZjbSmwL0qJJgfJxptI8kHtCGUvYynEFYHiK9zUVilQhu0Gbd\
U6LM8BDcVHOLBKFGMzNcF0C5nk3T875Vg+ixiY5afJqWIpA7iCXy0lOIAgwLePLm\
NxdLMEYH5IBtptiWLugs+BGzOA1mppvqySNb247i8xOOGlktqgLw7KSHZtzBP/XY\
ufTsgsbSPZUd5cBPhMnZo0QoBmrXRazwa2rvTl/4EYIeOGM0ZlDUPpNz+jDDZq3/\
ky2X7wMCAwEAATANBgkqhkiG9w0BAQUFAAOCAQEAj/ola09b5KROJ1WrIhVZPMq1\
CtRK26vdoV9TxaBXOcLORyu+OshWv8LZJxA6sQU8wHcxuzrTBXttmhwwjIDLk5Mq\
g6sFUYICABFna/OIYUdfA5PVWw3g8dShMjWFsjrbsIKr0csKvE+MW8VLADsfKoKm\
fjaF3H48ZwC15DtS4KjrXRX5xm3wrR0OhbepmnMUWluPQSjA1egtTaRezarZ7c7c\
2NU8Qh0XwRJdRTjDOPP8hS6DRkiy1yBfkjaP53kPmF6Z6PDQpLv1U70qzlmwr25/\
bLvSHgCwIe34QWKCudiyxLtGUPMxxY8BqHTr9Xgn2uf3ZkPznoM+IKrDNWCRzg==\
-----END CERTIFICATE-----\
\
GTE CyberTrust Global Root\
==========================\
\
MD5 Fingerprint=CA:3D:D3:68:F1:03:5C:D0:32:FA:B8:2B:59:E8:5A:DB\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number: 421 (0x1a5)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=US, O=GTE Corporation, OU=GTE CyberTrust Solutions, Inc., CN=GTE CyberTrust Global Root\
        Validity\
            Not Before: Aug 13 00:29:00 1998 GMT\
            Not After : Aug 13 23:59:00 2018 GMT\
        Subject: C=US, O=GTE Corporation, OU=GTE CyberTrust Solutions, Inc., CN=GTE CyberTrust Global Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:95:0f:a0:b6:f0:50:9c:e8:7a:c7:88:cd:dd:17:\
                    0e:2e:b0:94:d0:1b:3d:0e:f6:94:c0:8a:94:c7:06:\
                    c8:90:97:c8:b8:64:1a:7a:7e:6c:3c:53:e1:37:28:\
                    73:60:7f:b2:97:53:07:9f:53:f9:6d:58:94:d2:af:\
                    8d:6d:88:67:80:e6:ed:b2:95:cf:72:31:ca:a5:1c:\
                    72:ba:5c:02:e7:64:42:e7:f9:a9:2c:d6:3a:0d:ac:\
                    8d:42:aa:24:01:39:e6:9c:3f:01:85:57:0d:58:87:\
                    45:f8:d3:85:aa:93:69:26:85:70:48:80:3f:12:15:\
                    c7:79:b4:1f:05:2f:3b:62:99\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: md5WithRSAEncryption\
        6d:eb:1b:09:e9:5e:d9:51:db:67:22:61:a4:2a:3c:48:77:e3:\
        a0:7c:a6:de:73:a2:14:03:85:3d:fb:ab:0e:30:c5:83:16:33:\
        81:13:08:9e:7b:34:4e:df:40:c8:74:d7:b9:7d:dc:f4:76:55:\
        7d:9b:63:54:18:e9:f0:ea:f3:5c:b1:d9:8b:42:1e:b9:c0:95:\
        4e:ba:fa:d5:e2:7c:f5:68:61:bf:8e:ec:05:97:5f:5b:b0:d7:\
        a3:85:34:c4:24:a7:0d:0f:95:93:ef:cb:94:d8:9e:1f:9d:5c:\
        85:6d:c7:aa:ae:4f:1f:22:b5:cd:95:ad:ba:a7:cc:f9:ab:0b:\
        7a:7f\
-----BEGIN CERTIFICATE-----\
MIICWjCCAcMCAgGlMA0GCSqGSIb3DQEBBAUAMHUxCzAJBgNVBAYTAlVTMRgwFgYD\
VQQKEw9HVEUgQ29ycG9yYXRpb24xJzAlBgNVBAsTHkdURSBDeWJlclRydXN0IFNv\
bHV0aW9ucywgSW5jLjEjMCEGA1UEAxMaR1RFIEN5YmVyVHJ1c3QgR2xvYmFsIFJv\
b3QwHhcNOTgwODEzMDAyOTAwWhcNMTgwODEzMjM1OTAwWjB1MQswCQYDVQQGEwJV\
UzEYMBYGA1UEChMPR1RFIENvcnBvcmF0aW9uMScwJQYDVQQLEx5HVEUgQ3liZXJU\
cnVzdCBTb2x1dGlvbnMsIEluYy4xIzAhBgNVBAMTGkdURSBDeWJlclRydXN0IEds\
b2JhbCBSb290MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCVD6C28FCc6HrH\
iM3dFw4usJTQGz0O9pTAipTHBsiQl8i4ZBp6fmw8U+E3KHNgf7KXUwefU/ltWJTS\
r41tiGeA5u2ylc9yMcqlHHK6XALnZELn+aks1joNrI1CqiQBOeacPwGFVw1Yh0X4\
04Wqk2kmhXBIgD8SFcd5tB8FLztimQIDAQABMA0GCSqGSIb3DQEBBAUAA4GBAG3r\
GwnpXtlR22ciYaQqPEh346B8pt5zohQDhT37qw4wxYMWM4ETCJ57NE7fQMh017l9\
3PR2VX2bY1QY6fDq81yx2YtCHrnAlU66+tXifPVoYb+O7AWXX1uw16OFNMQkpw0P\
lZPvy5TYnh+dXIVtx6quTx8itc2VrbqnzPmrC3p/\
-----END CERTIFICATE-----\
\
Entrust.net Secure Server CA\
============================\
\
MD5 Fingerprint=DF:F2:80:73:CC:F1:E6:61:73:FC:F5:42:E9:C5:7C:EE\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 927650371 (0x374ad243)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=Entrust.net, OU=www.entrust.net/CPS incorp. by ref. (limits liab.), OU=(c) 1999 Entrust.net Limited, CN=Entrust.net Secure Server Certification Authority\
        Validity\
            Not Before: May 25 16:09:40 1999 GMT\
            Not After : May 25 16:39:40 2019 GMT\
        Subject: C=US, O=Entrust.net, OU=www.entrust.net/CPS incorp. by ref. (limits liab.), OU=(c) 1999 Entrust.net Limited, CN=Entrust.net Secure Server Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:cd:28:83:34:54:1b:89:f3:0f:af:37:91:31:ff:\
                    af:31:60:c9:a8:e8:b2:10:68:ed:9f:e7:93:36:f1:\
                    0a:64:bb:47:f5:04:17:3f:23:47:4d:c5:27:19:81:\
                    26:0c:54:72:0d:88:2d:d9:1f:9a:12:9f:bc:b3:71:\
                    d3:80:19:3f:47:66:7b:8c:35:28:d2:b9:0a:df:24:\
                    da:9c:d6:50:79:81:7a:5a:d3:37:f7:c2:4a:d8:29:\
                    92:26:64:d1:e4:98:6c:3a:00:8a:f5:34:9b:65:f8:\
                    ed:e3:10:ff:fd:b8:49:58:dc:a0:de:82:39:6b:81:\
                    b1:16:19:61:b9:54:b6:e6:43\
                Exponent: 3 (0x3)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 CRL Distribution Points: \
                DirName:/C=US/O=Entrust.net/OU=www.entrust.net/CPS incorp. by ref. (limits liab.)/OU=(c) 1999 Entrust.net Limited/CN=Entrust.net Secure Server Certification Authority/CN=CRL1\
                URI:http://www.entrust.net/CRL/net1.crl\
\
            X509v3 Private Key Usage Period: \
                Not Before: May 25 16:09:40 1999 GMT, Not After: May 25 16:09:40 2019 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:F0:17:62:13:55:3D:B3:FF:0A:00:6B:FB:50:84:97:F3:ED:62:D0:1A\
\
            X509v3 Subject Key Identifier: \
                F0:17:62:13:55:3D:B3:FF:0A:00:6B:FB:50:84:97:F3:ED:62:D0:1A\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0\
..V4.0....\
    Signature Algorithm: sha1WithRSAEncryption\
        90:dc:30:02:fa:64:74:c2:a7:0a:a5:7c:21:8d:34:17:a8:fb:\
        47:0e:ff:25:7c:8d:13:0a:fb:e4:98:b5:ef:8c:f8:c5:10:0d:\
        f7:92:be:f1:c3:d5:d5:95:6a:04:bb:2c:ce:26:36:65:c8:31:\
        c6:e7:ee:3f:e3:57:75:84:7a:11:ef:46:4f:18:f4:d3:98:bb:\
        a8:87:32:ba:72:f6:3c:e2:3d:9f:d7:1d:d9:c3:60:43:8c:58:\
        0e:22:96:2f:62:a3:2c:1f:ba:ad:05:ef:ab:32:78:87:a0:54:\
        73:19:b5:5c:05:f9:52:3e:6d:2d:45:0b:f7:0a:93:ea:ed:06:\
        f9:b2\
-----BEGIN CERTIFICATE-----\
MIIE2DCCBEGgAwIBAgIEN0rSQzANBgkqhkiG9w0BAQUFADCBwzELMAkGA1UEBhMC\
VVMxFDASBgNVBAoTC0VudHJ1c3QubmV0MTswOQYDVQQLEzJ3d3cuZW50cnVzdC5u\
ZXQvQ1BTIGluY29ycC4gYnkgcmVmLiAobGltaXRzIGxpYWIuKTElMCMGA1UECxMc\
KGMpIDE5OTkgRW50cnVzdC5uZXQgTGltaXRlZDE6MDgGA1UEAxMxRW50cnVzdC5u\
ZXQgU2VjdXJlIFNlcnZlciBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTAeFw05OTA1\
MjUxNjA5NDBaFw0xOTA1MjUxNjM5NDBaMIHDMQswCQYDVQQGEwJVUzEUMBIGA1UE\
ChMLRW50cnVzdC5uZXQxOzA5BgNVBAsTMnd3dy5lbnRydXN0Lm5ldC9DUFMgaW5j\
b3JwLiBieSByZWYuIChsaW1pdHMgbGlhYi4pMSUwIwYDVQQLExwoYykgMTk5OSBF\
bnRydXN0Lm5ldCBMaW1pdGVkMTowOAYDVQQDEzFFbnRydXN0Lm5ldCBTZWN1cmUg\
U2VydmVyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIGdMA0GCSqGSIb3DQEBAQUA\
A4GLADCBhwKBgQDNKIM0VBuJ8w+vN5Ex/68xYMmo6LIQaO2f55M28Qpku0f1BBc/\
I0dNxScZgSYMVHINiC3ZH5oSn7yzcdOAGT9HZnuMNSjSuQrfJNqc1lB5gXpa0zf3\
wkrYKZImZNHkmGw6AIr1NJtl+O3jEP/9uElY3KDegjlrgbEWGWG5VLbmQwIBA6OC\
AdcwggHTMBEGCWCGSAGG+EIBAQQEAwIABzCCARkGA1UdHwSCARAwggEMMIHeoIHb\
oIHYpIHVMIHSMQswCQYDVQQGEwJVUzEUMBIGA1UEChMLRW50cnVzdC5uZXQxOzA5\
BgNVBAsTMnd3dy5lbnRydXN0Lm5ldC9DUFMgaW5jb3JwLiBieSByZWYuIChsaW1p\
dHMgbGlhYi4pMSUwIwYDVQQLExwoYykgMTk5OSBFbnRydXN0Lm5ldCBMaW1pdGVk\
MTowOAYDVQQDEzFFbnRydXN0Lm5ldCBTZWN1cmUgU2VydmVyIENlcnRpZmljYXRp\
b24gQXV0aG9yaXR5MQ0wCwYDVQQDEwRDUkwxMCmgJ6AlhiNodHRwOi8vd3d3LmVu\
dHJ1c3QubmV0L0NSTC9uZXQxLmNybDArBgNVHRAEJDAigA8xOTk5MDUyNTE2MDk0\
MFqBDzIwMTkwNTI1MTYwOTQwWjALBgNVHQ8EBAMCAQYwHwYDVR0jBBgwFoAU8Bdi\
E1U9s/8KAGv7UISX8+1i0BowHQYDVR0OBBYEFPAXYhNVPbP/CgBr+1CEl/PtYtAa\
MAwGA1UdEwQFMAMBAf8wGQYJKoZIhvZ9B0EABAwwChsEVjQuMAMCBJAwDQYJKoZI\
hvcNAQEFBQADgYEAkNwwAvpkdMKnCqV8IY00F6j7Rw7/JXyNEwr75Ji174z4xRAN\
95K+8cPV1ZVqBLssziY2ZcgxxufuP+NXdYR6Ee9GTxj005i7qIcyunL2POI9n9cd\
2cNgQ4xYDiKWL2KjLB+6rQXvqzJ4h6BUcxm1XAX5Uj5tLUUL9wqT6u0G+bI=\
-----END CERTIFICATE-----\
\
Entrust.net Secure Personal CA\
==============================\
\
MD5 Fingerprint=0C:41:2F:13:5B:A0:54:F5:96:66:2D:7E:CD:0E:03:F4\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 939758062 (0x380391ee)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=US, O=Entrust.net, OU=www.entrust.net/Client_CA_Info/CPS incorp. by ref. limits liab., OU=(c) 1999 Entrust.net Limited, CN=Entrust.net Client Certification Authority\
        Validity\
            Not Before: Oct 12 19:24:30 1999 GMT\
            Not After : Oct 12 19:54:30 2019 GMT\
        Subject: C=US, O=Entrust.net, OU=www.entrust.net/Client_CA_Info/CPS incorp. by ref. limits liab., OU=(c) 1999 Entrust.net Limited, CN=Entrust.net Client Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:c8:3a:99:5e:31:17:df:ac:27:6f:90:7b:e4:19:\
                    ff:45:a3:34:c2:db:c1:a8:4f:f0:68:ea:84:fd:9f:\
                    75:79:cf:c1:8a:51:94:af:c7:57:03:47:64:9e:ad:\
                    82:1b:5a:da:7f:37:78:47:bb:37:98:12:96:ce:c6:\
                    13:7d:ef:d2:0c:30:51:a9:39:9e:55:f8:fb:b1:e7:\
                    30:de:83:b2:ba:3e:f1:d5:89:3b:3b:85:ba:aa:74:\
                    2c:fe:3f:31:6e:af:91:95:6e:06:d4:07:4d:4b:2c:\
                    56:47:18:04:52:da:0e:10:93:bf:63:90:9b:e1:df:\
                    8c:e6:02:a4:e6:4f:5e:f7:8b\
                Exponent: 3 (0x3)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 CRL Distribution Points: \
                DirName:/C=US/O=Entrust.net/OU=www.entrust.net/Client_CA_Info/CPS incorp. by ref. limits liab./OU=(c) 1999 Entrust.net Limited/CN=Entrust.net Client Certification Authority/CN=CRL1\
                URI:http://www.entrust.net/CRL/Client1.crl\
\
            X509v3 Private Key Usage Period: \
                Not Before: Oct 12 19:24:30 1999 GMT, Not After: Oct 12 19:24:30 2019 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:C4:FB:9C:29:7B:97:CD:4C:96:FC:EE:5B:B3:CA:99:74:8B:95:EA:4C\
\
            X509v3 Subject Key Identifier: \
                C4:FB:9C:29:7B:97:CD:4C:96:FC:EE:5B:B3:CA:99:74:8B:95:EA:4C\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0\
..V4.0....\
    Signature Algorithm: md5WithRSAEncryption\
        3f:ae:8a:f1:d7:66:03:05:9e:3e:fa:ea:1c:46:bb:a4:5b:8f:\
        78:9a:12:48:99:f9:f4:35:de:0c:36:07:02:6b:10:3a:89:14:\
        81:9c:31:a6:7c:b2:41:b2:6a:e7:07:01:a1:4b:f9:9f:25:3b:\
        96:ca:99:c3:3e:a1:51:1c:f3:c3:2e:44:f7:b0:67:46:aa:92:\
        e5:3b:da:1c:19:14:38:30:d5:e2:a2:31:25:2e:f1:ec:45:38:\
        ed:f8:06:58:03:73:62:b0:10:31:8f:40:bf:64:e0:5c:3e:c5:\
        4f:1f:da:12:43:ff:4c:e6:06:26:a8:9b:19:aa:44:3c:76:b2:\
        5c:ec\
-----BEGIN CERTIFICATE-----\
MIIE7TCCBFagAwIBAgIEOAOR7jANBgkqhkiG9w0BAQQFADCByTELMAkGA1UEBhMC\
VVMxFDASBgNVBAoTC0VudHJ1c3QubmV0MUgwRgYDVQQLFD93d3cuZW50cnVzdC5u\
ZXQvQ2xpZW50X0NBX0luZm8vQ1BTIGluY29ycC4gYnkgcmVmLiBsaW1pdHMgbGlh\
Yi4xJTAjBgNVBAsTHChjKSAxOTk5IEVudHJ1c3QubmV0IExpbWl0ZWQxMzAxBgNV\
BAMTKkVudHJ1c3QubmV0IENsaWVudCBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTAe\
Fw05OTEwMTIxOTI0MzBaFw0xOTEwMTIxOTU0MzBaMIHJMQswCQYDVQQGEwJVUzEU\
MBIGA1UEChMLRW50cnVzdC5uZXQxSDBGBgNVBAsUP3d3dy5lbnRydXN0Lm5ldC9D\
bGllbnRfQ0FfSW5mby9DUFMgaW5jb3JwLiBieSByZWYuIGxpbWl0cyBsaWFiLjEl\
MCMGA1UECxMcKGMpIDE5OTkgRW50cnVzdC5uZXQgTGltaXRlZDEzMDEGA1UEAxMq\
RW50cnVzdC5uZXQgQ2xpZW50IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIGdMA0G\
CSqGSIb3DQEBAQUAA4GLADCBhwKBgQDIOpleMRffrCdvkHvkGf9FozTC28GoT/Bo\
6oT9n3V5z8GKUZSvx1cDR2SerYIbWtp/N3hHuzeYEpbOxhN979IMMFGpOZ5V+Pux\
5zDeg7K6PvHViTs7hbqqdCz+PzFur5GVbgbUB01LLFZHGARS2g4Qk79jkJvh34zm\
AqTmT173iwIBA6OCAeAwggHcMBEGCWCGSAGG+EIBAQQEAwIABzCCASIGA1UdHwSC\
ARkwggEVMIHkoIHhoIHepIHbMIHYMQswCQYDVQQGEwJVUzEUMBIGA1UEChMLRW50\
cnVzdC5uZXQxSDBGBgNVBAsUP3d3dy5lbnRydXN0Lm5ldC9DbGllbnRfQ0FfSW5m\
by9DUFMgaW5jb3JwLiBieSByZWYuIGxpbWl0cyBsaWFiLjElMCMGA1UECxMcKGMp\
IDE5OTkgRW50cnVzdC5uZXQgTGltaXRlZDEzMDEGA1UEAxMqRW50cnVzdC5uZXQg\
Q2xpZW50IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MQ0wCwYDVQQDEwRDUkwxMCyg\
KqAohiZodHRwOi8vd3d3LmVudHJ1c3QubmV0L0NSTC9DbGllbnQxLmNybDArBgNV\
HRAEJDAigA8xOTk5MTAxMjE5MjQzMFqBDzIwMTkxMDEyMTkyNDMwWjALBgNVHQ8E\
BAMCAQYwHwYDVR0jBBgwFoAUxPucKXuXzUyW/O5bs8qZdIuV6kwwHQYDVR0OBBYE\
FMT7nCl7l81MlvzuW7PKmXSLlepMMAwGA1UdEwQFMAMBAf8wGQYJKoZIhvZ9B0EA\
BAwwChsEVjQuMAMCBJAwDQYJKoZIhvcNAQEEBQADgYEAP66K8ddmAwWePvrqHEa7\
pFuPeJoSSJn59DXeDDYHAmsQOokUgZwxpnyyQbJq5wcBoUv5nyU7lsqZwz6hURzz\
wy5E97BnRqqS5TvaHBkUODDV4qIxJS7x7EU47fgGWANzYrAQMY9Av2TgXD7FTx/a\
EkP/TOYGJqibGapEPHayXOw=\
-----END CERTIFICATE-----\
\
Entrust.net Premium 2048 Secure Server CA\
=========================================\
\
MD5 Fingerprint=BA:21:EA:20:D6:DD:DB:8F:C1:57:8B:40:AD:A1:FC:FC\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 946059622 (0x3863b966)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: O=Entrust.net, OU=www.entrust.net/CPS_2048 incorp. by ref. (limits liab.), OU=(c) 1999 Entrust.net Limited, CN=Entrust.net Certification Authority (2048)\
        Validity\
            Not Before: Dec 24 17:50:51 1999 GMT\
            Not After : Dec 24 18:20:51 2019 GMT\
        Subject: O=Entrust.net, OU=www.entrust.net/CPS_2048 incorp. by ref. (limits liab.), OU=(c) 1999 Entrust.net Limited, CN=Entrust.net Certification Authority (2048)\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:ad:4d:4b:a9:12:86:b2:ea:a3:20:07:15:16:64:\
                    2a:2b:4b:d1:bf:0b:4a:4d:8e:ed:80:76:a5:67:b7:\
                    78:40:c0:73:42:c8:68:c0:db:53:2b:dd:5e:b8:76:\
                    98:35:93:8b:1a:9d:7c:13:3a:0e:1f:5b:b7:1e:cf:\
                    e5:24:14:1e:b1:81:a9:8d:7d:b8:cc:6b:4b:03:f1:\
                    02:0c:dc:ab:a5:40:24:00:7f:74:94:a1:9d:08:29:\
                    b3:88:0b:f5:87:77:9d:55:cd:e4:c3:7e:d7:6a:64:\
                    ab:85:14:86:95:5b:97:32:50:6f:3d:c8:ba:66:0c:\
                    e3:fc:bd:b8:49:c1:76:89:49:19:fd:c0:a8:bd:89:\
                    a3:67:2f:c6:9f:bc:71:19:60:b8:2d:e9:2c:c9:90:\
                    76:66:7b:94:e2:af:78:d6:65:53:5d:3c:d6:9c:b2:\
                    cf:29:03:f9:2f:a4:50:b2:d4:48:ce:05:32:55:8a:\
                    fd:b2:64:4c:0e:e4:98:07:75:db:7f:df:b9:08:55:\
                    60:85:30:29:f9:7b:48:a4:69:86:e3:35:3f:1e:86:\
                    5d:7a:7a:15:bd:ef:00:8e:15:22:54:17:00:90:26:\
                    93:bc:0e:49:68:91:bf:f8:47:d3:9d:95:42:c1:0e:\
                    4d:df:6f:26:cf:c3:18:21:62:66:43:70:d6:d5:c0:\
                    07:e1\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Authority Key Identifier: \
                keyid:55:E4:81:D1:11:80:BE:D8:89:B9:08:A3:31:F9:A1:24:09:16:B9:70\
\
            X509v3 Subject Key Identifier: \
                55:E4:81:D1:11:80:BE:D8:89:B9:08:A3:31:F9:A1:24:09:16:B9:70\
            1.2.840.113533.7.65.0: \
                0...V5.0:4.0....\
    Signature Algorithm: sha1WithRSAEncryption\
        59:47:ac:21:84:8a:17:c9:9c:89:53:1e:ba:80:85:1a:c6:3c:\
        4e:3e:b1:9c:b6:7c:c6:92:5d:18:64:02:e3:d3:06:08:11:61:\
        7c:63:e3:2b:9d:31:03:70:76:d2:a3:28:a0:f4:bb:9a:63:73:\
        ed:6d:e5:2a:db:ed:14:a9:2b:c6:36:11:d0:2b:eb:07:8b:a5:\
        da:9e:5c:19:9d:56:12:f5:54:29:c8:05:ed:b2:12:2a:8d:f4:\
        03:1b:ff:e7:92:10:87:b0:3a:b5:c3:9d:05:37:12:a3:c7:f4:\
        15:b9:d5:a4:39:16:9b:53:3a:23:91:f1:a8:82:a2:6a:88:68:\
        c1:79:02:22:bc:aa:a6:d6:ae:df:b0:14:5f:b8:87:d0:dd:7c:\
        7f:7b:ff:af:1c:cf:e6:db:07:ad:5e:db:85:9d:d0:2b:0d:33:\
        db:04:d1:e6:49:40:13:2b:76:fb:3e:e9:9c:89:0f:15:ce:18:\
        b0:85:78:21:4f:6b:4f:0e:fa:36:67:cd:07:f2:ff:08:d0:e2:\
        de:d9:bf:2a:af:b8:87:86:21:3c:04:ca:b7:94:68:7f:cf:3c:\
        e9:98:d7:38:ff:ec:c0:d9:50:f0:2e:4b:58:ae:46:6f:d0:2e:\
        c3:60:da:72:55:72:bd:4c:45:9e:61:ba:bf:84:81:92:03:d1:\
        d2:69:7c:c5\
-----BEGIN CERTIFICATE-----\
MIIEXDCCA0SgAwIBAgIEOGO5ZjANBgkqhkiG9w0BAQUFADCBtDEUMBIGA1UEChML\
RW50cnVzdC5uZXQxQDA+BgNVBAsUN3d3dy5lbnRydXN0Lm5ldC9DUFNfMjA0OCBp\
bmNvcnAuIGJ5IHJlZi4gKGxpbWl0cyBsaWFiLikxJTAjBgNVBAsTHChjKSAxOTk5\
IEVudHJ1c3QubmV0IExpbWl0ZWQxMzAxBgNVBAMTKkVudHJ1c3QubmV0IENlcnRp\
ZmljYXRpb24gQXV0aG9yaXR5ICgyMDQ4KTAeFw05OTEyMjQxNzUwNTFaFw0xOTEy\
MjQxODIwNTFaMIG0MRQwEgYDVQQKEwtFbnRydXN0Lm5ldDFAMD4GA1UECxQ3d3d3\
LmVudHJ1c3QubmV0L0NQU18yMDQ4IGluY29ycC4gYnkgcmVmLiAobGltaXRzIGxp\
YWIuKTElMCMGA1UECxMcKGMpIDE5OTkgRW50cnVzdC5uZXQgTGltaXRlZDEzMDEG\
A1UEAxMqRW50cnVzdC5uZXQgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkgKDIwNDgp\
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArU1LqRKGsuqjIAcVFmQq\
K0vRvwtKTY7tgHalZ7d4QMBzQshowNtTK91euHaYNZOLGp18EzoOH1u3Hs/lJBQe\
sYGpjX24zGtLA/ECDNyrpUAkAH90lKGdCCmziAv1h3edVc3kw37XamSrhRSGlVuX\
MlBvPci6Zgzj/L24ScF2iUkZ/cCovYmjZy/Gn7xxGWC4LeksyZB2ZnuU4q941mVT\
XTzWnLLPKQP5L6RQstRIzgUyVYr9smRMDuSYB3Xbf9+5CFVghTAp+XtIpGmG4zU/\
HoZdenoVve8AjhUiVBcAkCaTvA5JaJG/+EfTnZVCwQ5N328mz8MYIWJmQ3DW1cAH\
4QIDAQABo3QwcjARBglghkgBhvhCAQEEBAMCAAcwHwYDVR0jBBgwFoAUVeSB0RGA\
vtiJuQijMfmhJAkWuXAwHQYDVR0OBBYEFFXkgdERgL7YibkIozH5oSQJFrlwMB0G\
CSqGSIb2fQdBAAQQMA4bCFY1LjA6NC4wAwIEkDANBgkqhkiG9w0BAQUFAAOCAQEA\
WUesIYSKF8mciVMeuoCFGsY8Tj6xnLZ8xpJdGGQC49MGCBFhfGPjK50xA3B20qMo\
oPS7mmNz7W3lKtvtFKkrxjYR0CvrB4ul2p5cGZ1WEvVUKcgF7bISKo30Axv/55IQ\
h7A6tcOdBTcSo8f0FbnVpDkWm1M6I5HxqIKiaohowXkCIryqptau37AUX7iH0N18\
f3v/rxzP5tsHrV7bhZ3QKw0z2wTR5klAEyt2+z7pnIkPFc4YsIV4IU9rTw76NmfN\
B/L/CNDi3tm/Kq+4h4YhPATKt5Rof8886ZjXOP/swNlQ8C5LWK5Gb9Auw2DaclVy\
vUxFnmG6v4SBkgPR0ml8xQ==\
-----END CERTIFICATE-----\
\
Baltimore CyberTrust Root\
=========================\
\
MD5 Fingerprint=AC:B6:94:A5:9C:17:E0:D7:91:52:9B:B1:97:06:A6:E4\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 33554617 (0x20000b9)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=IE, O=Baltimore, OU=CyberTrust, CN=Baltimore CyberTrust Root\
        Validity\
            Not Before: May 12 18:46:00 2000 GMT\
            Not After : May 12 23:59:00 2025 GMT\
        Subject: C=IE, O=Baltimore, OU=CyberTrust, CN=Baltimore CyberTrust Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:a3:04:bb:22:ab:98:3d:57:e8:26:72:9a:b5:79:\
                    d4:29:e2:e1:e8:95:80:b1:b0:e3:5b:8e:2b:29:9a:\
                    64:df:a1:5d:ed:b0:09:05:6d:db:28:2e:ce:62:a2:\
                    62:fe:b4:88:da:12:eb:38:eb:21:9d:c0:41:2b:01:\
                    52:7b:88:77:d3:1c:8f:c7:ba:b9:88:b5:6a:09:e7:\
                    73:e8:11:40:a7:d1:cc:ca:62:8d:2d:e5:8f:0b:a6:\
                    50:d2:a8:50:c3:28:ea:f5:ab:25:87:8a:9a:96:1c:\
                    a9:67:b8:3f:0c:d5:f7:f9:52:13:2f:c2:1b:d5:70:\
                    70:f0:8f:c0:12:ca:06:cb:9a:e1:d9:ca:33:7a:77:\
                    d6:f8:ec:b9:f1:68:44:42:48:13:d2:c0:c2:a4:ae:\
                    5e:60:fe:b6:a6:05:fc:b4:dd:07:59:02:d4:59:18:\
                    98:63:f5:a5:63:e0:90:0c:7d:5d:b2:06:7a:f3:85:\
                    ea:eb:d4:03:ae:5e:84:3e:5f:ff:15:ed:69:bc:f9:\
                    39:36:72:75:cf:77:52:4d:f3:c9:90:2c:b9:3d:e5:\
                    c9:23:53:3f:1f:24:98:21:5c:07:99:29:bd:c6:3a:\
                    ec:e7:6e:86:3a:6b:97:74:63:33:bd:68:18:31:f0:\
                    78:8d:76:bf:fc:9e:8e:5d:2a:86:a7:4d:90:dc:27:\
                    1a:39\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                E5:9D:59:30:82:47:58:CC:AC:FA:08:54:36:86:7B:3A:B5:04:4D:F0\
            X509v3 Basic Constraints: critical\
                CA:TRUE, pathlen:3\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        85:0c:5d:8e:e4:6f:51:68:42:05:a0:dd:bb:4f:27:25:84:03:\
        bd:f7:64:fd:2d:d7:30:e3:a4:10:17:eb:da:29:29:b6:79:3f:\
        76:f6:19:13:23:b8:10:0a:f9:58:a4:d4:61:70:bd:04:61:6a:\
        12:8a:17:d5:0a:bd:c5:bc:30:7c:d6:e9:0c:25:8d:86:40:4f:\
        ec:cc:a3:7e:38:c6:37:11:4f:ed:dd:68:31:8e:4c:d2:b3:01:\
        74:ee:be:75:5e:07:48:1a:7f:70:ff:16:5c:84:c0:79:85:b8:\
        05:fd:7f:be:65:11:a3:0f:c0:02:b4:f8:52:37:39:04:d5:a9:\
        31:7a:18:bf:a0:2a:f4:12:99:f7:a3:45:82:e3:3c:5e:f5:9d:\
        9e:b5:c8:9e:7c:2e:c8:a4:9e:4e:08:14:4b:6d:fd:70:6d:6b:\
        1a:63:bd:64:e6:1f:b7:ce:f0:f2:9f:2e:bb:1b:b7:f2:50:88:\
        73:92:c2:e2:e3:16:8d:9a:32:02:ab:8e:18:dd:e9:10:11:ee:\
        7e:35:ab:90:af:3e:30:94:7a:d0:33:3d:a7:65:0f:f5:fc:8e:\
        9e:62:cf:47:44:2c:01:5d:bb:1d:b5:32:d2:47:d2:38:2e:d0:\
        fe:81:dc:32:6a:1e:b5:ee:3c:d5:fc:e7:81:1d:19:c3:24:42:\
        ea:63:39:a9\
-----BEGIN CERTIFICATE-----\
MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\
RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\
VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\
DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\
ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\
VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\
mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\
IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\
mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\
XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\
dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\
jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\
BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\
DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\
9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\
jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\
Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\
ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\
R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\
-----END CERTIFICATE-----\
\
Equifax Secure Global eBusiness CA\
==================================\
\
MD5 Fingerprint=8F:5D:77:06:27:C4:98:3C:5B:93:78:E7:D7:7D:9B:CC\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=US, O=Equifax Secure Inc., CN=Equifax Secure Global eBusiness CA-1\
        Validity\
            Not Before: Jun 21 04:00:00 1999 GMT\
            Not After : Jun 21 04:00:00 2020 GMT\
        Subject: C=US, O=Equifax Secure Inc., CN=Equifax Secure Global eBusiness CA-1\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:ba:e7:17:90:02:65:b1:34:55:3c:49:c2:51:d5:\
                    df:a7:d1:37:8f:d1:e7:81:73:41:52:60:9b:9d:a1:\
                    17:26:78:ad:c7:b1:e8:26:94:32:b5:de:33:8d:3a:\
                    2f:db:f2:9a:7a:5a:73:98:a3:5c:e9:fb:8a:73:1b:\
                    5c:e7:c3:bf:80:6c:cd:a9:f4:d6:2b:c0:f7:f9:99:\
                    aa:63:a2:b1:47:02:0f:d4:e4:51:3a:12:3c:6c:8a:\
                    5a:54:84:70:db:c1:c5:90:cf:72:45:cb:a8:59:c0:\
                    cd:33:9d:3f:a3:96:eb:85:33:21:1c:3e:1e:3e:60:\
                    6e:76:9c:67:85:c5:c8:c3:61\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Authority Key Identifier: \
                keyid:BE:A8:A0:74:72:50:6B:44:B7:C9:23:D8:FB:A8:FF:B3:57:6B:68:6C\
\
            X509v3 Subject Key Identifier: \
                BE:A8:A0:74:72:50:6B:44:B7:C9:23:D8:FB:A8:FF:B3:57:6B:68:6C\
    Signature Algorithm: md5WithRSAEncryption\
        30:e2:01:51:aa:c7:ea:5f:da:b9:d0:65:0f:30:d6:3e:da:0d:\
        14:49:6e:91:93:27:14:31:ef:c4:f7:2d:45:f8:ec:c7:bf:a2:\
        41:0d:23:b4:92:f9:19:00:67:bd:01:af:cd:e0:71:fc:5a:cf:\
        64:c4:e0:96:98:d0:a3:40:e2:01:8a:ef:27:07:f1:65:01:8a:\
        44:2d:06:65:75:52:c0:86:10:20:21:5f:6c:6b:0f:6c:ae:09:\
        1c:af:f2:a2:18:34:c4:75:a4:73:1c:f1:8d:dc:ef:ad:f9:b3:\
        76:b4:92:bf:dc:95:10:1e:be:cb:c8:3b:5a:84:60:19:56:94:\
        a9:55\
-----BEGIN CERTIFICATE-----\
MIICkDCCAfmgAwIBAgIBATANBgkqhkiG9w0BAQQFADBaMQswCQYDVQQGEwJVUzEc\
MBoGA1UEChMTRXF1aWZheCBTZWN1cmUgSW5jLjEtMCsGA1UEAxMkRXF1aWZheCBT\
ZWN1cmUgR2xvYmFsIGVCdXNpbmVzcyBDQS0xMB4XDTk5MDYyMTA0MDAwMFoXDTIw\
MDYyMTA0MDAwMFowWjELMAkGA1UEBhMCVVMxHDAaBgNVBAoTE0VxdWlmYXggU2Vj\
dXJlIEluYy4xLTArBgNVBAMTJEVxdWlmYXggU2VjdXJlIEdsb2JhbCBlQnVzaW5l\
c3MgQ0EtMTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAuucXkAJlsTRVPEnC\
UdXfp9E3j9HngXNBUmCbnaEXJnitx7HoJpQytd4zjTov2/KaelpzmKNc6fuKcxtc\
58O/gGzNqfTWK8D3+ZmqY6KxRwIP1ORROhI8bIpaVIRw28HFkM9yRcuoWcDNM50/\
o5brhTMhHD4ePmBudpxnhcXIw2ECAwEAAaNmMGQwEQYJYIZIAYb4QgEBBAQDAgAH\
MA8GA1UdEwEB/wQFMAMBAf8wHwYDVR0jBBgwFoAUvqigdHJQa0S3ySPY+6j/s1dr\
aGwwHQYDVR0OBBYEFL6ooHRyUGtEt8kj2Puo/7NXa2hsMA0GCSqGSIb3DQEBBAUA\
A4GBADDiAVGqx+pf2rnQZQ8w1j7aDRRJbpGTJxQx78T3LUX47Me/okENI7SS+RkA\
Z70Br83gcfxaz2TE4JaY0KNA4gGK7ycH8WUBikQtBmV1UsCGECAhX2xrD2yuCRyv\
8qIYNMR1pHMc8Y3c7635s3a0kr/clRAevsvIO1qEYBlWlKlV\
-----END CERTIFICATE-----\
\
Equifax Secure eBusiness CA 1\
=============================\
\
MD5 Fingerprint=64:9C:EF:2E:44:FC:C6:8F:52:07:D0:51:73:8F:CB:3D\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 4 (0x4)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=US, O=Equifax Secure Inc., CN=Equifax Secure eBusiness CA-1\
        Validity\
            Not Before: Jun 21 04:00:00 1999 GMT\
            Not After : Jun 21 04:00:00 2020 GMT\
        Subject: C=US, O=Equifax Secure Inc., CN=Equifax Secure eBusiness CA-1\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:ce:2f:19:bc:17:b7:77:de:93:a9:5f:5a:0d:17:\
                    4f:34:1a:0c:98:f4:22:d9:59:d4:c4:68:46:f0:b4:\
                    35:c5:85:03:20:c6:af:45:a5:21:51:45:41:eb:16:\
                    58:36:32:6f:e2:50:62:64:f9:fd:51:9c:aa:24:d9:\
                    f4:9d:83:2a:87:0a:21:d3:12:38:34:6c:8d:00:6e:\
                    5a:a0:d9:42:ee:1a:21:95:f9:52:4c:55:5a:c5:0f:\
                    38:4f:46:fa:6d:f8:2e:35:d6:1d:7c:eb:e2:f0:b0:\
                    75:80:c8:a9:13:ac:be:88:ef:3a:6e:ab:5f:2a:38:\
                    62:02:b0:12:7b:fe:8f:a6:03\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Authority Key Identifier: \
                keyid:4A:78:32:52:11:DB:59:16:36:5E:DF:C1:14:36:40:6A:47:7C:4C:A1\
\
            X509v3 Subject Key Identifier: \
                4A:78:32:52:11:DB:59:16:36:5E:DF:C1:14:36:40:6A:47:7C:4C:A1\
    Signature Algorithm: md5WithRSAEncryption\
        75:5b:a8:9b:03:11:e6:e9:56:4c:cd:f9:a9:4c:c0:0d:9a:f3:\
        cc:65:69:e6:25:76:cc:59:b7:d6:54:c3:1d:cd:99:ac:19:dd:\
        b4:85:d5:e0:3d:fc:62:20:a7:84:4b:58:65:f1:e2:f9:95:21:\
        3f:f5:d4:7e:58:1e:47:87:54:3e:58:a1:b5:b5:f8:2a:ef:71:\
        e7:bc:c3:f6:b1:49:46:e2:d7:a0:6b:e5:56:7a:9a:27:98:7c:\
        46:62:14:e7:c9:fc:6e:03:12:79:80:38:1d:48:82:8d:fc:17:\
        fe:2a:96:2b:b5:62:a6:a6:3d:bd:7f:92:59:cd:5a:2a:82:b2:\
        37:79\
-----BEGIN CERTIFICATE-----\
MIICgjCCAeugAwIBAgIBBDANBgkqhkiG9w0BAQQFADBTMQswCQYDVQQGEwJVUzEc\
MBoGA1UEChMTRXF1aWZheCBTZWN1cmUgSW5jLjEmMCQGA1UEAxMdRXF1aWZheCBT\
ZWN1cmUgZUJ1c2luZXNzIENBLTEwHhcNOTkwNjIxMDQwMDAwWhcNMjAwNjIxMDQw\
MDAwWjBTMQswCQYDVQQGEwJVUzEcMBoGA1UEChMTRXF1aWZheCBTZWN1cmUgSW5j\
LjEmMCQGA1UEAxMdRXF1aWZheCBTZWN1cmUgZUJ1c2luZXNzIENBLTEwgZ8wDQYJ\
KoZIhvcNAQEBBQADgY0AMIGJAoGBAM4vGbwXt3fek6lfWg0XTzQaDJj0ItlZ1MRo\
RvC0NcWFAyDGr0WlIVFFQesWWDYyb+JQYmT5/VGcqiTZ9J2DKocKIdMSODRsjQBu\
WqDZQu4aIZX5UkxVWsUPOE9G+m34LjXWHXzr4vCwdYDIqROsvojvOm6rXyo4YgKw\
Env+j6YDAgMBAAGjZjBkMBEGCWCGSAGG+EIBAQQEAwIABzAPBgNVHRMBAf8EBTAD\
AQH/MB8GA1UdIwQYMBaAFEp4MlIR21kWNl7fwRQ2QGpHfEyhMB0GA1UdDgQWBBRK\
eDJSEdtZFjZe38EUNkBqR3xMoTANBgkqhkiG9w0BAQQFAAOBgQB1W6ibAxHm6VZM\
zfmpTMANmvPMZWnmJXbMWbfWVMMdzZmsGd20hdXgPfxiIKeES1hl8eL5lSE/9dR+\
WB5Hh1Q+WKG1tfgq73HnvMP2sUlG4tega+VWeponmHxGYhTnyfxuAxJ5gDgdSIKN\
/Bf+KpYrtWKmpj29f5JZzVoqgrI3eQ==\
-----END CERTIFICATE-----\
\
Equifax Secure eBusiness CA 2\
=============================\
\
MD5 Fingerprint=AA:BF:BF:64:97:DA:98:1D:6F:C6:08:3A:95:70:33:CA\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 930140085 (0x3770cfb5)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=Equifax Secure, OU=Equifax Secure eBusiness CA-2\
        Validity\
            Not Before: Jun 23 12:14:45 1999 GMT\
            Not After : Jun 23 12:14:45 2019 GMT\
        Subject: C=US, O=Equifax Secure, OU=Equifax Secure eBusiness CA-2\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:e4:39:39:93:1e:52:06:1b:28:36:f8:b2:a3:29:\
                    c5:ed:8e:b2:11:bd:fe:eb:e7:b4:74:c2:8f:ff:05:\
                    e7:d9:9d:06:bf:12:c8:3f:0e:f2:d6:d1:24:b2:11:\
                    de:d1:73:09:8a:d4:b1:2c:98:09:0d:1e:50:46:b2:\
                    83:a6:45:8d:62:68:bb:85:1b:20:70:32:aa:40:cd:\
                    a6:96:5f:c4:71:37:3f:04:f3:b7:41:24:39:07:1a:\
                    1e:2e:61:58:a0:12:0b:e5:a5:df:c5:ab:ea:37:71:\
                    cc:1c:c8:37:3a:b9:97:52:a7:ac:c5:6a:24:94:4e:\
                    9c:7b:cf:c0:6a:d6:df:21:bd\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 CRL Distribution Points: \
                DirName:/C=US/O=Equifax Secure/OU=Equifax Secure eBusiness CA-2/CN=CRL1\
\
            X509v3 Private Key Usage Period: \
                Not After: Jun 23 12:14:45 2019 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:50:9E:0B:EA:AF:5E:B9:20:48:A6:50:6A:CB:FD:D8:20:7A:A7:82:76\
\
            X509v3 Subject Key Identifier: \
                50:9E:0B:EA:AF:5E:B9:20:48:A6:50:6A:CB:FD:D8:20:7A:A7:82:76\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0...V3.0c....\
    Signature Algorithm: sha1WithRSAEncryption\
        0c:86:82:ad:e8:4e:1a:f5:8e:89:27:e2:35:58:3d:29:b4:07:\
        8f:36:50:95:bf:6e:c1:9e:eb:c4:90:b2:85:a8:bb:b7:42:e0:\
        0f:07:39:df:fb:9e:90:b2:d1:c1:3e:53:9f:03:44:b0:7e:4b:\
        f4:6f:e4:7c:1f:e7:e2:b1:e4:b8:9a:ef:c3:bd:ce:de:0b:32:\
        34:d9:de:28:ed:33:6b:c4:d4:d7:3d:12:58:ab:7d:09:2d:cb:\
        70:f5:13:8a:94:a1:27:a4:d6:70:c5:6d:94:b5:c9:7d:9d:a0:\
        d2:c6:08:49:d9:66:9b:a6:d3:f4:0b:dc:c5:26:57:e1:91:30:\
        ea:cd\
-----BEGIN CERTIFICATE-----\
MIIDIDCCAomgAwIBAgIEN3DPtTANBgkqhkiG9w0BAQUFADBOMQswCQYDVQQGEwJV\
UzEXMBUGA1UEChMORXF1aWZheCBTZWN1cmUxJjAkBgNVBAsTHUVxdWlmYXggU2Vj\
dXJlIGVCdXNpbmVzcyBDQS0yMB4XDTk5MDYyMzEyMTQ0NVoXDTE5MDYyMzEyMTQ0\
NVowTjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDkVxdWlmYXggU2VjdXJlMSYwJAYD\
VQQLEx1FcXVpZmF4IFNlY3VyZSBlQnVzaW5lc3MgQ0EtMjCBnzANBgkqhkiG9w0B\
AQEFAAOBjQAwgYkCgYEA5Dk5kx5SBhsoNviyoynF7Y6yEb3+6+e0dMKP/wXn2Z0G\
vxLIPw7y1tEkshHe0XMJitSxLJgJDR5QRrKDpkWNYmi7hRsgcDKqQM2mll/EcTc/\
BPO3QSQ5BxoeLmFYoBIL5aXfxavqN3HMHMg3OrmXUqesxWoklE6ce8/AatbfIb0C\
AwEAAaOCAQkwggEFMHAGA1UdHwRpMGcwZaBjoGGkXzBdMQswCQYDVQQGEwJVUzEX\
MBUGA1UEChMORXF1aWZheCBTZWN1cmUxJjAkBgNVBAsTHUVxdWlmYXggU2VjdXJl\
IGVCdXNpbmVzcyBDQS0yMQ0wCwYDVQQDEwRDUkwxMBoGA1UdEAQTMBGBDzIwMTkw\
NjIzMTIxNDQ1WjALBgNVHQ8EBAMCAQYwHwYDVR0jBBgwFoAUUJ4L6q9euSBIplBq\
y/3YIHqngnYwHQYDVR0OBBYEFFCeC+qvXrkgSKZQasv92CB6p4J2MAwGA1UdEwQF\
MAMBAf8wGgYJKoZIhvZ9B0EABA0wCxsFVjMuMGMDAgbAMA0GCSqGSIb3DQEBBQUA\
A4GBAAyGgq3oThr1jokn4jVYPSm0B482UJW/bsGe68SQsoWou7dC4A8HOd/7npCy\
0cE+U58DRLB+S/Rv5Hwf5+Kx5Lia78O9zt4LMjTZ3ijtM2vE1Nc9ElirfQkty3D1\
E4qUoSek1nDFbZS1yX2doNLGCEnZZpum0/QL3MUmV+GRMOrN\
-----END CERTIFICATE-----\
\
Visa International Global Root 2\
================================\
\
MD5 Fingerprint=35:48:95:36:4A:54:5A:72:96:8E:E0:64:CC:EF:2C:8C\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 798 (0x31e)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VISA, OU=Visa International Service Association, CN=GP Root 2\
        Validity\
            Not Before: Aug 16 22:51:00 2000 GMT\
            Not After : Aug 15 23:59:00 2020 GMT\
        Subject: C=US, O=VISA, OU=Visa International Service Association, CN=GP Root 2\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:a9:01:70:b5:aa:c4:40:f0:ab:6a:26:61:79:19:\
                    00:fc:bf:9b:37:59:0c:af:6f:64:1b:f8:da:95:94:\
                    24:69:33:11:70:ca:e3:56:74:a2:17:57:64:5c:20:\
                    06:e1:d6:ef:71:b7:3b:f7:ab:c1:69:d0:49:a4:b1:\
                    04:d7:f4:57:62:89:5c:b0:75:2d:17:24:69:e3:42:\
                    60:e4:ee:74:d6:ab:80:56:d8:88:28:e1:fb:6d:22:\
                    fd:23:7c:46:73:4f:7e:54:73:1e:a8:2c:55:58:75:\
                    b7:4c:f3:5a:45:a5:02:1a:fa:da:9d:c3:45:c3:22:\
                    5e:f3:8b:f1:60:29:d2:c7:5f:b4:0c:3a:51:83:ef:\
                    30:f8:d4:e7:c7:f2:fa:99:a3:22:50:be:f9:05:37:\
                    a3:ad:ed:9a:c3:e6:ec:88:1b:b6:19:27:1b:38:8b:\
                    80:4d:ec:b9:c7:c5:89:cb:fc:1a:32:ed:23:f0:b5:\
                    01:58:f9:f6:8f:e0:85:a9:4c:09:72:39:12:db:b3:\
                    f5:cf:4e:62:64:da:c6:19:15:3a:63:1d:e9:17:55:\
                    a1:4c:22:3c:34:32:46:f8:65:57:ba:2b:ef:36:8c:\
                    6a:fa:d9:d9:44:f4:aa:dd:84:d7:0d:1c:b2:54:ac:\
                    32:85:b4:64:0d:de:41:bb:b1:34:c6:01:86:32:64:\
                    d5:9f\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                9E:7D:4B:34:BF:71:AD:C2:05:F6:03:75:80:CE:A9:4F:1A:C4:24:4C\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        21:a5:76:14:55:f9:ad:27:70:8f:3c:f4:d5:6c:c8:cc:0a:ab:\
        a3:98:0b:8a:06:23:c5:c9:61:db:99:07:69:35:26:31:fe:c7:\
        2e:84:c2:99:61:d4:0d:e9:7d:2e:13:2b:7c:8e:85:b6:85:c7:\
        4b:cf:35:b6:2c:47:3d:ce:29:2f:d8:6f:9f:89:1c:64:93:bf:\
        08:bd:76:d0:90:8a:94:b3:7f:28:5b:6e:ac:4d:33:2c:ed:65:\
        dc:16:cc:e2:cd:ae:a4:3d:62:92:06:95:26:bf:df:b9:e4:20:\
        a6:73:6a:c1:be:f7:94:44:d6:4d:6f:2a:0b:6b:18:4d:74:10:\
        36:68:6a:5a:c1:6a:a7:dd:36:29:8c:b8:30:8b:4f:21:3f:00:\
        2e:54:30:07:3a:ba:8a:e4:c3:9e:ca:d8:b5:d8:7b:ce:75:45:\
        66:07:f4:6d:2d:d8:7a:ca:e9:89:8a:f2:23:d8:2f:cb:6e:00:\
        36:4f:fb:f0:2f:01:cc:0f:c0:22:65:f4:ab:e2:4e:61:2d:03:\
        82:7d:91:16:b5:30:d5:14:de:5e:c7:90:fc:a1:fc:ab:10:af:\
        5c:6b:70:a7:07:ef:29:86:e8:b2:25:c7:20:ff:26:dd:77:ef:\
        79:44:14:c4:bd:dd:3b:c5:03:9b:77:23:ec:a0:ec:bb:5a:39:\
        b5:cc:ad:06\
-----BEGIN CERTIFICATE-----\
MIIDgDCCAmigAwIBAgICAx4wDQYJKoZIhvcNAQEFBQAwYTELMAkGA1UEBhMCVVMx\
DTALBgNVBAoTBFZJU0ExLzAtBgNVBAsTJlZpc2EgSW50ZXJuYXRpb25hbCBTZXJ2\
aWNlIEFzc29jaWF0aW9uMRIwEAYDVQQDEwlHUCBSb290IDIwHhcNMDAwODE2MjI1\
MTAwWhcNMjAwODE1MjM1OTAwWjBhMQswCQYDVQQGEwJVUzENMAsGA1UEChMEVklT\
QTEvMC0GA1UECxMmVmlzYSBJbnRlcm5hdGlvbmFsIFNlcnZpY2UgQXNzb2NpYXRp\
b24xEjAQBgNVBAMTCUdQIFJvb3QgMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC\
AQoCggEBAKkBcLWqxEDwq2omYXkZAPy/mzdZDK9vZBv42pWUJGkzEXDK41Z0ohdX\
ZFwgBuHW73G3O/erwWnQSaSxBNf0V2KJXLB1LRckaeNCYOTudNargFbYiCjh+20i\
/SN8RnNPflRzHqgsVVh1t0zzWkWlAhr62p3DRcMiXvOL8WAp0sdftAw6UYPvMPjU\
58fy+pmjIlC++QU3o63tmsPm7IgbthknGziLgE3sucfFicv8GjLtI/C1AVj59o/g\
halMCXI5Etuz9c9OYmTaxhkVOmMd6RdVoUwiPDQyRvhlV7or7zaMavrZ2UT0qt2E\
1w0cslSsMoW0ZA3eQbuxNMYBhjJk1Z8CAwEAAaNCMEAwHQYDVR0OBBYEFJ59SzS/\
ca3CBfYDdYDOqU8axCRMMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgEG\
MA0GCSqGSIb3DQEBBQUAA4IBAQAhpXYUVfmtJ3CPPPTVbMjMCqujmAuKBiPFyWHb\
mQdpNSYx/scuhMKZYdQN6X0uEyt8joW2hcdLzzW2LEc9zikv2G+fiRxkk78IvXbQ\
kIqUs38oW26sTTMs7WXcFsziza6kPWKSBpUmv9+55CCmc2rBvveURNZNbyoLaxhN\
dBA2aGpawWqn3TYpjLgwi08hPwAuVDAHOrqK5MOeyti12HvOdUVmB/RtLdh6yumJ\
ivIj2C/LbgA2T/vwLwHMD8AiZfSr4k5hLQOCfZEWtTDVFN5ex5D8ofyrEK9ca3Cn\
B+8phuiyJccg/ybdd+95RBTEvd07xQObdyPsoOy7Wjm1zK0G\
-----END CERTIFICATE-----\
\
beTRUSTed Root CA\
=================\
\
MD5 Fingerprint=85:CA:76:5A:1B:D1:68:22:DC:A2:23:12:CA:C6:80:34\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 961510791 (0x394f7d87)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=WW, O=beTRUSTed, CN=beTRUSTed Root CAs, CN=beTRUSTed Root CA\
        Validity\
            Not Before: Jun 20 14:21:04 2000 GMT\
            Not After : Jun 20 13:21:04 2010 GMT\
        Subject: C=WW, O=beTRUSTed, CN=beTRUSTed Root CAs, CN=beTRUSTed Root CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:d4:b4:73:7a:13:0a:38:55:01:be:89:56:e1:94:\
                    9e:d4:be:5a:eb:4a:34:75:1b:61:29:c4:e1:ad:08:\
                    60:21:78:48:ff:b4:d0:fa:5e:41:8d:61:44:87:e8:\
                    ed:c9:58:fa:fc:93:9a:df:4f:ea:3e:35:7d:f8:33:\
                    7a:e6:f1:d7:cd:6f:49:4b:3d:4f:2d:6e:0e:83:3a:\
                    18:78:77:a3:cf:e7:f4:4d:73:d8:9a:3b:1a:1d:be:\
                    95:53:cf:20:97:c2:cf:3e:24:52:6c:0c:8e:65:59:\
                    c5:71:ff:62:09:8f:aa:c5:8f:cc:60:a0:73:4a:d7:\
                    38:3f:15:72:bf:a2:97:b7:70:e8:af:e2:7e:16:06:\
                    4c:f5:aa:64:26:72:07:25:ad:35:fc:18:b1:26:d7:\
                    d8:ff:19:0e:83:1b:8c:dc:78:45:67:34:3d:f4:af:\
                    1c:8d:e4:6d:6b:ed:20:b3:67:9a:b4:61:cb:17:6f:\
                    89:35:ff:e7:4e:c0:32:12:e7:ee:ec:df:ff:97:30:\
                    74:ed:8d:47:8e:eb:b4:c3:44:e6:a7:4c:7f:56:43:\
                    e8:b8:bc:b6:be:fa:83:97:e6:bb:fb:c4:b6:93:be:\
                    19:18:3e:8c:81:b9:73:88:16:f4:96:43:9c:67:73:\
                    17:90:d8:09:6e:63:ac:4a:b6:23:c4:01:a1:ad:a4:\
                    e4:c5\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Certificate Policies: \
                Policy: 1.3.6.1.4.1.6334.1.0.0\
                  User Notice:\
                    Explicit Text: Reliance on this certificate by any party assumes acceptance of the then applicable standard terms and conditions of use, and certification practice statement, which can be found at beTRUSTed's web site, https://www.beTRUSTed.com/vault/terms\
                  CPS: https://www.beTRUSTed.com/vault/terms\
\
            X509v3 CRL Distribution Points: \
                DirName:/O=beTRUSTed/C=WW\
\
            X509v3 Subject Key Identifier: \
                2A:B9:9B:69:2E:3B:9B:D8:CD:DE:2A:31:04:34:6B:CA:07:18:AB:67\
            X509v3 Authority Key Identifier: \
                keyid:2A:B9:9B:69:2E:3B:9B:D8:CD:DE:2A:31:04:34:6B:CA:07:18:AB:67\
\
            X509v3 Key Usage: critical\
                Digital Signature, Non Repudiation, Key Encipherment, Data Encipherment, Key Agreement, Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        79:61:db:a3:5e:6e:16:b1:ea:76:51:f9:cb:15:9b:cb:69:be:\
        e6:81:6b:9f:28:1f:65:3e:dd:11:85:92:d4:e8:41:bf:7e:33:\
        bd:23:e7:f1:20:bf:a4:b4:a6:19:01:c6:8c:8d:35:7c:65:a4:\
        4f:09:a4:d6:d8:23:15:05:13:a7:43:79:af:db:a3:0e:9b:7b:\
        78:1a:f3:04:86:5a:c6:f6:8c:20:47:38:49:50:06:9d:72:67:\
        3a:f0:98:03:ad:96:67:44:fc:3f:10:0d:86:4d:e4:00:3b:29:\
        7b:ce:3b:3b:99:86:61:25:40:84:dc:13:62:b7:fa:ca:59:d6:\
        03:1e:d6:53:01:cd:6d:4c:68:55:40:e1:ee:6b:c7:2a:00:00:\
        48:82:b3:0a:01:c3:60:2a:0c:f7:82:35:ee:48:86:96:e4:74:\
        d4:3d:ea:01:71:ba:04:75:40:a7:a9:7f:39:39:9a:55:97:29:\
        65:ae:19:55:25:05:72:47:d3:e8:18:dc:b8:e9:af:43:73:01:\
        12:74:a3:e1:5c:5f:15:5d:24:f3:f9:e4:f4:b6:67:67:12:e7:\
        64:22:8a:f6:a5:41:a6:1c:b6:60:63:45:8a:10:b4:ba:46:10:\
        ae:41:57:65:6c:3f:23:10:3f:21:10:59:b7:e4:40:dd:26:0c:\
        23:f6:aa:ae\
-----BEGIN CERTIFICATE-----\
MIIFLDCCBBSgAwIBAgIEOU99hzANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJX\
VzESMBAGA1UEChMJYmVUUlVTVGVkMRswGQYDVQQDExJiZVRSVVNUZWQgUm9vdCBD\
QXMxGjAYBgNVBAMTEWJlVFJVU1RlZCBSb290IENBMB4XDTAwMDYyMDE0MjEwNFoX\
DTEwMDYyMDEzMjEwNFowWjELMAkGA1UEBhMCV1cxEjAQBgNVBAoTCWJlVFJVU1Rl\
ZDEbMBkGA1UEAxMSYmVUUlVTVGVkIFJvb3QgQ0FzMRowGAYDVQQDExFiZVRSVVNU\
ZWQgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANS0c3oT\
CjhVAb6JVuGUntS+WutKNHUbYSnE4a0IYCF4SP+00PpeQY1hRIfo7clY+vyTmt9P\
6j41ffgzeubx181vSUs9Ty1uDoM6GHh3o8/n9E1z2Jo7Gh2+lVPPIJfCzz4kUmwM\
jmVZxXH/YgmPqsWPzGCgc0rXOD8Vcr+il7dw6K/ifhYGTPWqZCZyByWtNfwYsSbX\
2P8ZDoMbjNx4RWc0PfSvHI3kbWvtILNnmrRhyxdviTX/507AMhLn7uzf/5cwdO2N\
R47rtMNE5qdMf1ZD6Li8tr76g5fmu/vEtpO+GRg+jIG5c4gW9JZDnGdzF5DYCW5j\
rEq2I8QBoa2k5MUCAwEAAaOCAfgwggH0MA8GA1UdEwEB/wQFMAMBAf8wggFZBgNV\
HSAEggFQMIIBTDCCAUgGCisGAQQBsT4BAAAwggE4MIIBAQYIKwYBBQUHAgIwgfQa\
gfFSZWxpYW5jZSBvbiB0aGlzIGNlcnRpZmljYXRlIGJ5IGFueSBwYXJ0eSBhc3N1\
bWVzIGFjY2VwdGFuY2Ugb2YgdGhlIHRoZW4gYXBwbGljYWJsZSBzdGFuZGFyZCB0\
ZXJtcyBhbmQgY29uZGl0aW9ucyBvZiB1c2UsIGFuZCBjZXJ0aWZpY2F0aW9uIHBy\
YWN0aWNlIHN0YXRlbWVudCwgd2hpY2ggY2FuIGJlIGZvdW5kIGF0IGJlVFJVU1Rl\
ZCdzIHdlYiBzaXRlLCBodHRwczovL3d3dy5iZVRSVVNUZWQuY29tL3ZhdWx0L3Rl\
cm1zMDEGCCsGAQUFBwIBFiVodHRwczovL3d3dy5iZVRSVVNUZWQuY29tL3ZhdWx0\
L3Rlcm1zMDQGA1UdHwQtMCswKaAnoCWkIzAhMRIwEAYDVQQKEwliZVRSVVNUZWQx\
CzAJBgNVBAYTAldXMB0GA1UdDgQWBBQquZtpLjub2M3eKjEENGvKBxirZzAfBgNV\
HSMEGDAWgBQquZtpLjub2M3eKjEENGvKBxirZzAOBgNVHQ8BAf8EBAMCAf4wDQYJ\
KoZIhvcNAQEFBQADggEBAHlh26Nebhax6nZR+csVm8tpvuaBa58oH2U+3RGFktTo\
Qb9+M70j5/Egv6S0phkBxoyNNXxlpE8JpNbYIxUFE6dDea/bow6be3ga8wSGWsb2\
jCBHOElQBp1yZzrwmAOtlmdE/D8QDYZN5AA7KXvOOzuZhmElQITcE2K3+spZ1gMe\
1lMBzW1MaFVA4e5rxyoAAEiCswoBw2AqDPeCNe5IhpbkdNQ96gFxugR1QKepfzk5\
mlWXKWWuGVUlBXJH0+gY3Ljpr0NzARJ0o+FcXxVdJPP55PS2Z2cS52QiivalQaYc\
tmBjRYoQtLpGEK5BV2VsPyMQPyEQWbfkQN0mDCP2qq4=\
-----END CERTIFICATE-----\
\
AddTrust Low-Value Services Root\
================================\
\
MD5 Fingerprint=1E:42:95:02:33:92:6B:B9:5F:C0:7F:DA:D6:B2:4B:FC\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=SE, O=AddTrust AB, OU=AddTrust TTP Network, CN=AddTrust Class 1 CA Root\
        Validity\
            Not Before: May 30 10:38:31 2000 GMT\
            Not After : May 30 10:38:31 2020 GMT\
        Subject: C=SE, O=AddTrust AB, OU=AddTrust TTP Network, CN=AddTrust Class 1 CA Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:96:96:d4:21:49:60:e2:6b:e8:41:07:0c:de:c4:\
                    e0:dc:13:23:cd:c1:35:c7:fb:d6:4e:11:0a:67:5e:\
                    f5:06:5b:6b:a5:08:3b:5b:29:16:3a:e7:87:b2:34:\
                    06:c5:bc:05:a5:03:7c:82:cb:29:10:ae:e1:88:81:\
                    bd:d6:9e:d3:fe:2d:56:c1:15:ce:e3:26:9d:15:2e:\
                    10:fb:06:8f:30:04:de:a7:b4:63:b4:ff:b1:9c:ae:\
                    3c:af:77:b6:56:c5:b5:ab:a2:e9:69:3a:3d:0e:33:\
                    79:32:3f:70:82:92:99:61:6d:8d:30:08:8f:71:3f:\
                    a6:48:57:19:f8:25:dc:4b:66:5c:a5:74:8f:98:ae:\
                    c8:f9:c0:06:22:e7:ac:73:df:a5:2e:fb:52:dc:b1:\
                    15:65:20:fa:35:66:69:de:df:2c:f1:6e:bc:30:db:\
                    2c:24:12:db:eb:35:35:68:90:cb:00:b0:97:21:3d:\
                    74:21:23:65:34:2b:bb:78:59:a3:d6:e1:76:39:9a:\
                    a4:49:8e:8c:74:af:6e:a4:9a:a3:d9:9b:d2:38:5c:\
                    9b:a2:18:cc:75:23:84:be:eb:e2:4d:33:71:8e:1a:\
                    f0:c2:f8:c7:1d:a2:ad:03:97:2c:f8:cf:25:c6:f6:\
                    b8:24:31:b1:63:5d:92:7f:63:f0:25:c9:53:2e:1f:\
                    bf:4d\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                95:B1:B4:F0:94:B6:BD:C7:DA:D1:11:09:21:BE:C1:AF:49:FD:10:7B\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Authority Key Identifier: \
                keyid:95:B1:B4:F0:94:B6:BD:C7:DA:D1:11:09:21:BE:C1:AF:49:FD:10:7B\
                DirName:/C=SE/O=AddTrust AB/OU=AddTrust TTP Network/CN=AddTrust Class 1 CA Root\
                serial:01\
\
    Signature Algorithm: sha1WithRSAEncryption\
        2c:6d:64:1b:1f:cd:0d:dd:b9:01:fa:96:63:34:32:48:47:99:\
        ae:97:ed:fd:72:16:a6:73:47:5a:f4:eb:dd:e9:f5:d6:fb:45:\
        cc:29:89:44:5d:bf:46:39:3d:e8:ee:bc:4d:54:86:1e:1d:6c:\
        e3:17:27:43:e1:89:56:2b:a9:6f:72:4e:49:33:e3:72:7c:2a:\
        23:9a:bc:3e:ff:28:2a:ed:a3:ff:1c:23:ba:43:57:09:67:4d:\
        4b:62:06:2d:f8:ff:6c:9d:60:1e:d8:1c:4b:7d:b5:31:2f:d9:\
        d0:7c:5d:f8:de:6b:83:18:78:37:57:2f:e8:33:07:67:df:1e:\
        c7:6b:2a:95:76:ae:8f:57:a3:f0:f4:52:b4:a9:53:08:cf:e0:\
        4f:d3:7a:53:8b:fd:bb:1c:56:36:f2:fe:b2:b6:e5:76:bb:d5:\
        22:65:a7:3f:fe:d1:66:ad:0b:bc:6b:99:86:ef:3f:7d:f3:18:\
        32:ca:7b:c6:e3:ab:64:46:95:f8:26:69:d9:55:83:7b:2c:96:\
        07:ff:59:2c:44:a3:c6:e5:e9:a9:dc:a1:63:80:5a:21:5e:21:\
        cf:53:54:f0:ba:6f:89:db:a8:aa:95:cf:8b:e3:71:cc:1e:1b:\
        20:44:08:c0:7a:b6:40:fd:c4:e4:35:e1:1d:16:1c:d0:bc:2b:\
        8e:d6:71:d9\
-----BEGIN CERTIFICATE-----\
MIIEGDCCAwCgAwIBAgIBATANBgkqhkiG9w0BAQUFADBlMQswCQYDVQQGEwJTRTEU\
MBIGA1UEChMLQWRkVHJ1c3QgQUIxHTAbBgNVBAsTFEFkZFRydXN0IFRUUCBOZXR3\
b3JrMSEwHwYDVQQDExhBZGRUcnVzdCBDbGFzcyAxIENBIFJvb3QwHhcNMDAwNTMw\
MTAzODMxWhcNMjAwNTMwMTAzODMxWjBlMQswCQYDVQQGEwJTRTEUMBIGA1UEChML\
QWRkVHJ1c3QgQUIxHTAbBgNVBAsTFEFkZFRydXN0IFRUUCBOZXR3b3JrMSEwHwYD\
VQQDExhBZGRUcnVzdCBDbGFzcyAxIENBIFJvb3QwggEiMA0GCSqGSIb3DQEBAQUA\
A4IBDwAwggEKAoIBAQCWltQhSWDia+hBBwzexODcEyPNwTXH+9ZOEQpnXvUGW2ul\
CDtbKRY654eyNAbFvAWlA3yCyykQruGIgb3WntP+LVbBFc7jJp0VLhD7Bo8wBN6n\
tGO0/7Gcrjyvd7ZWxbWroulpOj0OM3kyP3CCkplhbY0wCI9xP6ZIVxn4JdxLZlyl\
dI+Yrsj5wAYi56xz36Uu+1LcsRVlIPo1Zmne3yzxbrww2ywkEtvrNTVokMsAsJch\
PXQhI2U0K7t4WaPW4XY5mqRJjox0r26kmqPZm9I4XJuiGMx1I4S+6+JNM3GOGvDC\
+Mcdoq0Dlyz4zyXG9rgkMbFjXZJ/Y/AlyVMuH79NAgMBAAGjgdIwgc8wHQYDVR0O\
BBYEFJWxtPCUtr3H2tERCSG+wa9J/RB7MAsGA1UdDwQEAwIBBjAPBgNVHRMBAf8E\
BTADAQH/MIGPBgNVHSMEgYcwgYSAFJWxtPCUtr3H2tERCSG+wa9J/RB7oWmkZzBl\
MQswCQYDVQQGEwJTRTEUMBIGA1UEChMLQWRkVHJ1c3QgQUIxHTAbBgNVBAsTFEFk\
ZFRydXN0IFRUUCBOZXR3b3JrMSEwHwYDVQQDExhBZGRUcnVzdCBDbGFzcyAxIENB\
IFJvb3SCAQEwDQYJKoZIhvcNAQEFBQADggEBACxtZBsfzQ3duQH6lmM0MkhHma6X\
7f1yFqZzR1r0693p9db7RcwpiURdv0Y5PejuvE1Uhh4dbOMXJ0PhiVYrqW9yTkkz\
43J8KiOavD7/KCrto/8cI7pDVwlnTUtiBi34/2ydYB7YHEt9tTEv2dB8Xfjea4MY\
eDdXL+gzB2ffHsdrKpV2ro9Xo/D0UrSpUwjP4E/TelOL/bscVjby/rK25Xa71SJl\
pz/+0WatC7xrmYbvP33zGDLKe8bjq2RGlfgmadlVg3sslgf/WSxEo8bl6ancoWOA\
WiFeIc9TVPC6b4nbqKqVz4vjccweGyBECMB6tkD9xOQ14R0WHNC8K47Wcdk=\
-----END CERTIFICATE-----\
\
Thawte Personal Basic CA\
========================\
\
MD5 Fingerprint=E6:0B:D2:C9:CA:2D:88:DB:1A:71:0E:4B:78:EB:02:41\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting, OU=Certification Services Division, CN=Thawte Personal Basic CA/emailAddress=personal-basic@thawte.com\
        Validity\
            Not Before: Jan  1 00:00:00 1996 GMT\
            Not After : Dec 31 23:59:59 2020 GMT\
        Subject: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting, OU=Certification Services Division, CN=Thawte Personal Basic CA/emailAddress=personal-basic@thawte.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:bc:bc:93:53:6d:c0:50:4f:82:15:e6:48:94:35:\
                    a6:5a:be:6f:42:fa:0f:47:ee:77:75:72:dd:8d:49:\
                    9b:96:57:a0:78:d4:ca:3f:51:b3:69:0b:91:76:17:\
                    22:07:97:6a:c4:51:93:4b:e0:8d:ef:37:95:a1:0c:\
                    4d:da:34:90:1d:17:89:97:e0:35:38:57:4a:c0:f4:\
                    08:70:e9:3c:44:7b:50:7e:61:9a:90:e3:23:d3:88:\
                    11:46:27:f5:0b:07:0e:bb:dd:d1:7f:20:0a:88:b9:\
                    56:0b:2e:1c:80:da:f1:e3:9e:29:ef:14:bd:0a:44:\
                    fb:1b:5b:18:d1:bf:23:93:21\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
    Signature Algorithm: md5WithRSAEncryption\
        2d:e2:99:6b:b0:3d:7a:89:d7:59:a2:94:01:1f:2b:dd:12:4b:\
        53:c2:ad:7f:aa:a7:00:5c:91:40:57:25:4a:38:aa:84:70:b9:\
        d9:80:0f:a5:7b:5c:fb:73:c6:bd:d7:8a:61:5c:03:e3:2d:27:\
        a8:17:e0:84:85:42:dc:5e:9b:c6:b7:b2:6d:bb:74:af:e4:3f:\
        cb:a7:b7:b0:e0:5d:be:78:83:25:94:d2:db:81:0f:79:07:6d:\
        4f:f4:39:15:5a:52:01:7b:de:32:d6:4d:38:f6:12:5c:06:50:\
        df:05:5b:bd:14:4b:a1:df:29:ba:3b:41:8d:f7:63:56:a1:df:\
        22:b1\
-----BEGIN CERTIFICATE-----\
MIIDITCCAoqgAwIBAgIBADANBgkqhkiG9w0BAQQFADCByzELMAkGA1UEBhMCWkEx\
FTATBgNVBAgTDFdlc3Rlcm4gQ2FwZTESMBAGA1UEBxMJQ2FwZSBUb3duMRowGAYD\
VQQKExFUaGF3dGUgQ29uc3VsdGluZzEoMCYGA1UECxMfQ2VydGlmaWNhdGlvbiBT\
ZXJ2aWNlcyBEaXZpc2lvbjEhMB8GA1UEAxMYVGhhd3RlIFBlcnNvbmFsIEJhc2lj\
IENBMSgwJgYJKoZIhvcNAQkBFhlwZXJzb25hbC1iYXNpY0B0aGF3dGUuY29tMB4X\
DTk2MDEwMTAwMDAwMFoXDTIwMTIzMTIzNTk1OVowgcsxCzAJBgNVBAYTAlpBMRUw\
EwYDVQQIEwxXZXN0ZXJuIENhcGUxEjAQBgNVBAcTCUNhcGUgVG93bjEaMBgGA1UE\
ChMRVGhhd3RlIENvbnN1bHRpbmcxKDAmBgNVBAsTH0NlcnRpZmljYXRpb24gU2Vy\
dmljZXMgRGl2aXNpb24xITAfBgNVBAMTGFRoYXd0ZSBQZXJzb25hbCBCYXNpYyBD\
QTEoMCYGCSqGSIb3DQEJARYZcGVyc29uYWwtYmFzaWNAdGhhd3RlLmNvbTCBnzAN\
BgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAvLyTU23AUE+CFeZIlDWmWr5vQvoPR+53\
dXLdjUmbllegeNTKP1GzaQuRdhciB5dqxFGTS+CN7zeVoQxN2jSQHReJl+A1OFdK\
wPQIcOk8RHtQfmGakOMj04gRRif1CwcOu93RfyAKiLlWCy4cgNrx454p7xS9CkT7\
G1sY0b8jkyECAwEAAaMTMBEwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQQF\
AAOBgQAt4plrsD16iddZopQBHyvdEktTwq1/qqcAXJFAVyVKOKqEcLnZgA+le1z7\
c8a914phXAPjLSeoF+CEhULcXpvGt7Jtu3Sv5D/Lp7ew4F2+eIMllNLbgQ95B21P\
9DkVWlIBe94y1k049hJcBlDfBVu9FEuh3ym6O0GN92NWod8isQ==\
-----END CERTIFICATE-----\
\
AddTrust External Root\
======================\
\
MD5 Fingerprint=1D:35:54:04:85:78:B0:3F:42:42:4D:BF:20:73:0A:3F\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=SE, O=AddTrust AB, OU=AddTrust External TTP Network, CN=AddTrust External CA Root\
        Validity\
            Not Before: May 30 10:48:38 2000 GMT\
            Not After : May 30 10:48:38 2020 GMT\
        Subject: C=SE, O=AddTrust AB, OU=AddTrust External TTP Network, CN=AddTrust External CA Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b7:f7:1a:33:e6:f2:00:04:2d:39:e0:4e:5b:ed:\
                    1f:bc:6c:0f:cd:b5:fa:23:b6:ce:de:9b:11:33:97:\
                    a4:29:4c:7d:93:9f:bd:4a:bc:93:ed:03:1a:e3:8f:\
                    cf:e5:6d:50:5a:d6:97:29:94:5a:80:b0:49:7a:db:\
                    2e:95:fd:b8:ca:bf:37:38:2d:1e:3e:91:41:ad:70:\
                    56:c7:f0:4f:3f:e8:32:9e:74:ca:c8:90:54:e9:c6:\
                    5f:0f:78:9d:9a:40:3c:0e:ac:61:aa:5e:14:8f:9e:\
                    87:a1:6a:50:dc:d7:9a:4e:af:05:b3:a6:71:94:9c:\
                    71:b3:50:60:0a:c7:13:9d:38:07:86:02:a8:e9:a8:\
                    69:26:18:90:ab:4c:b0:4f:23:ab:3a:4f:84:d8:df:\
                    ce:9f:e1:69:6f:bb:d7:42:d7:6b:44:e4:c7:ad:ee:\
                    6d:41:5f:72:5a:71:08:37:b3:79:65:a4:59:a0:94:\
                    37:f7:00:2f:0d:c2:92:72:da:d0:38:72:db:14:a8:\
                    45:c4:5d:2a:7d:b7:b4:d6:c4:ee:ac:cd:13:44:b7:\
                    c9:2b:dd:43:00:25:fa:61:b9:69:6a:58:23:11:b7:\
                    a7:33:8f:56:75:59:f5:cd:29:d7:46:b7:0a:2b:65:\
                    b6:d3:42:6f:15:b2:b8:7b:fb:ef:e9:5d:53:d5:34:\
                    5a:27\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                AD:BD:98:7A:34:B4:26:F7:FA:C4:26:54:EF:03:BD:E0:24:CB:54:1A\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Authority Key Identifier: \
                keyid:AD:BD:98:7A:34:B4:26:F7:FA:C4:26:54:EF:03:BD:E0:24:CB:54:1A\
                DirName:/C=SE/O=AddTrust AB/OU=AddTrust External TTP Network/CN=AddTrust External CA Root\
                serial:01\
\
    Signature Algorithm: sha1WithRSAEncryption\
        b0:9b:e0:85:25:c2:d6:23:e2:0f:96:06:92:9d:41:98:9c:d9:\
        84:79:81:d9:1e:5b:14:07:23:36:65:8f:b0:d8:77:bb:ac:41:\
        6c:47:60:83:51:b0:f9:32:3d:e7:fc:f6:26:13:c7:80:16:a5:\
        bf:5a:fc:87:cf:78:79:89:21:9a:e2:4c:07:0a:86:35:bc:f2:\
        de:51:c4:d2:96:b7:dc:7e:4e:ee:70:fd:1c:39:eb:0c:02:51:\
        14:2d:8e:bd:16:e0:c1:df:46:75:e7:24:ad:ec:f4:42:b4:85:\
        93:70:10:67:ba:9d:06:35:4a:18:d3:2b:7a:cc:51:42:a1:7a:\
        63:d1:e6:bb:a1:c5:2b:c2:36:be:13:0d:e6:bd:63:7e:79:7b:\
        a7:09:0d:40:ab:6a:dd:8f:8a:c3:f6:f6:8c:1a:42:05:51:d4:\
        45:f5:9f:a7:62:21:68:15:20:43:3c:99:e7:7c:bd:24:d8:a9:\
        91:17:73:88:3f:56:1b:31:38:18:b4:71:0f:9a:cd:c8:0e:9e:\
        8e:2e:1b:e1:8c:98:83:cb:1f:31:f1:44:4c:c6:04:73:49:76:\
        60:0f:c7:f8:bd:17:80:6b:2e:e9:cc:4c:0e:5a:9a:79:0f:20:\
        0a:2e:d5:9e:63:26:1e:55:92:94:d8:82:17:5a:7b:d0:bc:c7:\
        8f:4e:86:04\
-----BEGIN CERTIFICATE-----\
MIIENjCCAx6gAwIBAgIBATANBgkqhkiG9w0BAQUFADBvMQswCQYDVQQGEwJTRTEU\
MBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFkZFRydXN0IEV4dGVybmFs\
IFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBFeHRlcm5hbCBDQSBSb290\
MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFowbzELMAkGA1UEBhMCU0Ux\
FDASBgNVBAoTC0FkZFRydXN0IEFCMSYwJAYDVQQLEx1BZGRUcnVzdCBFeHRlcm5h\
bCBUVFAgTmV0d29yazEiMCAGA1UEAxMZQWRkVHJ1c3QgRXh0ZXJuYWwgQ0EgUm9v\
dDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALf3GjPm8gAELTngTlvt\
H7xsD821+iO2zt6bETOXpClMfZOfvUq8k+0DGuOPz+VtUFrWlymUWoCwSXrbLpX9\
uMq/NzgtHj6RQa1wVsfwTz/oMp50ysiQVOnGXw94nZpAPA6sYapeFI+eh6FqUNzX\
mk6vBbOmcZSccbNQYArHE504B4YCqOmoaSYYkKtMsE8jqzpPhNjfzp/haW+710LX\
a0Tkx63ubUFfclpxCDezeWWkWaCUN/cALw3CknLa0Dhy2xSoRcRdKn23tNbE7qzN\
E0S3ySvdQwAl+mG5aWpYIxG3pzOPVnVZ9c0p10a3CitlttNCbxWyuHv77+ldU9U0\
WicCAwEAAaOB3DCB2TAdBgNVHQ4EFgQUrb2YejS0Jvf6xCZU7wO94CTLVBowCwYD\
VR0PBAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wgZkGA1UdIwSBkTCBjoAUrb2YejS0\
Jvf6xCZU7wO94CTLVBqhc6RxMG8xCzAJBgNVBAYTAlNFMRQwEgYDVQQKEwtBZGRU\
cnVzdCBBQjEmMCQGA1UECxMdQWRkVHJ1c3QgRXh0ZXJuYWwgVFRQIE5ldHdvcmsx\
IjAgBgNVBAMTGUFkZFRydXN0IEV4dGVybmFsIENBIFJvb3SCAQEwDQYJKoZIhvcN\
AQEFBQADggEBALCb4IUlwtYj4g+WBpKdQZic2YR5gdkeWxQHIzZlj7DYd7usQWxH\
YINRsPkyPef89iYTx4AWpb9a/IfPeHmJIZriTAcKhjW88t5RxNKWt9x+Tu5w/Rw5\
6wwCURQtjr0W4MHfRnXnJK3s9EK0hZNwEGe6nQY1ShjTK3rMUUKhemPR5ruhxSvC\
Nr4TDea9Y355e6cJDUCrat2PisP29owaQgVR1EX1n6diIWgVIEM8med8vSTYqZEX\
c4g/VhsxOBi0cQ+azcgOno4uG+GMmIPLHzHxREzGBHNJdmAPx/i9F4BrLunMTA5a\
mnkPIAou1Z5jJh5VkpTYghdae9C8x49OhgQ=\
-----END CERTIFICATE-----\
\
AddTrust Public Services Root\
=============================\
\
MD5 Fingerprint=C1:62:3E:23:C5:82:73:9C:03:59:4B:2B:E9:77:49:7F\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=SE, O=AddTrust AB, OU=AddTrust TTP Network, CN=AddTrust Public CA Root\
        Validity\
            Not Before: May 30 10:41:50 2000 GMT\
            Not After : May 30 10:41:50 2020 GMT\
        Subject: C=SE, O=AddTrust AB, OU=AddTrust TTP Network, CN=AddTrust Public CA Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:e9:1a:30:8f:83:88:14:c1:20:d8:3c:9b:8f:1b:\
                    7e:03:74:bb:da:69:d3:46:a5:f8:8e:c2:0c:11:90:\
                    51:a5:2f:66:54:40:55:ea:db:1f:4a:56:ee:9f:23:\
                    6e:f4:39:cb:a1:b9:6f:f2:7e:f9:5d:87:26:61:9e:\
                    1c:f8:e2:ec:a6:81:f8:21:c5:24:cc:11:0c:3f:db:\
                    26:72:7a:c7:01:97:07:17:f9:d7:18:2c:30:7d:0e:\
                    7a:1e:62:1e:c6:4b:c0:fd:7d:62:77:d3:44:1e:27:\
                    f6:3f:4b:44:b3:b7:38:d9:39:1f:60:d5:51:92:73:\
                    03:b4:00:69:e3:f3:14:4e:ee:d1:dc:09:cf:77:34:\
                    46:50:b0:f8:11:f2:fe:38:79:f7:07:39:fe:51:92:\
                    97:0b:5b:08:5f:34:86:01:ad:88:97:eb:66:cd:5e:\
                    d1:ff:dc:7d:f2:84:da:ba:77:ad:dc:80:08:c7:a7:\
                    87:d6:55:9f:97:6a:e8:c8:11:64:ba:e7:19:29:3f:\
                    11:b3:78:90:84:20:52:5b:11:ef:78:d0:83:f6:d5:\
                    48:90:d0:30:1c:cf:80:f9:60:fe:79:e4:88:f2:dd:\
                    00:eb:94:45:eb:65:94:69:40:ba:c0:d5:b4:b8:ba:\
                    7d:04:11:a8:eb:31:05:96:94:4e:58:21:8e:9f:d0:\
                    60:fd\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                81:3E:37:D8:92:B0:1F:77:9F:5C:B4:AB:73:AA:E7:F6:34:60:2F:FA\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Authority Key Identifier: \
                keyid:81:3E:37:D8:92:B0:1F:77:9F:5C:B4:AB:73:AA:E7:F6:34:60:2F:FA\
                DirName:/C=SE/O=AddTrust AB/OU=AddTrust TTP Network/CN=AddTrust Public CA Root\
                serial:01\
\
    Signature Algorithm: sha1WithRSAEncryption\
        03:f7:15:4a:f8:24:da:23:56:16:93:76:dd:36:28:b9:ae:1b:\
        b8:c3:f1:64:ba:20:18:78:95:29:27:57:05:bc:7c:2a:f4:b9:\
        51:55:da:87:02:de:0f:16:17:31:f8:aa:79:2e:09:13:bb:af:\
        b2:20:19:12:e5:93:f9:4b:f9:83:e8:44:d5:b2:41:25:bf:88:\
        75:6f:ff:10:fc:4a:54:d0:5f:f0:fa:ef:36:73:7d:1b:36:45:\
        c6:21:6d:b4:15:b8:4e:cf:9c:5c:a5:3d:5a:00:8e:06:e3:3c:\
        6b:32:7b:f2:9f:f0:b6:fd:df:f0:28:18:48:f0:c6:bc:d0:bf:\
        34:80:96:c2:4a:b1:6d:8e:c7:90:45:de:2f:67:ac:45:04:a3:\
        7a:dc:55:92:c9:47:66:d8:1a:8c:c7:ed:9c:4e:9a:e0:12:bb:\
        b5:6a:4c:84:e1:e1:22:0d:87:00:64:fe:8c:7d:62:39:65:a6:\
        ef:42:b6:80:25:12:61:01:a8:24:13:70:00:11:26:5f:fa:35:\
        50:c5:48:cc:06:47:e8:27:d8:70:8d:5f:64:e6:a1:44:26:5e:\
        22:ec:92:cd:ff:42:9a:44:21:6d:5c:c5:e3:22:1d:5f:47:12:\
        e7:ce:5f:5d:fa:d8:aa:b1:33:2d:d9:76:f2:4e:3a:33:0c:2b:\
        b3:2d:90:06\
-----BEGIN CERTIFICATE-----\
MIIEFTCCAv2gAwIBAgIBATANBgkqhkiG9w0BAQUFADBkMQswCQYDVQQGEwJTRTEU\
MBIGA1UEChMLQWRkVHJ1c3QgQUIxHTAbBgNVBAsTFEFkZFRydXN0IFRUUCBOZXR3\
b3JrMSAwHgYDVQQDExdBZGRUcnVzdCBQdWJsaWMgQ0EgUm9vdDAeFw0wMDA1MzAx\
MDQxNTBaFw0yMDA1MzAxMDQxNTBaMGQxCzAJBgNVBAYTAlNFMRQwEgYDVQQKEwtB\
ZGRUcnVzdCBBQjEdMBsGA1UECxMUQWRkVHJ1c3QgVFRQIE5ldHdvcmsxIDAeBgNV\
BAMTF0FkZFRydXN0IFB1YmxpYyBDQSBSb290MIIBIjANBgkqhkiG9w0BAQEFAAOC\
AQ8AMIIBCgKCAQEA6Rowj4OIFMEg2Dybjxt+A3S72mnTRqX4jsIMEZBRpS9mVEBV\
6tsfSlbunyNu9DnLoblv8n75XYcmYZ4c+OLspoH4IcUkzBEMP9smcnrHAZcHF/nX\
GCwwfQ56HmIexkvA/X1id9NEHif2P0tEs7c42TkfYNVRknMDtABp4/MUTu7R3AnP\
dzRGULD4EfL+OHn3Bzn+UZKXC1sIXzSGAa2Il+tmzV7R/9x98oTaunet3IAIx6eH\
1lWfl2royBFkuucZKT8Rs3iQhCBSWxHveNCD9tVIkNAwHM+A+WD+eeSI8t0A65RF\
62WUaUC6wNW0uLp9BBGo6zEFlpROWCGOn9Bg/QIDAQABo4HRMIHOMB0GA1UdDgQW\
BBSBPjfYkrAfd59ctKtzquf2NGAv+jALBgNVHQ8EBAMCAQYwDwYDVR0TAQH/BAUw\
AwEB/zCBjgYDVR0jBIGGMIGDgBSBPjfYkrAfd59ctKtzquf2NGAv+qFopGYwZDEL\
MAkGA1UEBhMCU0UxFDASBgNVBAoTC0FkZFRydXN0IEFCMR0wGwYDVQQLExRBZGRU\
cnVzdCBUVFAgTmV0d29yazEgMB4GA1UEAxMXQWRkVHJ1c3QgUHVibGljIENBIFJv\
b3SCAQEwDQYJKoZIhvcNAQEFBQADggEBAAP3FUr4JNojVhaTdt02KLmuG7jD8WS6\
IBh4lSknVwW8fCr0uVFV2ocC3g8WFzH4qnkuCRO7r7IgGRLlk/lL+YPoRNWyQSW/\
iHVv/xD8SlTQX/D67zZzfRs2RcYhbbQVuE7PnFylPVoAjgbjPGsye/Kf8Lb93/Ao\
GEjwxrzQvzSAlsJKsW2Ox5BF3i9nrEUEo3rcVZLJR2bYGozH7ZxOmuASu7VqTITh\
4SINhwBk/ox9Yjllpu9CtoAlEmEBqCQTcAARJl/6NVDFSMwGR+gn2HCNX2TmoUQm\
XiLsks3/QppEIW1cxeMiHV9HEufOX1362KqxMy3ZdvJOOjMMK7MtkAY=\
-----END CERTIFICATE-----\
\
AddTrust Qualified Certificates Root\
====================================\
\
MD5 Fingerprint=27:EC:39:47:CD:DA:5A:AF:E2:9A:01:65:21:A9:4C:BB\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=SE, O=AddTrust AB, OU=AddTrust TTP Network, CN=AddTrust Qualified CA Root\
        Validity\
            Not Before: May 30 10:44:50 2000 GMT\
            Not After : May 30 10:44:50 2020 GMT\
        Subject: C=SE, O=AddTrust AB, OU=AddTrust TTP Network, CN=AddTrust Qualified CA Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:e4:1e:9a:fe:dc:09:5a:87:a4:9f:47:be:11:5f:\
                    af:84:34:db:62:3c:79:78:b7:e9:30:b5:ec:0c:1c:\
                    2a:c4:16:ff:e0:ec:71:eb:8a:f5:11:6e:ed:4f:0d:\
                    91:d2:12:18:2d:49:15:01:c2:a4:22:13:c7:11:64:\
                    ff:22:12:9a:b9:8e:5c:2f:08:cf:71:6a:b3:67:01:\
                    59:f1:5d:46:f3:b0:78:a5:f6:0e:42:7a:e3:7f:1b:\
                    cc:d0:f0:b7:28:fd:2a:ea:9e:b3:b0:b9:04:aa:fd:\
                    f6:c7:b4:b1:b8:2a:a0:fb:58:f1:19:a0:6f:70:25:\
                    7e:3e:69:4a:7f:0f:22:d8:ef:ad:08:11:9a:29:99:\
                    e1:aa:44:45:9a:12:5e:3e:9d:6d:52:fc:e7:a0:3d:\
                    68:2f:f0:4b:70:7c:13:38:ad:bc:15:25:f1:d6:ce:\
                    ab:a2:c0:31:d6:2f:9f:e0:ff:14:59:fc:84:93:d9:\
                    87:7c:4c:54:13:eb:9f:d1:2d:11:f8:18:3a:3a:de:\
                    25:d9:f7:d3:40:ed:a4:06:12:c4:3b:e1:91:c1:56:\
                    35:f0:14:dc:65:36:09:6e:ab:a4:07:c7:35:d1:c2:\
                    03:33:36:5b:75:26:6d:42:f1:12:6b:43:6f:4b:71:\
                    94:fa:34:1d:ed:13:6e:ca:80:7f:98:2f:6c:b9:65:\
                    d8:e9\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                39:95:8B:62:8B:5C:C9:D4:80:BA:58:0F:97:3F:15:08:43:CC:98:A7\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Authority Key Identifier: \
                keyid:39:95:8B:62:8B:5C:C9:D4:80:BA:58:0F:97:3F:15:08:43:CC:98:A7\
                DirName:/C=SE/O=AddTrust AB/OU=AddTrust TTP Network/CN=AddTrust Qualified CA Root\
                serial:01\
\
    Signature Algorithm: sha1WithRSAEncryption\
        19:ab:75:ea:f8:8b:65:61:95:13:ba:69:04:ef:86:ca:13:a0:\
        c7:aa:4f:64:1b:3f:18:f6:a8:2d:2c:55:8f:05:b7:30:ea:42:\
        6a:1d:c0:25:51:2d:a7:bf:0c:b3:ed:ef:08:7f:6c:3c:46:1a:\
        ea:18:43:df:76:cc:f9:66:86:9c:2c:68:f5:e9:17:f8:31:b3:\
        18:c4:d6:48:7d:23:4c:68:c1:7e:bb:01:14:6f:c5:d9:6e:de:\
        bb:04:42:6a:f8:f6:5c:7d:e5:da:fa:87:eb:0d:35:52:67:d0:\
        9e:97:76:05:93:3f:95:c7:01:e6:69:55:38:7f:10:61:99:c9:\
        e3:5f:a6:ca:3e:82:63:48:aa:e2:08:48:3e:aa:f2:b2:85:62:\
        a6:b4:a7:d9:bd:37:9c:68:b5:2d:56:7d:b0:b7:3f:a0:b1:07:\
        d6:e9:4f:dc:de:45:71:30:32:7f:1b:2e:09:f9:bf:52:a1:ee:\
        c2:80:3e:06:5c:2e:55:40:c1:1b:f5:70:45:b0:dc:5d:fa:f6:\
        72:5a:77:d2:63:cd:cf:58:89:00:42:63:3f:79:39:d0:44:b0:\
        82:6e:41:19:e8:dd:e0:c1:88:5a:d1:1e:71:93:1f:24:30:74:\
        e5:1e:a8:de:3c:27:37:7f:83:ae:9e:77:cf:f0:30:b1:ff:4b:\
        99:e8:c6:a1\
-----BEGIN CERTIFICATE-----\
MIIEHjCCAwagAwIBAgIBATANBgkqhkiG9w0BAQUFADBnMQswCQYDVQQGEwJTRTEU\
MBIGA1UEChMLQWRkVHJ1c3QgQUIxHTAbBgNVBAsTFEFkZFRydXN0IFRUUCBOZXR3\
b3JrMSMwIQYDVQQDExpBZGRUcnVzdCBRdWFsaWZpZWQgQ0EgUm9vdDAeFw0wMDA1\
MzAxMDQ0NTBaFw0yMDA1MzAxMDQ0NTBaMGcxCzAJBgNVBAYTAlNFMRQwEgYDVQQK\
EwtBZGRUcnVzdCBBQjEdMBsGA1UECxMUQWRkVHJ1c3QgVFRQIE5ldHdvcmsxIzAh\
BgNVBAMTGkFkZFRydXN0IFF1YWxpZmllZCBDQSBSb290MIIBIjANBgkqhkiG9w0B\
AQEFAAOCAQ8AMIIBCgKCAQEA5B6a/twJWoekn0e+EV+vhDTbYjx5eLfpMLXsDBwq\
xBb/4Oxx64r1EW7tTw2R0hIYLUkVAcKkIhPHEWT/IhKauY5cLwjPcWqzZwFZ8V1G\
87B4pfYOQnrjfxvM0PC3KP0q6p6zsLkEqv32x7SxuCqg+1jxGaBvcCV+PmlKfw8i\
2O+tCBGaKZnhqkRFmhJePp1tUvznoD1oL/BLcHwTOK28FSXx1s6rosAx1i+f4P8U\
WfyEk9mHfExUE+uf0S0R+Bg6Ot4l2ffTQO2kBhLEO+GRwVY18BTcZTYJbqukB8c1\
0cIDMzZbdSZtQvESa0NvS3GU+jQd7RNuyoB/mC9suWXY6QIDAQABo4HUMIHRMB0G\
A1UdDgQWBBQ5lYtii1zJ1IC6WA+XPxUIQ8yYpzALBgNVHQ8EBAMCAQYwDwYDVR0T\
AQH/BAUwAwEB/zCBkQYDVR0jBIGJMIGGgBQ5lYtii1zJ1IC6WA+XPxUIQ8yYp6Fr\
pGkwZzELMAkGA1UEBhMCU0UxFDASBgNVBAoTC0FkZFRydXN0IEFCMR0wGwYDVQQL\
ExRBZGRUcnVzdCBUVFAgTmV0d29yazEjMCEGA1UEAxMaQWRkVHJ1c3QgUXVhbGlm\
aWVkIENBIFJvb3SCAQEwDQYJKoZIhvcNAQEFBQADggEBABmrder4i2VhlRO6aQTv\
hsoToMeqT2QbPxj2qC0sVY8FtzDqQmodwCVRLae/DLPt7wh/bDxGGuoYQ992zPlm\
hpwsaPXpF/gxsxjE1kh9I0xowX67ARRvxdlu3rsEQmr49lx95dr6h+sNNVJn0J6X\
dgWTP5XHAeZpVTh/EGGZyeNfpso+gmNIquIISD6q8rKFYqa0p9m9N5xotS1WfbC3\
P6CxB9bpT9zeRXEwMn8bLgn5v1Kh7sKAPgZcLlVAwRv1cEWw3F369nJad9Jjzc9Y\
iQBCYz95OdBEsIJuQRno3eDBiFrRHnGTHyQwdOUeqN48Jzd/g66ed8/wMLH/S5no\
xqE=\
-----END CERTIFICATE-----\
\
Verisign Class 1 Public Primary OCSP Responder\
==============================================\
\
MD5 Fingerprint=7E:6F:3A:53:1B:7C:BE:B0:30:DB:43:1E:1E:94:89:B2\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            2b:68:d4:a3:46:9e:c5:3b:28:09:ab:38:5d:7f:27:20\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 1 Public Primary Certification Authority\
        Validity\
            Not Before: Aug  4 00:00:00 2000 GMT\
            Not After : Aug  3 23:59:59 2004 GMT\
        Subject: O=VeriSign, Inc., OU=VeriSign Trust Network, OU=Terms of use at https://www.verisign.com/RPA (c)00, CN=Class 1 Public Primary OCSP Responder\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:b9:ed:5e:7a:3a:77:5f:ce:5f:3a:52:fc:cd:64:\
                    f7:71:b5:6f:6a:96:c6:59:92:55:94:5d:2f:5b:2e:\
                    c1:11:ea:26:8a:cb:a7:81:3c:f6:5a:44:de:7a:13:\
                    2f:fd:5a:51:d9:7b:37:26:4a:c0:27:3f:04:03:6a:\
                    56:c1:83:2c:e1:6f:5b:a9:54:50:24:4a:c6:2e:7a:\
                    4c:a1:5b:37:54:24:21:31:1f:a1:78:18:76:a7:b1:\
                    70:da:22:d0:6a:fe:07:62:40:c6:f7:f6:9b:7d:0c:\
                    06:b8:4b:c7:28:e4:66:23:84:51:ef:46:b7:93:d8:\
                    81:33:cb:e5:36:ac:c6:e8:05\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Alternative Name: \
                DirName:/CN=OCSP 1-1\
            X509v3 CRL Distribution Points: \
                URI:http://crl.verisign.com/pca1.crl\
\
            X509v3 Extended Key Usage: \
                OCSP Signing\
            Authority Information Access: \
                OCSP - URI:http://ocsp.verisign.com/ocsp/status\
\
            X509v3 Certificate Policies: \
                Policy: 2.16.840.1.113733.1.7.1.1\
                  CPS: https://www.verisign.com/RPA\
\
            X509v3 Basic Constraints: \
                CA:FALSE\
            X509v3 Key Usage: \
                Digital Signature\
    Signature Algorithm: sha1WithRSAEncryption\
        70:90:dd:b8:e4:be:53:17:7c:7f:02:e9:d5:f7:8b:99:93:31:\
        60:8d:7e:e6:60:6b:24:ef:60:ac:d2:ce:91:de:80:6d:09:a4:\
        d3:b8:38:e5:44:ca:72:5e:0d:2d:c1:77:9c:bd:2c:03:78:29:\
        8d:a4:a5:77:87:f5:f1:2b:26:ad:cc:07:6c:3a:54:5a:28:e0:\
        09:f3:4d:0a:04:ca:d4:58:69:0b:a7:b3:f5:dd:01:a5:e7:dc:\
        f0:1f:ba:c1:5d:90:8d:b3:ea:4f:c1:11:59:97:6a:b2:2b:13:\
        b1:da:ad:97:a1:b3:b1:a0:20:5b:ca:32:ab:8d:cf:13:f0:1f:\
        29:c3\
-----BEGIN CERTIFICATE-----\
MIIDnjCCAwegAwIBAgIQK2jUo0aexTsoCas4XX8nIDANBgkqhkiG9w0BAQUFADBf\
MQswCQYDVQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xNzA1BgNVBAsT\
LkNsYXNzIDEgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkw\
HhcNMDAwODA0MDAwMDAwWhcNMDQwODAzMjM1OTU5WjCBpzEXMBUGA1UEChMOVmVy\
aVNpZ24sIEluYy4xHzAdBgNVBAsTFlZlcmlTaWduIFRydXN0IE5ldHdvcmsxOzA5\
BgNVBAsTMlRlcm1zIG9mIHVzZSBhdCBodHRwczovL3d3dy52ZXJpc2lnbi5jb20v\
UlBBIChjKTAwMS4wLAYDVQQDEyVDbGFzcyAxIFB1YmxpYyBQcmltYXJ5IE9DU1Ag\
UmVzcG9uZGVyMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC57V56Ondfzl86\
UvzNZPdxtW9qlsZZklWUXS9bLsER6iaKy6eBPPZaRN56Ey/9WlHZezcmSsAnPwQD\
albBgyzhb1upVFAkSsYuekyhWzdUJCExH6F4GHansXDaItBq/gdiQMb39pt9DAa4\
S8co5GYjhFHvRreT2IEzy+U2rMboBQIDAQABo4IBEDCCAQwwIAYDVR0RBBkwF6QV\
MBMxETAPBgNVBAMTCE9DU1AgMS0xMDEGA1UdHwQqMCgwJqAkoCKGIGh0dHA6Ly9j\
cmwudmVyaXNpZ24uY29tL3BjYTEuY3JsMBMGA1UdJQQMMAoGCCsGAQUFBwMJMEIG\
CCsGAQUFBwEBBDYwNDAyBggrBgEFBQcwAaYmFiRodHRwOi8vb2NzcC52ZXJpc2ln\
bi5jb20vb2NzcC9zdGF0dXMwRAYDVR0gBD0wOzA5BgtghkgBhvhFAQcBATAqMCgG\
CCsGAQUFBwIBFhxodHRwczovL3d3dy52ZXJpc2lnbi5jb20vUlBBMAkGA1UdEwQC\
MAAwCwYDVR0PBAQDAgeAMA0GCSqGSIb3DQEBBQUAA4GBAHCQ3bjkvlMXfH8C6dX3\
i5mTMWCNfuZgayTvYKzSzpHegG0JpNO4OOVEynJeDS3Bd5y9LAN4KY2kpXeH9fEr\
Jq3MB2w6VFoo4AnzTQoEytRYaQuns/XdAaXn3PAfusFdkI2z6k/BEVmXarIrE7Ha\
rZehs7GgIFvKMquNzxPwHynD\
-----END CERTIFICATE-----\
\
Verisign Class 2 Public Primary OCSP Responder\
==============================================\
\
MD5 Fingerprint=F3:45:BD:10:96:0D:85:4B:EF:9F:11:62:34:A7:5E:B5\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            09:46:17:e6:1d:d8:d4:1c:a0:0c:a0:62:e8:79:8a:a7\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 2 Public Primary Certification Authority\
        Validity\
            Not Before: Aug  1 00:00:00 2000 GMT\
            Not After : Jul 31 23:59:59 2004 GMT\
        Subject: O=VeriSign, Inc., OU=VeriSign Trust Network, OU=Terms of use at https://www.verisign.com/RPA (c)00, CN=Class 2 Public Primary OCSP Responder\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:d0:ca:63:31:61:7f:44:34:7c:05:7d:0b:3d:6a:\
                    90:cb:79:4b:77:0a:3f:4b:c7:23:e5:c0:62:2d:7e:\
                    9c:7e:3e:88:87:91:d0:ac:e8:4d:49:87:a2:96:90:\
                    8a:dd:04:a5:02:3f:8c:9b:e9:89:fe:62:a0:e2:5a:\
                    bd:c8:dd:b4:78:e6:a5:42:93:08:67:01:c0:20:4d:\
                    d7:5c:f4:5d:da:b3:e3:37:a6:52:1a:2c:4c:65:4d:\
                    8a:87:d9:a8:a3:f1:49:54:bb:3c:5c:80:51:68:c6:\
                    fb:49:ff:0b:55:ab:15:dd:fb:9a:c1:b9:1d:74:0d:\
                    b2:8c:44:5d:89:fc:9f:f9:83\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Alternative Name: \
                DirName:/CN=OCSP 1-2\
            X509v3 CRL Distribution Points: \
                URI:http://crl.verisign.com/pca2.crl\
\
            X509v3 Extended Key Usage: \
                OCSP Signing\
            Authority Information Access: \
                OCSP - URI:http://ocsp.verisign.com/ocsp/status\
\
            X509v3 Certificate Policies: \
                Policy: 2.16.840.1.113733.1.7.1.1\
                  CPS: https://www.verisign.com/RPA\
\
            X509v3 Basic Constraints: \
                CA:FALSE\
            X509v3 Key Usage: \
                Digital Signature\
    Signature Algorithm: sha1WithRSAEncryption\
        1f:7d:09:6e:24:46:75:04:9c:f3:26:9b:e3:39:6e:17:ef:bc:\
        bd:a2:1b:d2:02:84:86:ab:d0:40:97:2c:c4:43:88:37:19:6b:\
        22:a8:03:71:50:9d:20:dc:36:60:20:9a:73:2d:73:55:6c:58:\
        9b:2c:c2:b4:34:2c:7a:33:42:ca:91:d9:e9:43:af:cf:1e:e0:\
        f5:c4:7a:ab:3f:72:63:1e:a9:37:e1:5b:3b:88:b3:13:86:82:\
        90:57:cb:57:ff:f4:56:be:22:dd:e3:97:a8:e1:bc:22:43:c2:\
        dd:4d:db:f6:81:9e:92:14:9e:39:0f:13:54:de:82:d8:c0:5e:\
        34:8d\
-----BEGIN CERTIFICATE-----\
MIIDnjCCAwegAwIBAgIQCUYX5h3Y1BygDKBi6HmKpzANBgkqhkiG9w0BAQUFADBf\
MQswCQYDVQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xNzA1BgNVBAsT\
LkNsYXNzIDIgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkw\
HhcNMDAwODAxMDAwMDAwWhcNMDQwNzMxMjM1OTU5WjCBpzEXMBUGA1UEChMOVmVy\
aVNpZ24sIEluYy4xHzAdBgNVBAsTFlZlcmlTaWduIFRydXN0IE5ldHdvcmsxOzA5\
BgNVBAsTMlRlcm1zIG9mIHVzZSBhdCBodHRwczovL3d3dy52ZXJpc2lnbi5jb20v\
UlBBIChjKTAwMS4wLAYDVQQDEyVDbGFzcyAyIFB1YmxpYyBQcmltYXJ5IE9DU1Ag\
UmVzcG9uZGVyMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDQymMxYX9ENHwF\
fQs9apDLeUt3Cj9LxyPlwGItfpx+PoiHkdCs6E1Jh6KWkIrdBKUCP4yb6Yn+YqDi\
Wr3I3bR45qVCkwhnAcAgTddc9F3as+M3plIaLExlTYqH2aij8UlUuzxcgFFoxvtJ\
/wtVqxXd+5rBuR10DbKMRF2J/J/5gwIDAQABo4IBEDCCAQwwIAYDVR0RBBkwF6QV\
MBMxETAPBgNVBAMTCE9DU1AgMS0yMDEGA1UdHwQqMCgwJqAkoCKGIGh0dHA6Ly9j\
cmwudmVyaXNpZ24uY29tL3BjYTIuY3JsMBMGA1UdJQQMMAoGCCsGAQUFBwMJMEIG\
CCsGAQUFBwEBBDYwNDAyBggrBgEFBQcwAaYmFiRodHRwOi8vb2NzcC52ZXJpc2ln\
bi5jb20vb2NzcC9zdGF0dXMwRAYDVR0gBD0wOzA5BgtghkgBhvhFAQcBATAqMCgG\
CCsGAQUFBwIBFhxodHRwczovL3d3dy52ZXJpc2lnbi5jb20vUlBBMAkGA1UdEwQC\
MAAwCwYDVR0PBAQDAgeAMA0GCSqGSIb3DQEBBQUAA4GBAB99CW4kRnUEnPMmm+M5\
bhfvvL2iG9IChIar0ECXLMRDiDcZayKoA3FQnSDcNmAgmnMtc1VsWJsswrQ0LHoz\
QsqR2elDr88e4PXEeqs/cmMeqTfhWzuIsxOGgpBXy1f/9Fa+It3jl6jhvCJDwt1N\
2/aBnpIUnjkPE1TegtjAXjSN\
-----END CERTIFICATE-----\
\
Verisign Class 3 Public Primary OCSP Responder\
==============================================\
\
MD5 Fingerprint=7D:51:92:C9:76:83:98:16:DE:8C:B3:86:C4:7D:66:FB\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            2e:96:9e:bf:b6:62:6c:ec:7b:e9:73:cc:e3:6c:c1:84\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 3 Public Primary Certification Authority\
        Validity\
            Not Before: Aug  4 00:00:00 2000 GMT\
            Not After : Aug  3 23:59:59 2004 GMT\
        Subject: O=VeriSign, Inc., OU=VeriSign Trust Network, OU=Terms of use at https://www.verisign.com/RPA (c)00, CN=Class 3 Public Primary OCSP Responder\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:f1:e4:08:0e:83:bb:75:e3:48:e5:b8:db:a6:f0:\
                    b9:ab:e9:3c:62:c7:5e:35:5b:d0:02:54:11:d8:c9:\
                    d1:56:b9:76:4b:b9:ab:7a:e6:cd:ba:f6:0c:04:d6:\
                    7e:d6:b0:0a:65:ac:4e:39:e3:f1:f7:2d:a3:25:39:\
                    ef:b0:8b:cf:be:db:0c:5d:6e:70:f4:07:cd:70:f7:\
                    3a:c0:3e:35:16:ed:78:8c:43:cf:c2:26:2e:47:d6:\
                    86:7d:9c:f1:be:d6:67:0c:22:25:a4:ca:65:e6:1f:\
                    7a:78:28:2f:3f:05:db:04:21:bf:e1:45:66:fe:3c:\
                    b7:82:ed:5a:b8:16:15:b9:55\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Alternative Name: \
                DirName:/CN=OCSP 1-3\
            X509v3 CRL Distribution Points: \
                URI:http://crl.verisign.com/pca3.1.1.crl\
\
            X509v3 Extended Key Usage: \
                OCSP Signing\
            Authority Information Access: \
                OCSP - URI:http://ocsp.verisign.com/ocsp/status\
\
            X509v3 Certificate Policies: \
                Policy: 2.16.840.1.113733.1.7.1.1\
                  CPS: https://www.verisign.com/RPA\
\
            X509v3 Basic Constraints: \
                CA:FALSE\
            X509v3 Key Usage: \
                Digital Signature\
    Signature Algorithm: sha1WithRSAEncryption\
        02:f6:53:63:c0:a9:1e:f2:d0:8b:33:30:8f:48:9b:4c:b0:56:\
        b4:83:71:4a:be:dc:50:d8:f5:b6:e0:0b:db:bd:78:4f:e9:cf:\
        09:34:da:29:49:9d:01:73:5a:91:91:82:54:2c:13:0a:d3:77:\
        23:cf:37:fc:63:de:a7:e3:f6:b7:b5:69:45:28:49:c3:91:dc:\
        aa:47:1c:a9:88:99:2c:05:2a:8d:8d:8a:fa:62:e2:5a:b7:00:\
        20:5d:39:c4:28:c2:cb:fc:9e:a8:89:ae:5b:3d:8e:12:ea:32:\
        b2:fc:eb:14:d7:09:15:1a:c0:cd:1b:d5:b5:15:4e:41:d5:96:\
        e3:4e\
-----BEGIN CERTIFICATE-----\
MIIDojCCAwugAwIBAgIQLpaev7ZibOx76XPM42zBhDANBgkqhkiG9w0BAQUFADBf\
MQswCQYDVQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xNzA1BgNVBAsT\
LkNsYXNzIDMgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkw\
HhcNMDAwODA0MDAwMDAwWhcNMDQwODAzMjM1OTU5WjCBpzEXMBUGA1UEChMOVmVy\
aVNpZ24sIEluYy4xHzAdBgNVBAsTFlZlcmlTaWduIFRydXN0IE5ldHdvcmsxOzA5\
BgNVBAsTMlRlcm1zIG9mIHVzZSBhdCBodHRwczovL3d3dy52ZXJpc2lnbi5jb20v\
UlBBIChjKTAwMS4wLAYDVQQDEyVDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IE9DU1Ag\
UmVzcG9uZGVyMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDx5AgOg7t140jl\
uNum8Lmr6Txix141W9ACVBHYydFWuXZLuat65s269gwE1n7WsAplrE454/H3LaMl\
Oe+wi8++2wxdbnD0B81w9zrAPjUW7XiMQ8/CJi5H1oZ9nPG+1mcMIiWkymXmH3p4\
KC8/BdsEIb/hRWb+PLeC7Vq4FhW5VQIDAQABo4IBFDCCARAwIAYDVR0RBBkwF6QV\
MBMxETAPBgNVBAMTCE9DU1AgMS0zMDUGA1UdHwQuMCwwKqAooCaGJGh0dHA6Ly9j\
cmwudmVyaXNpZ24uY29tL3BjYTMuMS4xLmNybDATBgNVHSUEDDAKBggrBgEFBQcD\
CTBCBggrBgEFBQcBAQQ2MDQwMgYIKwYBBQUHMAGmJhYkaHR0cDovL29jc3AudmVy\
aXNpZ24uY29tL29jc3Avc3RhdHVzMEQGA1UdIAQ9MDswOQYLYIZIAYb4RQEHAQEw\
KjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cudmVyaXNpZ24uY29tL1JQQTAJBgNV\
HRMEAjAAMAsGA1UdDwQEAwIHgDANBgkqhkiG9w0BAQUFAAOBgQAC9lNjwKke8tCL\
MzCPSJtMsFa0g3FKvtxQ2PW24AvbvXhP6c8JNNopSZ0Bc1qRkYJULBMK03cjzzf8\
Y96n4/a3tWlFKEnDkdyqRxypiJksBSqNjYr6YuJatwAgXTnEKMLL/J6oia5bPY4S\
6jKy/OsU1wkVGsDNG9W1FU5B1ZbjTg==\
-----END CERTIFICATE-----\
\
Verisign Secure Server OCSP Responder\
=====================================\
\
MD5 Fingerprint=2C:62:C3:D8:80:01:16:09:EA:59:EA:78:AB:10:43:F6\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            ff:45:d5:27:5d:24:fb:b3:c2:39:24:53:57:e1:4f:de\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=RSA Data Security, Inc., OU=Secure Server Certification Authority\
        Validity\
            Not Before: Aug  4 00:00:00 2000 GMT\
            Not After : Aug  3 23:59:59 2004 GMT\
        Subject: O=VeriSign, Inc., OU=VeriSign Trust Network, OU=Terms of use at https://www.verisign.com/RPA (c)00, CN=Secure Server OCSP Responder\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:b8:51:99:64:85:0e:ee:b3:0a:68:f0:bf:63:76:\
                    1d:53:f5:fc:a1:78:8c:33:ee:9f:f4:be:39:da:9b:\
                    0f:4d:47:a9:8f:20:e8:4b:44:bd:ce:cd:7b:90:d1:\
                    30:e8:90:c4:25:7b:89:28:de:bd:f6:93:1d:ff:b9:\
                    ff:92:b5:a9:8d:e4:ae:cc:e2:c3:07:83:6a:a3:72:\
                    10:01:27:62:22:a6:35:26:39:2d:9e:cf:60:0c:fc:\
                    47:a4:d7:d0:42:78:a7:1d:6c:d0:cb:4f:15:a7:29:\
                    0a:b4:95:45:c4:b1:e7:5a:09:d7:39:95:d8:1d:35:\
                    9e:c2:bd:b3:5d:c1:0c:4b:1f\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Alternative Name: \
                DirName:/CN=OCSP 1-4\
            X509v3 CRL Distribution Points: \
                URI:http://crl.verisign.com/RSASecureServer-p.crl\
\
            X509v3 Extended Key Usage: \
                OCSP Signing\
            Authority Information Access: \
                OCSP - URI:http://ocsp.verisign.com/ocsp/status\
\
            X509v3 Certificate Policies: \
                Policy: 2.16.840.1.113733.1.7.1.1\
                  CPS: https://www.verisign.com/RPA\
\
            X509v3 Basic Constraints: \
                CA:FALSE\
            X509v3 Key Usage: \
                Digital Signature\
    Signature Algorithm: sha1WithRSAEncryption\
        00:b3:10:53:66:9c:49:93:2e:31:a0:02:42:d2:58:57:7e:66:\
        a1:fe:1b:8a:61:18:50:40:2c:1e:2b:41:a5:d6:db:ff:ac:08:\
        1c:5a:05:6d:02:5c:2a:b6:96:4f:47:db:be:4e:db:ce:cc:ba:\
        86:b8:18:ce:b1:12:91:5f:63:f7:f3:48:3e:cc:f1:4d:13:e4:\
        6d:09:94:78:00:92:cb:a3:20:9d:06:0b:6a:a0:43:07:ce:d1:\
        19:6c:8f:18:75:9a:9f:17:33:fd:a9:26:b8:e3:e2:de:c2:a8:\
        c4:5a:8a:7f:98:d6:07:06:6b:cc:56:9e:86:70:ce:d4:ef\
-----BEGIN CERTIFICATE-----\
MIIDnzCCAwygAwIBAgIRAP9F1SddJPuzwjkkU1fhT94wDQYJKoZIhvcNAQEFBQAw\
XzELMAkGA1UEBhMCVVMxIDAeBgNVBAoTF1JTQSBEYXRhIFNlY3VyaXR5LCBJbmMu\
MS4wLAYDVQQLEyVTZWN1cmUgU2VydmVyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5\
MB4XDTAwMDgwNDAwMDAwMFoXDTA0MDgwMzIzNTk1OVowgZ4xFzAVBgNVBAoTDlZl\
cmlTaWduLCBJbmMuMR8wHQYDVQQLExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTsw\
OQYDVQQLEzJUZXJtcyBvZiB1c2UgYXQgaHR0cHM6Ly93d3cudmVyaXNpZ24uY29t\
L1JQQSAoYykwMDElMCMGA1UEAxMcU2VjdXJlIFNlcnZlciBPQ1NQIFJlc3BvbmRl\
cjCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAuFGZZIUO7rMKaPC/Y3YdU/X8\
oXiMM+6f9L452psPTUepjyDoS0S9zs17kNEw6JDEJXuJKN699pMd/7n/krWpjeSu\
zOLDB4Nqo3IQASdiIqY1Jjktns9gDPxHpNfQQninHWzQy08VpykKtJVFxLHnWgnX\
OZXYHTWewr2zXcEMSx8CAwEAAaOCAR0wggEZMCAGA1UdEQQZMBekFTATMREwDwYD\
VQQDEwhPQ1NQIDEtNDA+BgNVHR8ENzA1MDOgMaAvhi1odHRwOi8vY3JsLnZlcmlz\
aWduLmNvbS9SU0FTZWN1cmVTZXJ2ZXItcC5jcmwwEwYDVR0lBAwwCgYIKwYBBQUH\
AwkwQgYIKwYBBQUHAQEENjA0MDIGCCsGAQUFBzABpiYWJGh0dHA6Ly9vY3NwLnZl\
cmlzaWduLmNvbS9vY3NwL3N0YXR1czBEBgNVHSAEPTA7MDkGC2CGSAGG+EUBBwEB\
MCowKAYIKwYBBQUHAgEWHGh0dHBzOi8vd3d3LnZlcmlzaWduLmNvbS9SUEEwCQYD\
VR0TBAIwADALBgNVHQ8EBAMCB4AwDQYJKoZIhvcNAQEFBQADfgAAsxBTZpxJky4x\
oAJC0lhXfmah/huKYRhQQCweK0Gl1tv/rAgcWgVtAlwqtpZPR9u+TtvOzLqGuBjO\
sRKRX2P380g+zPFNE+RtCZR4AJLLoyCdBgtqoEMHztEZbI8YdZqfFzP9qSa44+Le\
wqjEWop/mNYHBmvMVp6GcM7U7w==\
-----END CERTIFICATE-----\
\
Verisign Time Stamping Authority CA\
===================================\
\
MD5 Fingerprint=89:49:54:8C:C8:68:9A:83:29:EC:DC:06:73:21:AB:97\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            53:61:b2:60:ae:db:71:8e:a7:94:b3:13:33:f4:07:09\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VeriSign, Inc., OU=Class 3 Public Primary Certification Authority - G2, OU=(c) 1998 VeriSign, Inc. - For authorized use only, OU=VeriSign Trust Network\
        Validity\
            Not Before: Sep 26 00:00:00 2000 GMT\
            Not After : Sep 25 23:59:59 2010 GMT\
        Subject: O=VeriSign, Inc., OU=VeriSign Trust Network, OU=Terms of use at https://www.verisign.com/rpa (c)00, CN=VeriSign Time Stamping Authority CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:d2:19:9d:67:c2:00:21:59:62:ce:b4:09:22:44:\
                    69:8a:f8:25:5a:db:ed:0d:b7:36:7e:4e:e0:bb:94:\
                    3e:90:25:87:c2:61:47:29:d9:bd:54:b8:63:cc:2c:\
                    7d:69:b4:33:36:f4:37:07:9a:c1:dd:40:54:fc:e0:\
                    78:9d:a0:93:b9:09:3d:23:51:7f:44:c2:14:74:db:\
                    0a:be:cb:c9:30:34:40:98:3e:d0:d7:25:10:81:94:\
                    bd:07:4f:9c:d6:54:27:df:2e:a8:bf:cb:90:8c:8d:\
                    75:4b:bc:e2:e8:44:87:cd:e6:41:0a:25:6e:e8:f4:\
                    24:02:c5:52:0f:6e:ec:98:75\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: \
                CA:TRUE, pathlen:0\
            X509v3 Certificate Policies: \
                Policy: 2.16.840.1.113733.1.7.23.1.3\
                  CPS: https://www.verisign.com/rpa\
\
            X509v3 CRL Distribution Points: \
                URI:http://crl.verisign.com/pca3.crl\
\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            Authority Information Access: \
                OCSP - URI:http://ocsp.verisign.com/ocsp/status\
\
    Signature Algorithm: sha1WithRSAEncryption\
        82:70:68:95:df:b6:0d:c2:01:70:19:4a:d2:54:56:1e:ac:f2:\
        45:4c:87:b8:f5:35:eb:78:4b:05:a9:c8:9d:3b:19:21:2e:70:\
        34:4a:a2:f5:89:e0:15:75:45:e7:28:37:00:34:27:29:e8:37:\
        4b:f2:ef:44:97:6b:17:51:1a:c3:56:9d:3c:1a:8a:f6:4a:46:\
        46:37:8c:fa:cb:f5:64:5a:38:68:2e:1c:c3:ef:70:ce:b8:46:\
        06:16:bf:f7:7e:e7:b5:a8:3e:45:ac:a9:25:75:22:7b:6f:3f:\
        b0:9c:94:e7:c7:73:ab:ac:1f:ee:25:9b:c0:16:ed:b7:ca:5b:\
        f0:14\
-----BEGIN CERTIFICATE-----\
MIIDzTCCAzagAwIBAgIQU2GyYK7bcY6nlLMTM/QHCTANBgkqhkiG9w0BAQUFADCB\
wTELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMTwwOgYDVQQL\
EzNDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5\
IC0gRzIxOjA4BgNVBAsTMShjKSAxOTk4IFZlcmlTaWduLCBJbmMuIC0gRm9yIGF1\
dGhvcml6ZWQgdXNlIG9ubHkxHzAdBgNVBAsTFlZlcmlTaWduIFRydXN0IE5ldHdv\
cmswHhcNMDAwOTI2MDAwMDAwWhcNMTAwOTI1MjM1OTU5WjCBpTEXMBUGA1UEChMO\
VmVyaVNpZ24sIEluYy4xHzAdBgNVBAsTFlZlcmlTaWduIFRydXN0IE5ldHdvcmsx\
OzA5BgNVBAsTMlRlcm1zIG9mIHVzZSBhdCBodHRwczovL3d3dy52ZXJpc2lnbi5j\
b20vcnBhIChjKTAwMSwwKgYDVQQDEyNWZXJpU2lnbiBUaW1lIFN0YW1waW5nIEF1\
dGhvcml0eSBDQTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA0hmdZ8IAIVli\
zrQJIkRpivglWtvtDbc2fk7gu5Q+kCWHwmFHKdm9VLhjzCx9abQzNvQ3B5rB3UBU\
/OB4naCTuQk9I1F/RMIUdNsKvsvJMDRAmD7Q1yUQgZS9B0+c1lQn3y6ov8uQjI11\
S7zi6ESHzeZBCiVu6PQkAsVSD27smHUCAwEAAaOB3zCB3DAPBgNVHRMECDAGAQH/\
AgEAMEUGA1UdIAQ+MDwwOgYMYIZIAYb4RQEHFwEDMCowKAYIKwYBBQUHAgEWHGh0\
dHBzOi8vd3d3LnZlcmlzaWduLmNvbS9ycGEwMQYDVR0fBCowKDAmoCSgIoYgaHR0\
cDovL2NybC52ZXJpc2lnbi5jb20vcGNhMy5jcmwwCwYDVR0PBAQDAgEGMEIGCCsG\
AQUFBwEBBDYwNDAyBggrBgEFBQcwAaYmFiRodHRwOi8vb2NzcC52ZXJpc2lnbi5j\
b20vb2NzcC9zdGF0dXMwDQYJKoZIhvcNAQEFBQADgYEAgnBold+2DcIBcBlK0lRW\
HqzyRUyHuPU163hLBanInTsZIS5wNEqi9YngFXVF5yg3ADQnKeg3S/LvRJdrF1Ea\
w1adPBqK9kpGRjeM+sv1ZFo4aC4cw+9wzrhGBha/937ntag+RaypJXUie28/sJyU\
58dzq6wf7iWbwBbtt8pb8BQ=\
-----END CERTIFICATE-----\
\
Thawte Time Stamping CA\
=======================\
\
MD5 Fingerprint=7F:66:7A:71:D3:EB:69:78:20:9A:51:14:9D:83:DA:20\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=ZA, ST=Western Cape, L=Durbanville, O=Thawte, OU=Thawte Certification, CN=Thawte Timestamping CA\
        Validity\
            Not Before: Jan  1 00:00:00 1997 GMT\
            Not After : Dec 31 23:59:59 2020 GMT\
        Subject: C=ZA, ST=Western Cape, L=Durbanville, O=Thawte, OU=Thawte Certification, CN=Thawte Timestamping CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:d6:2b:58:78:61:45:86:53:ea:34:7b:51:9c:ed:\
                    b0:e6:2e:18:0e:fe:e0:5f:a8:27:d3:b4:c9:e0:7c:\
                    59:4e:16:0e:73:54:60:c1:7f:f6:9f:2e:e9:3a:85:\
                    24:15:3c:db:47:04:63:c3:9e:c4:94:1a:5a:df:4c:\
                    7a:f3:d9:43:1d:3c:10:7a:79:25:db:90:fe:f0:51:\
                    e7:30:d6:41:00:fd:9f:28:df:79:be:94:bb:9d:b6:\
                    14:e3:23:85:d7:a9:41:e0:4c:a4:79:b0:2b:1a:8b:\
                    f2:f8:3b:8a:3e:45:ac:71:92:00:b4:90:41:98:fb:\
                    5f:ed:fa:b7:2e:8a:f8:88:37\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
    Signature Algorithm: md5WithRSAEncryption\
        67:db:e2:c2:e6:87:3d:40:83:86:37:35:7d:1f:ce:9a:c3:0c:\
        66:20:a8:ba:aa:04:89:86:c2:f5:10:08:0d:bf:cb:a2:05:8a:\
        d0:4d:36:3e:f4:d7:ef:69:c6:5e:e4:b0:94:6f:4a:b9:e7:de:\
        5b:88:b6:7b:db:e3:27:e5:76:c3:f0:35:c1:cb:b5:27:9b:33:\
        79:dc:90:a6:00:9e:77:fa:fc:cd:27:94:42:16:9c:d3:1c:68:\
        ec:bf:5c:dd:e5:a9:7b:10:0a:32:74:54:13:31:8b:85:03:84:\
        91:b7:58:01:30:14:38:af:28:ca:fc:b1:50:19:19:09:ac:89:\
        49:d3\
-----BEGIN CERTIFICATE-----\
MIICoTCCAgqgAwIBAgIBADANBgkqhkiG9w0BAQQFADCBizELMAkGA1UEBhMCWkEx\
FTATBgNVBAgTDFdlc3Rlcm4gQ2FwZTEUMBIGA1UEBxMLRHVyYmFudmlsbGUxDzAN\
BgNVBAoTBlRoYXd0ZTEdMBsGA1UECxMUVGhhd3RlIENlcnRpZmljYXRpb24xHzAd\
BgNVBAMTFlRoYXd0ZSBUaW1lc3RhbXBpbmcgQ0EwHhcNOTcwMTAxMDAwMDAwWhcN\
MjAxMjMxMjM1OTU5WjCBizELMAkGA1UEBhMCWkExFTATBgNVBAgTDFdlc3Rlcm4g\
Q2FwZTEUMBIGA1UEBxMLRHVyYmFudmlsbGUxDzANBgNVBAoTBlRoYXd0ZTEdMBsG\
A1UECxMUVGhhd3RlIENlcnRpZmljYXRpb24xHzAdBgNVBAMTFlRoYXd0ZSBUaW1l\
c3RhbXBpbmcgQ0EwgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBANYrWHhhRYZT\
6jR7UZztsOYuGA7+4F+oJ9O0yeB8WU4WDnNUYMF/9p8u6TqFJBU820cEY8OexJQa\
Wt9MevPZQx08EHp5JduQ/vBR5zDWQQD9nyjfeb6Uu522FOMjhdepQeBMpHmwKxqL\
8vg7ij5FrHGSALSQQZj7X+36ty6K+Ig3AgMBAAGjEzARMA8GA1UdEwEB/wQFMAMB\
Af8wDQYJKoZIhvcNAQEEBQADgYEAZ9viwuaHPUCDhjc1fR/OmsMMZiCouqoEiYbC\
9RAIDb/LogWK0E02PvTX72nGXuSwlG9KuefeW4i2e9vjJ+V2w/A1wcu1J5szedyQ\
pgCed/r8zSeUQhac0xxo7L9c3eWpexAKMnRUEzGLhQOEkbdYATAUOK8oyvyxUBkZ\
CayJSdM=\
-----END CERTIFICATE-----\
\
Entrust.net Global Secure Server CA\
===================================\
\
MD5 Fingerprint=9D:66:6A:CC:FF:D5:F5:43:B4:BF:8C:16:D1:2B:A8:99\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 949686588 (0x389b113c)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: O=Entrust.net, OU=www.entrust.net/SSL_CPS incorp. by ref. (limits liab.), OU=(c) 2000 Entrust.net Limited, CN=Entrust.net Secure Server Certification Authority\
        Validity\
            Not Before: Feb  4 17:20:00 2000 GMT\
            Not After : Feb  4 17:50:00 2020 GMT\
        Subject: O=Entrust.net, OU=www.entrust.net/SSL_CPS incorp. by ref. (limits liab.), OU=(c) 2000 Entrust.net Limited, CN=Entrust.net Secure Server Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:c7:c1:5f:4e:71:f1:ce:f0:60:86:0f:d2:58:7f:\
                    d3:33:97:2d:17:a2:75:30:b5:96:64:26:2f:68:c3:\
                    44:ab:a8:75:e6:00:67:34:57:9e:65:c7:22:9b:73:\
                    e6:d3:dd:08:0e:37:55:aa:25:46:81:6c:bd:fe:a8:\
                    f6:75:57:57:8c:90:6c:4a:c3:3e:8b:4b:43:0a:c9:\
                    11:56:9a:9a:27:22:99:cf:55:9e:61:d9:02:e2:7c:\
                    b6:7c:38:07:dc:e3:7f:4f:9a:b9:03:41:80:b6:75:\
                    67:13:0b:9f:e8:57:36:c8:5d:00:36:de:66:14:da:\
                    6e:76:1f:4f:37:8c:82:13:89\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 CRL Distribution Points: \
                DirName:/O=Entrust.net/OU=www.entrust.net/SSL_CPS incorp. by ref. (limits liab.)/OU=(c) 2000 Entrust.net Limited/CN=Entrust.net Secure Server Certification Authority/CN=CRL1\
\
            X509v3 Private Key Usage Period: \
                Not Before: Feb  4 17:20:00 2000 GMT, Not After: Feb  4 17:50:00 2020 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:CB:6C:C0:6B:E3:BB:3E:CB:FC:22:9C:FE:FB:8B:92:9C:B0:F2:6E:22\
\
            X509v3 Subject Key Identifier: \
                CB:6C:C0:6B:E3:BB:3E:CB:FC:22:9C:FE:FB:8B:92:9C:B0:F2:6E:22\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0...V5.0:4.0....\
    Signature Algorithm: md5WithRSAEncryption\
        62:db:81:91:ce:c8:9a:77:42:2f:ec:bd:27:a3:53:0f:50:1b:\
        ea:4e:92:f0:a9:af:a9:a0:ba:48:61:cb:ef:c9:06:ef:1f:d5:\
        f4:ee:df:56:2d:e6:ca:6a:19:73:aa:53:be:92:b3:50:02:b6:\
        85:26:72:63:d8:75:50:62:75:14:b7:b3:50:1a:3f:ca:11:00:\
        0b:85:45:69:6d:b6:a5:ae:51:e1:4a:dc:82:3f:6c:8c:34:b2:\
        77:6b:d9:02:f6:7f:0e:ea:65:04:f1:cd:54:ca:ba:c9:cc:e0:\
        84:f7:c8:3e:11:97:d3:60:09:18:bc:05:ff:6c:89:33:f0:ec:\
        15:0f\
-----BEGIN CERTIFICATE-----\
MIIElTCCA/6gAwIBAgIEOJsRPDANBgkqhkiG9w0BAQQFADCBujEUMBIGA1UEChML\
RW50cnVzdC5uZXQxPzA9BgNVBAsUNnd3dy5lbnRydXN0Lm5ldC9TU0xfQ1BTIGlu\
Y29ycC4gYnkgcmVmLiAobGltaXRzIGxpYWIuKTElMCMGA1UECxMcKGMpIDIwMDAg\
RW50cnVzdC5uZXQgTGltaXRlZDE6MDgGA1UEAxMxRW50cnVzdC5uZXQgU2VjdXJl\
IFNlcnZlciBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTAeFw0wMDAyMDQxNzIwMDBa\
Fw0yMDAyMDQxNzUwMDBaMIG6MRQwEgYDVQQKEwtFbnRydXN0Lm5ldDE/MD0GA1UE\
CxQ2d3d3LmVudHJ1c3QubmV0L1NTTF9DUFMgaW5jb3JwLiBieSByZWYuIChsaW1p\
dHMgbGlhYi4pMSUwIwYDVQQLExwoYykgMjAwMCBFbnRydXN0Lm5ldCBMaW1pdGVk\
MTowOAYDVQQDEzFFbnRydXN0Lm5ldCBTZWN1cmUgU2VydmVyIENlcnRpZmljYXRp\
b24gQXV0aG9yaXR5MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDHwV9OcfHO\
8GCGD9JYf9Mzly0XonUwtZZkJi9ow0SrqHXmAGc0V55lxyKbc+bT3QgON1WqJUaB\
bL3+qPZ1V1eMkGxKwz6LS0MKyRFWmponIpnPVZ5h2QLifLZ8OAfc439PmrkDQYC2\
dWcTC5/oVzbIXQA23mYU2m52H083jIITiQIDAQABo4IBpDCCAaAwEQYJYIZIAYb4\
QgEBBAQDAgAHMIHjBgNVHR8EgdswgdgwgdWggdKggc+kgcwwgckxFDASBgNVBAoT\
C0VudHJ1c3QubmV0MT8wPQYDVQQLFDZ3d3cuZW50cnVzdC5uZXQvU1NMX0NQUyBp\
bmNvcnAuIGJ5IHJlZi4gKGxpbWl0cyBsaWFiLikxJTAjBgNVBAsTHChjKSAyMDAw\
IEVudHJ1c3QubmV0IExpbWl0ZWQxOjA4BgNVBAMTMUVudHJ1c3QubmV0IFNlY3Vy\
ZSBTZXJ2ZXIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxDTALBgNVBAMTBENSTDEw\
KwYDVR0QBCQwIoAPMjAwMDAyMDQxNzIwMDBagQ8yMDIwMDIwNDE3NTAwMFowCwYD\
VR0PBAQDAgEGMB8GA1UdIwQYMBaAFMtswGvjuz7L/CKc/vuLkpyw8m4iMB0GA1Ud\
DgQWBBTLbMBr47s+y/winP77i5KcsPJuIjAMBgNVHRMEBTADAQH/MB0GCSqGSIb2\
fQdBAAQQMA4bCFY1LjA6NC4wAwIEkDANBgkqhkiG9w0BAQQFAAOBgQBi24GRzsia\
d0Iv7L0no1MPUBvqTpLwqa+poLpIYcvvyQbvH9X07t9WLebKahlzqlO+krNQAraF\
JnJj2HVQYnUUt7NQGj/KEQALhUVpbbalrlHhStyCP2yMNLJ3a9kC9n8O6mUE8c1U\
yrrJzOCE98g+EZfTYAkYvAX/bIkz8OwVDw==\
-----END CERTIFICATE-----\
\
Thawte Personal Premium CA\
==========================\
\
MD5 Fingerprint=3A:B2:DE:22:9A:20:93:49:F9:ED:C8:D2:8A:E7:68:0D\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting, OU=Certification Services Division, CN=Thawte Personal Premium CA/emailAddress=personal-premium@thawte.com\
        Validity\
            Not Before: Jan  1 00:00:00 1996 GMT\
            Not After : Dec 31 23:59:59 2020 GMT\
        Subject: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting, OU=Certification Services Division, CN=Thawte Personal Premium CA/emailAddress=personal-premium@thawte.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:c9:66:d9:f8:07:44:cf:b9:8c:2e:f0:a1:ef:13:\
                    45:6c:05:df:de:27:16:51:36:41:11:6c:6c:3b:ed:\
                    fe:10:7d:12:9e:e5:9b:42:9a:fe:60:31:c3:66:b7:\
                    73:3a:48:ae:4e:d0:32:37:94:88:b5:0d:b6:d9:f3:\
                    f2:44:d9:d5:88:12:dd:76:4d:f2:1a:fc:6f:23:1e:\
                    7a:f1:d8:98:45:4e:07:10:ef:16:42:d0:43:75:6d:\
                    4a:de:e2:aa:c9:31:ff:1f:00:70:7c:66:cf:10:25:\
                    08:ba:fa:ee:00:e9:46:03:66:27:11:15:3b:aa:5b:\
                    f2:98:dd:36:42:b2:da:88:75\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
    Signature Algorithm: md5WithRSAEncryption\
        69:36:89:f7:34:2a:33:72:2f:6d:3b:d4:22:b2:b8:6f:9a:c5:\
        36:66:0e:1b:3c:a1:b1:75:5a:e6:fd:35:d3:f8:a8:f2:07:6f:\
        85:67:8e:de:2b:b9:e2:17:b0:3a:a0:f0:0e:a2:00:9a:df:f3:\
        14:15:6e:bb:c8:85:5a:98:80:f9:ff:be:74:1d:3d:f3:fe:30:\
        25:d1:37:34:67:fa:a5:71:79:30:61:29:72:c0:e0:2c:4c:fb:\
        56:e4:3a:a8:6f:e5:32:59:52:db:75:28:50:59:0c:f8:0b:19:\
        e4:ac:d9:af:96:8d:2f:50:db:07:c3:ea:1f:ab:33:e0:f5:2b:\
        31:89\
-----BEGIN CERTIFICATE-----\
MIIDKTCCApKgAwIBAgIBADANBgkqhkiG9w0BAQQFADCBzzELMAkGA1UEBhMCWkEx\
FTATBgNVBAgTDFdlc3Rlcm4gQ2FwZTESMBAGA1UEBxMJQ2FwZSBUb3duMRowGAYD\
VQQKExFUaGF3dGUgQ29uc3VsdGluZzEoMCYGA1UECxMfQ2VydGlmaWNhdGlvbiBT\
ZXJ2aWNlcyBEaXZpc2lvbjEjMCEGA1UEAxMaVGhhd3RlIFBlcnNvbmFsIFByZW1p\
dW0gQ0ExKjAoBgkqhkiG9w0BCQEWG3BlcnNvbmFsLXByZW1pdW1AdGhhd3RlLmNv\
bTAeFw05NjAxMDEwMDAwMDBaFw0yMDEyMzEyMzU5NTlaMIHPMQswCQYDVQQGEwJa\
QTEVMBMGA1UECBMMV2VzdGVybiBDYXBlMRIwEAYDVQQHEwlDYXBlIFRvd24xGjAY\
BgNVBAoTEVRoYXd0ZSBDb25zdWx0aW5nMSgwJgYDVQQLEx9DZXJ0aWZpY2F0aW9u\
IFNlcnZpY2VzIERpdmlzaW9uMSMwIQYDVQQDExpUaGF3dGUgUGVyc29uYWwgUHJl\
bWl1bSBDQTEqMCgGCSqGSIb3DQEJARYbcGVyc29uYWwtcHJlbWl1bUB0aGF3dGUu\
Y29tMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDJZtn4B0TPuYwu8KHvE0Vs\
Bd/eJxZRNkERbGw77f4QfRKe5ZtCmv5gMcNmt3M6SK5O0DI3lIi1DbbZ8/JE2dWI\
Et12TfIa/G8jHnrx2JhFTgcQ7xZC0EN1bUre4qrJMf8fAHB8Zs8QJQi6+u4A6UYD\
ZicRFTuqW/KY3TZCstqIdQIDAQABoxMwETAPBgNVHRMBAf8EBTADAQH/MA0GCSqG\
SIb3DQEBBAUAA4GBAGk2ifc0KjNyL2071CKyuG+axTZmDhs8obF1Wub9NdP4qPIH\
b4Vnjt4rueIXsDqg8A6iAJrf8xQVbrvIhVqYgPn/vnQdPfP+MCXRNzRn+qVxeTBh\
KXLA4CxM+1bkOqhv5TJZUtt1KFBZDPgLGeSs2a+WjS9Q2wfD6h+rM+D1KzGJ\
-----END CERTIFICATE-----\
\
Entrust.net Global Secure Personal CA\
=====================================\
\
MD5 Fingerprint=9A:77:19:18:ED:96:CF:DF:1B:B7:0E:F5:8D:B9:88:2E\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 949941988 (0x389ef6e4)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: O=Entrust.net, OU=www.entrust.net/GCCA_CPS incorp. by ref. (limits liab.), OU=(c) 2000 Entrust.net Limited, CN=Entrust.net Client Certification Authority\
        Validity\
            Not Before: Feb  7 16:16:40 2000 GMT\
            Not After : Feb  7 16:46:40 2020 GMT\
        Subject: O=Entrust.net, OU=www.entrust.net/GCCA_CPS incorp. by ref. (limits liab.), OU=(c) 2000 Entrust.net Limited, CN=Entrust.net Client Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:93:74:b4:b6:e4:c5:4b:d6:a1:68:7f:62:d5:ec:\
                    f7:51:57:b3:72:4a:98:f5:d0:89:c9:ad:63:cd:4d:\
                    35:51:6a:84:d4:ad:c9:68:79:6f:b8:eb:11:db:87:\
                    ae:5c:24:51:13:f1:54:25:84:af:29:2b:9f:e3:80:\
                    e2:d9:cb:dd:c6:45:49:34:88:90:5e:01:97:ef:ea:\
                    53:a6:dd:fc:c1:de:4b:2a:25:e4:e9:35:fa:55:05:\
                    06:e5:89:7a:ea:a4:11:57:3b:fc:7c:3d:36:cd:67:\
                    35:6d:a4:a9:25:59:bd:66:f5:f9:27:e4:95:67:d6:\
                    3f:92:80:5e:f2:34:7d:2b:85\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 CRL Distribution Points: \
                DirName:/O=Entrust.net/OU=www.entrust.net/GCCA_CPS incorp. by ref. (limits liab.)/OU=(c) 2000 Entrust.net Limited/CN=Entrust.net Client Certification Authority/CN=CRL1\
\
            X509v3 Private Key Usage Period: \
                Not Before: Feb  7 16:16:40 2000 GMT, Not After: Feb  7 16:46:40 2020 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:84:8B:74:FD:C5:8D:C0:FF:27:6D:20:37:45:7C:FE:2D:CE:BA:D3:7D\
\
            X509v3 Subject Key Identifier: \
                84:8B:74:FD:C5:8D:C0:FF:27:6D:20:37:45:7C:FE:2D:CE:BA:D3:7D\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0...V5.0:4.0....\
    Signature Algorithm: md5WithRSAEncryption\
        4e:6f:35:80:3b:d1:8a:f5:0e:a7:20:cb:2d:65:55:d0:92:f4:\
        e7:84:b5:06:26:83:12:84:0b:ac:3b:b2:44:ee:bd:cf:40:db:\
        20:0e:ba:6e:14:ea:30:e0:3b:62:7c:7f:8b:6b:7c:4a:a7:d5:\
        35:3c:be:a8:5c:ea:4b:bb:93:8e:80:66:ab:0f:29:fd:4d:2d:\
        bf:1a:9b:0a:90:c5:ab:da:d1:b3:86:d4:2f:24:52:5c:7a:6d:\
        c6:f2:fe:e5:4d:1a:30:8c:90:f2:ba:d7:4a:3e:43:7e:d4:c8:\
        50:1a:87:f8:4f:81:c7:76:0b:84:3a:72:9d:ce:65:66:97:ae:\
        26:5e\
-----BEGIN CERTIFICATE-----\
MIIEgzCCA+ygAwIBAgIEOJ725DANBgkqhkiG9w0BAQQFADCBtDEUMBIGA1UEChML\
RW50cnVzdC5uZXQxQDA+BgNVBAsUN3d3dy5lbnRydXN0Lm5ldC9HQ0NBX0NQUyBp\
bmNvcnAuIGJ5IHJlZi4gKGxpbWl0cyBsaWFiLikxJTAjBgNVBAsTHChjKSAyMDAw\
IEVudHJ1c3QubmV0IExpbWl0ZWQxMzAxBgNVBAMTKkVudHJ1c3QubmV0IENsaWVu\
dCBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTAeFw0wMDAyMDcxNjE2NDBaFw0yMDAy\
MDcxNjQ2NDBaMIG0MRQwEgYDVQQKEwtFbnRydXN0Lm5ldDFAMD4GA1UECxQ3d3d3\
LmVudHJ1c3QubmV0L0dDQ0FfQ1BTIGluY29ycC4gYnkgcmVmLiAobGltaXRzIGxp\
YWIuKTElMCMGA1UECxMcKGMpIDIwMDAgRW50cnVzdC5uZXQgTGltaXRlZDEzMDEG\
A1UEAxMqRW50cnVzdC5uZXQgQ2xpZW50IENlcnRpZmljYXRpb24gQXV0aG9yaXR5\
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCTdLS25MVL1qFof2LV7PdRV7Ny\
Spj10InJrWPNTTVRaoTUrcloeW+46xHbh65cJFET8VQlhK8pK5/jgOLZy93GRUk0\
iJBeAZfv6lOm3fzB3ksqJeTpNfpVBQbliXrqpBFXO/x8PTbNZzVtpKklWb1m9fkn\
5JVn1j+SgF7yNH0rhQIDAQABo4IBnjCCAZowEQYJYIZIAYb4QgEBBAQDAgAHMIHd\
BgNVHR8EgdUwgdIwgc+ggcyggcmkgcYwgcMxFDASBgNVBAoTC0VudHJ1c3QubmV0\
MUAwPgYDVQQLFDd3d3cuZW50cnVzdC5uZXQvR0NDQV9DUFMgaW5jb3JwLiBieSBy\
ZWYuIChsaW1pdHMgbGlhYi4pMSUwIwYDVQQLExwoYykgMjAwMCBFbnRydXN0Lm5l\
dCBMaW1pdGVkMTMwMQYDVQQDEypFbnRydXN0Lm5ldCBDbGllbnQgQ2VydGlmaWNh\
dGlvbiBBdXRob3JpdHkxDTALBgNVBAMTBENSTDEwKwYDVR0QBCQwIoAPMjAwMDAy\
MDcxNjE2NDBagQ8yMDIwMDIwNzE2NDY0MFowCwYDVR0PBAQDAgEGMB8GA1UdIwQY\
MBaAFISLdP3FjcD/J20gN0V8/i3OutN9MB0GA1UdDgQWBBSEi3T9xY3A/ydtIDdF\
fP4tzrrTfTAMBgNVHRMEBTADAQH/MB0GCSqGSIb2fQdBAAQQMA4bCFY1LjA6NC4w\
AwIEkDANBgkqhkiG9w0BAQQFAAOBgQBObzWAO9GK9Q6nIMstZVXQkvTnhLUGJoMS\
hAusO7JE7r3PQNsgDrpuFOow4DtifH+La3xKp9U1PL6oXOpLu5OOgGarDyn9TS2/\
GpsKkMWr2tGzhtQvJFJcem3G8v7lTRowjJDyutdKPkN+1MhQGof4T4HHdguEOnKd\
zmVml64mXg==\
-----END CERTIFICATE-----\
\
AOL Time Warner Root Certification Authority 1\
==============================================\
\
MD5 Fingerprint=E7:7A:DC:B1:1F:6E:06:1F:74:6C:59:16:27:C3:4B:C0\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=AOL Time Warner Inc., OU=America Online Inc., CN=AOL Time Warner Root Certification Authority 1\
        Validity\
            Not Before: May 29 06:00:00 2002 GMT\
            Not After : Nov 20 15:03:00 2037 GMT\
        Subject: C=US, O=AOL Time Warner Inc., OU=America Online Inc., CN=AOL Time Warner Root Certification Authority 1\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:99:de:8f:c3:25:a3:69:34:e8:05:f7:74:b9:bf:\
                    5a:97:19:b9:2f:94:d2:93:e5:2d:89:ca:84:7c:3f:\
                    10:43:1b:8c:8b:7c:84:58:f8:24:7c:48:cf:2a:fd:\
                    c0:15:d9:18:7e:84:1a:17:d3:db:9e:d7:ca:e4:d9:\
                    d7:aa:58:51:87:f0:f0:8b:48:4e:e2:c2:c4:59:69:\
                    30:62:b6:30:a2:8c:0b:11:99:61:35:6d:7e:ef:c5:\
                    b1:19:06:20:12:8e:42:e1:df:0f:96:10:52:a8:cf:\
                    9c:5f:95:14:d8:af:3b:75:0b:31:20:1f:44:2f:a2:\
                    62:41:b3:bb:18:21:db:ca:71:3c:8c:ec:b6:b9:0d:\
                    9f:ef:51:ef:4d:7b:12:f2:0b:0c:e1:ac:40:8f:77:\
                    7f:b0:ca:78:71:0c:5d:16:71:70:a2:d7:c2:3a:85:\
                    cd:0e:9a:c4:e0:00:b0:d5:25:ea:dc:2b:e4:94:2d:\
                    38:9c:89:41:57:64:28:65:19:1c:b6:44:b4:c8:31:\
                    6b:8e:01:7b:76:59:25:7f:15:1c:84:08:7c:73:65:\
                    20:0a:a1:04:2e:1a:32:a8:9a:20:b1:9c:2c:21:59:\
                    e7:fb:cf:ee:70:2d:08:ca:63:3e:2c:9b:93:19:6a:\
                    a4:c2:97:ff:b7:86:57:88:85:6c:9e:15:16:2b:4d:\
                    2c:b3\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                A1:36:30:16:CB:86:90:00:45:80:53:B1:8F:C8:D8:3D:7C:BE:5F:12\
            X509v3 Authority Key Identifier: \
                keyid:A1:36:30:16:CB:86:90:00:45:80:53:B1:8F:C8:D8:3D:7C:BE:5F:12\
\
            X509v3 Key Usage: critical\
                Digital Signature, Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        8a:20:18:a5:be:b3:2f:b4:a6:84:00:40:30:29:fa:b4:14:73:\
        4c:79:45:a7:f6:70:e0:e8:7e:64:1e:0a:95:7c:6a:61:c2:ef:\
        4e:1f:be:ff:c9:99:1f:07:61:4a:e1:5d:4c:cd:ad:ee:d0:52:\
        32:d9:59:32:bc:da:79:72:d6:7b:09:e8:02:81:35:d3:0a:df:\
        11:1d:c9:79:a0:80:4d:fe:5a:d7:56:d6:ed:0f:2a:af:a7:18:\
        75:33:0c:ea:c1:61:05:4f:6a:9a:89:f2:8d:b9:9f:2e:ef:b0:\
        5f:5a:00:eb:be:ad:a0:f8:44:05:67:bc:cb:04:ef:9e:64:c5:\
        e9:c8:3f:05:bf:c6:2f:07:1c:c3:36:71:86:ca:38:66:4a:cd:\
        d6:b8:4b:c6:6c:a7:97:3b:fa:13:2d:6e:23:61:87:a1:63:42:\
        ac:c2:cb:97:9f:61:68:cf:2d:4c:04:9d:d7:25:4f:0a:0e:4d:\
        90:8b:18:56:a8:93:48:57:dc:6f:ae:bd:9e:67:57:77:89:50:\
        b3:be:11:9b:45:67:83:86:19:87:d3:98:bd:08:1a:16:1f:58:\
        82:0b:e1:96:69:05:4b:8e:ec:83:51:31:07:d5:d4:9f:ff:59:\
        7b:a8:6e:85:cf:d3:4b:a9:49:b0:5f:b0:39:28:68:0e:73:dd:\
        25:9a:de:12\
-----BEGIN CERTIFICATE-----\
MIID5jCCAs6gAwIBAgIBATANBgkqhkiG9w0BAQUFADCBgzELMAkGA1UEBhMCVVMx\
HTAbBgNVBAoTFEFPTCBUaW1lIFdhcm5lciBJbmMuMRwwGgYDVQQLExNBbWVyaWNh\
IE9ubGluZSBJbmMuMTcwNQYDVQQDEy5BT0wgVGltZSBXYXJuZXIgUm9vdCBDZXJ0\
aWZpY2F0aW9uIEF1dGhvcml0eSAxMB4XDTAyMDUyOTA2MDAwMFoXDTM3MTEyMDE1\
MDMwMFowgYMxCzAJBgNVBAYTAlVTMR0wGwYDVQQKExRBT0wgVGltZSBXYXJuZXIg\
SW5jLjEcMBoGA1UECxMTQW1lcmljYSBPbmxpbmUgSW5jLjE3MDUGA1UEAxMuQU9M\
IFRpbWUgV2FybmVyIFJvb3QgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkgMTCCASIw\
DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJnej8Mlo2k06AX3dLm/WpcZuS+U\
0pPlLYnKhHw/EEMbjIt8hFj4JHxIzyr9wBXZGH6EGhfT257XyuTZ16pYUYfw8ItI\
TuLCxFlpMGK2MKKMCxGZYTVtfu/FsRkGIBKOQuHfD5YQUqjPnF+VFNivO3ULMSAf\
RC+iYkGzuxgh28pxPIzstrkNn+9R7017EvILDOGsQI93f7DKeHEMXRZxcKLXwjqF\
zQ6axOAAsNUl6twr5JQtOJyJQVdkKGUZHLZEtMgxa44Be3ZZJX8VHIQIfHNlIAqh\
BC4aMqiaILGcLCFZ5/vP7nAtCMpjPiybkxlqpMKX/7eGV4iFbJ4VFitNLLMCAwEA\
AaNjMGEwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUoTYwFsuGkABFgFOxj8jY\
PXy+XxIwHwYDVR0jBBgwFoAUoTYwFsuGkABFgFOxj8jYPXy+XxIwDgYDVR0PAQH/\
BAQDAgGGMA0GCSqGSIb3DQEBBQUAA4IBAQCKIBilvrMvtKaEAEAwKfq0FHNMeUWn\
9nDg6H5kHgqVfGphwu9OH77/yZkfB2FK4V1Mza3u0FIy2VkyvNp5ctZ7CegCgTXT\
Ct8RHcl5oIBN/lrXVtbtDyqvpxh1MwzqwWEFT2qaifKNuZ8u77BfWgDrvq2g+EQF\
Z7zLBO+eZMXpyD8Fv8YvBxzDNnGGyjhmSs3WuEvGbKeXO/oTLW4jYYehY0KswsuX\
n2Fozy1MBJ3XJU8KDk2QixhWqJNIV9xvrr2eZ1d3iVCzvhGbRWeDhhmH05i9CBoW\
H1iCC+GWaQVLjuyDUTEH1dSf/1l7qG6Fz9NLqUmwX7A5KGgOc90lmt4S\
-----END CERTIFICATE-----\
\
AOL Time Warner Root Certification Authority 2\
==============================================\
\
MD5 Fingerprint=01:5A:99:C3:D6:4F:A9:4B:3C:3B:B1:A3:AB:27:4C:BF\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=AOL Time Warner Inc., OU=America Online Inc., CN=AOL Time Warner Root Certification Authority 2\
        Validity\
            Not Before: May 29 06:00:00 2002 GMT\
            Not After : Sep 28 23:43:00 2037 GMT\
        Subject: C=US, O=AOL Time Warner Inc., OU=America Online Inc., CN=AOL Time Warner Root Certification Authority 2\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (4096 bit)\
                Modulus (4096 bit):\
                    00:b4:37:5a:08:16:99:14:e8:55:b1:1b:24:6b:fc:\
                    c7:8b:e6:87:a9:89:ee:8b:99:cd:4f:40:86:a4:b6:\
                    4d:c9:d9:b1:dc:3c:4d:0d:85:4c:15:6c:46:8b:52:\
                    78:9f:f8:23:fd:67:f5:24:3a:68:5d:d0:f7:64:61:\
                    41:54:a3:8b:a5:08:d2:29:5b:9b:60:4f:26:83:d1:\
                    63:12:56:49:76:a4:16:c2:a5:9d:45:ac:8b:84:95:\
                    a8:16:b1:ec:9f:ea:24:1a:ef:b9:57:5c:9a:24:21:\
                    2c:4d:0e:71:1f:a6:ac:5d:45:74:03:98:c4:54:8c:\
                    16:4a:41:77:86:95:75:0c:47:01:66:60:fc:15:f1:\
                    0f:ea:f5:14:78:c7:0e:d7:6e:81:1c:5e:bf:5e:e7:\
                    3a:2a:d8:97:17:30:7c:00:ad:08:9d:33:af:b8:99:\
                    61:80:8b:a8:95:7e:14:dc:12:6c:a4:d0:d8:ef:40:\
                    49:02:36:f9:6e:a9:d6:1d:96:56:04:b2:b3:2d:16:\
                    56:86:8f:d9:20:57:80:cd:67:10:6d:b0:4c:f0:da:\
                    46:b6:ea:25:2e:46:af:8d:b0:85:38:34:8b:14:26:\
                    82:2b:ac:ae:99:0b:8e:14:d7:52:bd:9e:69:c3:86:\
                    02:0b:ea:76:75:31:09:ce:33:19:21:85:43:e6:89:\
                    2d:9f:25:37:67:f1:23:6a:d2:00:6d:97:f9:9f:e7:\
                    29:ca:dd:1f:d7:06:ea:b8:c9:b9:09:21:9f:c8:3f:\
                    06:c5:d2:e9:12:46:00:4e:7b:08:eb:42:3d:2b:48:\
                    6e:9d:67:dd:4b:02:e4:44:f3:93:19:a5:27:ce:69:\
                    7a:be:67:d3:fc:50:a4:2c:ab:c3:6b:b9:e3:80:4c:\
                    cf:05:61:4b:2b:dc:1b:b9:a6:d2:d0:aa:f5:2b:73:\
                    fb:ce:90:35:9f:0c:52:1c:bf:5c:21:61:11:5b:15:\
                    4b:a9:24:51:fc:a4:5c:f7:17:9d:b0:d2:fa:07:e9:\
                    8f:56:e4:1a:8c:68:8a:04:d3:7c:5a:e3:9e:a2:a1:\
                    ca:71:5b:a2:d4:a0:e7:29:85:5d:03:68:2a:4f:d2:\
                    06:d7:3d:f9:c3:03:2f:3f:65:f9:67:1e:47:40:d3:\
                    63:0f:e3:d5:8e:f9:85:ab:97:4c:b3:d7:26:eb:96:\
                    0a:94:de:85:36:9c:c8:7f:81:09:02:49:2a:0e:f5:\
                    64:32:0c:82:d1:ba:6a:82:1b:b3:4b:74:11:f3:8c:\
                    77:d6:9f:bf:dc:37:a4:a7:55:04:2f:d4:31:e8:d3:\
                    46:b9:03:7c:da:12:4e:59:64:b7:51:31:31:50:a0:\
                    ca:1c:27:d9:10:2e:ad:d6:bd:10:66:2b:c3:b0:22:\
                    4a:12:5b\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                4F:69:6D:03:7E:9D:9F:07:18:43:BC:B7:10:4E:D5:BF:A9:C4:20:28\
            X509v3 Authority Key Identifier: \
                keyid:4F:69:6D:03:7E:9D:9F:07:18:43:BC:B7:10:4E:D5:BF:A9:C4:20:28\
\
            X509v3 Key Usage: critical\
                Digital Signature, Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        3b:f3:ae:ca:e8:2e:87:85:fb:65:59:e7:ad:11:14:a5:57:bc:\
        58:9f:24:12:57:bb:fb:3f:34:da:ee:ad:7a:2a:34:72:70:31:\
        6b:c7:19:98:80:c9:82:de:37:77:5e:54:8b:8e:f2:ea:67:4f:\
        c9:74:84:91:56:09:d5:e5:7a:9a:81:b6:81:c2:ad:36:e4:f1:\
        54:11:53:f3:34:45:01:26:c8:e5:1a:bc:34:44:21:de:ad:25:\
        fc:76:16:77:21:90:80:98:57:9d:4e:ea:ec:2f:aa:3c:14:7b:\
        57:c1:7e:18:14:67:ee:24:c6:bd:ba:15:b0:d2:18:bd:b7:55:\
        81:ac:53:c0:e8:dd:69:12:13:42:b7:02:b5:05:41:ca:79:50:\
        6e:82:0e:71:72:93:46:e8:9d:0d:5d:bd:ae:ce:29:ad:63:d5:\
        55:16:80:30:27:ff:76:ba:f7:b8:d6:4a:e3:d9:b5:f9:52:d0:\
        4e:40:a9:c7:e5:c2:32:c7:aa:76:24:e1:6b:05:50:eb:c5:bf:\
        0a:54:e5:b9:42:3c:24:fb:b7:07:9c:30:9f:79:5a:e6:e0:40:\
        52:15:f4:fc:aa:f4:56:f9:44:97:87:ed:0e:65:72:5e:be:26:\
        fb:4d:a4:2d:08:07:de:d8:5c:a0:dc:81:33:99:18:25:11:77:\
        a7:eb:fd:58:09:2c:99:6b:1b:8a:f3:52:3f:1a:4d:48:60:f1:\
        a0:f6:33:02:53:8b:ed:25:09:b8:0d:2d:ed:97:73:ec:d7:96:\
        1f:8e:60:0e:da:10:9b:2f:18:24:f6:a6:4d:0a:f9:3b:cb:75:\
        c2:cc:2f:ce:24:69:c9:0a:22:8e:59:a7:f7:82:0c:d7:d7:6b:\
        35:9c:43:00:6a:c4:95:67:ba:9c:45:cb:b8:0e:37:f7:dc:4e:\
        01:4f:be:0a:b6:03:d3:ad:8a:45:f7:da:27:4d:29:b1:48:df:\
        e4:11:e4:96:46:bd:6c:02:3e:d6:51:c8:95:17:01:15:a9:f2:\
        aa:aa:f2:bf:2f:65:1b:6f:d0:b9:1a:93:f5:8e:35:c4:80:87:\
        3e:94:2f:66:e4:e9:a8:ff:41:9c:70:2a:4f:2a:39:18:95:1e:\
        7e:fb:61:01:3c:51:08:2e:28:18:a4:16:0f:31:fd:3a:6c:23:\
        93:20:76:e1:fd:07:85:d1:5b:3f:d2:1c:73:32:dd:fa:b9:f8:\
        8c:cf:02:87:7a:9a:96:e4:ed:4f:89:8d:53:43:ab:0e:13:c0:\
        01:15:b4:79:38:db:fc:6e:3d:9e:51:b6:b8:13:8b:67:cf:f9:\
        7c:d9:22:1d:f6:5d:c5:1c:01:2f:98:e8:7a:24:18:bc:84:d7:\
        fa:dc:72:5b:f7:c1:3a:68\
-----BEGIN CERTIFICATE-----\
MIIF5jCCA86gAwIBAgIBATANBgkqhkiG9w0BAQUFADCBgzELMAkGA1UEBhMCVVMx\
HTAbBgNVBAoTFEFPTCBUaW1lIFdhcm5lciBJbmMuMRwwGgYDVQQLExNBbWVyaWNh\
IE9ubGluZSBJbmMuMTcwNQYDVQQDEy5BT0wgVGltZSBXYXJuZXIgUm9vdCBDZXJ0\
aWZpY2F0aW9uIEF1dGhvcml0eSAyMB4XDTAyMDUyOTA2MDAwMFoXDTM3MDkyODIz\
NDMwMFowgYMxCzAJBgNVBAYTAlVTMR0wGwYDVQQKExRBT0wgVGltZSBXYXJuZXIg\
SW5jLjEcMBoGA1UECxMTQW1lcmljYSBPbmxpbmUgSW5jLjE3MDUGA1UEAxMuQU9M\
IFRpbWUgV2FybmVyIFJvb3QgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkgMjCCAiIw\
DQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBALQ3WggWmRToVbEbJGv8x4vmh6mJ\
7ouZzU9AhqS2TcnZsdw8TQ2FTBVsRotSeJ/4I/1n9SQ6aF3Q92RhQVSji6UI0ilb\
m2BPJoPRYxJWSXakFsKlnUWsi4SVqBax7J/qJBrvuVdcmiQhLE0OcR+mrF1FdAOY\
xFSMFkpBd4aVdQxHAWZg/BXxD+r1FHjHDtdugRxev17nOirYlxcwfACtCJ0zr7iZ\
YYCLqJV+FNwSbKTQ2O9ASQI2+W6p1h2WVgSysy0WVoaP2SBXgM1nEG2wTPDaRrbq\
JS5Gr42whTg0ixQmgiusrpkLjhTXUr2eacOGAgvqdnUxCc4zGSGFQ+aJLZ8lN2fx\
I2rSAG2X+Z/nKcrdH9cG6rjJuQkhn8g/BsXS6RJGAE57COtCPStIbp1n3UsC5ETz\
kxmlJ85per5n0/xQpCyrw2u544BMzwVhSyvcG7mm0tCq9Stz+86QNZ8MUhy/XCFh\
EVsVS6kkUfykXPcXnbDS+gfpj1bkGoxoigTTfFrjnqKhynFbotSg5ymFXQNoKk/S\
Btc9+cMDLz9l+WceR0DTYw/j1Y75hauXTLPXJuuWCpTehTacyH+BCQJJKg71ZDIM\
gtG6aoIbs0t0EfOMd9afv9w3pKdVBC/UMejTRrkDfNoSTllkt1ExMVCgyhwn2RAu\
rda9EGYrw7AiShJbAgMBAAGjYzBhMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYE\
FE9pbQN+nZ8HGEO8txBO1b+pxCAoMB8GA1UdIwQYMBaAFE9pbQN+nZ8HGEO8txBO\
1b+pxCAoMA4GA1UdDwEB/wQEAwIBhjANBgkqhkiG9w0BAQUFAAOCAgEAO/Ouyugu\
h4X7ZVnnrREUpVe8WJ8kEle7+z802u6teio0cnAxa8cZmIDJgt43d15Ui47y6mdP\
yXSEkVYJ1eV6moG2gcKtNuTxVBFT8zRFASbI5Rq8NEQh3q0l/HYWdyGQgJhXnU7q\
7C+qPBR7V8F+GBRn7iTGvboVsNIYvbdVgaxTwOjdaRITQrcCtQVBynlQboIOcXKT\
RuidDV29rs4prWPVVRaAMCf/drr3uNZK49m1+VLQTkCpx+XCMseqdiThawVQ68W/\
ClTluUI8JPu3B5wwn3la5uBAUhX0/Kr0VvlEl4ftDmVyXr4m+02kLQgH3thcoNyB\
M5kYJRF3p+v9WAksmWsbivNSPxpNSGDxoPYzAlOL7SUJuA0t7Zdz7NeWH45gDtoQ\
my8YJPamTQr5O8t1wswvziRpyQoijlmn94IM19drNZxDAGrElWe6nEXLuA4399xO\
AU++CrYD062KRffaJ00psUjf5BHklka9bAI+1lHIlRcBFanyqqryvy9lG2/QuRqT\
9Y41xICHPpQvZuTpqP9BnHAqTyo5GJUefvthATxRCC4oGKQWDzH9OmwjkyB24f0H\
hdFbP9IcczLd+rn4jM8Ch3qaluTtT4mNU0OrDhPAARW0eTjb/G49nlG2uBOLZ8/5\
fNkiHfZdxRwBL5joeiQYvITX+txyW/fBOmg=\
-----END CERTIFICATE-----\
\
beTRUSTed Root CA-Baltimore Implementation\
==========================================\
\
MD5 Fingerprint=81:35:B9:FB:FB:12:CA:18:69:36:EB:AE:69:78:A1:F1\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1018510662 (0x3cb53d46)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: O=beTRUSTed, OU=beTRUSTed Root CAs, CN=beTRUSTed Root CA-Baltimore Implementation\
        Validity\
            Not Before: Apr 11 07:38:51 2002 GMT\
            Not After : Apr 11 07:38:51 2022 GMT\
        Subject: O=beTRUSTed, OU=beTRUSTed Root CAs, CN=beTRUSTed Root CA-Baltimore Implementation\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:bc:7e:c4:39:9c:8c:e3:d6:1c:86:ff:ca:62:ad:\
                    e0:7f:30:45:7a:8e:1a:b3:b8:c7:f9:d1:36:ff:22:\
                    f3:4e:6a:5f:84:10:fb:66:81:c3:94:79:31:d2:91:\
                    e1:77:8e:18:2a:c3:14:de:51:f5:4f:a3:2b:bc:18:\
                    16:e2:b5:dd:79:de:22:f8:82:7e:cb:81:1f:fd:27:\
                    2c:8f:fa:97:64:22:8e:f8:ff:61:a3:9c:1b:1e:92:\
                    8f:c0:a8:09:df:09:11:ec:b7:7d:31:9a:1a:ea:83:\
                    21:06:3c:9f:ba:5c:ff:94:ea:6a:b8:c3:6b:55:34:\
                    4f:3d:32:1f:dd:81:14:e0:c4:3c:cd:9d:30:f8:30:\
                    a9:97:d3:ee:cc:a3:d0:1f:5f:1c:13:81:d4:18:ab:\
                    94:d1:63:c3:9e:7f:35:92:9e:5f:44:ea:ec:f4:22:\
                    5c:b7:e8:3d:7d:a4:f9:89:a9:91:b2:2a:d9:eb:33:\
                    87:ee:a5:fd:e3:da:cc:88:e6:89:26:6e:c7:2b:82:\
                    d0:5e:9d:59:db:14:ec:91:83:05:c3:5e:0e:c6:2a:\
                    d0:04:dd:71:3d:20:4e:58:27:fc:53:fb:78:78:19:\
                    14:b2:fc:90:52:89:38:62:60:07:b4:a0:ec:ac:6b:\
                    50:d6:fd:b9:28:6b:ef:52:2d:3a:b2:ff:f1:01:40:\
                    ac:37\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Certificate Policies: \
                Policy: 1.3.6.1.4.1.6334.0.0.1.9.40.51377\
                  User Notice:\
                    Explicit Text: Reliance on or use of this Certificate creates an acknowledgment and acceptance of the then applicable standard terms and conditions of use, the Certification Practice Statement and the Relying Party Agreement, which can be found at the beTRUSTed web site, http://www.betrusted.com/products_services/index.html\
                  CPS: http://www.betrusted.com/products_services/index.html\
\
            X509v3 Subject Key Identifier: \
                45:3D:C3:A9:D1:DC:3F:24:56:98:1C:73:18:88:6A:FF:83:47:ED:B6\
            X509v3 Authority Key Identifier: \
                keyid:45:3D:C3:A9:D1:DC:3F:24:56:98:1C:73:18:88:6A:FF:83:47:ED:B6\
\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        49:92:bc:a3:ee:ac:bd:fa:0d:c9:8b:79:86:1c:23:76:b0:80:\
        59:77:fc:da:7f:b4:4b:df:c3:64:4b:6a:4e:0e:ad:f2:7d:59:\
        77:05:ad:0a:89:73:b0:fa:bc:cb:dc:8d:00:88:8f:a6:a0:b2:\
        ea:ac:52:27:bf:a1:48:7c:97:10:7b:ba:ed:13:1d:9a:07:6e:\
        cb:31:62:12:e8:63:03:aa:7d:6d:e3:f8:1b:76:21:78:1b:9f:\
        4b:43:8c:d3:49:86:f6:1b:5c:f6:2e:60:15:d3:e9:e3:7b:75:\
        3f:d0:02:83:d0:18:82:41:cd:65:37:ea:8e:32:7e:bd:6b:99:\
        5d:30:11:c8:db:48:54:1c:3b:e1:a7:13:d3:6a:48:93:f7:3d:\
        8c:7f:05:e8:ce:f3:88:2a:63:04:b8:ea:7e:58:7c:01:7b:5b:\
        e1:c5:7d:ef:21:e0:8d:0e:5d:51:7d:b1:67:fd:a3:bd:38:36:\
        c6:f2:38:86:87:1a:96:68:60:46:fb:28:14:47:55:e1:a7:80:\
        0c:6b:e2:ea:df:4d:7c:90:48:a0:36:bd:09:17:89:7f:c3:f2:\
        d3:9c:9c:e3:dd:c4:1b:dd:f5:b7:71:b3:53:05:89:06:d0:cb:\
        4a:80:c1:c8:53:90:b5:3c:31:88:17:50:9f:c9:c4:0e:8b:d8:\
        a8:02:63:0d\
-----BEGIN CERTIFICATE-----\
MIIFajCCBFKgAwIBAgIEPLU9RjANBgkqhkiG9w0BAQUFADBmMRIwEAYDVQQKEwli\
ZVRSVVNUZWQxGzAZBgNVBAsTEmJlVFJVU1RlZCBSb290IENBczEzMDEGA1UEAxMq\
YmVUUlVTVGVkIFJvb3QgQ0EtQmFsdGltb3JlIEltcGxlbWVudGF0aW9uMB4XDTAy\
MDQxMTA3Mzg1MVoXDTIyMDQxMTA3Mzg1MVowZjESMBAGA1UEChMJYmVUUlVTVGVk\
MRswGQYDVQQLExJiZVRSVVNUZWQgUm9vdCBDQXMxMzAxBgNVBAMTKmJlVFJVU1Rl\
ZCBSb290IENBLUJhbHRpbW9yZSBJbXBsZW1lbnRhdGlvbjCCASIwDQYJKoZIhvcN\
AQEBBQADggEPADCCAQoCggEBALx+xDmcjOPWHIb/ymKt4H8wRXqOGrO4x/nRNv8i\
805qX4QQ+2aBw5R5MdKR4XeOGCrDFN5R9U+jK7wYFuK13XneIviCfsuBH/0nLI/6\
l2Qijvj/YaOcGx6Sj8CoCd8JEey3fTGaGuqDIQY8n7pc/5TqarjDa1U0Tz0yH92B\
FODEPM2dMPgwqZfT7syj0B9fHBOB1BirlNFjw55/NZKeX0Tq7PQiXLfoPX2k+Ymp\
kbIq2eszh+6l/ePazIjmiSZuxyuC0F6dWdsU7JGDBcNeDsYq0ATdcT0gTlgn/FP7\
eHgZFLL8kFKJOGJgB7Sg7KxrUNb9uShr71ItOrL/8QFArDcCAwEAAaOCAh4wggIa\
MA8GA1UdEwEB/wQFMAMBAf8wggG1BgNVHSAEggGsMIIBqDCCAaQGDysGAQQBsT4A\
AAEJKIORMTCCAY8wggFIBggrBgEFBQcCAjCCAToaggE2UmVsaWFuY2Ugb24gb3Ig\
dXNlIG9mIHRoaXMgQ2VydGlmaWNhdGUgY3JlYXRlcyBhbiBhY2tub3dsZWRnbWVu\
dCBhbmQgYWNjZXB0YW5jZSBvZiB0aGUgdGhlbiBhcHBsaWNhYmxlIHN0YW5kYXJk\
IHRlcm1zIGFuZCBjb25kaXRpb25zIG9mIHVzZSwgdGhlIENlcnRpZmljYXRpb24g\
UHJhY3RpY2UgU3RhdGVtZW50IGFuZCB0aGUgUmVseWluZyBQYXJ0eSBBZ3JlZW1l\
bnQsIHdoaWNoIGNhbiBiZSBmb3VuZCBhdCB0aGUgYmVUUlVTVGVkIHdlYiBzaXRl\
LCBodHRwOi8vd3d3LmJldHJ1c3RlZC5jb20vcHJvZHVjdHNfc2VydmljZXMvaW5k\
ZXguaHRtbDBBBggrBgEFBQcCARY1aHR0cDovL3d3dy5iZXRydXN0ZWQuY29tL3By\
b2R1Y3RzX3NlcnZpY2VzL2luZGV4Lmh0bWwwHQYDVR0OBBYEFEU9w6nR3D8kVpgc\
cxiIav+DR+22MB8GA1UdIwQYMBaAFEU9w6nR3D8kVpgccxiIav+DR+22MA4GA1Ud\
DwEB/wQEAwIBBjANBgkqhkiG9w0BAQUFAAOCAQEASZK8o+6svfoNyYt5hhwjdrCA\
WXf82n+0S9/DZEtqTg6t8n1ZdwWtColzsPq8y9yNAIiPpqCy6qxSJ7+hSHyXEHu6\
7RMdmgduyzFiEuhjA6p9beP4G3YheBufS0OM00mG9htc9i5gFdPp43t1P9ACg9AY\
gkHNZTfqjjJ+vWuZXTARyNtIVBw74acT02pIk/c9jH8F6M7ziCpjBLjqflh8AXtb\
4cV97yHgjQ5dUX2xZ/2jvTg2xvI4hocalmhgRvsoFEdV4aeADGvi6t9NfJBIoDa9\
CReJf8Py05yc493EG931t3GzUwWJBtDLSoDByFOQtTwxiBdQn8nEDovYqAJjDQ==\
-----END CERTIFICATE-----\
\
beTRUSTed Root CA - Entrust Implementation\
==========================================\
\
MD5 Fingerprint=7D:86:90:8F:5B:F1:F2:40:C0:F7:3D:62:B5:A4:A9:3B\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1018515264 (0x3cb54f40)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: O=beTRUSTed, OU=beTRUSTed Root CAs, CN=beTRUSTed Root CA - Entrust Implementation\
        Validity\
            Not Before: Apr 11 08:24:27 2002 GMT\
            Not After : Apr 11 08:54:27 2022 GMT\
        Subject: O=beTRUSTed, OU=beTRUSTed Root CAs, CN=beTRUSTed Root CA - Entrust Implementation\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:ba:f4:44:03:aa:12:6a:b5:43:ec:55:92:b6:30:\
                    7d:35:57:0c:db:f3:0d:27:6e:4c:f7:50:a8:9b:4e:\
                    2b:6f:db:f5:ad:1c:4b:5d:b3:a9:c1:fe:7b:44:eb:\
                    5b:a3:05:0d:1f:c5:34:2b:30:00:29:f1:78:40:b2:\
                    a4:ff:3a:f4:01:88:17:7e:e6:d4:26:d3:ba:4c:ea:\
                    32:fb:43:77:97:87:23:c5:db:43:a3:f5:2a:a3:51:\
                    5e:e1:3b:d2:65:69:7e:55:15:9b:7a:e7:69:f7:44:\
                    e0:57:b5:15:e8:66:60:0f:0d:03:fb:82:8e:a3:e8:\
                    11:7b:6c:be:c7:63:0e:17:93:df:cf:4b:ae:6e:73:\
                    75:e0:f3:aa:b9:a4:c0:09:1b:85:ea:71:29:88:41:\
                    32:f9:f0:2a:0e:6c:09:f2:74:6b:66:6c:52:13:1f:\
                    18:bc:d4:3e:f7:d8:6e:20:9e:ca:fe:fc:21:94:ee:\
                    13:28:4b:d7:5c:5e:0c:66:ee:e9:bb:0f:c1:34:b1:\
                    7f:08:76:f3:3d:26:70:c9:8b:25:1d:62:24:0c:ea:\
                    1c:75:4e:c0:12:e4:ba:13:1d:30:29:2d:56:33:05:\
                    bb:97:59:7e:c6:49:4f:89:d7:2f:24:a8:b6:88:40:\
                    b5:64:92:53:56:24:e4:a2:a0:85:b3:5e:90:b4:12:\
                    33:cd\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Certificate Policies: \
                Policy: 1.3.6.1.4.1.6334.0.0.2.9.40.51377\
                  User Notice:\
                    Explicit Text: Reliance on or use of this Certificate creates an acknowledgment and acceptance of the then applicable standard terms and conditions of use, the Certification Practice Statement and the Relying Party Agreement, which can be found at the beTRUSTed web site, https://www.betrusted.com/products_services/index.html\
                  CPS: https://www.betrusted.com/products_services/index.html\
\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 CRL Distribution Points: \
                DirName:/O=beTRUSTed/OU=beTRUSTed Root CAs/CN=beTRUSTed Root CA - Entrust Implementation/CN=CRL1\
\
            X509v3 Private Key Usage Period: \
                Not Before: Apr 11 08:24:27 2002 GMT, Not After: Apr 11 08:54:27 2022 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:7D:70:E5:AE:38:8B:06:3F:AA:1C:1A:8F:F9:CF:24:30:AA:84:84:16\
\
            X509v3 Subject Key Identifier: \
                7D:70:E5:AE:38:8B:06:3F:AA:1C:1A:8F:F9:CF:24:30:AA:84:84:16\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0...V6.0:4.0....\
    Signature Algorithm: sha1WithRSAEncryption\
        2a:b8:17:ce:1f:10:94:eb:b8:9a:b7:b9:5f:ec:da:f7:92:24:\
        ac:dc:92:3b:c7:20:8d:f2:99:e5:5d:38:a1:c2:34:ed:c5:13:\
        59:5c:05:b5:2b:4f:61:9b:91:fb:41:fc:fc:d5:3c:4d:98:76:\
        06:f5:81:7d:eb:dd:90:e6:d1:56:54:da:e3:2d:0c:9f:11:32:\
        94:22:01:7a:f6:6c:2c:74:67:04:cc:a5:8f:8e:2c:b3:43:b5:\
        94:a2:d0:7d:e9:62:7f:06:be:27:01:83:9e:3a:fd:8a:ee:98:\
        43:4a:6b:d7:b5:97:3b:3a:bf:4f:6d:b4:63:fa:33:00:34:2e:\
        2d:6d:96:c9:7b:ca:99:63:ba:be:f4:f6:30:a0:2d:98:96:e9:\
        56:44:05:a9:44:a3:61:10:eb:82:a1:67:5d:bc:5d:27:75:aa:\
        8a:28:36:2a:38:92:d9:dd:a4:5e:00:a5:cc:cc:7c:29:2a:de:\
        28:90:ab:b7:e1:b6:ff:7d:25:0b:40:d8:aa:34:a3:2d:de:07:\
        eb:5f:ce:0a:dd:ca:7e:3a:7d:26:c1:62:68:3a:e6:2f:37:f3:\
        81:86:21:c4:a9:64:aa:ef:45:36:d1:1a:66:7c:f8:e9:37:d6:\
        d6:61:be:a2:ad:48:e7:df:e6:74:fe:d3:6d:7d:d2:25:dc:ac:\
        62:57:a9:f7\
-----BEGIN CERTIFICATE-----\
MIIGUTCCBTmgAwIBAgIEPLVPQDANBgkqhkiG9w0BAQUFADBmMRIwEAYDVQQKEwli\
ZVRSVVNUZWQxGzAZBgNVBAsTEmJlVFJVU1RlZCBSb290IENBczEzMDEGA1UEAxMq\
YmVUUlVTVGVkIFJvb3QgQ0EgLSBFbnRydXN0IEltcGxlbWVudGF0aW9uMB4XDTAy\
MDQxMTA4MjQyN1oXDTIyMDQxMTA4NTQyN1owZjESMBAGA1UEChMJYmVUUlVTVGVk\
MRswGQYDVQQLExJiZVRSVVNUZWQgUm9vdCBDQXMxMzAxBgNVBAMTKmJlVFJVU1Rl\
ZCBSb290IENBIC0gRW50cnVzdCBJbXBsZW1lbnRhdGlvbjCCASIwDQYJKoZIhvcN\
AQEBBQADggEPADCCAQoCggEBALr0RAOqEmq1Q+xVkrYwfTVXDNvzDSduTPdQqJtO\
K2/b9a0cS12zqcH+e0TrW6MFDR/FNCswACnxeECypP869AGIF37m1CbTukzqMvtD\
d5eHI8XbQ6P1KqNRXuE70mVpflUVm3rnafdE4Fe1FehmYA8NA/uCjqPoEXtsvsdj\
DheT389Lrm5zdeDzqrmkwAkbhepxKYhBMvnwKg5sCfJ0a2ZsUhMfGLzUPvfYbiCe\
yv78IZTuEyhL11xeDGbu6bsPwTSxfwh28z0mcMmLJR1iJAzqHHVOwBLkuhMdMCkt\
VjMFu5dZfsZJT4nXLySotohAtWSSU1Yk5KKghbNekLQSM80CAwEAAaOCAwUwggMB\
MIIBtwYDVR0gBIIBrjCCAaowggGmBg8rBgEEAbE+AAACCSiDkTEwggGRMIIBSQYI\
KwYBBQUHAgIwggE7GoIBN1JlbGlhbmNlIG9uIG9yIHVzZSBvZiB0aGlzIENlcnRp\
ZmljYXRlIGNyZWF0ZXMgYW4gYWNrbm93bGVkZ21lbnQgYW5kIGFjY2VwdGFuY2Ug\
b2YgdGhlIHRoZW4gYXBwbGljYWJsZSBzdGFuZGFyZCB0ZXJtcyBhbmQgY29uZGl0\
aW9ucyBvZiB1c2UsIHRoZSBDZXJ0aWZpY2F0aW9uIFByYWN0aWNlIFN0YXRlbWVu\
dCBhbmQgdGhlIFJlbHlpbmcgUGFydHkgQWdyZWVtZW50LCB3aGljaCBjYW4gYmUg\
Zm91bmQgYXQgdGhlIGJlVFJVU1RlZCB3ZWIgc2l0ZSwgaHR0cHM6Ly93d3cuYmV0\
cnVzdGVkLmNvbS9wcm9kdWN0c19zZXJ2aWNlcy9pbmRleC5odG1sMEIGCCsGAQUF\
BwIBFjZodHRwczovL3d3dy5iZXRydXN0ZWQuY29tL3Byb2R1Y3RzX3NlcnZpY2Vz\
L2luZGV4Lmh0bWwwEQYJYIZIAYb4QgEBBAQDAgAHMIGJBgNVHR8EgYEwfzB9oHug\
eaR3MHUxEjAQBgNVBAoTCWJlVFJVU1RlZDEbMBkGA1UECxMSYmVUUlVTVGVkIFJv\
b3QgQ0FzMTMwMQYDVQQDEypiZVRSVVNUZWQgUm9vdCBDQSAtIEVudHJ1c3QgSW1w\
bGVtZW50YXRpb24xDTALBgNVBAMTBENSTDEwKwYDVR0QBCQwIoAPMjAwMjA0MTEw\
ODI0MjdagQ8yMDIyMDQxMTA4NTQyN1owCwYDVR0PBAQDAgEGMB8GA1UdIwQYMBaA\
FH1w5a44iwY/qhwaj/nPJDCqhIQWMB0GA1UdDgQWBBR9cOWuOIsGP6ocGo/5zyQw\
qoSEFjAMBgNVHRMEBTADAQH/MB0GCSqGSIb2fQdBAAQQMA4bCFY2LjA6NC4wAwIE\
kDANBgkqhkiG9w0BAQUFAAOCAQEAKrgXzh8QlOu4mre5X+za95IkrNySO8cgjfKZ\
5V04ocI07cUTWVwFtStPYZuR+0H8/NU8TZh2BvWBfevdkObRVlTa4y0MnxEylCIB\
evZsLHRnBMylj44ss0O1lKLQfelifwa+JwGDnjr9iu6YQ0pr17WXOzq/T220Y/oz\
ADQuLW2WyXvKmWO6vvT2MKAtmJbpVkQFqUSjYRDrgqFnXbxdJ3Wqiig2KjiS2d2k\
XgClzMx8KSreKJCrt+G2/30lC0DYqjSjLd4H61/OCt3Kfjp9JsFiaDrmLzfzgYYh\
xKlkqu9FNtEaZnz46TfW1mG+oq1I59/mdP7TbX3SJdysYlep9w==\
-----END CERTIFICATE-----\
\
beTRUSTed Root CA - RSA Implementation\
======================================\
\
MD5 Fingerprint=86:42:05:09:BC:A7:9D:EC:1D:F3:2E:0E:BA:D8:1D:D0\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            3b:59:c7:7b:cd:5b:57:9e:bd:37:52:ac:76:b4:aa:1a\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: O=beTRUSTed, OU=beTRUSTed Root CAs, CN=beTRUSTed Root CA - RSA Implementation\
        Validity\
            Not Before: Apr 11 11:18:13 2002 GMT\
            Not After : Apr 12 11:07:25 2022 GMT\
        Subject: O=beTRUSTed, OU=beTRUSTed Root CAs, CN=beTRUSTed Root CA - RSA Implementation\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:e4:ba:34:30:09:8e:57:d0:b9:06:2c:6f:6e:24:\
                    80:22:bf:5d:43:a6:fa:4f:ac:82:e7:1c:68:70:85:\
                    1b:a3:6e:b5:aa:78:d9:6e:07:4b:3f:e9:df:f5:ea:\
                    e8:54:a1:61:8a:0e:2f:69:75:18:b7:0c:e5:14:8d:\
                    71:6e:98:b8:55:fc:0c:95:d0:9b:6e:e1:2d:88:d4:\
                    3a:40:6b:92:f1:99:96:64:de:db:ff:78:f4:ee:96:\
                    1d:47:89:7c:d4:be:b9:88:77:23:3a:09:e6:04:9e:\
                    6d:aa:5e:d2:c8:bd:9a:4e:19:df:89:ea:5b:0e:7e:\
                    c3:e4:b4:f0:e0:69:3b:88:0f:41:90:f8:d4:71:43:\
                    24:c1:8f:26:4b:3b:56:e9:ff:8c:6c:37:e9:45:ad:\
                    85:8c:53:c3:60:86:90:4a:96:c9:b3:54:b0:bb:17:\
                    f0:1c:45:d9:d4:1b:19:64:56:0a:19:f7:cc:e1:ff:\
                    86:af:7e:58:5e:ac:7a:90:1f:c9:28:39:45:7b:a2:\
                    b6:c7:9c:1f:da:85:d4:21:86:59:30:93:be:53:33:\
                    37:f6:ef:41:cf:33:c7:ab:72:6b:25:f5:f3:53:1b:\
                    0c:4c:2e:f1:75:4b:ef:a0:87:f7:fe:8a:15:d0:6c:\
                    d5:cb:f9:68:53:b9:70:15:13:c2:f5:2e:fb:43:35:\
                    75:2d\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: \
                CA:TRUE\
            X509v3 Certificate Policies: \
                Policy: 1.3.6.1.4.1.6334.0.0.3.9.40.51377\
                  CPS: http://www.betrusted.com/products_services/index.html\
                  User Notice:\
                    Explicit Text: Reliance on or use of this Certificate creates an acknowledgment and acceptance of the then applicable standard terms and conditions of use, the Certification Practice Statement and the Relying Party Agreement, which can be found at the beTRUSTed web site, http://www.betrusted.com/products_services/index.html\
\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:A9:EC:14:7E:F9:D9:43:CC:53:2B:14:AD:CF:F7:F0:59:89:41:CD:19\
\
            X509v3 Subject Key Identifier: \
                A9:EC:14:7E:F9:D9:43:CC:53:2B:14:AD:CF:F7:F0:59:89:41:CD:19\
    Signature Algorithm: sha1WithRSAEncryption\
        db:97:b0:75:ea:0c:c4:c1:98:ca:56:05:c0:a8:ad:26:48:af:\
        2d:20:e8:81:c7:b6:df:43:c1:2c:1d:75:4b:d4:42:8d:e7:7a:\
        a8:74:dc:66:42:59:87:b3:f5:69:6d:d9:a9:9e:b3:7d:1c:31:\
        c1:f5:54:e2:59:24:49:e5:ee:bd:39:a6:6b:8a:98:44:fb:9b:\
        d7:2a:83:97:34:2d:c7:7d:35:4c:2d:34:b8:3e:0d:c4:ec:88:\
        27:af:9e:92:fd:50:61:82:a8:60:07:14:53:cc:65:13:c1:f6:\
        47:44:69:d2:31:c8:a6:dd:2e:b3:0b:de:4a:8d:5b:3d:ab:0d:\
        c2:35:52:a2:56:37:cc:32:8b:28:85:42:9c:91:40:7a:70:2b:\
        38:36:d5:e1:73:1a:1f:e5:fa:7e:5f:dc:d6:9c:3b:30:ea:db:\
        c0:5b:27:5c:d3:73:07:c1:c2:f3:4c:9b:6f:9f:1b:ca:1e:aa:\
        a8:38:33:09:58:b2:ae:fc:07:e8:36:dc:55:ba:2f:4f:40:fe:\
        7a:bd:06:a6:81:c1:93:22:7c:86:11:0a:06:77:48:ae:35:b7:\
        2f:32:9a:61:5e:8b:be:29:9f:29:24:88:56:39:2c:a8:d2:ab:\
        96:03:5a:d4:48:9f:b9:40:84:0b:98:68:fb:01:43:d6:1b:e2:\
        09:b1:97:1c\
-----BEGIN CERTIFICATE-----\
MIIFaDCCBFCgAwIBAgIQO1nHe81bV569N1KsdrSqGjANBgkqhkiG9w0BAQUFADBi\
MRIwEAYDVQQKEwliZVRSVVNUZWQxGzAZBgNVBAsTEmJlVFJVU1RlZCBSb290IENB\
czEvMC0GA1UEAxMmYmVUUlVTVGVkIFJvb3QgQ0EgLSBSU0EgSW1wbGVtZW50YXRp\
b24wHhcNMDIwNDExMTExODEzWhcNMjIwNDEyMTEwNzI1WjBiMRIwEAYDVQQKEwli\
ZVRSVVNUZWQxGzAZBgNVBAsTEmJlVFJVU1RlZCBSb290IENBczEvMC0GA1UEAxMm\
YmVUUlVTVGVkIFJvb3QgQ0EgLSBSU0EgSW1wbGVtZW50YXRpb24wggEiMA0GCSqG\
SIb3DQEBAQUAA4IBDwAwggEKAoIBAQDkujQwCY5X0LkGLG9uJIAiv11DpvpPrILn\
HGhwhRujbrWqeNluB0s/6d/16uhUoWGKDi9pdRi3DOUUjXFumLhV/AyV0Jtu4S2I\
1DpAa5LxmZZk3tv/ePTulh1HiXzUvrmIdyM6CeYEnm2qXtLIvZpOGd+J6lsOfsPk\
tPDgaTuID0GQ+NRxQyTBjyZLO1bp/4xsN+lFrYWMU8NghpBKlsmzVLC7F/AcRdnU\
GxlkVgoZ98zh/4avflherHqQH8koOUV7orbHnB/ahdQhhlkwk75TMzf270HPM8er\
cmsl9fNTGwxMLvF1S++gh/f+ihXQbNXL+WhTuXAVE8L1LvtDNXUtAgMBAAGjggIY\
MIICFDAMBgNVHRMEBTADAQH/MIIBtQYDVR0gBIIBrDCCAagwggGkBg8rBgEEAbE+\
AAADCSiDkTEwggGPMEEGCCsGAQUFBwIBFjVodHRwOi8vd3d3LmJldHJ1c3RlZC5j\
b20vcHJvZHVjdHNfc2VydmljZXMvaW5kZXguaHRtbDCCAUgGCCsGAQUFBwICMIIB\
OhqCATZSZWxpYW5jZSBvbiBvciB1c2Ugb2YgdGhpcyBDZXJ0aWZpY2F0ZSBjcmVh\
dGVzIGFuIGFja25vd2xlZGdtZW50IGFuZCBhY2NlcHRhbmNlIG9mIHRoZSB0aGVu\
IGFwcGxpY2FibGUgc3RhbmRhcmQgdGVybXMgYW5kIGNvbmRpdGlvbnMgb2YgdXNl\
LCB0aGUgQ2VydGlmaWNhdGlvbiBQcmFjdGljZSBTdGF0ZW1lbnQgYW5kIHRoZSBS\
ZWx5aW5nIFBhcnR5IEFncmVlbWVudCwgd2hpY2ggY2FuIGJlIGZvdW5kIGF0IHRo\
ZSBiZVRSVVNUZWQgd2ViIHNpdGUsIGh0dHA6Ly93d3cuYmV0cnVzdGVkLmNvbS9w\
cm9kdWN0c19zZXJ2aWNlcy9pbmRleC5odG1sMAsGA1UdDwQEAwIBBjAfBgNVHSME\
GDAWgBSp7BR++dlDzFMrFK3P9/BZiUHNGTAdBgNVHQ4EFgQUqewUfvnZQ8xTKxSt\
z/fwWYlBzRkwDQYJKoZIhvcNAQEFBQADggEBANuXsHXqDMTBmMpWBcCorSZIry0g\
6IHHtt9DwSwddUvUQo3neqh03GZCWYez9Wlt2ames30cMcH1VOJZJEnl7r05pmuK\
mET7m9cqg5c0Lcd9NUwtNLg+DcTsiCevnpL9UGGCqGAHFFPMZRPB9kdEadIxyKbd\
LrML3kqNWz2rDcI1UqJWN8wyiyiFQpyRQHpwKzg21eFzGh/l+n5f3NacOzDq28Bb\
J1zTcwfBwvNMm2+fG8oeqqg4MwlYsq78B+g23FW6L09A/nq9BqaBwZMifIYRCgZ3\
SK41ty8ymmFei74pnykkiFY5LKjSq5YDWtRIn7lAhAuYaPsBQ9Yb4gmxlxw=\
-----END CERTIFICATE-----\
\
RSA Security 2048 v3\
====================\
\
MD5 Fingerprint=77:0D:19:B1:21:FD:00:42:9C:3E:0C:A5:DD:0B:02:8E\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            0a:01:01:01:00:00:02:7c:00:00:00:0a:00:00:00:02\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: O=RSA Security Inc, OU=RSA Security 2048 V3\
        Validity\
            Not Before: Feb 22 20:39:23 2001 GMT\
            Not After : Feb 22 20:39:23 2026 GMT\
        Subject: O=RSA Security Inc, OU=RSA Security 2048 V3\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b7:8f:55:71:d2:80:dd:7b:69:79:a7:f0:18:50:\
                    32:3c:62:67:f6:0a:95:07:dd:e6:1b:f3:9e:d9:d2:\
                    41:54:6b:ad:9f:7c:be:19:cd:fb:46:ab:41:68:1e:\
                    18:ea:55:c8:2f:91:78:89:28:fb:27:29:60:ff:df:\
                    8f:8c:3b:c9:49:9b:b5:a4:94:ce:01:ea:3e:b5:63:\
                    7b:7f:26:fd:19:dd:c0:21:bd:84:d1:2d:4f:46:c3:\
                    4e:dc:d8:37:39:3b:28:af:cb:9d:1a:ea:2b:af:21:\
                    a5:c1:23:22:b8:b8:1b:5a:13:87:57:83:d1:f0:20:\
                    e7:e8:4f:23:42:b0:00:a5:7d:89:e9:e9:61:73:94:\
                    98:71:26:bc:2d:6a:e0:f7:4d:f0:f1:b6:2a:38:31:\
                    81:0d:29:e1:00:c1:51:0f:4c:52:f8:04:5a:aa:7d:\
                    72:d3:b8:87:2a:bb:63:10:03:2a:b3:a1:4f:0d:5a:\
                    5e:46:b7:3d:0e:f5:74:ec:99:9f:f9:3d:24:81:88:\
                    a6:dd:60:54:e8:95:36:3d:c6:09:93:9a:a3:12:80:\
                    00:55:99:19:47:bd:d0:a5:7c:c3:ba:fb:1f:f7:f5:\
                    0f:f8:ac:b9:b5:f4:37:98:13:18:de:85:5b:b7:0c:\
                    82:3b:87:6f:95:39:58:30:da:6e:01:68:17:22:cc:\
                    c0:0b\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:07:C3:51:30:A4:AA:E9:45:AE:35:24:FA:FF:24:2C:33:D0:B1:9D:8C\
\
            X509v3 Subject Key Identifier: \
                07:C3:51:30:A4:AA:E9:45:AE:35:24:FA:FF:24:2C:33:D0:B1:9D:8C\
    Signature Algorithm: sha1WithRSAEncryption\
        5f:3e:86:76:6e:b8:35:3c:4e:36:1c:1e:79:98:bf:fd:d5:12:\
        11:79:52:0e:ee:31:89:bc:dd:7f:f9:d1:c6:15:21:e8:8a:01:\
        54:0d:3a:fb:54:b9:d6:63:d4:b1:aa:96:4d:a2:42:4d:d4:53:\
        1f:8b:10:de:7f:65:be:60:13:27:71:88:a4:73:e3:84:63:d1:\
        a4:55:e1:50:93:e6:1b:0e:79:d0:67:bc:46:c8:bf:3f:17:0d:\
        95:e6:c6:90:69:de:e7:b4:2f:de:95:7d:d0:12:3f:3d:3e:7f:\
        4d:3f:14:68:f5:11:50:d5:c1:f4:90:a5:08:1d:31:60:ff:60:\
        8c:23:54:0a:af:fe:a1:6e:c5:d1:7a:2a:68:78:cf:1e:82:0a:\
        20:b4:1f:ad:e5:85:b2:6a:68:75:4e:ad:25:37:94:85:be:bd:\
        a1:d4:ea:b7:0c:4b:3c:9d:e8:12:00:f0:5f:ac:0d:e1:ac:70:\
        63:73:f7:7f:79:9f:32:25:42:74:05:80:28:bf:bd:c1:24:96:\
        58:15:b1:17:21:e9:89:4b:db:07:88:67:f4:15:ad:70:3e:2f:\
        4d:85:3b:c2:b7:db:fe:98:68:23:89:e1:74:0f:de:f4:c5:84:\
        63:29:1b:cc:cb:07:c9:00:a4:a9:d7:c2:22:4f:67:d7:77:ec:\
        20:05:61:de\
-----BEGIN CERTIFICATE-----\
MIIDYTCCAkmgAwIBAgIQCgEBAQAAAnwAAAAKAAAAAjANBgkqhkiG9w0BAQUFADA6\
MRkwFwYDVQQKExBSU0EgU2VjdXJpdHkgSW5jMR0wGwYDVQQLExRSU0EgU2VjdXJp\
dHkgMjA0OCBWMzAeFw0wMTAyMjIyMDM5MjNaFw0yNjAyMjIyMDM5MjNaMDoxGTAX\
BgNVBAoTEFJTQSBTZWN1cml0eSBJbmMxHTAbBgNVBAsTFFJTQSBTZWN1cml0eSAy\
MDQ4IFYzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAt49VcdKA3Xtp\
eafwGFAyPGJn9gqVB93mG/Oe2dJBVGutn3y+Gc37RqtBaB4Y6lXIL5F4iSj7Jylg\
/9+PjDvJSZu1pJTOAeo+tWN7fyb9Gd3AIb2E0S1PRsNO3Ng3OTsor8udGuorryGl\
wSMiuLgbWhOHV4PR8CDn6E8jQrAApX2J6elhc5SYcSa8LWrg903w8bYqODGBDSnh\
AMFRD0xS+ARaqn1y07iHKrtjEAMqs6FPDVpeRrc9DvV07Jmf+T0kgYim3WBU6JU2\
PcYJk5qjEoAAVZkZR73QpXzDuvsf9/UP+Ky5tfQ3mBMY3oVbtwyCO4dvlTlYMNpu\
AWgXIszACwIDAQABo2MwYTAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIB\
BjAfBgNVHSMEGDAWgBQHw1EwpKrpRa41JPr/JCwz0LGdjDAdBgNVHQ4EFgQUB8NR\
MKSq6UWuNST6/yQsM9CxnYwwDQYJKoZIhvcNAQEFBQADggEBAF8+hnZuuDU8TjYc\
HnmYv/3VEhF5Ug7uMYm83X/50cYVIeiKAVQNOvtUudZj1LGqlk2iQk3UUx+LEN5/\
Zb5gEydxiKRz44Rj0aRV4VCT5hsOedBnvEbIvz8XDZXmxpBp3ue0L96VfdASPz0+\
f00/FGj1EVDVwfSQpQgdMWD/YIwjVAqv/qFuxdF6Kmh4zx6CCiC0H63lhbJqaHVO\
rSU3lIW+vaHU6rcMSzyd6BIA8F+sDeGscGNz9395nzIlQnQFgCi/vcEkllgVsRch\
6YlL2weIZ/QVrXA+L02FO8K32/6YaCOJ4XQP3vTFhGMpG8zLB8kApKnXwiJPZ9d3\
7CAFYd4=\
-----END CERTIFICATE-----\
\
RSA Security 1024 v3\
====================\
\
MD5 Fingerprint=3A:E5:50:B0:39:BE:C7:46:36:33:A1:FE:82:3E:8D:94\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            0a:01:01:01:00:00:02:7c:00:00:00:0b:00:00:00:02\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: O=RSA Security Inc, OU=RSA Security 1024 V3\
        Validity\
            Not Before: Feb 22 21:01:49 2001 GMT\
            Not After : Feb 22 20:01:49 2026 GMT\
        Subject: O=RSA Security Inc, OU=RSA Security 1024 V3\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:d5:dd:fe:66:09:cf:24:3c:3e:ae:81:4e:4e:8a:\
                    c4:69:80:5b:59:3b:df:b9:4d:4c:ca:b5:2d:c3:27:\
                    2d:3c:af:00:42:6d:bc:28:a6:96:cf:7f:d7:58:ac:\
                    83:0a:a3:55:b5:7b:17:90:15:84:4c:8a:ee:26:99:\
                    dc:58:ef:c7:38:a6:aa:af:d0:8e:42:c8:62:d7:ab:\
                    ac:a9:fb:4a:7d:bf:ea:fe:12:4d:dd:ff:26:2d:6f:\
                    36:54:68:c8:d2:84:56:ee:92:53:61:09:b3:3f:39:\
                    9b:a8:c9:9b:bd:ce:9f:7e:d4:19:6a:16:29:18:be:\
                    d7:3a:69:dc:25:5b:33:1a:51\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:C4:C0:1C:A4:07:94:FD:CD:4D:01:D4:54:DA:A5:0C:5F:DE:AE:05:5A\
\
            X509v3 Subject Key Identifier: \
                C4:C0:1C:A4:07:94:FD:CD:4D:01:D4:54:DA:A5:0C:5F:DE:AE:05:5A\
    Signature Algorithm: sha1WithRSAEncryption\
        3f:2d:6a:e3:26:43:95:7d:89:97:65:fb:75:e4:72:1d:46:57:\
        c4:61:6b:69:9f:12:9b:2c:d5:5a:e8:c0:a2:f0:43:95:e3:1f:\
        e9:76:cd:dc:eb:bc:93:a0:65:0a:c7:4d:4f:5f:a7:af:a2:46:\
        14:b9:0c:f3:cc:bd:6a:6e:b7:9d:de:25:42:d0:54:ff:9e:68:\
        73:63:dc:24:eb:22:bf:a8:72:f2:5e:00:e1:0d:4e:3a:43:6e:\
        99:4e:3f:89:78:03:98:ca:f3:55:cc:9d:ae:8e:c1:aa:45:98:\
        fa:8f:1a:a0:8d:88:23:f1:15:41:0d:a5:46:3e:91:3f:8b:eb:\
        f7:71\
-----BEGIN CERTIFICATE-----\
MIICXDCCAcWgAwIBAgIQCgEBAQAAAnwAAAALAAAAAjANBgkqhkiG9w0BAQUFADA6\
MRkwFwYDVQQKExBSU0EgU2VjdXJpdHkgSW5jMR0wGwYDVQQLExRSU0EgU2VjdXJp\
dHkgMTAyNCBWMzAeFw0wMTAyMjIyMTAxNDlaFw0yNjAyMjIyMDAxNDlaMDoxGTAX\
BgNVBAoTEFJTQSBTZWN1cml0eSBJbmMxHTAbBgNVBAsTFFJTQSBTZWN1cml0eSAx\
MDI0IFYzMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDV3f5mCc8kPD6ugU5O\
isRpgFtZO9+5TUzKtS3DJy08rwBCbbwoppbPf9dYrIMKo1W1exeQFYRMiu4mmdxY\
78c4pqqv0I5CyGLXq6yp+0p9v+r+Ek3d/yYtbzZUaMjShFbuklNhCbM/OZuoyZu9\
zp9+1BlqFikYvtc6adwlWzMaUQIDAQABo2MwYTAPBgNVHRMBAf8EBTADAQH/MA4G\
A1UdDwEB/wQEAwIBBjAfBgNVHSMEGDAWgBTEwBykB5T9zU0B1FTapQxf3q4FWjAd\
BgNVHQ4EFgQUxMAcpAeU/c1NAdRU2qUMX96uBVowDQYJKoZIhvcNAQEFBQADgYEA\
Py1q4yZDlX2Jl2X7deRyHUZXxGFraZ8SmyzVWujAovBDleMf6XbN3Ou8k6BlCsdN\
T1+nr6JGFLkM88y9am63nd4lQtBU/55oc2PcJOsiv6hy8l4A4Q1OOkNumU4/iXgD\
mMrzVcydro7BqkWY+o8aoI2II/EVQQ2lRj6RP4vr93E=\
-----END CERTIFICATE-----\
\
GeoTrust Global CA\
==================\
\
MD5 Fingerprint=F7:75:AB:29:FB:51:4E:B7:77:5E:FF:05:3C:99:8E:F5\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 144470 (0x23456)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=GeoTrust Inc., CN=GeoTrust Global CA\
        Validity\
            Not Before: May 21 04:00:00 2002 GMT\
            Not After : May 21 04:00:00 2022 GMT\
        Subject: C=US, O=GeoTrust Inc., CN=GeoTrust Global CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:da:cc:18:63:30:fd:f4:17:23:1a:56:7e:5b:df:\
                    3c:6c:38:e4:71:b7:78:91:d4:bc:a1:d8:4c:f8:a8:\
                    43:b6:03:e9:4d:21:07:08:88:da:58:2f:66:39:29:\
                    bd:05:78:8b:9d:38:e8:05:b7:6a:7e:71:a4:e6:c4:\
                    60:a6:b0:ef:80:e4:89:28:0f:9e:25:d6:ed:83:f3:\
                    ad:a6:91:c7:98:c9:42:18:35:14:9d:ad:98:46:92:\
                    2e:4f:ca:f1:87:43:c1:16:95:57:2d:50:ef:89:2d:\
                    80:7a:57:ad:f2:ee:5f:6b:d2:00:8d:b9:14:f8:14:\
                    15:35:d9:c0:46:a3:7b:72:c8:91:bf:c9:55:2b:cd:\
                    d0:97:3e:9c:26:64:cc:df:ce:83:19:71:ca:4e:e6:\
                    d4:d5:7b:a9:19:cd:55:de:c8:ec:d2:5e:38:53:e5:\
                    5c:4f:8c:2d:fe:50:23:36:fc:66:e6:cb:8e:a4:39:\
                    19:00:b7:95:02:39:91:0b:0e:fe:38:2e:d1:1d:05:\
                    9a:f6:4d:3e:6f:0f:07:1d:af:2c:1e:8f:60:39:e2:\
                    fa:36:53:13:39:d4:5e:26:2b:db:3d:a8:14:bd:32:\
                    eb:18:03:28:52:04:71:e5:ab:33:3d:e1:38:bb:07:\
                    36:84:62:9c:79:ea:16:30:f4:5f:c0:2b:e8:71:6b:\
                    e4:f9\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                C0:7A:98:68:8D:89:FB:AB:05:64:0C:11:7D:AA:7D:65:B8:CA:CC:4E\
            X509v3 Authority Key Identifier: \
                keyid:C0:7A:98:68:8D:89:FB:AB:05:64:0C:11:7D:AA:7D:65:B8:CA:CC:4E\
\
    Signature Algorithm: sha1WithRSAEncryption\
        35:e3:29:6a:e5:2f:5d:54:8e:29:50:94:9f:99:1a:14:e4:8f:\
        78:2a:62:94:a2:27:67:9e:d0:cf:1a:5e:47:e9:c1:b2:a4:cf:\
        dd:41:1a:05:4e:9b:4b:ee:4a:6f:55:52:b3:24:a1:37:0a:eb:\
        64:76:2a:2e:2c:f3:fd:3b:75:90:bf:fa:71:d8:c7:3d:37:d2:\
        b5:05:95:62:b9:a6:de:89:3d:36:7b:38:77:48:97:ac:a6:20:\
        8f:2e:a6:c9:0c:c2:b2:99:45:00:c7:ce:11:51:22:22:e0:a5:\
        ea:b6:15:48:09:64:ea:5e:4f:74:f7:05:3e:c7:8a:52:0c:db:\
        15:b4:bd:6d:9b:e5:c6:b1:54:68:a9:e3:69:90:b6:9a:a5:0f:\
        b8:b9:3f:20:7d:ae:4a:b5:b8:9c:e4:1d:b6:ab:e6:94:a5:c1:\
        c7:83:ad:db:f5:27:87:0e:04:6c:d5:ff:dd:a0:5d:ed:87:52:\
        b7:2b:15:02:ae:39:a6:6a:74:e9:da:c4:e7:bc:4d:34:1e:a9:\
        5c:4d:33:5f:92:09:2f:88:66:5d:77:97:c7:1d:76:13:a9:d5:\
        e5:f1:16:09:11:35:d5:ac:db:24:71:70:2c:98:56:0b:d9:17:\
        b4:d1:e3:51:2b:5e:75:e8:d5:d0:dc:4f:34:ed:c2:05:66:80:\
        a1:cb:e6:33\
-----BEGIN CERTIFICATE-----\
MIIDVDCCAjygAwIBAgIDAjRWMA0GCSqGSIb3DQEBBQUAMEIxCzAJBgNVBAYTAlVT\
MRYwFAYDVQQKEw1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9i\
YWwgQ0EwHhcNMDIwNTIxMDQwMDAwWhcNMjIwNTIxMDQwMDAwWjBCMQswCQYDVQQG\
EwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5jLjEbMBkGA1UEAxMSR2VvVHJ1c3Qg\
R2xvYmFsIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2swYYzD9\
9BcjGlZ+W988bDjkcbd4kdS8odhM+KhDtgPpTSEHCIjaWC9mOSm9BXiLnTjoBbdq\
fnGk5sRgprDvgOSJKA+eJdbtg/OtppHHmMlCGDUUna2YRpIuT8rxh0PBFpVXLVDv\
iS2Aelet8u5fa9IAjbkU+BQVNdnARqN7csiRv8lVK83Qlz6cJmTM386DGXHKTubU\
1XupGc1V3sjs0l44U+VcT4wt/lAjNvxm5suOpDkZALeVAjmRCw7+OC7RHQWa9k0+\
bw8HHa8sHo9gOeL6NlMTOdReJivbPagUvTLrGAMoUgRx5aszPeE4uwc2hGKceeoW\
MPRfwCvocWvk+QIDAQABo1MwUTAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTA\
ephojYn7qwVkDBF9qn1luMrMTjAfBgNVHSMEGDAWgBTAephojYn7qwVkDBF9qn1l\
uMrMTjANBgkqhkiG9w0BAQUFAAOCAQEANeMpauUvXVSOKVCUn5kaFOSPeCpilKIn\
Z57QzxpeR+nBsqTP3UEaBU6bS+5Kb1VSsyShNwrrZHYqLizz/Tt1kL/6cdjHPTfS\
tQWVYrmm3ok9Nns4d0iXrKYgjy6myQzCsplFAMfOEVEiIuCl6rYVSAlk6l5PdPcF\
PseKUgzbFbS9bZvlxrFUaKnjaZC2mqUPuLk/IH2uSrW4nOQdtqvmlKXBx4Ot2/Un\
hw4EbNX/3aBd7YdStysVAq45pmp06drE57xNNB6pXE0zX5IJL4hmXXeXxx12E6nV\
5fEWCRE11azbJHFwLJhWC9kXtNHjUStedejV0NxPNO3CBWaAocvmMw==\
-----END CERTIFICATE-----\
\
UTN-USER First-Network Applications\
===================================\
\
MD5 Fingerprint=BF:60:59:A3:5B:BA:F6:A7:76:42:DA:6F:1A:7B:50:CF\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            44:be:0c:8b:50:00:24:b4:11:d3:36:30:4b:c0:33:77\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN-USERFirst-Network Applications\
        Validity\
            Not Before: Jul  9 18:48:39 1999 GMT\
            Not After : Jul  9 18:57:49 2019 GMT\
        Subject: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN-USERFirst-Network Applications\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b3:fb:91:a1:e4:36:55:85:ac:06:34:5b:a0:9a:\
                    58:b2:f8:b5:0f:05:77:83:ae:32:b1:76:92:68:ec:\
                    23:4a:c9:76:3f:e3:9c:b6:37:79:03:b9:ab:69:8d:\
                    07:25:b6:19:67:e4:b0:1b:18:73:61:4a:e8:7e:cd:\
                    d3:2f:64:e3:a6:7c:0c:fa:17:80:a3:0d:47:89:4f:\
                    51:71:2f:ee:fc:3f:f9:b8:16:80:87:89:93:25:20:\
                    9a:43:82:69:24:76:28:59:35:a1:1d:c0:7f:83:06:\
                    64:16:20:2c:d3:49:a4:85:b4:c0:61:7f:51:08:f8:\
                    68:15:91:80:cb:a5:d5:ee:3b:3a:f4:84:04:5e:60:\
                    59:a7:8c:34:72:ee:b8:78:c5:d1:3b:12:4a:6f:7e:\
                    65:27:b9:a4:55:c5:b9:6f:43:a4:c5:1d:2c:99:c0:\
                    52:a4:78:4c:15:b3:40:98:08:6b:43:c6:01:b0:7a:\
                    7b:f5:6b:1c:22:3f:cb:ef:ff:a8:d0:3a:4b:76:15:\
                    9e:d2:d1:c6:2e:e3:db:57:1b:32:a2:b8:6f:e8:86:\
                    a6:3f:70:ab:e5:70:92:ab:44:1e:40:50:fb:9c:a3:\
                    62:e4:6c:6e:a0:c8:de:e2:80:42:fa:e9:2f:e8:ce:\
                    32:04:8f:7c:8d:b7:1c:a3:35:3c:15:dd:9e:c3:ae:\
                    97:a5\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                FA:86:C9:DB:E0:BA:E9:78:F5:4B:A8:D6:15:DF:F0:D3:E1:6A:14:3C\
            X509v3 CRL Distribution Points: \
                URI:http://crl.usertrust.com/UTN-USERFirst-NetworkApplications.crl\
\
    Signature Algorithm: sha1WithRSAEncryption\
        a4:f3:25:cc:d1:d4:91:83:22:d0:cc:32:ab:9b:96:4e:34:91:\
        54:20:25:34:61:5f:2a:02:15:e1:8b:aa:ff:7d:64:51:cf:0a:\
        ff:bc:7d:d8:21:6a:78:cb:2f:51:6f:f8:42:1d:33:bd:eb:b5:\
        7b:94:c3:c3:a9:a0:2d:df:d1:29:1f:1d:fe:8f:3f:bb:a8:45:\
        2a:7f:d1:6e:55:24:e2:bb:02:fb:31:3f:be:e8:bc:ec:40:2b:\
        f8:01:d4:56:38:e4:ca:44:82:b5:61:20:21:67:65:f6:f0:0b:\
        e7:34:f8:a5:c2:9c:a3:5c:40:1f:85:93:95:06:de:4f:d4:27:\
        a9:b6:a5:fc:16:cd:73:31:3f:b8:65:27:cf:d4:53:1a:f0:ac:\
        6e:9f:4f:05:0c:03:81:a7:84:29:c4:5a:bd:64:57:72:ad:3b:\
        cf:37:18:a6:98:c6:ad:06:b4:dc:08:a3:04:d5:29:a4:96:9a:\
        12:67:4a:8c:60:45:9d:f1:23:9a:b0:00:9c:68:b5:98:50:d3:\
        ef:8e:2e:92:65:b1:48:3e:21:be:15:30:2a:0d:b5:0c:a3:6b:\
        3f:ae:7f:57:f5:1f:96:7c:df:6f:dd:82:30:2c:65:1b:40:4a:\
        cd:68:b9:72:ec:71:76:ec:54:8e:1f:85:0c:01:6a:fa:a6:38:\
        ac:1f:c4:84\
-----BEGIN CERTIFICATE-----\
MIIEZDCCA0ygAwIBAgIQRL4Mi1AAJLQR0zYwS8AzdzANBgkqhkiG9w0BAQUFADCB\
ozELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAlVUMRcwFQYDVQQHEw5TYWx0IExha2Ug\
Q2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMSEwHwYDVQQLExho\
dHRwOi8vd3d3LnVzZXJ0cnVzdC5jb20xKzApBgNVBAMTIlVUTi1VU0VSRmlyc3Qt\
TmV0d29yayBBcHBsaWNhdGlvbnMwHhcNOTkwNzA5MTg0ODM5WhcNMTkwNzA5MTg1\
NzQ5WjCBozELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAlVUMRcwFQYDVQQHEw5TYWx0\
IExha2UgQ2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMSEwHwYD\
VQQLExhodHRwOi8vd3d3LnVzZXJ0cnVzdC5jb20xKzApBgNVBAMTIlVUTi1VU0VS\
Rmlyc3QtTmV0d29yayBBcHBsaWNhdGlvbnMwggEiMA0GCSqGSIb3DQEBAQUAA4IB\
DwAwggEKAoIBAQCz+5Gh5DZVhawGNFugmliy+LUPBXeDrjKxdpJo7CNKyXY/45y2\
N3kDuatpjQclthln5LAbGHNhSuh+zdMvZOOmfAz6F4CjDUeJT1FxL+78P/m4FoCH\
iZMlIJpDgmkkdihZNaEdwH+DBmQWICzTSaSFtMBhf1EI+GgVkYDLpdXuOzr0hARe\
YFmnjDRy7rh4xdE7EkpvfmUnuaRVxblvQ6TFHSyZwFKkeEwVs0CYCGtDxgGwenv1\
axwiP8vv/6jQOkt2FZ7S0cYu49tXGzKiuG/ohqY/cKvlcJKrRB5AUPuco2LkbG6g\
yN7igEL66S/ozjIEj3yNtxyjNTwV3Z7DrpelAgMBAAGjgZEwgY4wCwYDVR0PBAQD\
AgHGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFPqGydvguul49Uuo1hXf8NPh\
ahQ8ME8GA1UdHwRIMEYwRKBCoECGPmh0dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9V\
VE4tVVNFUkZpcnN0LU5ldHdvcmtBcHBsaWNhdGlvbnMuY3JsMA0GCSqGSIb3DQEB\
BQUAA4IBAQCk8yXM0dSRgyLQzDKrm5ZONJFUICU0YV8qAhXhi6r/fWRRzwr/vH3Y\
IWp4yy9Rb/hCHTO967V7lMPDqaAt39EpHx3+jz+7qEUqf9FuVSTiuwL7MT++6Lzs\
QCv4AdRWOOTKRIK1YSAhZ2X28AvnNPilwpyjXEAfhZOVBt5P1CeptqX8Fs1zMT+4\
ZSfP1FMa8Kxun08FDAOBp4QpxFq9ZFdyrTvPNximmMatBrTcCKME1SmklpoSZ0qM\
YEWd8SOasACcaLWYUNPvji6SZbFIPiG+FTAqDbUMo2s/rn9X9R+WfN9v3YIwLGUb\
QErNaLly7HF27FSOH4UMAWr6pjisH8SE\
-----END CERTIFICATE-----\
\
Thawte Personal Freemail CA\
===========================\
\
MD5 Fingerprint=1E:74:C3:86:3C:0C:35:C5:3E:C2:7F:EF:3C:AA:3C:D9\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting, OU=Certification Services Division, CN=Thawte Personal Freemail CA/emailAddress=personal-freemail@thawte.com\
        Validity\
            Not Before: Jan  1 00:00:00 1996 GMT\
            Not After : Dec 31 23:59:59 2020 GMT\
        Subject: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting, OU=Certification Services Division, CN=Thawte Personal Freemail CA/emailAddress=personal-freemail@thawte.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:d4:69:d7:d4:b0:94:64:5b:71:e9:47:d8:0c:51:\
                    b6:ea:72:91:b0:84:5e:7d:2d:0d:8f:7b:12:df:85:\
                    25:75:28:74:3a:42:2c:63:27:9f:95:7b:4b:ef:7e:\
                    19:87:1d:86:ea:a3:dd:b9:ce:96:64:1a:c2:14:6e:\
                    44:ac:7c:e6:8f:e8:4d:0f:71:1f:40:38:a6:00:a3:\
                    87:78:f6:f9:94:86:5e:ad:ea:c0:5e:76:eb:d9:14:\
                    a3:5d:6e:7a:7c:0c:a5:4b:55:7f:06:19:29:7f:9e:\
                    9a:26:d5:6a:bb:38:24:08:6a:98:c7:b1:da:a3:98:\
                    91:fd:79:db:e5:5a:c4:1c:b9\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
    Signature Algorithm: md5WithRSAEncryption\
        c7:ec:92:7e:4e:f8:f5:96:a5:67:62:2a:a4:f0:4d:11:60:d0:\
        6f:8d:60:58:61:ac:26:bb:52:35:5c:08:cf:30:fb:a8:4a:96:\
        8a:1f:62:42:23:8c:17:0f:f4:ba:64:9c:17:ac:47:29:df:9d:\
        98:5e:d2:6c:60:71:5c:a2:ac:dc:79:e3:e7:6e:00:47:1f:b5:\
        0d:28:e8:02:9d:e4:9a:fd:13:f4:a6:d9:7c:b1:f8:dc:5f:23:\
        26:09:91:80:73:d0:14:1b:de:43:a9:83:25:f2:e6:9c:2f:15:\
        ca:fe:a6:ab:8a:07:75:8b:0c:dd:51:84:6b:e4:f8:d1:ce:77:\
        a2:81\
-----BEGIN CERTIFICATE-----\
MIIDLTCCApagAwIBAgIBADANBgkqhkiG9w0BAQQFADCB0TELMAkGA1UEBhMCWkEx\
FTATBgNVBAgTDFdlc3Rlcm4gQ2FwZTESMBAGA1UEBxMJQ2FwZSBUb3duMRowGAYD\
VQQKExFUaGF3dGUgQ29uc3VsdGluZzEoMCYGA1UECxMfQ2VydGlmaWNhdGlvbiBT\
ZXJ2aWNlcyBEaXZpc2lvbjEkMCIGA1UEAxMbVGhhd3RlIFBlcnNvbmFsIEZyZWVt\
YWlsIENBMSswKQYJKoZIhvcNAQkBFhxwZXJzb25hbC1mcmVlbWFpbEB0aGF3dGUu\
Y29tMB4XDTk2MDEwMTAwMDAwMFoXDTIwMTIzMTIzNTk1OVowgdExCzAJBgNVBAYT\
AlpBMRUwEwYDVQQIEwxXZXN0ZXJuIENhcGUxEjAQBgNVBAcTCUNhcGUgVG93bjEa\
MBgGA1UEChMRVGhhd3RlIENvbnN1bHRpbmcxKDAmBgNVBAsTH0NlcnRpZmljYXRp\
b24gU2VydmljZXMgRGl2aXNpb24xJDAiBgNVBAMTG1RoYXd0ZSBQZXJzb25hbCBG\
cmVlbWFpbCBDQTErMCkGCSqGSIb3DQEJARYccGVyc29uYWwtZnJlZW1haWxAdGhh\
d3RlLmNvbTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA1GnX1LCUZFtx6UfY\
DFG26nKRsIRefS0Nj3sS34UldSh0OkIsYyeflXtL734Zhx2G6qPduc6WZBrCFG5E\
rHzmj+hND3EfQDimAKOHePb5lIZererAXnbr2RSjXW56fAylS1V/Bhkpf56aJtVq\
uzgkCGqYx7Hao5iR/Xnb5VrEHLkCAwEAAaMTMBEwDwYDVR0TAQH/BAUwAwEB/zAN\
BgkqhkiG9w0BAQQFAAOBgQDH7JJ+Tvj1lqVnYiqk8E0RYNBvjWBYYawmu1I1XAjP\
MPuoSpaKH2JCI4wXD/S6ZJwXrEcp352YXtJsYHFcoqzceePnbgBHH7UNKOgCneSa\
/RP0ptl8sfjcXyMmCZGAc9AUG95DqYMl8uacLxXK/qarigd1iwzdUYRr5PjRznei\
gQ==\
-----END CERTIFICATE-----\
\
America Online Root Certification Authority 1\
=============================================\
\
MD5 Fingerprint=14:F1:08:AD:9D:FA:64:E2:89:E7:1C:CF:A8:AD:7D:5E\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=America Online Inc., CN=America Online Root Certification Authority 1\
        Validity\
            Not Before: May 28 06:00:00 2002 GMT\
            Not After : Nov 19 20:43:00 2037 GMT\
        Subject: C=US, O=America Online Inc., CN=America Online Root Certification Authority 1\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:a8:2f:e8:a4:69:06:03:47:c3:e9:2a:98:ff:19:\
                    a2:70:9a:c6:50:b2:7e:a5:df:68:4d:1b:7c:0f:b6:\
                    97:68:7d:2d:a6:8b:97:e9:64:86:c9:a3:ef:a0:86:\
                    bf:60:65:9c:4b:54:88:c2:48:c5:4a:39:bf:14:e3:\
                    59:55:e5:19:b4:74:c8:b4:05:39:5c:16:a5:e2:95:\
                    05:e0:12:ae:59:8b:a2:33:68:58:1c:a6:d4:15:b7:\
                    d8:9f:d7:dc:71:ab:7e:9a:bf:9b:8e:33:0f:22:fd:\
                    1f:2e:e7:07:36:ef:62:39:c5:dd:cb:ba:25:14:23:\
                    de:0c:c6:3d:3c:ce:82:08:e6:66:3e:da:51:3b:16:\
                    3a:a3:05:7f:a0:dc:87:d5:9c:fc:72:a9:a0:7d:78:\
                    e4:b7:31:55:1e:65:bb:d4:61:b0:21:60:ed:10:32:\
                    72:c5:92:25:1e:f8:90:4a:18:78:47:df:7e:30:37:\
                    3e:50:1b:db:1c:d3:6b:9a:86:53:07:b0:ef:ac:06:\
                    78:f8:84:99:fe:21:8d:4c:80:b6:0c:82:f6:66:70:\
                    79:1a:d3:4f:a3:cf:f1:cf:46:b0:4b:0f:3e:dd:88:\
                    62:b8:8c:a9:09:28:3b:7a:c7:97:e1:1e:e5:f4:9f:\
                    c0:c0:ae:24:a0:c8:a1:d9:0f:d6:7b:26:82:69:32:\
                    3d:a7\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                00:AD:D9:A3:F6:79:F6:6E:74:A9:7F:33:3D:81:17:D7:4C:CF:33:DE\
            X509v3 Authority Key Identifier: \
                keyid:00:AD:D9:A3:F6:79:F6:6E:74:A9:7F:33:3D:81:17:D7:4C:CF:33:DE\
\
            X509v3 Key Usage: critical\
                Digital Signature, Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        7c:8a:d1:1f:18:37:82:e0:b8:b0:a3:ed:56:95:c8:62:61:9c:\
        05:a2:cd:c2:62:26:61:cd:10:16:d7:cc:b4:65:34:d0:11:8a:\
        ad:a8:a9:05:66:ef:74:f3:6d:5f:9d:99:af:f6:8b:fb:eb:52:\
        b2:05:98:a2:6f:2a:c5:54:bd:25:bd:5f:ae:c8:86:ea:46:2c:\
        c1:b3:bd:c1:e9:49:70:18:16:97:08:13:8c:20:e0:1b:2e:3a:\
        47:cb:1e:e4:00:30:95:5b:f4:45:a3:c0:1a:b0:01:4e:ab:bd:\
        c0:23:6e:63:3f:80:4a:c5:07:ed:dc:e2:6f:c7:c1:62:f1:e3:\
        72:d6:04:c8:74:67:0b:fa:88:ab:a1:01:c8:6f:f0:14:af:d2:\
        99:cd:51:93:7e:ed:2e:38:c7:bd:ce:46:50:3d:72:e3:79:25:\
        9d:9b:88:2b:10:20:dd:a5:b8:32:9f:8d:e0:29:df:21:74:86:\
        82:db:2f:82:30:c6:c7:35:86:b3:f9:96:5f:46:db:0c:45:fd:\
        f3:50:c3:6f:c6:c3:48:ad:46:a6:e1:27:47:0a:1d:0e:9b:b6:\
        c2:77:7f:63:f2:e0:7d:1a:be:fc:e0:df:d7:c7:a7:6c:b0:f9:\
        ae:ba:3c:fd:74:b4:11:e8:58:0d:80:bc:d3:a8:80:3a:99:ed:\
        75:cc:46:7b\
-----BEGIN CERTIFICATE-----\
MIIDpDCCAoygAwIBAgIBATANBgkqhkiG9w0BAQUFADBjMQswCQYDVQQGEwJVUzEc\
MBoGA1UEChMTQW1lcmljYSBPbmxpbmUgSW5jLjE2MDQGA1UEAxMtQW1lcmljYSBP\
bmxpbmUgUm9vdCBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAxMB4XDTAyMDUyODA2\
MDAwMFoXDTM3MTExOTIwNDMwMFowYzELMAkGA1UEBhMCVVMxHDAaBgNVBAoTE0Ft\
ZXJpY2EgT25saW5lIEluYy4xNjA0BgNVBAMTLUFtZXJpY2EgT25saW5lIFJvb3Qg\
Q2VydGlmaWNhdGlvbiBBdXRob3JpdHkgMTCCASIwDQYJKoZIhvcNAQEBBQADggEP\
ADCCAQoCggEBAKgv6KRpBgNHw+kqmP8ZonCaxlCyfqXfaE0bfA+2l2h9LaaLl+lk\
hsmj76CGv2BlnEtUiMJIxUo5vxTjWVXlGbR0yLQFOVwWpeKVBeASrlmLojNoWBym\
1BW32J/X3HGrfpq/m44zDyL9Hy7nBzbvYjnF3cu6JRQj3gzGPTzOggjmZj7aUTsW\
OqMFf6Dch9Wc/HKpoH145LcxVR5lu9RhsCFg7RAycsWSJR74kEoYeEfffjA3PlAb\
2xzTa5qGUwew76wGePiEmf4hjUyAtgyC9mZweRrTT6PP8c9GsEsPPt2IYriMqQko\
O3rHl+Ee5fSfwMCuJKDIodkP1nsmgmkyPacCAwEAAaNjMGEwDwYDVR0TAQH/BAUw\
AwEB/zAdBgNVHQ4EFgQUAK3Zo/Z59m50qX8zPYEX10zPM94wHwYDVR0jBBgwFoAU\
AK3Zo/Z59m50qX8zPYEX10zPM94wDgYDVR0PAQH/BAQDAgGGMA0GCSqGSIb3DQEB\
BQUAA4IBAQB8itEfGDeC4Liwo+1WlchiYZwFos3CYiZhzRAW18y0ZTTQEYqtqKkF\
Zu90821fnZmv9ov761KyBZiibyrFVL0lvV+uyIbqRizBs73B6UlwGBaXCBOMIOAb\
LjpHyx7kADCVW/RFo8AasAFOq73AI25jP4BKxQft3OJvx8Fi8eNy1gTIdGcL+oir\
oQHIb/AUr9KZzVGTfu0uOMe9zkZQPXLjeSWdm4grECDdpbgyn43gKd8hdIaC2y+C\
MMbHNYaz+ZZfRtsMRf3zUMNvxsNIrUam4SdHCh0Om7bCd39j8uB9Gr784N/Xx6ds\
sPmuujz9dLQR6FgNgLzTqIA6me11zEZ7\
-----END CERTIFICATE-----\
\
America Online Root Certification Authority 2\
=============================================\
\
MD5 Fingerprint=D6:ED:3C:CA:E2:66:0F:AF:10:43:0D:77:9B:04:09:BF\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=America Online Inc., CN=America Online Root Certification Authority 2\
        Validity\
            Not Before: May 28 06:00:00 2002 GMT\
            Not After : Sep 29 14:08:00 2037 GMT\
        Subject: C=US, O=America Online Inc., CN=America Online Root Certification Authority 2\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (4096 bit)\
                Modulus (4096 bit):\
                    00:cc:41:45:1d:e9:3d:4d:10:f6:8c:b1:41:c9:e0:\
                    5e:cb:0d:b7:bf:47:73:d3:f0:55:4d:dd:c6:0c:fa:\
                    b1:66:05:6a:cd:78:b4:dc:02:db:4e:81:f3:d7:a7:\
                    7c:71:bc:75:63:a0:5d:e3:07:0c:48:ec:25:c4:03:\
                    20:f4:ff:0e:3b:12:ff:9b:8d:e1:c6:d5:1b:b4:6d:\
                    22:e3:b1:db:7f:21:64:af:86:bc:57:22:2a:d6:47:\
                    81:57:44:82:56:53:bd:86:14:01:0b:fc:7f:74:a4:\
                    5a:ae:f1:ba:11:b5:9b:58:5a:80:b4:37:78:09:33:\
                    7c:32:47:03:5c:c4:a5:83:48:f4:57:56:6e:81:36:\
                    27:18:4f:ec:9b:28:c2:d4:b4:d7:7c:0c:3e:0c:2b:\
                    df:ca:04:d7:c6:8e:ea:58:4e:a8:a4:a5:18:1c:6c:\
                    45:98:a3:41:d1:2d:d2:c7:6d:8d:19:f1:ad:79:b7:\
                    81:3f:bd:06:82:27:2d:10:58:05:b5:78:05:b9:2f:\
                    db:0c:6b:90:90:7e:14:59:38:bb:94:24:13:e5:d1:\
                    9d:14:df:d3:82:4d:46:f0:80:39:52:32:0f:e3:84:\
                    b2:7a:43:f2:5e:de:5f:3f:1d:dd:e3:b2:1b:a0:a1:\
                    2a:23:03:6e:2e:01:15:87:5c:a6:75:75:c7:97:61:\
                    be:de:86:dc:d4:48:db:bd:2a:bf:4a:55:da:e8:7d:\
                    50:fb:b4:80:17:b8:94:bf:01:3d:ea:da:ba:7c:e0:\
                    58:67:17:b9:58:e0:88:86:46:67:6c:9d:10:47:58:\
                    32:d0:35:7c:79:2a:90:a2:5a:10:11:23:35:ad:2f:\
                    cc:e4:4a:5b:a7:c8:27:f2:83:de:5e:bb:5e:77:e7:\
                    e8:a5:6e:63:c2:0d:5d:61:d0:8c:d2:6c:5a:21:0e:\
                    ca:28:a3:ce:2a:e9:95:c7:48:cf:96:6f:1d:92:25:\
                    c8:c6:c6:c1:c1:0c:05:ac:26:c4:d2:75:d2:e1:2a:\
                    67:c0:3d:5b:a5:9a:eb:cf:7b:1a:a8:9d:14:45:e5:\
                    0f:a0:9a:65:de:2f:28:bd:ce:6f:94:66:83:48:29:\
                    d8:ea:65:8c:af:93:d9:64:9f:55:57:26:bf:6f:cb:\
                    37:31:99:a3:60:bb:1c:ad:89:34:32:62:b8:43:21:\
                    06:72:0c:a1:5c:6d:46:c5:fa:29:cf:30:de:89:dc:\
                    71:5b:dd:b6:37:3e:df:50:f5:b8:07:25:26:e5:bc:\
                    b5:fe:3c:02:b3:b7:f8:be:43:c1:87:11:94:9e:23:\
                    6c:17:8a:b8:8a:27:0c:54:47:f0:a9:b3:c0:80:8c:\
                    a0:27:eb:1d:19:e3:07:8e:77:70:ca:2b:f4:7d:76:\
                    e0:78:67\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                4D:45:C1:68:38:BB:73:A9:69:A1:20:E7:ED:F5:22:A1:23:14:D7:9E\
            X509v3 Authority Key Identifier: \
                keyid:4D:45:C1:68:38:BB:73:A9:69:A1:20:E7:ED:F5:22:A1:23:14:D7:9E\
\
            X509v3 Key Usage: critical\
                Digital Signature, Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        67:6b:06:b9:5f:45:3b:2a:4b:33:b3:e6:1b:6b:59:4e:22:cc:\
        b9:b7:a4:25:c9:a7:c4:f0:54:96:0b:64:f3:b1:58:4f:5e:51:\
        fc:b2:97:7b:27:65:c2:e5:ca:e7:0d:0c:25:7b:62:e3:fa:9f:\
        b4:87:b7:45:46:af:83:a5:97:48:8c:a5:bd:f1:16:2b:9b:76:\
        2c:7a:35:60:6c:11:80:97:cc:a9:92:52:e6:2b:e6:69:ed:a9:\
        f8:36:2d:2c:77:bf:61:48:d1:63:0b:b9:5b:52:ed:18:b0:43:\
        42:22:a6:b1:77:ae:de:69:c5:cd:c7:1c:a1:b1:a5:1c:10:fb:\
        18:be:1a:70:dd:c1:92:4b:be:29:5a:9d:3f:35:be:e5:7d:51:\
        f8:55:e0:25:75:23:87:1e:5c:dc:ba:9d:b0:ac:b3:69:db:17:\
        83:c9:f7:de:0c:bc:08:dc:91:9e:a8:d0:d7:15:37:73:a5:35:\
        b8:fc:7e:c5:44:40:06:c3:eb:f8:22:80:5c:47:ce:02:e3:11:\
        9f:44:ff:fd:9a:32:cc:7d:64:51:0e:eb:57:26:76:3a:e3:1e:\
        22:3c:c2:a6:36:dd:19:ef:a7:fc:12:f3:26:c0:59:31:85:4c:\
        9c:d8:cf:df:a4:cc:cc:29:93:ff:94:6d:76:5c:13:08:97:f2:\
        ed:a5:0b:4d:dd:e8:c9:68:0e:66:d3:00:0e:33:12:5b:bc:95:\
        e5:32:90:a8:b3:c6:6c:83:ad:77:ee:8b:7e:7e:b1:a9:ab:d3:\
        e1:f1:b6:c0:b1:ea:88:c0:e7:d3:90:e9:28:92:94:7b:68:7b:\
        97:2a:0a:67:2d:85:02:38:10:e4:03:61:d4:da:25:36:c7:08:\
        58:2d:a1:a7:51:af:30:0a:49:f5:a6:69:87:07:2d:44:46:76:\
        8e:2a:e5:9a:3b:d7:18:a2:fc:9c:38:10:cc:c6:3b:d2:b5:17:\
        3a:6f:fd:ae:25:bd:f5:72:59:64:b1:74:2a:38:5f:18:4c:df:\
        cf:71:04:5a:36:d4:bf:2f:99:9c:e8:d9:ba:b1:95:e6:02:4b:\
        21:a1:5b:d5:c1:4f:8f:ae:69:6d:53:db:01:93:b5:5c:1e:18:\
        dd:64:5a:ca:18:28:3e:63:04:11:fd:1c:8d:00:0f:b8:37:df:\
        67:8a:9d:66:a9:02:6a:91:ff:13:ca:2f:5d:83:bc:87:93:6c:\
        dc:24:51:16:04:25:66:fa:b3:d9:c2:ba:29:be:9a:48:38:82:\
        99:f4:bf:3b:4a:31:19:f9:bf:8e:21:33:14:ca:4f:54:5f:fb:\
        ce:fb:8f:71:7f:fd:5e:19:a0:0f:4b:91:b8:c4:54:bc:06:b0:\
        45:8f:26:91:a2:8e:fe:a9\
-----BEGIN CERTIFICATE-----\
MIIFpDCCA4ygAwIBAgIBATANBgkqhkiG9w0BAQUFADBjMQswCQYDVQQGEwJVUzEc\
MBoGA1UEChMTQW1lcmljYSBPbmxpbmUgSW5jLjE2MDQGA1UEAxMtQW1lcmljYSBP\
bmxpbmUgUm9vdCBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eSAyMB4XDTAyMDUyODA2\
MDAwMFoXDTM3MDkyOTE0MDgwMFowYzELMAkGA1UEBhMCVVMxHDAaBgNVBAoTE0Ft\
ZXJpY2EgT25saW5lIEluYy4xNjA0BgNVBAMTLUFtZXJpY2EgT25saW5lIFJvb3Qg\
Q2VydGlmaWNhdGlvbiBBdXRob3JpdHkgMjCCAiIwDQYJKoZIhvcNAQEBBQADggIP\
ADCCAgoCggIBAMxBRR3pPU0Q9oyxQcngXssNt79Hc9PwVU3dxgz6sWYFas14tNwC\
206B89enfHG8dWOgXeMHDEjsJcQDIPT/DjsS/5uN4cbVG7RtIuOx238hZK+GvFci\
KtZHgVdEglZTvYYUAQv8f3SkWq7xuhG1m1hagLQ3eAkzfDJHA1zEpYNI9FdWboE2\
JxhP7JsowtS013wMPgwr38oE18aO6lhOqKSlGBxsRZijQdEt0sdtjRnxrXm3gT+9\
BoInLRBYBbV4Bbkv2wxrkJB+FFk4u5QkE+XRnRTf04JNRvCAOVIyD+OEsnpD8l7e\
Xz8d3eOyG6ChKiMDbi4BFYdcpnV1x5dhvt6G3NRI270qv0pV2uh9UPu0gBe4lL8B\
PeraunzgWGcXuVjgiIZGZ2ydEEdYMtA1fHkqkKJaEBEjNa0vzORKW6fIJ/KD3l67\
Xnfn6KVuY8INXWHQjNJsWiEOyiijzirplcdIz5ZvHZIlyMbGwcEMBawmxNJ10uEq\
Z8A9W6Wa6897GqidFEXlD6CaZd4vKL3Ob5Rmg0gp2OpljK+T2WSfVVcmv2/LNzGZ\
o2C7HK2JNDJiuEMhBnIMoVxtRsX6Kc8w3onccVvdtjc+31D1uAclJuW8tf48ArO3\
+L5DwYcRlJ4jbBeKuIonDFRH8KmzwICMoCfrHRnjB453cMor9H124HhnAgMBAAGj\
YzBhMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFE1FwWg4u3OpaaEg5+31IqEj\
FNeeMB8GA1UdIwQYMBaAFE1FwWg4u3OpaaEg5+31IqEjFNeeMA4GA1UdDwEB/wQE\
AwIBhjANBgkqhkiG9w0BAQUFAAOCAgEAZ2sGuV9FOypLM7PmG2tZTiLMubekJcmn\
xPBUlgtk87FYT15R/LKXeydlwuXK5w0MJXti4/qftIe3RUavg6WXSIylvfEWK5t2\
LHo1YGwRgJfMqZJS5ivmae2p+DYtLHe/YUjRYwu5W1LtGLBDQiKmsXeu3mnFzccc\
obGlHBD7GL4acN3Bkku+KVqdPzW+5X1R+FXgJXUjhx5c3LqdsKyzadsXg8n33gy8\
CNyRnqjQ1xU3c6U1uPx+xURABsPr+CKAXEfOAuMRn0T//ZoyzH1kUQ7rVyZ2OuMe\
IjzCpjbdGe+n/BLzJsBZMYVMnNjP36TMzCmT/5RtdlwTCJfy7aULTd3oyWgOZtMA\
DjMSW7yV5TKQqLPGbIOtd+6Lfn6xqavT4fG2wLHqiMDn05DpKJKUe2h7lyoKZy2F\
AjgQ5ANh1NolNscIWC2hp1GvMApJ9aZphwctREZ2jirlmjvXGKL8nDgQzMY70rUX\
Om/9riW99XJZZLF0KjhfGEzfz3EEWjbUvy+ZnOjZurGV5gJLIaFb1cFPj65pbVPb\
AZO1XB4Y3WRayhgoPmMEEf0cjQAPuDffZ4qdZqkCapH/E8ovXYO8h5Ns3CRRFgQl\
Zvqz2cK6Kb6aSDiCmfS/O0oxGfm/jiEzFMpPVF/7zvuPcX/9XhmgD0uRuMRUvAaw\
RY8mkaKO/qk=\
-----END CERTIFICATE-----\
\
Visa eCommerce Root\
===================\
\
MD5 Fingerprint=FC:11:B8:D8:08:93:30:00:6D:23:F9:7E:EB:52:1E:02\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            13:86:35:4d:1d:3f:06:f2:c1:f9:65:05:d5:90:1c:62\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=VISA, OU=Visa International Service Association, CN=Visa eCommerce Root\
        Validity\
            Not Before: Jun 26 02:18:36 2002 GMT\
            Not After : Jun 24 00:16:12 2022 GMT\
        Subject: C=US, O=VISA, OU=Visa International Service Association, CN=Visa eCommerce Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:af:57:de:56:1e:6e:a1:da:60:b1:94:27:cb:17:\
                    db:07:3f:80:85:4f:c8:9c:b6:d0:f4:6f:4f:cf:99:\
                    d8:e1:db:c2:48:5c:3a:ac:39:33:c7:1f:6a:8b:26:\
                    3d:2b:35:f5:48:b1:91:c1:02:4e:04:96:91:7b:b0:\
                    33:f0:b1:14:4e:11:6f:b5:40:af:1b:45:a5:4a:ef:\
                    7e:b6:ac:f2:a0:1f:58:3f:12:46:60:3c:8d:a1:e0:\
                    7d:cf:57:3e:33:1e:fb:47:f1:aa:15:97:07:55:66:\
                    a5:b5:2d:2e:d8:80:59:b2:a7:0d:b7:46:ec:21:63:\
                    ff:35:ab:a5:02:cf:2a:f4:4c:fe:7b:f5:94:5d:84:\
                    4d:a8:f2:60:8f:db:0e:25:3c:9f:73:71:cf:94:df:\
                    4a:ea:db:df:72:38:8c:f3:96:bd:f1:17:bc:d2:ba:\
                    3b:45:5a:c6:a7:f6:c6:17:8b:01:9d:fc:19:a8:2a:\
                    83:16:b8:3a:48:fe:4e:3e:a0:ab:06:19:e9:53:f3:\
                    80:13:07:ed:2d:bf:3f:0a:3c:55:20:39:2c:2c:00:\
                    69:74:95:4a:bc:20:b2:a9:79:e5:18:89:91:a8:dc:\
                    1c:4d:ef:bb:7e:37:0b:5d:fe:39:a5:88:52:8c:00:\
                    6c:ec:18:7c:41:bd:f6:8b:75:77:ba:60:9d:84:e7:\
                    fe:2d\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Subject Key Identifier: \
                15:38:83:0F:3F:2C:3F:70:33:1E:CD:46:FE:07:8C:20:E0:D7:C3:B7\
    Signature Algorithm: sha1WithRSAEncryption\
        5f:f1:41:7d:7c:5c:08:b9:2b:e0:d5:92:47:fa:67:5c:a5:13:\
        c3:03:21:9b:2b:4c:89:46:cf:59:4d:c9:fe:a5:40:b6:63:cd:\
        dd:71:28:95:67:11:cc:24:ac:d3:44:6c:71:ae:01:20:6b:03:\
        a2:8f:18:b7:29:3a:7d:e5:16:60:53:78:3c:c0:af:15:83:f7:\
        8f:52:33:24:bd:64:93:97:ee:8b:f7:db:18:a8:6d:71:b3:f7:\
        2c:17:d0:74:25:69:f7:fe:6b:3c:94:be:4d:4b:41:8c:4e:e2:\
        73:d0:e3:90:22:73:43:cd:f3:ef:ea:73:ce:45:8a:b0:a6:49:\
        ff:4c:7d:9d:71:88:c4:76:1d:90:5b:1d:ee:fd:cc:f7:ee:fd:\
        60:a5:b1:7a:16:71:d1:16:d0:7c:12:3c:6c:69:97:db:ae:5f:\
        39:9a:70:2f:05:3c:19:46:04:99:20:36:d0:60:6e:61:06:bb:\
        16:42:8c:70:f7:30:fb:e0:db:66:a3:00:01:bd:e6:2c:da:91:\
        5f:a0:46:8b:4d:6a:9c:3d:3d:dd:05:46:fe:76:bf:a0:0a:3c:\
        e4:00:e6:27:b7:ff:84:2d:de:ba:22:27:96:10:71:eb:22:ed:\
        df:df:33:9c:cf:e3:ad:ae:8e:d4:8e:e6:4f:51:af:16:92:e0:\
        5c:f6:07:0f\
-----BEGIN CERTIFICATE-----\
MIIDojCCAoqgAwIBAgIQE4Y1TR0/BvLB+WUF1ZAcYjANBgkqhkiG9w0BAQUFADBr\
MQswCQYDVQQGEwJVUzENMAsGA1UEChMEVklTQTEvMC0GA1UECxMmVmlzYSBJbnRl\
cm5hdGlvbmFsIFNlcnZpY2UgQXNzb2NpYXRpb24xHDAaBgNVBAMTE1Zpc2EgZUNv\
bW1lcmNlIFJvb3QwHhcNMDIwNjI2MDIxODM2WhcNMjIwNjI0MDAxNjEyWjBrMQsw\
CQYDVQQGEwJVUzENMAsGA1UEChMEVklTQTEvMC0GA1UECxMmVmlzYSBJbnRlcm5h\
dGlvbmFsIFNlcnZpY2UgQXNzb2NpYXRpb24xHDAaBgNVBAMTE1Zpc2EgZUNvbW1l\
cmNlIFJvb3QwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvV95WHm6h\
2mCxlCfLF9sHP4CFT8icttD0b0/Pmdjh28JIXDqsOTPHH2qLJj0rNfVIsZHBAk4E\
lpF7sDPwsRROEW+1QK8bRaVK7362rPKgH1g/EkZgPI2h4H3PVz4zHvtH8aoVlwdV\
ZqW1LS7YgFmypw23RuwhY/81q6UCzyr0TP579ZRdhE2o8mCP2w4lPJ9zcc+U30rq\
299yOIzzlr3xF7zSujtFWsan9sYXiwGd/BmoKoMWuDpI/k4+oKsGGelT84ATB+0t\
vz8KPFUgOSwsAGl0lUq8ILKpeeUYiZGo3BxN77t+Nwtd/jmliFKMAGzsGHxBvfaL\
dXe6YJ2E5/4tAgMBAAGjQjBAMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQD\
AgEGMB0GA1UdDgQWBBQVOIMPPyw/cDMezUb+B4wg4NfDtzANBgkqhkiG9w0BAQUF\
AAOCAQEAX/FBfXxcCLkr4NWSR/pnXKUTwwMhmytMiUbPWU3J/qVAtmPN3XEolWcR\
zCSs00Rsca4BIGsDoo8Ytyk6feUWYFN4PMCvFYP3j1IzJL1kk5fui/fbGKhtcbP3\
LBfQdCVp9/5rPJS+TUtBjE7ic9DjkCJzQ83z7+pzzkWKsKZJ/0x9nXGIxHYdkFsd\
7v3M9+79YKWxehZx0RbQfBI8bGmX265fOZpwLwU8GUYEmSA20GBuYQa7FkKMcPcw\
++DbZqMAAb3mLNqRX6BGi01qnD093QVG/na/oAo85ADmJ7f/hC3euiInlhBx6yLt\
398znM/jra6O1I7mT1GvFpLgXPYHDw==\
-----END CERTIFICATE-----\
\
TC TrustCenter, Germany, Class 2 CA\
===================================\
\
MD5 Fingerprint=B8:16:33:4C:4C:4C:F2:D8:D3:4D:06:B4:A6:5B:40:03\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1002 (0x3ea)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=DE, ST=Hamburg, L=Hamburg, O=TC TrustCenter for Security in Data Networks GmbH, OU=TC TrustCenter Class 2 CA/emailAddress=certificate@trustcenter.de\
        Validity\
            Not Before: Mar  9 11:59:59 1998 GMT\
            Not After : Jan  1 11:59:59 2011 GMT\
        Subject: C=DE, ST=Hamburg, L=Hamburg, O=TC TrustCenter for Security in Data Networks GmbH, OU=TC TrustCenter Class 2 CA/emailAddress=certificate@trustcenter.de\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:da:38:e8:ed:32:00:29:71:83:01:0d:bf:8c:01:\
                    dc:da:c6:ad:39:a4:a9:8a:2f:d5:8b:5c:68:5f:50:\
                    c6:62:f5:66:bd:ca:91:22:ec:aa:1d:51:d7:3d:b3:\
                    51:b2:83:4e:5d:cb:49:b0:f0:4c:55:e5:6b:2d:c7:\
                    85:0b:30:1c:92:4e:82:d4:ca:02:ed:f7:6f:be:dc:\
                    e0:e3:14:b8:05:53:f2:9a:f4:56:8b:5a:9e:85:93:\
                    d1:b4:82:56:ae:4d:bb:a8:4b:57:16:bc:fe:f8:58:\
                    9e:f8:29:8d:b0:7b:cd:78:c9:4f:ac:8b:67:0c:f1:\
                    9c:fb:fc:57:9b:57:5c:4f:0d\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Key Usage: critical\
                Digital Signature, Certificate Sign, CRL Sign\
            Netscape CA Policy Url: \
                http://www.trustcenter.de/guidelines\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
    Signature Algorithm: md5WithRSAEncryption\
        84:52:fb:28:df:ff:1f:75:01:bc:01:be:04:56:97:6a:74:42:\
        24:31:83:f9:46:b1:06:8a:89:cf:96:2c:33:bf:8c:b5:5f:7a:\
        72:a1:85:06:ce:86:f8:05:8e:e8:f9:25:ca:da:83:8c:06:ac:\
        eb:36:6d:85:91:34:04:36:f4:42:f0:f8:79:2e:0a:48:5c:ab:\
        cc:51:4f:78:76:a0:d9:ac:19:bd:2a:d1:69:04:28:91:ca:36:\
        10:27:80:57:5b:d2:5c:f5:c2:5b:ab:64:81:63:74:51:f4:97:\
        bf:cd:12:28:f7:4d:66:7f:a7:f0:1c:01:26:78:b2:66:47:70:\
        51:64\
-----BEGIN CERTIFICATE-----\
MIIDXDCCAsWgAwIBAgICA+owDQYJKoZIhvcNAQEEBQAwgbwxCzAJBgNVBAYTAkRF\
MRAwDgYDVQQIEwdIYW1idXJnMRAwDgYDVQQHEwdIYW1idXJnMTowOAYDVQQKEzFU\
QyBUcnVzdENlbnRlciBmb3IgU2VjdXJpdHkgaW4gRGF0YSBOZXR3b3JrcyBHbWJI\
MSIwIAYDVQQLExlUQyBUcnVzdENlbnRlciBDbGFzcyAyIENBMSkwJwYJKoZIhvcN\
AQkBFhpjZXJ0aWZpY2F0ZUB0cnVzdGNlbnRlci5kZTAeFw05ODAzMDkxMTU5NTla\
Fw0xMTAxMDExMTU5NTlaMIG8MQswCQYDVQQGEwJERTEQMA4GA1UECBMHSGFtYnVy\
ZzEQMA4GA1UEBxMHSGFtYnVyZzE6MDgGA1UEChMxVEMgVHJ1c3RDZW50ZXIgZm9y\
IFNlY3VyaXR5IGluIERhdGEgTmV0d29ya3MgR21iSDEiMCAGA1UECxMZVEMgVHJ1\
c3RDZW50ZXIgQ2xhc3MgMiBDQTEpMCcGCSqGSIb3DQEJARYaY2VydGlmaWNhdGVA\
dHJ1c3RjZW50ZXIuZGUwgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBANo46O0y\
AClxgwENv4wB3NrGrTmkqYov1YtcaF9QxmL1Zr3KkSLsqh1R1z2zUbKDTl3LSbDw\
TFXlay3HhQswHJJOgtTKAu33b77c4OMUuAVT8pr0VotanoWT0bSCVq5Nu6hLVxa8\
/vhYnvgpjbB7zXjJT6yLZwzxnPv8V5tXXE8NAgMBAAGjazBpMA8GA1UdEwEB/wQF\
MAMBAf8wDgYDVR0PAQH/BAQDAgGGMDMGCWCGSAGG+EIBCAQmFiRodHRwOi8vd3d3\
LnRydXN0Y2VudGVyLmRlL2d1aWRlbGluZXMwEQYJYIZIAYb4QgEBBAQDAgAHMA0G\
CSqGSIb3DQEBBAUAA4GBAIRS+yjf/x91AbwBvgRWl2p0QiQxg/lGsQaKic+WLDO/\
jLVfenKhhQbOhvgFjuj5Jcrag4wGrOs2bYWRNAQ29ELw+HkuCkhcq8xRT3h2oNms\
Gb0q0WkEKJHKNhAngFdb0lz1wlurZIFjdFH0l7/NEij3TWZ/p/AcASZ4smZHcFFk\
-----END CERTIFICATE-----\
\
TC TrustCenter, Germany, Class 3 CA\
===================================\
\
MD5 Fingerprint=5F:94:4A:73:22:B8:F7:D1:31:EC:59:39:F7:8E:FE:6E\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1003 (0x3eb)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=DE, ST=Hamburg, L=Hamburg, O=TC TrustCenter for Security in Data Networks GmbH, OU=TC TrustCenter Class 3 CA/emailAddress=certificate@trustcenter.de\
        Validity\
            Not Before: Mar  9 11:59:59 1998 GMT\
            Not After : Jan  1 11:59:59 2011 GMT\
        Subject: C=DE, ST=Hamburg, L=Hamburg, O=TC TrustCenter for Security in Data Networks GmbH, OU=TC TrustCenter Class 3 CA/emailAddress=certificate@trustcenter.de\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:b6:b4:c1:35:05:2e:0d:8d:ec:a0:40:6a:1c:0e:\
                    27:a6:50:92:6b:50:1b:07:de:2e:e7:76:cc:e0:da:\
                    fc:84:a8:5e:8c:63:6a:2b:4d:d9:4e:02:76:11:c1:\
                    0b:f2:8d:79:ca:00:b6:f1:b0:0e:d7:fb:a4:17:3d:\
                    af:ab:69:7a:96:27:bf:af:33:a1:9a:2a:59:aa:c4:\
                    b5:37:08:f2:12:a5:31:b6:43:f5:32:96:71:28:28:\
                    ab:8d:28:86:df:bb:ee:e3:0c:7d:30:d6:c3:52:ab:\
                    8f:5d:27:9c:6b:c0:a3:e7:05:6b:57:49:44:b3:6e:\
                    ea:64:cf:d2:8e:7a:50:77:77\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Key Usage: critical\
                Digital Signature, Certificate Sign, CRL Sign\
            Netscape CA Policy Url: \
                http://www.trustcenter.de/guidelines\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
    Signature Algorithm: md5WithRSAEncryption\
        16:3d:c6:cd:c1:bb:85:71:85:46:9f:3e:20:8f:51:28:99:ec:\
        2d:45:21:63:23:5b:04:bb:4c:90:b8:88:92:04:4d:bd:7d:01:\
        a3:3f:f6:ec:ce:f1:de:fe:7d:e5:e1:3e:bb:c6:ab:5e:0b:dd:\
        3d:96:c4:cb:a9:d4:f9:26:e6:06:4e:9e:0c:a5:7a:ba:6e:c3:\
        7c:82:19:d1:c7:b1:b1:c3:db:0d:8e:9b:40:7c:37:0b:f1:5d:\
        e8:fd:1f:90:88:a5:0e:4e:37:64:21:a8:4e:8d:b4:9f:f1:de:\
        48:ad:d5:56:18:52:29:8b:47:34:12:09:d4:bb:92:35:ef:0f:\
        db:34\
-----BEGIN CERTIFICATE-----\
MIIDXDCCAsWgAwIBAgICA+swDQYJKoZIhvcNAQEEBQAwgbwxCzAJBgNVBAYTAkRF\
MRAwDgYDVQQIEwdIYW1idXJnMRAwDgYDVQQHEwdIYW1idXJnMTowOAYDVQQKEzFU\
QyBUcnVzdENlbnRlciBmb3IgU2VjdXJpdHkgaW4gRGF0YSBOZXR3b3JrcyBHbWJI\
MSIwIAYDVQQLExlUQyBUcnVzdENlbnRlciBDbGFzcyAzIENBMSkwJwYJKoZIhvcN\
AQkBFhpjZXJ0aWZpY2F0ZUB0cnVzdGNlbnRlci5kZTAeFw05ODAzMDkxMTU5NTla\
Fw0xMTAxMDExMTU5NTlaMIG8MQswCQYDVQQGEwJERTEQMA4GA1UECBMHSGFtYnVy\
ZzEQMA4GA1UEBxMHSGFtYnVyZzE6MDgGA1UEChMxVEMgVHJ1c3RDZW50ZXIgZm9y\
IFNlY3VyaXR5IGluIERhdGEgTmV0d29ya3MgR21iSDEiMCAGA1UECxMZVEMgVHJ1\
c3RDZW50ZXIgQ2xhc3MgMyBDQTEpMCcGCSqGSIb3DQEJARYaY2VydGlmaWNhdGVA\
dHJ1c3RjZW50ZXIuZGUwgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBALa0wTUF\
Lg2N7KBAahwOJ6ZQkmtQGwfeLud2zODa/ISoXoxjaitN2U4CdhHBC/KNecoAtvGw\
Dtf7pBc9r6tpepYnv68zoZoqWarEtTcI8hKlMbZD9TKWcSgoq40oht+77uMMfTDW\
w1Krj10nnGvAo+cFa1dJRLNu6mTP0o56UHd3AgMBAAGjazBpMA8GA1UdEwEB/wQF\
MAMBAf8wDgYDVR0PAQH/BAQDAgGGMDMGCWCGSAGG+EIBCAQmFiRodHRwOi8vd3d3\
LnRydXN0Y2VudGVyLmRlL2d1aWRlbGluZXMwEQYJYIZIAYb4QgEBBAQDAgAHMA0G\
CSqGSIb3DQEBBAUAA4GBABY9xs3Bu4VxhUafPiCPUSiZ7C1FIWMjWwS7TJC4iJIE\
Tb19AaM/9uzO8d7+feXhPrvGq14L3T2WxMup1Pkm5gZOngylerpuw3yCGdHHsbHD\
2w2Om0B8NwvxXej9H5CIpQ5ON2QhqE6NtJ/x3kit1VYYUimLRzQSCdS7kjXvD9s0\
-----END CERTIFICATE-----\
\
Certum Root CA\
==============\
\
MD5 Fingerprint=2C:8F:9F:66:1D:18:90:B1:47:26:9D:8E:86:82:8C:A9\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 65568 (0x10020)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=PL, O=Unizeto Sp. z o.o., CN=Certum CA\
        Validity\
            Not Before: Jun 11 10:46:39 2002 GMT\
            Not After : Jun 11 10:46:39 2027 GMT\
        Subject: C=PL, O=Unizeto Sp. z o.o., CN=Certum CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:ce:b1:c1:2e:d3:4f:7c:cd:25:ce:18:3e:4f:c4:\
                    8c:6f:80:6a:73:c8:5b:51:f8:9b:d2:dc:bb:00:5c:\
                    b1:a0:fc:75:03:ee:81:f0:88:ee:23:52:e9:e6:15:\
                    33:8d:ac:2d:09:c5:76:f9:2b:39:80:89:e4:97:4b:\
                    90:a5:a8:78:f8:73:43:7b:a4:61:b0:d8:58:cc:e1:\
                    6c:66:7e:9c:f3:09:5e:55:63:84:d5:a8:ef:f3:b1:\
                    2e:30:68:b3:c4:3c:d8:ac:6e:8d:99:5a:90:4e:34:\
                    dc:36:9a:8f:81:88:50:b7:6d:96:42:09:f3:d7:95:\
                    83:0d:41:4b:b0:6a:6b:f8:fc:0f:7e:62:9f:67:c4:\
                    ed:26:5f:10:26:0f:08:4f:f0:a4:57:28:ce:8f:b8:\
                    ed:45:f6:6e:ee:25:5d:aa:6e:39:be:e4:93:2f:d9:\
                    47:a0:72:eb:fa:a6:5b:af:ca:53:3f:e2:0e:c6:96:\
                    56:11:6e:f7:e9:66:a9:26:d8:7f:95:53:ed:0a:85:\
                    88:ba:4f:29:a5:42:8c:5e:b6:fc:85:20:00:aa:68:\
                    0b:a1:1a:85:01:9c:c4:46:63:82:88:b6:22:b1:ee:\
                    fe:aa:46:59:7e:cf:35:2c:d5:b6:da:5d:f7:48:33:\
                    14:54:b6:eb:d9:6f:ce:cd:88:d6:ab:1b:da:96:3b:\
                    1d:59\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
    Signature Algorithm: sha1WithRSAEncryption\
        b8:8d:ce:ef:e7:14:ba:cf:ee:b0:44:92:6c:b4:39:3e:a2:84:\
        6e:ad:b8:21:77:d2:d4:77:82:87:e6:20:41:81:ee:e2:f8:11:\
        b7:63:d1:17:37:be:19:76:24:1c:04:1a:4c:eb:3d:aa:67:6f:\
        2d:d4:cd:fe:65:31:70:c5:1b:a6:02:0a:ba:60:7b:6d:58:c2:\
        9a:49:fe:63:32:0b:6b:e3:3a:c0:ac:ab:3b:b0:e8:d3:09:51:\
        8c:10:83:c6:34:e0:c5:2b:e0:1a:b6:60:14:27:6c:32:77:8c:\
        bc:b2:72:98:cf:cd:cc:3f:b9:c8:24:42:14:d6:57:fc:e6:26:\
        43:a9:1d:e5:80:90:ce:03:54:28:3e:f7:3f:d3:f8:4d:ed:6a:\
        0a:3a:93:13:9b:3b:14:23:13:63:9c:3f:d1:87:27:79:e5:4c:\
        51:e3:01:ad:85:5d:1a:3b:b1:d5:73:10:a4:d3:f2:bc:6e:64:\
        f5:5a:56:90:a8:c7:0e:4c:74:0f:2e:71:3b:f7:c8:47:f4:69:\
        6f:15:f2:11:5e:83:1e:9c:7c:52:ae:fd:02:da:12:a8:59:67:\
        18:db:bc:70:dd:9b:b1:69:ed:80:ce:89:40:48:6a:0e:35:ca:\
        29:66:15:21:94:2c:e8:60:2a:9b:85:4a:40:f3:6b:8a:24:ec:\
        06:16:2c:73\
-----BEGIN CERTIFICATE-----\
MIIDDDCCAfSgAwIBAgIDAQAgMA0GCSqGSIb3DQEBBQUAMD4xCzAJBgNVBAYTAlBM\
MRswGQYDVQQKExJVbml6ZXRvIFNwLiB6IG8uby4xEjAQBgNVBAMTCUNlcnR1bSBD\
QTAeFw0wMjA2MTExMDQ2MzlaFw0yNzA2MTExMDQ2MzlaMD4xCzAJBgNVBAYTAlBM\
MRswGQYDVQQKExJVbml6ZXRvIFNwLiB6IG8uby4xEjAQBgNVBAMTCUNlcnR1bSBD\
QTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAM6xwS7TT3zNJc4YPk/E\
jG+AanPIW1H4m9LcuwBcsaD8dQPugfCI7iNS6eYVM42sLQnFdvkrOYCJ5JdLkKWo\
ePhzQ3ukYbDYWMzhbGZ+nPMJXlVjhNWo7/OxLjBos8Q82KxujZlakE403Daaj4GI\
ULdtlkIJ89eVgw1BS7Bqa/j8D35in2fE7SZfECYPCE/wpFcozo+47UX2bu4lXapu\
Ob7kky/ZR6By6/qmW6/KUz/iDsaWVhFu9+lmqSbYf5VT7QqFiLpPKaVCjF62/IUg\
AKpoC6EahQGcxEZjgoi2IrHu/qpGWX7PNSzVttpd90gzFFS269lvzs2I1qsb2pY7\
HVkCAwEAAaMTMBEwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQUFAAOCAQEA\
uI3O7+cUus/usESSbLQ5PqKEbq24IXfS1HeCh+YgQYHu4vgRt2PRFze+GXYkHAQa\
TOs9qmdvLdTN/mUxcMUbpgIKumB7bVjCmkn+YzILa+M6wKyrO7Do0wlRjBCDxjTg\
xSvgGrZgFCdsMneMvLJymM/NzD+5yCRCFNZX/OYmQ6kd5YCQzgNUKD73P9P4Te1q\
CjqTE5s7FCMTY5w/0YcneeVMUeMBrYVdGjux1XMQpNPyvG5k9VpWkKjHDkx0Dy5x\
O/fIR/RpbxXyEV6DHpx8Uq79AtoSqFlnGNu8cN2bsWntgM6JQEhqDjXKKWYVIZQs\
6GAqm4VKQPNriiTsBhYscw==\
-----END CERTIFICATE-----\
\
Comodo AAA Services root\
========================\
\
MD5 Fingerprint=49:79:04:B0:EB:87:19:AC:47:B0:BC:11:51:9B:74:D0\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=GB, ST=Greater Manchester, L=Salford, O=Comodo CA Limited, CN=AAA Certificate Services\
        Validity\
            Not Before: Jan  1 00:00:00 2004 GMT\
            Not After : Dec 31 23:59:59 2028 GMT\
        Subject: C=GB, ST=Greater Manchester, L=Salford, O=Comodo CA Limited, CN=AAA Certificate Services\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:be:40:9d:f4:6e:e1:ea:76:87:1c:4d:45:44:8e:\
                    be:46:c8:83:06:9d:c1:2a:fe:18:1f:8e:e4:02:fa:\
                    f3:ab:5d:50:8a:16:31:0b:9a:06:d0:c5:70:22:cd:\
                    49:2d:54:63:cc:b6:6e:68:46:0b:53:ea:cb:4c:24:\
                    c0:bc:72:4e:ea:f1:15:ae:f4:54:9a:12:0a:c3:7a:\
                    b2:33:60:e2:da:89:55:f3:22:58:f3:de:dc:cf:ef:\
                    83:86:a2:8c:94:4f:9f:68:f2:98:90:46:84:27:c7:\
                    76:bf:e3:cc:35:2c:8b:5e:07:64:65:82:c0:48:b0:\
                    a8:91:f9:61:9f:76:20:50:a8:91:c7:66:b5:eb:78:\
                    62:03:56:f0:8a:1a:13:ea:31:a3:1e:a0:99:fd:38:\
                    f6:f6:27:32:58:6f:07:f5:6b:b8:fb:14:2b:af:b7:\
                    aa:cc:d6:63:5f:73:8c:da:05:99:a8:38:a8:cb:17:\
                    78:36:51:ac:e9:9e:f4:78:3a:8d:cf:0f:d9:42:e2:\
                    98:0c:ab:2f:9f:0e:01:de:ef:9f:99:49:f1:2d:df:\
                    ac:74:4d:1b:98:b5:47:c5:e5:29:d1:f9:90:18:c7:\
                    62:9c:be:83:c7:26:7b:3e:8a:25:c7:c0:dd:9d:e6:\
                    35:68:10:20:9d:8f:d8:de:d2:c3:84:9c:0d:5e:e8:\
                    2f:c9\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                A0:11:0A:23:3E:96:F1:07:EC:E2:AF:29:EF:82:A5:7F:D0:30:A4:B4\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 CRL Distribution Points: \
                URI:http://crl.comodoca.com/AAACertificateServices.crl\
                URI:http://crl.comodo.net/AAACertificateServices.crl\
\
    Signature Algorithm: sha1WithRSAEncryption\
        08:56:fc:02:f0:9b:e8:ff:a4:fa:d6:7b:c6:44:80:ce:4f:c4:\
        c5:f6:00:58:cc:a6:b6:bc:14:49:68:04:76:e8:e6:ee:5d:ec:\
        02:0f:60:d6:8d:50:18:4f:26:4e:01:e3:e6:b0:a5:ee:bf:bc:\
        74:54:41:bf:fd:fc:12:b8:c7:4f:5a:f4:89:60:05:7f:60:b7:\
        05:4a:f3:f6:f1:c2:bf:c4:b9:74:86:b6:2d:7d:6b:cc:d2:f3:\
        46:dd:2f:c6:e0:6a:c3:c3:34:03:2c:7d:96:dd:5a:c2:0e:a7:\
        0a:99:c1:05:8b:ab:0c:2f:f3:5c:3a:cf:6c:37:55:09:87:de:\
        53:40:6c:58:ef:fc:b6:ab:65:6e:04:f6:1b:dc:3c:e0:5a:15:\
        c6:9e:d9:f1:59:48:30:21:65:03:6c:ec:e9:21:73:ec:9b:03:\
        a1:e0:37:ad:a0:15:18:8f:fa:ba:02:ce:a7:2c:a9:10:13:2c:\
        d4:e5:08:26:ab:22:97:60:f8:90:5e:74:d4:a2:9a:53:bd:f2:\
        a9:68:e0:a2:6e:c2:d7:6c:b1:a3:0f:9e:bf:eb:68:e7:56:f2:\
        ae:f2:e3:2b:38:3a:09:81:b5:6b:85:d7:be:2d:ed:3f:1a:b7:\
        b2:63:e2:f5:62:2c:82:d4:6a:00:41:50:f1:39:83:9f:95:e9:\
        36:96:98:6e\
-----BEGIN CERTIFICATE-----\
MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb\
MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\
GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj\
YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL\
MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\
BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM\
GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP\
ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua\
BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe\
3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4\
YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR\
rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm\
ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU\
oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\
MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v\
QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t\
b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF\
AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q\
GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz\
Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2\
G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi\
l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3\
smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==\
-----END CERTIFICATE-----\
\
Comodo Secure Services root\
===========================\
\
MD5 Fingerprint=D3:D9:BD:AE:9F:AC:67:24:B3:C8:1B:52:E1:B9:A9:BD\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=GB, ST=Greater Manchester, L=Salford, O=Comodo CA Limited, CN=Secure Certificate Services\
        Validity\
            Not Before: Jan  1 00:00:00 2004 GMT\
            Not After : Dec 31 23:59:59 2028 GMT\
        Subject: C=GB, ST=Greater Manchester, L=Salford, O=Comodo CA Limited, CN=Secure Certificate Services\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:c0:71:33:82:8a:d0:70:eb:73:87:82:40:d5:1d:\
                    e4:cb:c9:0e:42:90:f9:de:34:b9:a1:ba:11:f4:25:\
                    85:f3:cc:72:6d:f2:7b:97:6b:b3:07:f1:77:24:91:\
                    5f:25:8f:f6:74:3d:e4:80:c2:f8:3c:0d:f3:bf:40:\
                    ea:f7:c8:52:d1:72:6f:ef:c8:ab:41:b8:6e:2e:17:\
                    2a:95:69:0c:cd:d2:1e:94:7b:2d:94:1d:aa:75:d7:\
                    b3:98:cb:ac:bc:64:53:40:bc:8f:ac:ac:36:cb:5c:\
                    ad:bb:dd:e0:94:17:ec:d1:5c:d0:bf:ef:a5:95:c9:\
                    90:c5:b0:ac:fb:1b:43:df:7a:08:5d:b7:b8:f2:40:\
                    1b:2b:27:9e:50:ce:5e:65:82:88:8c:5e:d3:4e:0c:\
                    7a:ea:08:91:b6:36:aa:2b:42:fb:ea:c2:a3:39:e5:\
                    db:26:38:ad:8b:0a:ee:19:63:c7:1c:24:df:03:78:\
                    da:e6:ea:c1:47:1a:0b:0b:46:09:dd:02:fc:de:cb:\
                    87:5f:d7:30:63:68:a1:ae:dc:32:a1:ba:be:fe:44:\
                    ab:68:b6:a5:17:15:fd:bd:d5:a7:a7:9a:e4:44:33:\
                    e9:88:8e:fc:ed:51:eb:93:71:4e:ad:01:e7:44:8e:\
                    ab:2d:cb:a8:fe:01:49:48:f0:c0:dd:c7:68:d8:92:\
                    fe:3d\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                3C:D8:93:88:C2:C0:82:09:CC:01:99:06:93:20:E9:9E:70:09:63:4F\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 CRL Distribution Points: \
                URI:http://crl.comodoca.com/SecureCertificateServices.crl\
                URI:http://crl.comodo.net/SecureCertificateServices.crl\
\
    Signature Algorithm: sha1WithRSAEncryption\
        87:01:6d:23:1d:7e:5b:17:7d:c1:61:32:cf:8f:e7:f3:8a:94:\
        59:66:e0:9e:28:a8:5e:d3:b7:f4:34:e6:aa:39:b2:97:16:c5:\
        82:6f:32:a4:e9:8c:e7:af:fd:ef:c2:e8:b9:4b:aa:a3:f4:e6:\
        da:8d:65:21:fb:ba:80:eb:26:28:85:1a:fe:39:8c:de:5b:04:\
        04:b4:54:f9:a3:67:9e:41:fa:09:52:cc:05:48:a8:c9:3f:21:\
        04:1e:ce:48:6b:fc:85:e8:c2:7b:af:7f:b7:cc:f8:5f:3a:fd:\
        35:c6:0d:ef:97:dc:4c:ab:11:e1:6b:cb:31:d1:6c:fb:48:80:\
        ab:dc:9c:37:b8:21:14:4b:0d:71:3d:ec:83:33:6e:d1:6e:32:\
        16:ec:98:c7:16:8b:59:a6:34:ab:05:57:2d:93:f7:aa:13:cb:\
        d2:13:e2:b7:2e:3b:cd:6b:50:17:09:68:3e:b5:26:57:ee:b6:\
        e0:b6:dd:b9:29:80:79:7d:8f:a3:f0:a4:28:a4:15:c4:85:f4:\
        27:d4:6b:bf:e5:5c:e4:65:02:76:54:b4:e3:37:66:24:d3:19:\
        61:c8:52:10:e5:8b:37:9a:b9:a9:f9:1d:bf:ea:99:92:61:96:\
        ff:01:cd:a1:5f:0d:bc:71:bc:0e:ac:0b:1d:47:45:1d:c1:ec:\
        7c:ec:fd:29\
-----BEGIN CERTIFICATE-----\
MIIEPzCCAyegAwIBAgIBATANBgkqhkiG9w0BAQUFADB+MQswCQYDVQQGEwJHQjEb\
MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\
GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEkMCIGA1UEAwwbU2VjdXJlIENlcnRp\
ZmljYXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVow\
fjELMAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\
A1UEBwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxJDAiBgNV\
BAMMG1NlY3VyZSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEB\
BQADggEPADCCAQoCggEBAMBxM4KK0HDrc4eCQNUd5MvJDkKQ+d40uaG6EfQlhfPM\
cm3ye5drswfxdySRXyWP9nQ95IDC+DwN879A6vfIUtFyb+/Iq0G4bi4XKpVpDM3S\
HpR7LZQdqnXXs5jLrLxkU0C8j6ysNstcrbvd4JQX7NFc0L/vpZXJkMWwrPsbQ996\
CF23uPJAGysnnlDOXmWCiIxe004MeuoIkbY2qitC++rCoznl2yY4rYsK7hljxxwk\
3wN42ubqwUcaCwtGCd0C/N7Lh1/XMGNooa7cMqG6vv5Eq2i2pRcV/b3Vp6ea5EQz\
6YiO/O1R65NxTq0B50SOqy3LqP4BSUjwwN3HaNiS/j0CAwEAAaOBxzCBxDAdBgNV\
HQ4EFgQUPNiTiMLAggnMAZkGkyDpnnAJY08wDgYDVR0PAQH/BAQDAgEGMA8GA1Ud\
EwEB/wQFMAMBAf8wgYEGA1UdHwR6MHgwO6A5oDeGNWh0dHA6Ly9jcmwuY29tb2Rv\
Y2EuY29tL1NlY3VyZUNlcnRpZmljYXRlU2VydmljZXMuY3JsMDmgN6A1hjNodHRw\
Oi8vY3JsLmNvbW9kby5uZXQvU2VjdXJlQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmww\
DQYJKoZIhvcNAQEFBQADggEBAIcBbSMdflsXfcFhMs+P5/OKlFlm4J4oqF7Tt/Q0\
5qo5spcWxYJvMqTpjOev/e/C6LlLqqP05tqNZSH7uoDrJiiFGv45jN5bBAS0VPmj\
Z55B+glSzAVIqMk/IQQezkhr/IXownuvf7fM+F86/TXGDe+X3EyrEeFryzHRbPtI\
gKvcnDe4IRRLDXE97IMzbtFuMhbsmMcWi1mmNKsFVy2T96oTy9IT4rcuO81rUBcJ\
aD61JlfutuC23bkpgHl9j6PwpCikFcSF9CfUa7/lXORlAnZUtOM3ZiTTGWHIUhDl\
izeauan5Hb/qmZJhlv8BzaFfDbxxvA6sCx1HRR3B7Hzs/Sk=\
-----END CERTIFICATE-----\
\
Comodo Trusted Services root\
============================\
\
MD5 Fingerprint=91:1B:3F:6E:CD:9E:AB:EE:07:FE:1F:71:D2:B3:61:27\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=GB, ST=Greater Manchester, L=Salford, O=Comodo CA Limited, CN=Trusted Certificate Services\
        Validity\
            Not Before: Jan  1 00:00:00 2004 GMT\
            Not After : Dec 31 23:59:59 2028 GMT\
        Subject: C=GB, ST=Greater Manchester, L=Salford, O=Comodo CA Limited, CN=Trusted Certificate Services\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:df:71:6f:36:58:53:5a:f2:36:54:57:80:c4:74:\
                    08:20:ed:18:7f:2a:1d:e6:35:9a:1e:25:ac:9c:e5:\
                    96:7e:72:52:a0:15:42:db:59:dd:64:7a:1a:d0:b8:\
                    7b:dd:39:15:bc:55:48:c4:ed:3a:00:ea:31:11:ba:\
                    f2:71:74:1a:67:b8:cf:33:cc:a8:31:af:a3:e3:d7:\
                    7f:bf:33:2d:4c:6a:3c:ec:8b:c3:92:d2:53:77:24:\
                    74:9c:07:6e:70:fc:bd:0b:5b:76:ba:5f:f2:ff:d7:\
                    37:4b:4a:60:78:f7:f0:fa:ca:70:b4:ea:59:aa:a3:\
                    ce:48:2f:a9:c3:b2:0b:7e:17:72:16:0c:a6:07:0c:\
                    1b:38:cf:c9:62:b7:3f:a0:93:a5:87:41:f2:b7:70:\
                    40:77:d8:be:14:7c:e3:a8:c0:7a:8e:e9:63:6a:d1:\
                    0f:9a:c6:d2:f4:8b:3a:14:04:56:d4:ed:b8:cc:6e:\
                    f5:fb:e2:2c:58:bd:7f:4f:6b:2b:f7:60:24:58:24:\
                    ce:26:ef:34:91:3a:d5:e3:81:d0:b2:f0:04:02:d7:\
                    5b:b7:3e:92:ac:6b:12:8a:f9:e4:05:b0:3b:91:49:\
                    5c:b2:eb:53:ea:f8:9f:47:86:ee:bf:95:c0:c0:06:\
                    9f:d2:5b:5e:11:1b:f4:c7:04:35:29:d2:55:5c:e4:\
                    ed:eb\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                C5:7B:58:BD:ED:DA:25:69:D2:F7:59:16:A8:B3:32:C0:7B:27:5B:F4\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 CRL Distribution Points: \
                URI:http://crl.comodoca.com/TrustedCertificateServices.crl\
                URI:http://crl.comodo.net/TrustedCertificateServices.crl\
\
    Signature Algorithm: sha1WithRSAEncryption\
        c8:93:81:3b:89:b4:af:b8:84:12:4c:8d:d2:f0:db:70:ba:57:\
        86:15:34:10:b9:2f:7f:1e:b0:a8:89:60:a1:8a:c2:77:0c:50:\
        4a:9b:00:8b:d8:8b:f4:41:e2:d0:83:8a:4a:1c:14:06:b0:a3:\
        68:05:70:31:30:a7:53:9b:0e:e9:4a:a0:58:69:67:0e:ae:9d:\
        f6:a5:2c:41:bf:3c:06:6b:e4:59:cc:6d:10:f1:96:6f:1f:df:\
        f4:04:02:a4:9f:45:3e:c8:d8:fa:36:46:44:50:3f:82:97:91:\
        1f:28:db:18:11:8c:2a:e4:65:83:57:12:12:8c:17:3f:94:36:\
        fe:5d:b0:c0:04:77:13:b8:f4:15:d5:3f:38:cc:94:3a:55:d0:\
        ac:98:f5:ba:00:5f:e0:86:19:81:78:2f:28:c0:7e:d3:cc:42:\
        0a:f5:ae:50:a0:d1:3e:c6:a1:71:ec:3f:a0:20:8c:66:3a:89:\
        b4:8e:d4:d8:b1:4d:25:47:ee:2f:88:c8:b5:e1:05:45:c0:be:\
        14:71:de:7a:fd:8e:7b:7d:4d:08:96:a5:12:73:f0:2d:ca:37:\
        27:74:12:27:4c:cb:b6:97:e9:d9:ae:08:6d:5a:39:40:dd:05:\
        47:75:6a:5a:21:b3:a3:18:cf:4e:f7:2e:57:b7:98:70:5e:c8:\
        c4:78:b0:62\
-----BEGIN CERTIFICATE-----\
MIIEQzCCAyugAwIBAgIBATANBgkqhkiG9w0BAQUFADB/MQswCQYDVQQGEwJHQjEb\
MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\
GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDElMCMGA1UEAwwcVHJ1c3RlZCBDZXJ0\
aWZpY2F0ZSBTZXJ2aWNlczAeFw0wNDAxMDEwMDAwMDBaFw0yODEyMzEyMzU5NTla\
MH8xCzAJBgNVBAYTAkdCMRswGQYDVQQIDBJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\
BgNVBAcMB1NhbGZvcmQxGjAYBgNVBAoMEUNvbW9kbyBDQSBMaW1pdGVkMSUwIwYD\
VQQDDBxUcnVzdGVkIENlcnRpZmljYXRlIFNlcnZpY2VzMIIBIjANBgkqhkiG9w0B\
AQEFAAOCAQ8AMIIBCgKCAQEA33FvNlhTWvI2VFeAxHQIIO0Yfyod5jWaHiWsnOWW\
fnJSoBVC21ndZHoa0Lh73TkVvFVIxO06AOoxEbrycXQaZ7jPM8yoMa+j49d/vzMt\
TGo87IvDktJTdyR0nAducPy9C1t2ul/y/9c3S0pgePfw+spwtOpZqqPOSC+pw7IL\
fhdyFgymBwwbOM/JYrc/oJOlh0Hyt3BAd9i+FHzjqMB6juljatEPmsbS9Is6FARW\
1O24zG71++IsWL1/T2sr92AkWCTOJu80kTrV44HQsvAEAtdbtz6SrGsSivnkBbA7\
kUlcsutT6vifR4buv5XAwAaf0lteERv0xwQ1KdJVXOTt6wIDAQABo4HJMIHGMB0G\
A1UdDgQWBBTFe1i97doladL3WRaoszLAeydb9DAOBgNVHQ8BAf8EBAMCAQYwDwYD\
VR0TAQH/BAUwAwEB/zCBgwYDVR0fBHwwejA8oDqgOIY2aHR0cDovL2NybC5jb21v\
ZG9jYS5jb20vVHJ1c3RlZENlcnRpZmljYXRlU2VydmljZXMuY3JsMDqgOKA2hjRo\
dHRwOi8vY3JsLmNvbW9kby5uZXQvVHJ1c3RlZENlcnRpZmljYXRlU2VydmljZXMu\
Y3JsMA0GCSqGSIb3DQEBBQUAA4IBAQDIk4E7ibSvuIQSTI3S8NtwuleGFTQQuS9/\
HrCoiWChisJ3DFBKmwCL2Iv0QeLQg4pKHBQGsKNoBXAxMKdTmw7pSqBYaWcOrp32\
pSxBvzwGa+RZzG0Q8ZZvH9/0BAKkn0U+yNj6NkZEUD+Cl5EfKNsYEYwq5GWDVxIS\
jBc/lDb+XbDABHcTuPQV1T84zJQ6VdCsmPW6AF/ghhmBeC8owH7TzEIK9a5QoNE+\
xqFx7D+gIIxmOom0jtTYsU0lR+4viMi14QVFwL4Ucd56/Y57fU0IlqUSc/Atyjcn\
dBInTMu2l+nZrghtWjlA3QVHdWpaIbOjGM9O9y5Xt5hwXsjEeLBi\
-----END CERTIFICATE-----\
\
IPS Chained CAs root\
====================\
\
MD5 Fingerprint=8D:72:51:DB:A0:3A:CF:20:77:DF:F2:65:06:5E:DF:EF\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA Chained CAs Certification Authority, CN=IPS CA Chained CAs Certification Authority/emailAddress=ips@mail.ips.es\
        Validity\
            Not Before: Dec 29 00:53:58 2001 GMT\
            Not After : Dec 27 00:53:58 2025 GMT\
        Subject: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA Chained CAs Certification Authority, CN=IPS CA Chained CAs Certification Authority/emailAddress=ips@mail.ips.es\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:dc:56:92:49:b2:94:20:bc:98:4f:50:eb:68:a4:\
                    a7:49:0b:bf:d2:31:e8:c7:4f:c2:86:0b:fa:68:fd:\
                    43:5a:8a:f3:60:92:35:99:38:bb:4d:03:52:21:5b:\
                    f0:37:99:35:e1:41:20:81:85:81:05:71:81:9d:b4:\
                    95:19:a9:5f:76:34:2e:63:37:35:57:8e:b4:1f:42:\
                    3f:15:5c:e1:7a:c1:5f:13:18:32:31:c9:ad:be:a3:\
                    c7:83:66:1e:b9:9c:04:13:cb:69:c1:06:de:30:06:\
                    bb:33:a3:b5:1f:f0:8f:6f:ce:ff:96:e8:54:be:66:\
                    80:ae:6b:db:41:84:36:a2:3d\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                A1:AD:31:B1:F9:3E:E1:17:A6:C8:AB:34:FC:52:87:09:1E:62:52:41\
            X509v3 Authority Key Identifier: \
                keyid:A1:AD:31:B1:F9:3E:E1:17:A6:C8:AB:34:FC:52:87:09:1E:62:52:41\
                DirName:/C=ES/ST=Barcelona/L=Barcelona/O=IPS Internet publishing Services s.l./O=ips@mail.ips.es C.I.F.  B-60929452/OU=IPS CA Chained CAs Certification Authority/CN=IPS CA Chained CAs Certification Authority/emailAddress=ips@mail.ips.es\
                serial:00\
\
            X509v3 Basic Constraints: \
                CA:TRUE\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Key Encipherment, Data Encipherment, Key Agreement, Certificate Sign, CRL Sign, Encipher Only, Decipher Only\
            X509v3 Extended Key Usage: \
                TLS Web Server Authentication, TLS Web Client Authentication, Code Signing, E-mail Protection, Time Stamping, Microsoft Individual Code Signing, Microsoft Commercial Code Signing, Microsoft Trust List Signing, Microsoft Encrypted File System\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Subject Alternative Name: \
                email:ips@mail.ips.es\
            X509v3 Issuer Alternative Name: \
                email:ips@mail.ips.es\
            Netscape Comment: \
                Chained CA Certificate issued by http://www.ips.es/\
            Netscape Base Url: \
                http://www.ips.es/ips2002/\
            Netscape CA Revocation Url: \
                http://www.ips.es/ips2002/ips2002CAC.crl\
            Netscape Revocation Url: \
                http://www.ips.es/ips2002/revocationCAC.html?\
            Netscape Renewal Url: \
                http://www.ips.es/ips2002/renewalCAC.html?\
            Netscape CA Policy Url: \
                http://www.ips.es/ips2002/policyCAC.html\
            X509v3 CRL Distribution Points: \
                URI:http://www.ips.es/ips2002/ips2002CAC.crl\
                URI:http://wwwback.ips.es/ips2002/ips2002CAC.crl\
\
            Authority Information Access: \
                OCSP - URI:http://ocsp.ips.es/\
\
    Signature Algorithm: sha1WithRSAEncryption\
        44:72:30:9d:56:58:a2:41:1b:28:b7:95:e1:a6:1a:95:5f:a7:\
        78:40:2b:ef:db:96:4a:fc:4c:71:63:d9:73:95:bd:02:e2:a2:\
        06:c7:be:97:2a:93:80:34:86:03:fa:dc:d8:3d:1e:07:cd:1e:\
        73:43:24:60:f5:1d:61:dc:dc:96:a0:bc:fb:1d:e3:e7:12:00:\
        27:33:02:c0:c0:2b:53:3d:d8:6b:03:81:a3:db:d6:93:95:20:\
        ef:d3:96:7e:26:90:89:9c:26:9b:cd:6f:66:ab:ed:03:22:44:\
        38:cc:59:bd:9f:db:f6:07:a2:01:7f:26:c4:63:f5:25:42:5e:\
        62:bd\
-----BEGIN CERTIFICATE-----\
MIIH9zCCB2CgAwIBAgIBADANBgkqhkiG9w0BAQUFADCCARwxCzAJBgNVBAYTAkVT\
MRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQBgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UE\
ChMlSVBTIEludGVybmV0IHB1Ymxpc2hpbmcgU2VydmljZXMgcy5sLjErMCkGA1UE\
ChQiaXBzQG1haWwuaXBzLmVzIEMuSS5GLiAgQi02MDkyOTQ1MjEzMDEGA1UECxMq\
SVBTIENBIENoYWluZWQgQ0FzIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MTMwMQYD\
VQQDEypJUFMgQ0EgQ2hhaW5lZCBDQXMgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkx\
HjAcBgkqhkiG9w0BCQEWD2lwc0BtYWlsLmlwcy5lczAeFw0wMTEyMjkwMDUzNTha\
Fw0yNTEyMjcwMDUzNThaMIIBHDELMAkGA1UEBhMCRVMxEjAQBgNVBAgTCUJhcmNl\
bG9uYTESMBAGA1UEBxMJQmFyY2Vsb25hMS4wLAYDVQQKEyVJUFMgSW50ZXJuZXQg\
cHVibGlzaGluZyBTZXJ2aWNlcyBzLmwuMSswKQYDVQQKFCJpcHNAbWFpbC5pcHMu\
ZXMgQy5JLkYuICBCLTYwOTI5NDUyMTMwMQYDVQQLEypJUFMgQ0EgQ2hhaW5lZCBD\
QXMgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxMzAxBgNVBAMTKklQUyBDQSBDaGFp\
bmVkIENBcyBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEeMBwGCSqGSIb3DQEJARYP\
aXBzQG1haWwuaXBzLmVzMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDcVpJJ\
spQgvJhPUOtopKdJC7/SMejHT8KGC/po/UNaivNgkjWZOLtNA1IhW/A3mTXhQSCB\
hYEFcYGdtJUZqV92NC5jNzVXjrQfQj8VXOF6wV8TGDIxya2+o8eDZh65nAQTy2nB\
Bt4wBrszo7Uf8I9vzv+W6FS+ZoCua9tBhDaiPQIDAQABo4IEQzCCBD8wHQYDVR0O\
BBYEFKGtMbH5PuEXpsirNPxShwkeYlJBMIIBTgYDVR0jBIIBRTCCAUGAFKGtMbH5\
PuEXpsirNPxShwkeYlJBoYIBJKSCASAwggEcMQswCQYDVQQGEwJFUzESMBAGA1UE\
CBMJQmFyY2Vsb25hMRIwEAYDVQQHEwlCYXJjZWxvbmExLjAsBgNVBAoTJUlQUyBJ\
bnRlcm5ldCBwdWJsaXNoaW5nIFNlcnZpY2VzIHMubC4xKzApBgNVBAoUImlwc0Bt\
YWlsLmlwcy5lcyBDLkkuRi4gIEItNjA5Mjk0NTIxMzAxBgNVBAsTKklQUyBDQSBD\
aGFpbmVkIENBcyBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEzMDEGA1UEAxMqSVBT\
IENBIENoYWluZWQgQ0FzIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MR4wHAYJKoZI\
hvcNAQkBFg9pcHNAbWFpbC5pcHMuZXOCAQAwDAYDVR0TBAUwAwEB/zAMBgNVHQ8E\
BQMDB/+AMGsGA1UdJQRkMGIGCCsGAQUFBwMBBggrBgEFBQcDAgYIKwYBBQUHAwMG\
CCsGAQUFBwMEBggrBgEFBQcDCAYKKwYBBAGCNwIBFQYKKwYBBAGCNwIBFgYKKwYB\
BAGCNwoDAQYKKwYBBAGCNwoDBDARBglghkgBhvhCAQEEBAMCAAcwGgYDVR0RBBMw\
EYEPaXBzQG1haWwuaXBzLmVzMBoGA1UdEgQTMBGBD2lwc0BtYWlsLmlwcy5lczBC\
BglghkgBhvhCAQ0ENRYzQ2hhaW5lZCBDQSBDZXJ0aWZpY2F0ZSBpc3N1ZWQgYnkg\
aHR0cDovL3d3dy5pcHMuZXMvMCkGCWCGSAGG+EIBAgQcFhpodHRwOi8vd3d3Lmlw\
cy5lcy9pcHMyMDAyLzA3BglghkgBhvhCAQQEKhYoaHR0cDovL3d3dy5pcHMuZXMv\
aXBzMjAwMi9pcHMyMDAyQ0FDLmNybDA8BglghkgBhvhCAQMELxYtaHR0cDovL3d3\
dy5pcHMuZXMvaXBzMjAwMi9yZXZvY2F0aW9uQ0FDLmh0bWw/MDkGCWCGSAGG+EIB\
BwQsFipodHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyL3JlbmV3YWxDQUMuaHRtbD8w\
NwYJYIZIAYb4QgEIBCoWKGh0dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvcG9saWN5\
Q0FDLmh0bWwwbQYDVR0fBGYwZDAuoCygKoYoaHR0cDovL3d3dy5pcHMuZXMvaXBz\
MjAwMi9pcHMyMDAyQ0FDLmNybDAyoDCgLoYsaHR0cDovL3d3d2JhY2suaXBzLmVz\
L2lwczIwMDIvaXBzMjAwMkNBQy5jcmwwLwYIKwYBBQUHAQEEIzAhMB8GCCsGAQUF\
BzABhhNodHRwOi8vb2NzcC5pcHMuZXMvMA0GCSqGSIb3DQEBBQUAA4GBAERyMJ1W\
WKJBGyi3leGmGpVfp3hAK+/blkr8THFj2XOVvQLiogbHvpcqk4A0hgP63Ng9HgfN\
HnNDJGD1HWHc3JagvPsd4+cSACczAsDAK1M92GsDgaPb1pOVIO/Tln4mkImcJpvN\
b2ar7QMiRDjMWb2f2/YHogF/JsRj9SVCXmK9\
-----END CERTIFICATE-----\
\
Thawte Server CA\
================\
\
MD5 Fingerprint=C5:70:C4:A2:ED:53:78:0C:C8:10:53:81:64:CB:D0:1D\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting cc, OU=Certification Services Division, CN=Thawte Server CA/emailAddress=server-certs@thawte.com\
        Validity\
            Not Before: Aug  1 00:00:00 1996 GMT\
            Not After : Dec 31 23:59:59 2020 GMT\
        Subject: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting cc, OU=Certification Services Division, CN=Thawte Server CA/emailAddress=server-certs@thawte.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:d3:a4:50:6e:c8:ff:56:6b:e6:cf:5d:b6:ea:0c:\
                    68:75:47:a2:aa:c2:da:84:25:fc:a8:f4:47:51:da:\
                    85:b5:20:74:94:86:1e:0f:75:c9:e9:08:61:f5:06:\
                    6d:30:6e:15:19:02:e9:52:c0:62:db:4d:99:9e:e2:\
                    6a:0c:44:38:cd:fe:be:e3:64:09:70:c5:fe:b1:6b:\
                    29:b6:2f:49:c8:3b:d4:27:04:25:10:97:2f:e7:90:\
                    6d:c0:28:42:99:d7:4c:43:de:c3:f5:21:6d:54:9f:\
                    5d:c3:58:e1:c0:e4:d9:5b:b0:b8:dc:b4:7b:df:36:\
                    3a:c2:b5:66:22:12:d6:87:0d\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
    Signature Algorithm: md5WithRSAEncryption\
        07:fa:4c:69:5c:fb:95:cc:46:ee:85:83:4d:21:30:8e:ca:d9:\
        a8:6f:49:1a:e6:da:51:e3:60:70:6c:84:61:11:a1:1a:c8:48:\
        3e:59:43:7d:4f:95:3d:a1:8b:b7:0b:62:98:7a:75:8a:dd:88:\
        4e:4e:9e:40:db:a8:cc:32:74:b9:6f:0d:c6:e3:b3:44:0b:d9:\
        8a:6f:9a:29:9b:99:18:28:3b:d1:e3:40:28:9a:5a:3c:d5:b5:\
        e7:20:1b:8b:ca:a4:ab:8d:e9:51:d9:e2:4c:2c:59:a9:da:b9:\
        b2:75:1b:f6:42:f2:ef:c7:f2:18:f9:89:bc:a3:ff:8a:23:2e:\
        70:47\
-----BEGIN CERTIFICATE-----\
MIIDEzCCAnygAwIBAgIBATANBgkqhkiG9w0BAQQFADCBxDELMAkGA1UEBhMCWkEx\
FTATBgNVBAgTDFdlc3Rlcm4gQ2FwZTESMBAGA1UEBxMJQ2FwZSBUb3duMR0wGwYD\
VQQKExRUaGF3dGUgQ29uc3VsdGluZyBjYzEoMCYGA1UECxMfQ2VydGlmaWNhdGlv\
biBTZXJ2aWNlcyBEaXZpc2lvbjEZMBcGA1UEAxMQVGhhd3RlIFNlcnZlciBDQTEm\
MCQGCSqGSIb3DQEJARYXc2VydmVyLWNlcnRzQHRoYXd0ZS5jb20wHhcNOTYwODAx\
MDAwMDAwWhcNMjAxMjMxMjM1OTU5WjCBxDELMAkGA1UEBhMCWkExFTATBgNVBAgT\
DFdlc3Rlcm4gQ2FwZTESMBAGA1UEBxMJQ2FwZSBUb3duMR0wGwYDVQQKExRUaGF3\
dGUgQ29uc3VsdGluZyBjYzEoMCYGA1UECxMfQ2VydGlmaWNhdGlvbiBTZXJ2aWNl\
cyBEaXZpc2lvbjEZMBcGA1UEAxMQVGhhd3RlIFNlcnZlciBDQTEmMCQGCSqGSIb3\
DQEJARYXc2VydmVyLWNlcnRzQHRoYXd0ZS5jb20wgZ8wDQYJKoZIhvcNAQEBBQAD\
gY0AMIGJAoGBANOkUG7I/1Zr5s9dtuoMaHVHoqrC2oQl/Kj0R1HahbUgdJSGHg91\
yekIYfUGbTBuFRkC6VLAYttNmZ7iagxEOM3+vuNkCXDF/rFrKbYvScg71CcEJRCX\
L+eQbcAoQpnXTEPew/UhbVSfXcNY4cDk2VuwuNy0e982OsK1ZiIS1ocNAgMBAAGj\
EzARMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEEBQADgYEAB/pMaVz7lcxG\
7oWDTSEwjsrZqG9JGubaUeNgcGyEYRGhGshIPllDfU+VPaGLtwtimHp1it2ITk6e\
QNuozDJ0uW8NxuOzRAvZim+aKZuZGCg70eNAKJpaPNW15yAbi8qkq43pUdniTCxZ\
qdq5snUb9kLy78fyGPmJvKP/iiMucEc=\
-----END CERTIFICATE-----\
\
IPS CLASE1 root\
===============\
\
MD5 Fingerprint=84:90:1D:95:30:49:56:FC:41:81:F0:45:D7:76:C4:6B\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA CLASE1 Certification Authority, CN=IPS CA CLASE1 Certification Authority/emailAddress=ips@mail.ips.es\
        Validity\
            Not Before: Dec 29 00:59:38 2001 GMT\
            Not After : Dec 27 00:59:38 2025 GMT\
        Subject: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA CLASE1 Certification Authority, CN=IPS CA CLASE1 Certification Authority/emailAddress=ips@mail.ips.es\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:e0:51:27:a7:0b:dd:af:d1:b9:43:5b:82:37:45:\
                    56:72:ef:9a:b6:c2:12:ef:2c:12:cc:76:f9:06:59:\
                    af:5d:21:d4:d2:5a:b8:a0:d4:f3:6a:fd:ca:69:8d:\
                    66:48:f7:74:e6:ee:36:bd:e8:96:91:75:a6:71:28:\
                    ca:e7:22:12:32:69:b0:3e:1e:6b:f4:50:52:62:62:\
                    fd:63:3b:7d:7e:ec:ee:38:ea:62:f4:6c:a8:71:8d:\
                    e1:e9:8b:c9:3f:c6:b5:cd:94:42:6f:dd:82:45:3c:\
                    e8:df:09:e8:ef:0a:55:a9:56:47:61:4c:49:64:73:\
                    10:28:3f:ca:bf:09:ff:c6:2f\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                EB:B3:19:79:F3:C1:A5:1C:AC:DC:BA:1F:66:A2:B2:9B:69:D0:78:08\
            X509v3 Authority Key Identifier: \
                keyid:EB:B3:19:79:F3:C1:A5:1C:AC:DC:BA:1F:66:A2:B2:9B:69:D0:78:08\
                DirName:/C=ES/ST=Barcelona/L=Barcelona/O=IPS Internet publishing Services s.l./O=ips@mail.ips.es C.I.F.  B-60929452/OU=IPS CA CLASE1 Certification Authority/CN=IPS CA CLASE1 Certification Authority/emailAddress=ips@mail.ips.es\
                serial:00\
\
            X509v3 Basic Constraints: \
                CA:TRUE\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Key Encipherment, Data Encipherment, Key Agreement, Certificate Sign, CRL Sign, Encipher Only, Decipher Only\
            X509v3 Extended Key Usage: \
                TLS Web Server Authentication, TLS Web Client Authentication, Code Signing, E-mail Protection, Time Stamping, Microsoft Individual Code Signing, Microsoft Commercial Code Signing, Microsoft Trust List Signing, Microsoft Encrypted File System\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Subject Alternative Name: \
                email:ips@mail.ips.es\
            X509v3 Issuer Alternative Name: \
                email:ips@mail.ips.es\
            Netscape Comment: \
                CLASE1 CA Certificate issued by http://www.ips.es/\
            Netscape Base Url: \
                http://www.ips.es/ips2002/\
            Netscape CA Revocation Url: \
                http://www.ips.es/ips2002/ips2002CLASE1.crl\
            Netscape Revocation Url: \
                http://www.ips.es/ips2002/revocationCLASE1.html?\
            Netscape Renewal Url: \
                http://www.ips.es/ips2002/renewalCLASE1.html?\
            Netscape CA Policy Url: \
                http://www.ips.es/ips2002/policyCLASE1.html\
            X509v3 CRL Distribution Points: \
                URI:http://www.ips.es/ips2002/ips2002CLASE1.crl\
                URI:http://wwwback.ips.es/ips2002/ips2002CLASE1.crl\
\
            Authority Information Access: \
                OCSP - URI:http://ocsp.ips.es/\
\
    Signature Algorithm: sha1WithRSAEncryption\
        2b:d0:eb:fd:da:c8:ca:59:6a:da:d3:cc:32:2e:c9:54:1b:8a:\
        62:7e:15:2d:e9:d9:31:d3:2e:f4:27:23:ff:5b:ab:c5:4a:b6:\
        72:40:ae:53:74:f4:bc:05:b4:c6:d9:c8:c9:77:fb:b7:f9:34:\
        7f:78:00:f8:d6:a4:e4:52:3f:2c:4a:63:57:81:75:5a:8e:e8:\
        8c:fb:02:c0:94:c6:29:ba:b3:dc:1c:e8:b2:af:d2:2e:62:5b:\
        1a:a9:8e:0e:cc:c5:57:45:51:14:e9:4e:1c:88:a5:91:f4:a3:\
        f7:8e:51:c8:a9:be:86:33:3e:e6:2f:48:6e:af:54:90:4e:ad:\
        b1:25\
-----BEGIN CERTIFICATE-----\
MIIH6jCCB1OgAwIBAgIBADANBgkqhkiG9w0BAQUFADCCARIxCzAJBgNVBAYTAkVT\
MRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQBgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UE\
ChMlSVBTIEludGVybmV0IHB1Ymxpc2hpbmcgU2VydmljZXMgcy5sLjErMCkGA1UE\
ChQiaXBzQG1haWwuaXBzLmVzIEMuSS5GLiAgQi02MDkyOTQ1MjEuMCwGA1UECxMl\
SVBTIENBIENMQVNFMSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEuMCwGA1UEAxMl\
SVBTIENBIENMQVNFMSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEeMBwGCSqGSIb3\
DQEJARYPaXBzQG1haWwuaXBzLmVzMB4XDTAxMTIyOTAwNTkzOFoXDTI1MTIyNzAw\
NTkzOFowggESMQswCQYDVQQGEwJFUzESMBAGA1UECBMJQmFyY2Vsb25hMRIwEAYD\
VQQHEwlCYXJjZWxvbmExLjAsBgNVBAoTJUlQUyBJbnRlcm5ldCBwdWJsaXNoaW5n\
IFNlcnZpY2VzIHMubC4xKzApBgNVBAoUImlwc0BtYWlsLmlwcy5lcyBDLkkuRi4g\
IEItNjA5Mjk0NTIxLjAsBgNVBAsTJUlQUyBDQSBDTEFTRTEgQ2VydGlmaWNhdGlv\
biBBdXRob3JpdHkxLjAsBgNVBAMTJUlQUyBDQSBDTEFTRTEgQ2VydGlmaWNhdGlv\
biBBdXRob3JpdHkxHjAcBgkqhkiG9w0BCQEWD2lwc0BtYWlsLmlwcy5lczCBnzAN\
BgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA4FEnpwvdr9G5Q1uCN0VWcu+atsIS7ywS\
zHb5BlmvXSHU0lq4oNTzav3KaY1mSPd05u42veiWkXWmcSjK5yISMmmwPh5r9FBS\
YmL9Yzt9fuzuOOpi9GyocY3h6YvJP8a1zZRCb92CRTzo3wno7wpVqVZHYUxJZHMQ\
KD/Kvwn/xi8CAwEAAaOCBEowggRGMB0GA1UdDgQWBBTrsxl588GlHKzcuh9morKb\
adB4CDCCAUQGA1UdIwSCATswggE3gBTrsxl588GlHKzcuh9morKbadB4CKGCARqk\
ggEWMIIBEjELMAkGA1UEBhMCRVMxEjAQBgNVBAgTCUJhcmNlbG9uYTESMBAGA1UE\
BxMJQmFyY2Vsb25hMS4wLAYDVQQKEyVJUFMgSW50ZXJuZXQgcHVibGlzaGluZyBT\
ZXJ2aWNlcyBzLmwuMSswKQYDVQQKFCJpcHNAbWFpbC5pcHMuZXMgQy5JLkYuICBC\
LTYwOTI5NDUyMS4wLAYDVQQLEyVJUFMgQ0EgQ0xBU0UxIENlcnRpZmljYXRpb24g\
QXV0aG9yaXR5MS4wLAYDVQQDEyVJUFMgQ0EgQ0xBU0UxIENlcnRpZmljYXRpb24g\
QXV0aG9yaXR5MR4wHAYJKoZIhvcNAQkBFg9pcHNAbWFpbC5pcHMuZXOCAQAwDAYD\
VR0TBAUwAwEB/zAMBgNVHQ8EBQMDB/+AMGsGA1UdJQRkMGIGCCsGAQUFBwMBBggr\
BgEFBQcDAgYIKwYBBQUHAwMGCCsGAQUFBwMEBggrBgEFBQcDCAYKKwYBBAGCNwIB\
FQYKKwYBBAGCNwIBFgYKKwYBBAGCNwoDAQYKKwYBBAGCNwoDBDARBglghkgBhvhC\
AQEEBAMCAAcwGgYDVR0RBBMwEYEPaXBzQG1haWwuaXBzLmVzMBoGA1UdEgQTMBGB\
D2lwc0BtYWlsLmlwcy5lczBBBglghkgBhvhCAQ0ENBYyQ0xBU0UxIENBIENlcnRp\
ZmljYXRlIGlzc3VlZCBieSBodHRwOi8vd3d3Lmlwcy5lcy8wKQYJYIZIAYb4QgEC\
BBwWGmh0dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvMDoGCWCGSAGG+EIBBAQtFito\
dHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyL2lwczIwMDJDTEFTRTEuY3JsMD8GCWCG\
SAGG+EIBAwQyFjBodHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyL3Jldm9jYXRpb25D\
TEFTRTEuaHRtbD8wPAYJYIZIAYb4QgEHBC8WLWh0dHA6Ly93d3cuaXBzLmVzL2lw\
czIwMDIvcmVuZXdhbENMQVNFMS5odG1sPzA6BglghkgBhvhCAQgELRYraHR0cDov\
L3d3dy5pcHMuZXMvaXBzMjAwMi9wb2xpY3lDTEFTRTEuaHRtbDBzBgNVHR8EbDBq\
MDGgL6AthitodHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyL2lwczIwMDJDTEFTRTEu\
Y3JsMDWgM6Axhi9odHRwOi8vd3d3YmFjay5pcHMuZXMvaXBzMjAwMi9pcHMyMDAy\
Q0xBU0UxLmNybDAvBggrBgEFBQcBAQQjMCEwHwYIKwYBBQUHMAGGE2h0dHA6Ly9v\
Y3NwLmlwcy5lcy8wDQYJKoZIhvcNAQEFBQADgYEAK9Dr/drIyllq2tPMMi7JVBuK\
Yn4VLenZMdMu9Ccj/1urxUq2ckCuU3T0vAW0xtnIyXf7t/k0f3gA+Nak5FI/LEpj\
V4F1Wo7ojPsCwJTGKbqz3Bzosq/SLmJbGqmODszFV0VRFOlOHIilkfSj945RyKm+\
hjM+5i9Ibq9UkE6tsSU=\
-----END CERTIFICATE-----\
\
IPS CLASE3 root\
===============\
\
MD5 Fingerprint=42:76:97:68:CF:A6:B4:38:24:AA:A1:1B:F2:67:DE:CA\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA CLASE3 Certification Authority, CN=IPS CA CLASE3 Certification Authority/emailAddress=ips@mail.ips.es\
        Validity\
            Not Before: Dec 29 01:01:44 2001 GMT\
            Not After : Dec 27 01:01:44 2025 GMT\
        Subject: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA CLASE3 Certification Authority, CN=IPS CA CLASE3 Certification Authority/emailAddress=ips@mail.ips.es\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:ab:17:fe:0e:b0:c6:68:1b:53:f0:52:be:9f:fa:\
                    da:fa:8b:13:04:bb:01:8f:32:d9:1f:8f:4d:ce:36:\
                    98:da:e4:00:44:8c:28:d8:13:44:2a:a4:6b:4e:17:\
                    24:42:9c:d3:88:a4:41:82:d6:23:fb:8b:c9:86:e5:\
                    b9:a9:82:05:dc:f1:de:1f:e0:0c:99:55:98:f2:38:\
                    ec:6c:9d:20:03:c0:ef:aa:a3:c6:64:04:51:2d:78:\
                    0d:a3:d2:a8:3a:d6:24:4c:e9:96:7a:18:ac:13:23:\
                    22:1b:7c:e8:31:11:b3:5f:09:aa:30:70:71:46:25:\
                    6b:49:71:80:2b:95:01:b2:1f\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                B8:93:FF:2E:CB:DC:2C:8E:A2:E7:7A:FE:36:51:21:A3:98:5B:0C:34\
            X509v3 Authority Key Identifier: \
                keyid:B8:93:FF:2E:CB:DC:2C:8E:A2:E7:7A:FE:36:51:21:A3:98:5B:0C:34\
                DirName:/C=ES/ST=Barcelona/L=Barcelona/O=IPS Internet publishing Services s.l./O=ips@mail.ips.es C.I.F.  B-60929452/OU=IPS CA CLASE3 Certification Authority/CN=IPS CA CLASE3 Certification Authority/emailAddress=ips@mail.ips.es\
                serial:00\
\
            X509v3 Basic Constraints: \
                CA:TRUE\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Key Encipherment, Data Encipherment, Key Agreement, Certificate Sign, CRL Sign, Encipher Only, Decipher Only\
            X509v3 Extended Key Usage: \
                TLS Web Server Authentication, TLS Web Client Authentication, Code Signing, E-mail Protection, Time Stamping, Microsoft Individual Code Signing, Microsoft Commercial Code Signing, Microsoft Trust List Signing, Microsoft Encrypted File System\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Subject Alternative Name: \
                email:ips@mail.ips.es\
            X509v3 Issuer Alternative Name: \
                email:ips@mail.ips.es\
            Netscape Comment: \
                CLASE3 CA Certificate issued by http://www.ips.es/\
            Netscape Base Url: \
                http://www.ips.es/ips2002/\
            Netscape CA Revocation Url: \
                http://www.ips.es/ips2002/ips2002CLASE3.crl\
            Netscape Revocation Url: \
                http://www.ips.es/ips2002/revocationCLASE3.html?\
            Netscape Renewal Url: \
                http://www.ips.es/ips2002/renewalCLASE3.html?\
            Netscape CA Policy Url: \
                http://www.ips.es/ips2002/policyCLASE3.html\
            X509v3 CRL Distribution Points: \
                URI:http://www.ips.es/ips2002/ips2002CLASE3.crl\
                URI:http://wwwback.ips.es/ips2002/ips2002CLASE3.crl\
\
            Authority Information Access: \
                OCSP - URI:http://ocsp.ips.es/\
\
    Signature Algorithm: sha1WithRSAEncryption\
        17:65:5c:99:95:43:03:27:af:26:e5:eb:d0:b3:17:23:f7:43:\
        aa:c7:f0:7d:ec:0f:c6:a9:ae:ae:96:0f:76:29:1c:e2:06:2d:\
        7e:26:c5:3c:fa:a1:c1:81:ce:53:b0:42:d1:97:57:1a:17:7e:\
        a4:51:61:c6:ee:e9:5e:ef:05:ba:eb:bd:0f:a7:92:6f:d8:a3:\
        06:68:29:8e:79:f5:ff:bf:f9:a7:af:e4:b1:ce:c2:d1:80:42:\
        27:05:04:34:f8:c3:7f:16:78:23:0c:07:24:f2:46:47:ad:3b:\
        54:d0:af:d5:31:b2:af:7d:c8:ea:e9:d4:56:d9:0e:13:b2:c5:\
        45:50\
-----BEGIN CERTIFICATE-----\
MIIH6jCCB1OgAwIBAgIBADANBgkqhkiG9w0BAQUFADCCARIxCzAJBgNVBAYTAkVT\
MRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQBgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UE\
ChMlSVBTIEludGVybmV0IHB1Ymxpc2hpbmcgU2VydmljZXMgcy5sLjErMCkGA1UE\
ChQiaXBzQG1haWwuaXBzLmVzIEMuSS5GLiAgQi02MDkyOTQ1MjEuMCwGA1UECxMl\
SVBTIENBIENMQVNFMyBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEuMCwGA1UEAxMl\
SVBTIENBIENMQVNFMyBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTEeMBwGCSqGSIb3\
DQEJARYPaXBzQG1haWwuaXBzLmVzMB4XDTAxMTIyOTAxMDE0NFoXDTI1MTIyNzAx\
MDE0NFowggESMQswCQYDVQQGEwJFUzESMBAGA1UECBMJQmFyY2Vsb25hMRIwEAYD\
VQQHEwlCYXJjZWxvbmExLjAsBgNVBAoTJUlQUyBJbnRlcm5ldCBwdWJsaXNoaW5n\
IFNlcnZpY2VzIHMubC4xKzApBgNVBAoUImlwc0BtYWlsLmlwcy5lcyBDLkkuRi4g\
IEItNjA5Mjk0NTIxLjAsBgNVBAsTJUlQUyBDQSBDTEFTRTMgQ2VydGlmaWNhdGlv\
biBBdXRob3JpdHkxLjAsBgNVBAMTJUlQUyBDQSBDTEFTRTMgQ2VydGlmaWNhdGlv\
biBBdXRob3JpdHkxHjAcBgkqhkiG9w0BCQEWD2lwc0BtYWlsLmlwcy5lczCBnzAN\
BgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAqxf+DrDGaBtT8FK+n/ra+osTBLsBjzLZ\
H49NzjaY2uQARIwo2BNEKqRrThckQpzTiKRBgtYj+4vJhuW5qYIF3PHeH+AMmVWY\
8jjsbJ0gA8DvqqPGZARRLXgNo9KoOtYkTOmWehisEyMiG3zoMRGzXwmqMHBxRiVr\
SXGAK5UBsh8CAwEAAaOCBEowggRGMB0GA1UdDgQWBBS4k/8uy9wsjqLnev42USGj\
mFsMNDCCAUQGA1UdIwSCATswggE3gBS4k/8uy9wsjqLnev42USGjmFsMNKGCARqk\
ggEWMIIBEjELMAkGA1UEBhMCRVMxEjAQBgNVBAgTCUJhcmNlbG9uYTESMBAGA1UE\
BxMJQmFyY2Vsb25hMS4wLAYDVQQKEyVJUFMgSW50ZXJuZXQgcHVibGlzaGluZyBT\
ZXJ2aWNlcyBzLmwuMSswKQYDVQQKFCJpcHNAbWFpbC5pcHMuZXMgQy5JLkYuICBC\
LTYwOTI5NDUyMS4wLAYDVQQLEyVJUFMgQ0EgQ0xBU0UzIENlcnRpZmljYXRpb24g\
QXV0aG9yaXR5MS4wLAYDVQQDEyVJUFMgQ0EgQ0xBU0UzIENlcnRpZmljYXRpb24g\
QXV0aG9yaXR5MR4wHAYJKoZIhvcNAQkBFg9pcHNAbWFpbC5pcHMuZXOCAQAwDAYD\
VR0TBAUwAwEB/zAMBgNVHQ8EBQMDB/+AMGsGA1UdJQRkMGIGCCsGAQUFBwMBBggr\
BgEFBQcDAgYIKwYBBQUHAwMGCCsGAQUFBwMEBggrBgEFBQcDCAYKKwYBBAGCNwIB\
FQYKKwYBBAGCNwIBFgYKKwYBBAGCNwoDAQYKKwYBBAGCNwoDBDARBglghkgBhvhC\
AQEEBAMCAAcwGgYDVR0RBBMwEYEPaXBzQG1haWwuaXBzLmVzMBoGA1UdEgQTMBGB\
D2lwc0BtYWlsLmlwcy5lczBBBglghkgBhvhCAQ0ENBYyQ0xBU0UzIENBIENlcnRp\
ZmljYXRlIGlzc3VlZCBieSBodHRwOi8vd3d3Lmlwcy5lcy8wKQYJYIZIAYb4QgEC\
BBwWGmh0dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvMDoGCWCGSAGG+EIBBAQtFito\
dHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyL2lwczIwMDJDTEFTRTMuY3JsMD8GCWCG\
SAGG+EIBAwQyFjBodHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyL3Jldm9jYXRpb25D\
TEFTRTMuaHRtbD8wPAYJYIZIAYb4QgEHBC8WLWh0dHA6Ly93d3cuaXBzLmVzL2lw\
czIwMDIvcmVuZXdhbENMQVNFMy5odG1sPzA6BglghkgBhvhCAQgELRYraHR0cDov\
L3d3dy5pcHMuZXMvaXBzMjAwMi9wb2xpY3lDTEFTRTMuaHRtbDBzBgNVHR8EbDBq\
MDGgL6AthitodHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyL2lwczIwMDJDTEFTRTMu\
Y3JsMDWgM6Axhi9odHRwOi8vd3d3YmFjay5pcHMuZXMvaXBzMjAwMi9pcHMyMDAy\
Q0xBU0UzLmNybDAvBggrBgEFBQcBAQQjMCEwHwYIKwYBBQUHMAGGE2h0dHA6Ly9v\
Y3NwLmlwcy5lcy8wDQYJKoZIhvcNAQEFBQADgYEAF2VcmZVDAyevJuXr0LMXI/dD\
qsfwfewPxqmurpYPdikc4gYtfibFPPqhwYHOU7BC0ZdXGhd+pFFhxu7pXu8Fuuu9\
D6eSb9ijBmgpjnn1/7/5p6/ksc7C0YBCJwUENPjDfxZ4IwwHJPJGR607VNCv1TGy\
r33I6unUVtkOE7LFRVA=\
-----END CERTIFICATE-----\
\
IPS CLASEA1 root\
================\
\
MD5 Fingerprint=0C:F8:9E:17:FC:D4:03:BD:E6:8D:9B:3C:05:87:FE:84\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA CLASEA1 Certification Authority, CN=IPS CA CLASEA1 Certification Authority/emailAddress=ips@mail.ips.es\
        Validity\
            Not Before: Dec 29 01:05:32 2001 GMT\
            Not After : Dec 27 01:05:32 2025 GMT\
        Subject: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA CLASEA1 Certification Authority, CN=IPS CA CLASEA1 Certification Authority/emailAddress=ips@mail.ips.es\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:bb:30:d7:dc:d0:54:bd:35:4e:9f:c5:4c:82:ea:\
                    d1:50:3c:47:98:fc:9b:69:9d:77:cd:6e:e0:3f:ee:\
                    eb:32:5f:5f:9f:d2:d0:79:e5:95:73:44:21:32:e0:\
                    0a:db:9d:d7:ce:8d:ab:52:8b:2b:78:e0:9b:5b:7d:\
                    f4:fd:6d:09:e5:ae:e1:6c:1d:07:23:a0:17:d1:f9:\
                    7d:a8:46:46:91:22:a8:b2:69:c6:ad:f7:f5:f5:94:\
                    a1:30:94:bd:00:cc:44:7f:ee:c4:9e:c9:c1:e6:8f:\
                    0a:36:c1:fd:24:3d:01:a0:f5:7b:e2:7c:78:66:43:\
                    8b:4f:59:f2:9b:d9:fa:49:b3\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                67:26:96:E7:A1:BF:D8:B5:03:9D:FE:3B:DC:FE:F2:8A:E6:15:DD:30\
            X509v3 Authority Key Identifier: \
                keyid:67:26:96:E7:A1:BF:D8:B5:03:9D:FE:3B:DC:FE:F2:8A:E6:15:DD:30\
                DirName:/C=ES/ST=Barcelona/L=Barcelona/O=IPS Internet publishing Services s.l./O=ips@mail.ips.es C.I.F.  B-60929452/OU=IPS CA CLASEA1 Certification Authority/CN=IPS CA CLASEA1 Certification Authority/emailAddress=ips@mail.ips.es\
                serial:00\
\
            X509v3 Basic Constraints: \
                CA:TRUE\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Key Encipherment, Data Encipherment, Key Agreement, Certificate Sign, CRL Sign, Encipher Only, Decipher Only\
            X509v3 Extended Key Usage: \
                TLS Web Server Authentication, TLS Web Client Authentication, Code Signing, E-mail Protection, Time Stamping, Microsoft Individual Code Signing, Microsoft Commercial Code Signing, Microsoft Trust List Signing, Microsoft Encrypted File System\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Subject Alternative Name: \
                email:ips@mail.ips.es\
            X509v3 Issuer Alternative Name: \
                email:ips@mail.ips.es\
            Netscape Comment: \
                CLASEA1 CA Certificate issued by http://www.ips.es/\
            Netscape Base Url: \
                http://www.ips.es/ips2002/\
            Netscape CA Revocation Url: \
                http://www.ips.es/ips2002/ips2002CLASEA1.crl\
            Netscape Revocation Url: \
                http://www.ips.es/ips2002/revocationCLASEA1.html?\
            Netscape Renewal Url: \
                http://www.ips.es/ips2002/renewalCLASEA1.html?\
            Netscape CA Policy Url: \
                http://www.ips.es/ips2002/policyCLASEA1.html\
            X509v3 CRL Distribution Points: \
                URI:http://www.ips.es/ips2002/ips2002CLASEA1.crl\
                URI:http://wwwback.ips.es/ips2002/ips2002CLASEA1.crl\
\
            Authority Information Access: \
                OCSP - URI:http://ocsp.ips.es/\
\
    Signature Algorithm: sha1WithRSAEncryption\
        7e:ba:8a:ac:80:00:84:15:0a:d5:98:51:0c:64:c5:9c:02:58:\
        83:66:ca:ad:1e:07:cd:7e:6a:da:80:07:df:03:34:4a:1c:93:\
        c4:4b:58:20:35:36:71:ed:a2:0a:35:12:a5:a6:65:a7:85:69:\
        0a:0e:e3:61:ee:ea:be:28:93:33:d5:ec:e8:be:c4:db:5f:7f:\
        a8:f9:63:31:c8:6b:96:e2:29:c2:5b:a0:e7:97:36:9d:77:5e:\
        31:6b:fe:d3:a7:db:2a:db:db:96:8b:1f:66:de:b6:03:c0:2b:\
        b3:78:d6:55:07:e5:8f:39:50:de:07:23:72:e6:bd:20:14:4b:\
        b4:86\
-----BEGIN CERTIFICATE-----\
MIIH9zCCB2CgAwIBAgIBADANBgkqhkiG9w0BAQUFADCCARQxCzAJBgNVBAYTAkVT\
MRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQBgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UE\
ChMlSVBTIEludGVybmV0IHB1Ymxpc2hpbmcgU2VydmljZXMgcy5sLjErMCkGA1UE\
ChQiaXBzQG1haWwuaXBzLmVzIEMuSS5GLiAgQi02MDkyOTQ1MjEvMC0GA1UECxMm\
SVBTIENBIENMQVNFQTEgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxLzAtBgNVBAMT\
JklQUyBDQSBDTEFTRUExIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MR4wHAYJKoZI\
hvcNAQkBFg9pcHNAbWFpbC5pcHMuZXMwHhcNMDExMjI5MDEwNTMyWhcNMjUxMjI3\
MDEwNTMyWjCCARQxCzAJBgNVBAYTAkVTMRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQ\
BgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UEChMlSVBTIEludGVybmV0IHB1Ymxpc2hp\
bmcgU2VydmljZXMgcy5sLjErMCkGA1UEChQiaXBzQG1haWwuaXBzLmVzIEMuSS5G\
LiAgQi02MDkyOTQ1MjEvMC0GA1UECxMmSVBTIENBIENMQVNFQTEgQ2VydGlmaWNh\
dGlvbiBBdXRob3JpdHkxLzAtBgNVBAMTJklQUyBDQSBDTEFTRUExIENlcnRpZmlj\
YXRpb24gQXV0aG9yaXR5MR4wHAYJKoZIhvcNAQkBFg9pcHNAbWFpbC5pcHMuZXMw\
gZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBALsw19zQVL01Tp/FTILq0VA8R5j8\
m2mdd81u4D/u6zJfX5/S0HnllXNEITLgCtud186Nq1KLK3jgm1t99P1tCeWu4Wwd\
ByOgF9H5fahGRpEiqLJpxq339fWUoTCUvQDMRH/uxJ7JweaPCjbB/SQ9AaD1e+J8\
eGZDi09Z8pvZ+kmzAgMBAAGjggRTMIIETzAdBgNVHQ4EFgQUZyaW56G/2LUDnf47\
3P7yiuYV3TAwggFGBgNVHSMEggE9MIIBOYAUZyaW56G/2LUDnf473P7yiuYV3TCh\
ggEcpIIBGDCCARQxCzAJBgNVBAYTAkVTMRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQ\
BgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UEChMlSVBTIEludGVybmV0IHB1Ymxpc2hp\
bmcgU2VydmljZXMgcy5sLjErMCkGA1UEChQiaXBzQG1haWwuaXBzLmVzIEMuSS5G\
LiAgQi02MDkyOTQ1MjEvMC0GA1UECxMmSVBTIENBIENMQVNFQTEgQ2VydGlmaWNh\
dGlvbiBBdXRob3JpdHkxLzAtBgNVBAMTJklQUyBDQSBDTEFTRUExIENlcnRpZmlj\
YXRpb24gQXV0aG9yaXR5MR4wHAYJKoZIhvcNAQkBFg9pcHNAbWFpbC5pcHMuZXOC\
AQAwDAYDVR0TBAUwAwEB/zAMBgNVHQ8EBQMDB/+AMGsGA1UdJQRkMGIGCCsGAQUF\
BwMBBggrBgEFBQcDAgYIKwYBBQUHAwMGCCsGAQUFBwMEBggrBgEFBQcDCAYKKwYB\
BAGCNwIBFQYKKwYBBAGCNwIBFgYKKwYBBAGCNwoDAQYKKwYBBAGCNwoDBDARBglg\
hkgBhvhCAQEEBAMCAAcwGgYDVR0RBBMwEYEPaXBzQG1haWwuaXBzLmVzMBoGA1Ud\
EgQTMBGBD2lwc0BtYWlsLmlwcy5lczBCBglghkgBhvhCAQ0ENRYzQ0xBU0VBMSBD\
QSBDZXJ0aWZpY2F0ZSBpc3N1ZWQgYnkgaHR0cDovL3d3dy5pcHMuZXMvMCkGCWCG\
SAGG+EIBAgQcFhpodHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyLzA7BglghkgBhvhC\
AQQELhYsaHR0cDovL3d3dy5pcHMuZXMvaXBzMjAwMi9pcHMyMDAyQ0xBU0VBMS5j\
cmwwQAYJYIZIAYb4QgEDBDMWMWh0dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvcmV2\
b2NhdGlvbkNMQVNFQTEuaHRtbD8wPQYJYIZIAYb4QgEHBDAWLmh0dHA6Ly93d3cu\
aXBzLmVzL2lwczIwMDIvcmVuZXdhbENMQVNFQTEuaHRtbD8wOwYJYIZIAYb4QgEI\
BC4WLGh0dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvcG9saWN5Q0xBU0VBMS5odG1s\
MHUGA1UdHwRuMGwwMqAwoC6GLGh0dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvaXBz\
MjAwMkNMQVNFQTEuY3JsMDagNKAyhjBodHRwOi8vd3d3YmFjay5pcHMuZXMvaXBz\
MjAwMi9pcHMyMDAyQ0xBU0VBMS5jcmwwLwYIKwYBBQUHAQEEIzAhMB8GCCsGAQUF\
BzABhhNodHRwOi8vb2NzcC5pcHMuZXMvMA0GCSqGSIb3DQEBBQUAA4GBAH66iqyA\
AIQVCtWYUQxkxZwCWINmyq0eB81+atqAB98DNEock8RLWCA1NnHtogo1EqWmZaeF\
aQoO42Hu6r4okzPV7Oi+xNtff6j5YzHIa5biKcJboOeXNp13XjFr/tOn2yrb25aL\
H2betgPAK7N41lUH5Y85UN4HI3LmvSAUS7SG\
-----END CERTIFICATE-----\
\
IPS CLASEA3 root\
================\
\
MD5 Fingerprint=06:F9:EB:EC:CC:56:9D:88:BA:90:F5:BA:B0:1A:E0:02\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA CLASEA3 Certification Authority, CN=IPS CA CLASEA3 Certification Authority/emailAddress=ips@mail.ips.es\
        Validity\
            Not Before: Dec 29 01:07:50 2001 GMT\
            Not After : Dec 27 01:07:50 2025 GMT\
        Subject: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA CLASEA3 Certification Authority, CN=IPS CA CLASEA3 Certification Authority/emailAddress=ips@mail.ips.es\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:ee:80:00:f6:1a:64:2e:ad:6a:c8:83:b1:8b:a7:\
                    ee:8f:d9:b6:db:cd:1b:bb:86:06:22:76:33:0c:12:\
                    6d:48:56:61:d2:dc:82:25:62:2f:9f:d2:69:30:65:\
                    03:42:23:58:bc:47:dc:6b:d6:75:5d:17:3c:e1:ff:\
                    f2:58:67:79:a0:c1:81:b1:d4:56:a2:f2:8d:11:99:\
                    fd:f6:7d:f1:c7:c4:5e:02:2a:9a:e2:4a:b5:13:8a:\
                    00:fd:8c:77:86:e6:d7:94:f5:20:75:2e:0e:4c:bf:\
                    74:c4:3f:81:3e:83:b4:a3:38:36:29:e7:e8:2a:f5:\
                    8c:88:41:aa:80:a6:e3:6c:ef\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                1E:9F:57:50:47:B6:61:93:39:D3:2C:FC:DA:5D:3D:05:75:B7:99:02\
            X509v3 Authority Key Identifier: \
                keyid:1E:9F:57:50:47:B6:61:93:39:D3:2C:FC:DA:5D:3D:05:75:B7:99:02\
                DirName:/C=ES/ST=Barcelona/L=Barcelona/O=IPS Internet publishing Services s.l./O=ips@mail.ips.es C.I.F.  B-60929452/OU=IPS CA CLASEA3 Certification Authority/CN=IPS CA CLASEA3 Certification Authority/emailAddress=ips@mail.ips.es\
                serial:00\
\
            X509v3 Basic Constraints: \
                CA:TRUE\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Key Encipherment, Data Encipherment, Key Agreement, Certificate Sign, CRL Sign, Encipher Only, Decipher Only\
            X509v3 Extended Key Usage: \
                TLS Web Server Authentication, TLS Web Client Authentication, Code Signing, E-mail Protection, Time Stamping, Microsoft Individual Code Signing, Microsoft Commercial Code Signing, Microsoft Trust List Signing, Microsoft Encrypted File System\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Subject Alternative Name: \
                email:ips@mail.ips.es\
            X509v3 Issuer Alternative Name: \
                email:ips@mail.ips.es\
            Netscape Comment: \
                CLASEA3 CA Certificate issued by http://www.ips.es/\
            Netscape Base Url: \
                http://www.ips.es/ips2002/\
            Netscape CA Revocation Url: \
                http://www.ips.es/ips2002/ips2002CLASEA3.crl\
            Netscape Revocation Url: \
                http://www.ips.es/ips2002/revocationCLASEA3.html?\
            Netscape Renewal Url: \
                http://www.ips.es/ips2002/renewalCLASEA3.html?\
            Netscape CA Policy Url: \
                http://www.ips.es/ips2002/policyCLASEA3.html\
            X509v3 CRL Distribution Points: \
                URI:http://www.ips.es/ips2002/ips2002CLASEA3.crl\
                URI:http://wwwback.ips.es/ips2002/ips2002CLASEA3.crl\
\
            Authority Information Access: \
                OCSP - URI:http://ocsp.ips.es/\
\
    Signature Algorithm: sha1WithRSAEncryption\
        4a:3d:20:47:1a:da:89:f4:7a:2b:31:79:ec:01:c0:cc:01:f5:\
        d6:c1:fc:c8:c3:f3:50:02:51:90:58:2a:9f:e7:35:09:5b:30:\
        0a:81:00:25:47:af:d4:0f:0e:9e:60:26:a8:95:a7:83:08:df:\
        2d:ac:e9:0e:f7:9c:c8:9f:cb:93:45:f1:ba:6a:c6:67:51:4a:\
        69:4f:6b:fe:7d:0b:2f:52:29:c2:50:ad:24:44:ed:23:b3:48:\
        cb:44:40:c1:03:95:0c:0a:78:06:12:01:f5:91:31:2d:49:8d:\
        bb:3f:45:4e:2c:e0:e8:cd:b5:c9:14:15:0c:e3:07:83:9b:26:\
        75:ef\
-----BEGIN CERTIFICATE-----\
MIIH9zCCB2CgAwIBAgIBADANBgkqhkiG9w0BAQUFADCCARQxCzAJBgNVBAYTAkVT\
MRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQBgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UE\
ChMlSVBTIEludGVybmV0IHB1Ymxpc2hpbmcgU2VydmljZXMgcy5sLjErMCkGA1UE\
ChQiaXBzQG1haWwuaXBzLmVzIEMuSS5GLiAgQi02MDkyOTQ1MjEvMC0GA1UECxMm\
SVBTIENBIENMQVNFQTMgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxLzAtBgNVBAMT\
JklQUyBDQSBDTEFTRUEzIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MR4wHAYJKoZI\
hvcNAQkBFg9pcHNAbWFpbC5pcHMuZXMwHhcNMDExMjI5MDEwNzUwWhcNMjUxMjI3\
MDEwNzUwWjCCARQxCzAJBgNVBAYTAkVTMRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQ\
BgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UEChMlSVBTIEludGVybmV0IHB1Ymxpc2hp\
bmcgU2VydmljZXMgcy5sLjErMCkGA1UEChQiaXBzQG1haWwuaXBzLmVzIEMuSS5G\
LiAgQi02MDkyOTQ1MjEvMC0GA1UECxMmSVBTIENBIENMQVNFQTMgQ2VydGlmaWNh\
dGlvbiBBdXRob3JpdHkxLzAtBgNVBAMTJklQUyBDQSBDTEFTRUEzIENlcnRpZmlj\
YXRpb24gQXV0aG9yaXR5MR4wHAYJKoZIhvcNAQkBFg9pcHNAbWFpbC5pcHMuZXMw\
gZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBAO6AAPYaZC6tasiDsYun7o/ZttvN\
G7uGBiJ2MwwSbUhWYdLcgiViL5/SaTBlA0IjWLxH3GvWdV0XPOH/8lhneaDBgbHU\
VqLyjRGZ/fZ98cfEXgIqmuJKtROKAP2Md4bm15T1IHUuDky/dMQ/gT6DtKM4Ninn\
6Cr1jIhBqoCm42zvAgMBAAGjggRTMIIETzAdBgNVHQ4EFgQUHp9XUEe2YZM50yz8\
2l09BXW3mQIwggFGBgNVHSMEggE9MIIBOYAUHp9XUEe2YZM50yz82l09BXW3mQKh\
ggEcpIIBGDCCARQxCzAJBgNVBAYTAkVTMRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQ\
BgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UEChMlSVBTIEludGVybmV0IHB1Ymxpc2hp\
bmcgU2VydmljZXMgcy5sLjErMCkGA1UEChQiaXBzQG1haWwuaXBzLmVzIEMuSS5G\
LiAgQi02MDkyOTQ1MjEvMC0GA1UECxMmSVBTIENBIENMQVNFQTMgQ2VydGlmaWNh\
dGlvbiBBdXRob3JpdHkxLzAtBgNVBAMTJklQUyBDQSBDTEFTRUEzIENlcnRpZmlj\
YXRpb24gQXV0aG9yaXR5MR4wHAYJKoZIhvcNAQkBFg9pcHNAbWFpbC5pcHMuZXOC\
AQAwDAYDVR0TBAUwAwEB/zAMBgNVHQ8EBQMDB/+AMGsGA1UdJQRkMGIGCCsGAQUF\
BwMBBggrBgEFBQcDAgYIKwYBBQUHAwMGCCsGAQUFBwMEBggrBgEFBQcDCAYKKwYB\
BAGCNwIBFQYKKwYBBAGCNwIBFgYKKwYBBAGCNwoDAQYKKwYBBAGCNwoDBDARBglg\
hkgBhvhCAQEEBAMCAAcwGgYDVR0RBBMwEYEPaXBzQG1haWwuaXBzLmVzMBoGA1Ud\
EgQTMBGBD2lwc0BtYWlsLmlwcy5lczBCBglghkgBhvhCAQ0ENRYzQ0xBU0VBMyBD\
QSBDZXJ0aWZpY2F0ZSBpc3N1ZWQgYnkgaHR0cDovL3d3dy5pcHMuZXMvMCkGCWCG\
SAGG+EIBAgQcFhpodHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyLzA7BglghkgBhvhC\
AQQELhYsaHR0cDovL3d3dy5pcHMuZXMvaXBzMjAwMi9pcHMyMDAyQ0xBU0VBMy5j\
cmwwQAYJYIZIAYb4QgEDBDMWMWh0dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvcmV2\
b2NhdGlvbkNMQVNFQTMuaHRtbD8wPQYJYIZIAYb4QgEHBDAWLmh0dHA6Ly93d3cu\
aXBzLmVzL2lwczIwMDIvcmVuZXdhbENMQVNFQTMuaHRtbD8wOwYJYIZIAYb4QgEI\
BC4WLGh0dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvcG9saWN5Q0xBU0VBMy5odG1s\
MHUGA1UdHwRuMGwwMqAwoC6GLGh0dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvaXBz\
MjAwMkNMQVNFQTMuY3JsMDagNKAyhjBodHRwOi8vd3d3YmFjay5pcHMuZXMvaXBz\
MjAwMi9pcHMyMDAyQ0xBU0VBMy5jcmwwLwYIKwYBBQUHAQEEIzAhMB8GCCsGAQUF\
BzABhhNodHRwOi8vb2NzcC5pcHMuZXMvMA0GCSqGSIb3DQEBBQUAA4GBAEo9IEca\
2on0eisxeewBwMwB9dbB/MjD81ACUZBYKp/nNQlbMAqBACVHr9QPDp5gJqiVp4MI\
3y2s6Q73nMify5NF8bpqxmdRSmlPa/59Cy9SKcJQrSRE7SOzSMtEQMEDlQwKeAYS\
AfWRMS1Jjbs/RU4s4OjNtckUFQzjB4ObJnXv\
-----END CERTIFICATE-----\
\
IPS Servidores root\
===================\
\
MD5 Fingerprint=7B:B5:08:99:9A:8C:18:BF:85:27:7D:0E:AE:DA:B2:AB\
Certificate:\
    Data:\
        Version: 1 (0x0)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=ES, ST=BARCELONA, L=BARCELONA, O=IPS Seguridad CA, OU=Certificaciones, CN=IPS SERVIDORES/emailAddress=ips@mail.ips.es\
        Validity\
            Not Before: Jan  1 23:21:07 1998 GMT\
            Not After : Dec 29 23:21:07 2009 GMT\
        Subject: C=ES, ST=BARCELONA, L=BARCELONA, O=IPS Seguridad CA, OU=Certificaciones, CN=IPS SERVIDORES/emailAddress=ips@mail.ips.es\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:ac:4f:52:74:9f:39:ea:8e:dc:25:c4:bc:98:5d:\
                    98:64:24:09:3c:21:b3:cc:19:b5:8e:94:8e:87:d1:\
                    f8:37:3e:a1:c8:2d:58:a4:80:35:5b:a1:75:6c:1d:\
                    45:0c:1f:61:63:6a:5e:6f:9b:0a:4c:c1:c8:b8:61:\
                    23:35:81:ff:fe:ac:78:70:2d:68:e1:3a:07:98:95:\
                    02:54:dd:cd:23:b7:80:53:d7:c8:37:45:72:06:24:\
                    12:ba:13:61:21:8a:6e:75:28:e0:c5:0f:34:fd:36:\
                    d8:45:7f:e1:b8:36:ef:b3:e1:c6:20:8e:e8:b4:38:\
                    bc:e1:3e:f6:11:de:8c:9d:01\
                Exponent: 65537 (0x10001)\
    Signature Algorithm: md5WithRSAEncryption\
        2c:f3:c3:79:58:24:de:c6:3b:d1:e0:42:69:b8:ee:64:b3:3d:\
        62:01:b9:b3:84:df:23:7d:dd:98:cf:10:a9:fe:00:d8:22:96:\
        05:13:07:54:57:c5:a7:de:cb:d9:b8:88:42:f6:99:db:14:77:\
        1f:b6:fe:25:3d:e1:a2:3e:03:a9:81:d2:2d:6c:47:f5:96:46:\
        8c:22:ab:c8:cc:0d:0e:97:5e:8b:41:b4:3b:c4:0a:06:40:1d:\
        dd:46:f4:01:dd:ba:82:2e:3c:3d:78:70:9e:7c:18:d0:ab:f8:\
        b8:77:07:46:71:f1:ca:0b:63:5c:6a:f9:72:94:d5:01:4f:a0:\
        db:42\
-----BEGIN CERTIFICATE-----\
MIICtzCCAiACAQAwDQYJKoZIhvcNAQEEBQAwgaMxCzAJBgNVBAYTAkVTMRIwEAYD\
VQQIEwlCQVJDRUxPTkExEjAQBgNVBAcTCUJBUkNFTE9OQTEZMBcGA1UEChMQSVBT\
IFNlZ3VyaWRhZCBDQTEYMBYGA1UECxMPQ2VydGlmaWNhY2lvbmVzMRcwFQYDVQQD\
Ew5JUFMgU0VSVklET1JFUzEeMBwGCSqGSIb3DQEJARYPaXBzQG1haWwuaXBzLmVz\
MB4XDTk4MDEwMTIzMjEwN1oXDTA5MTIyOTIzMjEwN1owgaMxCzAJBgNVBAYTAkVT\
MRIwEAYDVQQIEwlCQVJDRUxPTkExEjAQBgNVBAcTCUJBUkNFTE9OQTEZMBcGA1UE\
ChMQSVBTIFNlZ3VyaWRhZCBDQTEYMBYGA1UECxMPQ2VydGlmaWNhY2lvbmVzMRcw\
FQYDVQQDEw5JUFMgU0VSVklET1JFUzEeMBwGCSqGSIb3DQEJARYPaXBzQG1haWwu\
aXBzLmVzMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCsT1J0nznqjtwlxLyY\
XZhkJAk8IbPMGbWOlI6H0fg3PqHILVikgDVboXVsHUUMH2Fjal5vmwpMwci4YSM1\
gf/+rHhwLWjhOgeYlQJU3c0jt4BT18g3RXIGJBK6E2Ehim51KODFDzT9NthFf+G4\
Nu+z4cYgjui0OLzhPvYR3oydAQIDAQABMA0GCSqGSIb3DQEBBAUAA4GBACzzw3lY\
JN7GO9HgQmm47mSzPWIBubOE3yN93ZjPEKn+ANgilgUTB1RXxafey9m4iEL2mdsU\
dx+2/iU94aI+A6mB0i1sR/WWRowiq8jMDQ6XXotBtDvECgZAHd1G9AHduoIuPD14\
cJ58GNCr+Lh3B0Zx8coLY1xq+XKU1QFPoNtC\
-----END CERTIFICATE-----\
\
IPS Timestamping root\
=====================\
\
MD5 Fingerprint=2E:03:FD:C5:F5:D7:2B:94:64:C1:BE:89:31:F1:16:9B\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA Timestamping Certification Authority, CN=IPS CA Timestamping Certification Authority/emailAddress=ips@mail.ips.es\
        Validity\
            Not Before: Dec 29 01:10:18 2001 GMT\
            Not After : Dec 27 01:10:18 2025 GMT\
        Subject: C=ES, ST=Barcelona, L=Barcelona, O=IPS Internet publishing Services s.l., O=ips@mail.ips.es C.I.F.  B-60929452, OU=IPS CA Timestamping Certification Authority, CN=IPS CA Timestamping Certification Authority/emailAddress=ips@mail.ips.es\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:bc:b8:ee:56:a5:9a:8c:e6:36:c9:c2:62:a0:66:\
                    81:8d:1a:d5:7a:d2:73:9f:0e:84:64:ba:95:b4:90:\
                    a7:78:af:ca:fe:54:61:5b:ce:b2:20:57:01:ae:44:\
                    92:43:10:38:11:f7:68:fc:17:40:a5:68:27:32:3b:\
                    c4:a7:e6:42:71:c5:99:ef:76:ff:2b:95:24:f5:49:\
                    92:18:68:ca:00:b5:a4:5a:2f:6e:cb:d6:1b:2c:0d:\
                    54:67:6b:7a:29:a1:58:ab:a2:5a:00:d6:5b:bb:18:\
                    c2:df:f6:1e:13:56:76:9b:a5:68:e2:98:ce:c6:03:\
                    8a:34:db:4c:83:41:a6:a9:a3\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                8B:D0:10:50:09:81:F2:9D:09:D5:0E:60:78:03:22:A2:3F:C8:CA:66\
            X509v3 Authority Key Identifier: \
                keyid:8B:D0:10:50:09:81:F2:9D:09:D5:0E:60:78:03:22:A2:3F:C8:CA:66\
                DirName:/C=ES/ST=Barcelona/L=Barcelona/O=IPS Internet publishing Services s.l./O=ips@mail.ips.es C.I.F.  B-60929452/OU=IPS CA Timestamping Certification Authority/CN=IPS CA Timestamping Certification Authority/emailAddress=ips@mail.ips.es\
                serial:00\
\
            X509v3 Basic Constraints: \
                CA:TRUE\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Key Encipherment, Data Encipherment, Key Agreement, Certificate Sign, CRL Sign, Encipher Only, Decipher Only\
            X509v3 Extended Key Usage: \
                TLS Web Server Authentication, TLS Web Client Authentication, Code Signing, E-mail Protection, Time Stamping, Microsoft Individual Code Signing, Microsoft Commercial Code Signing, Microsoft Trust List Signing, Microsoft Encrypted File System\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Subject Alternative Name: \
                email:ips@mail.ips.es\
            X509v3 Issuer Alternative Name: \
                email:ips@mail.ips.es\
            Netscape Comment: \
                Timestamping CA Certificate issued by http://www.ips.es/\
            Netscape Base Url: \
                http://www.ips.es/ips2002/\
            Netscape CA Revocation Url: \
                http://www.ips.es/ips2002/ips2002Timestamping.crl\
            Netscape Revocation Url: \
                http://www.ips.es/ips2002/revocationTimestamping.html?\
            Netscape Renewal Url: \
                http://www.ips.es/ips2002/renewalTimestamping.html?\
            Netscape CA Policy Url: \
                http://www.ips.es/ips2002/policyTimestamping.html\
            X509v3 CRL Distribution Points: \
                URI:http://www.ips.es/ips2002/ips2002Timestamping.crl\
                URI:http://wwwback.ips.es/ips2002/ips2002Timestamping.crl\
\
            Authority Information Access: \
                OCSP - URI:http://ocsp.ips.es/\
\
    Signature Algorithm: sha1WithRSAEncryption\
        65:ba:c1:cc:00:1a:95:91:ca:e9:6c:3a:bf:3a:1e:14:08:7c:\
        fb:83:ee:6b:62:51:d3:33:91:b5:60:79:7e:04:d8:5d:79:37:\
        e8:c3:5b:b0:c4:67:2d:68:5a:b2:5f:0e:0a:fa:cd:3f:3a:45:\
        a1:ea:36:cf:26:1e:a7:11:28:c5:94:8f:84:4c:53:08:c5:93:\
        b3:fc:e2:7f:f5:8d:f3:b1:a9:85:5f:88:de:91:96:ee:17:5b:\
        ae:a5:ea:70:65:78:2c:21:64:01:95:ce:ce:4c:3e:50:f4:b6:\
        59:cb:63:8d:b6:bd:18:d4:87:4a:5f:dc:ef:e9:56:f0:0a:0c:\
        e8:75\
-----BEGIN CERTIFICATE-----\
MIIIODCCB6GgAwIBAgIBADANBgkqhkiG9w0BAQUFADCCAR4xCzAJBgNVBAYTAkVT\
MRIwEAYDVQQIEwlCYXJjZWxvbmExEjAQBgNVBAcTCUJhcmNlbG9uYTEuMCwGA1UE\
ChMlSVBTIEludGVybmV0IHB1Ymxpc2hpbmcgU2VydmljZXMgcy5sLjErMCkGA1UE\
ChQiaXBzQG1haWwuaXBzLmVzIEMuSS5GLiAgQi02MDkyOTQ1MjE0MDIGA1UECxMr\
SVBTIENBIFRpbWVzdGFtcGluZyBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTE0MDIG\
A1UEAxMrSVBTIENBIFRpbWVzdGFtcGluZyBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0\
eTEeMBwGCSqGSIb3DQEJARYPaXBzQG1haWwuaXBzLmVzMB4XDTAxMTIyOTAxMTAx\
OFoXDTI1MTIyNzAxMTAxOFowggEeMQswCQYDVQQGEwJFUzESMBAGA1UECBMJQmFy\
Y2Vsb25hMRIwEAYDVQQHEwlCYXJjZWxvbmExLjAsBgNVBAoTJUlQUyBJbnRlcm5l\
dCBwdWJsaXNoaW5nIFNlcnZpY2VzIHMubC4xKzApBgNVBAoUImlwc0BtYWlsLmlw\
cy5lcyBDLkkuRi4gIEItNjA5Mjk0NTIxNDAyBgNVBAsTK0lQUyBDQSBUaW1lc3Rh\
bXBpbmcgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxNDAyBgNVBAMTK0lQUyBDQSBU\
aW1lc3RhbXBpbmcgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxHjAcBgkqhkiG9w0B\
CQEWD2lwc0BtYWlsLmlwcy5lczCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA\
vLjuVqWajOY2ycJioGaBjRrVetJznw6EZLqVtJCneK/K/lRhW86yIFcBrkSSQxA4\
Efdo/BdApWgnMjvEp+ZCccWZ73b/K5Uk9UmSGGjKALWkWi9uy9YbLA1UZ2t6KaFY\
q6JaANZbuxjC3/YeE1Z2m6Vo4pjOxgOKNNtMg0GmqaMCAwEAAaOCBIAwggR8MB0G\
A1UdDgQWBBSL0BBQCYHynQnVDmB4AyKiP8jKZjCCAVAGA1UdIwSCAUcwggFDgBSL\
0BBQCYHynQnVDmB4AyKiP8jKZqGCASakggEiMIIBHjELMAkGA1UEBhMCRVMxEjAQ\
BgNVBAgTCUJhcmNlbG9uYTESMBAGA1UEBxMJQmFyY2Vsb25hMS4wLAYDVQQKEyVJ\
UFMgSW50ZXJuZXQgcHVibGlzaGluZyBTZXJ2aWNlcyBzLmwuMSswKQYDVQQKFCJp\
cHNAbWFpbC5pcHMuZXMgQy5JLkYuICBCLTYwOTI5NDUyMTQwMgYDVQQLEytJUFMg\
Q0EgVGltZXN0YW1waW5nIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MTQwMgYDVQQD\
EytJUFMgQ0EgVGltZXN0YW1waW5nIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MR4w\
HAYJKoZIhvcNAQkBFg9pcHNAbWFpbC5pcHMuZXOCAQAwDAYDVR0TBAUwAwEB/zAM\
BgNVHQ8EBQMDB/+AMGsGA1UdJQRkMGIGCCsGAQUFBwMBBggrBgEFBQcDAgYIKwYB\
BQUHAwMGCCsGAQUFBwMEBggrBgEFBQcDCAYKKwYBBAGCNwIBFQYKKwYBBAGCNwIB\
FgYKKwYBBAGCNwoDAQYKKwYBBAGCNwoDBDARBglghkgBhvhCAQEEBAMCAAcwGgYD\
VR0RBBMwEYEPaXBzQG1haWwuaXBzLmVzMBoGA1UdEgQTMBGBD2lwc0BtYWlsLmlw\
cy5lczBHBglghkgBhvhCAQ0EOhY4VGltZXN0YW1waW5nIENBIENlcnRpZmljYXRl\
IGlzc3VlZCBieSBodHRwOi8vd3d3Lmlwcy5lcy8wKQYJYIZIAYb4QgECBBwWGmh0\
dHA6Ly93d3cuaXBzLmVzL2lwczIwMDIvMEAGCWCGSAGG+EIBBAQzFjFodHRwOi8v\
d3d3Lmlwcy5lcy9pcHMyMDAyL2lwczIwMDJUaW1lc3RhbXBpbmcuY3JsMEUGCWCG\
SAGG+EIBAwQ4FjZodHRwOi8vd3d3Lmlwcy5lcy9pcHMyMDAyL3Jldm9jYXRpb25U\
aW1lc3RhbXBpbmcuaHRtbD8wQgYJYIZIAYb4QgEHBDUWM2h0dHA6Ly93d3cuaXBz\
LmVzL2lwczIwMDIvcmVuZXdhbFRpbWVzdGFtcGluZy5odG1sPzBABglghkgBhvhC\
AQgEMxYxaHR0cDovL3d3dy5pcHMuZXMvaXBzMjAwMi9wb2xpY3lUaW1lc3RhbXBp\
bmcuaHRtbDB/BgNVHR8EeDB2MDegNaAzhjFodHRwOi8vd3d3Lmlwcy5lcy9pcHMy\
MDAyL2lwczIwMDJUaW1lc3RhbXBpbmcuY3JsMDugOaA3hjVodHRwOi8vd3d3YmFj\
ay5pcHMuZXMvaXBzMjAwMi9pcHMyMDAyVGltZXN0YW1waW5nLmNybDAvBggrBgEF\
BQcBAQQjMCEwHwYIKwYBBQUHMAGGE2h0dHA6Ly9vY3NwLmlwcy5lcy8wDQYJKoZI\
hvcNAQEFBQADgYEAZbrBzAAalZHK6Ww6vzoeFAh8+4Pua2JR0zORtWB5fgTYXXk3\
6MNbsMRnLWhasl8OCvrNPzpFoeo2zyYepxEoxZSPhExTCMWTs/zif/WN87GphV+I\
3pGW7hdbrqXqcGV4LCFkAZXOzkw+UPS2Wctjjba9GNSHSl/c7+lW8AoM6HU=\
-----END CERTIFICATE-----\
\
QuoVadis Root CA\
================\
\
MD5 Fingerprint=27:DE:36:FE:72:B7:00:03:00:9D:F4:F0:1E:6C:04:24\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 985026699 (0x3ab6508b)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=BM, O=QuoVadis Limited, OU=Root Certification Authority, CN=QuoVadis Root Certification Authority\
        Validity\
            Not Before: Mar 19 18:33:33 2001 GMT\
            Not After : Mar 17 18:33:33 2021 GMT\
        Subject: C=BM, O=QuoVadis Limited, OU=Root Certification Authority, CN=QuoVadis Root Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:bf:61:b5:95:53:ba:57:fc:fa:f2:67:0b:3a:1a:\
                    df:11:80:64:95:b4:d1:bc:cd:7a:cf:f6:29:96:2e:\
                    24:54:40:24:38:f7:1a:85:dc:58:4c:cb:a4:27:42:\
                    97:d0:9f:83:8a:c3:e4:06:03:5b:00:a5:51:1e:70:\
                    04:74:e2:c1:d4:3a:ab:d7:ad:3b:07:18:05:8e:fd:\
                    83:ac:ea:66:d9:18:1b:68:8a:f5:57:1a:98:ba:f5:\
                    ed:76:3d:7c:d9:de:94:6a:3b:4b:17:c1:d5:8f:bd:\
                    65:38:3a:95:d0:3d:55:36:4e:df:79:57:31:2a:1e:\
                    d8:59:65:49:58:20:98:7e:ab:5f:7e:9f:e9:d6:4d:\
                    ec:83:74:a9:c7:6c:d8:ee:29:4a:85:2a:06:14:f9:\
                    54:e6:d3:da:65:07:8b:63:37:12:d7:d0:ec:c3:7b:\
                    20:41:44:a3:ed:cb:a0:17:e1:71:65:ce:1d:66:31:\
                    f7:76:01:19:c8:7d:03:58:b6:95:49:1d:a6:12:26:\
                    e8:c6:0c:76:e0:e3:66:cb:ea:5d:a6:26:ee:e5:cc:\
                    5f:bd:67:a7:01:27:0e:a2:ca:54:c5:b1:7a:95:1d:\
                    71:1e:4a:29:8a:03:dc:6a:45:c1:a4:19:5e:6f:36:\
                    cd:c3:a2:b0:b7:fe:5c:38:e2:52:bc:f8:44:43:e6:\
                    90:bb\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            Authority Information Access: \
                OCSP - URI:https://ocsp.quovadisoffshore.com\
\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Certificate Policies: \
                Policy: 1.3.6.1.4.1.8024.0.1\
                  User Notice:\
                    Explicit Text: Reliance on the QuoVadis Root Certificate by any party assumes acceptance of the then applicable standard terms and conditions of use, certification practices, and the QuoVadis Certificate Policy.\
                  CPS: http://www.quovadis.bm\
\
            X509v3 Subject Key Identifier: \
                8B:4B:6D:ED:D3:29:B9:06:19:EC:39:39:A9:F0:97:84:6A:CB:EF:DF\
            X509v3 Authority Key Identifier: \
                keyid:8B:4B:6D:ED:D3:29:B9:06:19:EC:39:39:A9:F0:97:84:6A:CB:EF:DF\
                DirName:/C=BM/O=QuoVadis Limited/OU=Root Certification Authority/CN=QuoVadis Root Certification Authority\
                serial:3A:B6:50:8B\
\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        8a:d4:14:b5:fe:f4:9a:92:a7:19:d4:a4:7e:72:18:8f:d9:68:\
        7c:52:24:dd:67:6f:39:7a:c4:aa:5e:3d:e2:58:b0:4d:70:98:\
        84:61:e8:1b:e3:69:18:0e:ce:fb:47:50:a0:4e:ff:f0:24:1f:\
        bd:b2:ce:f5:27:fc:ec:2f:53:aa:73:7b:03:3d:74:6e:e6:16:\
        9e:eb:a5:2e:c4:bf:56:27:50:2b:62:ba:be:4b:1c:3c:55:5c:\
        41:1d:24:be:82:20:47:5d:d5:44:7e:7a:16:68:df:7d:4d:51:\
        70:78:57:1d:33:1e:fd:02:99:9c:0c:cd:0a:05:4f:c7:bb:8e:\
        a4:75:fa:4a:6d:b1:80:8e:09:56:b9:9c:1a:60:fe:5d:c1:d7:\
        7a:dc:11:78:d0:d6:5d:c1:b7:d5:ad:32:99:03:3a:8a:cc:54:\
        25:39:31:81:7b:13:22:51:ba:46:6c:a1:bb:9e:fa:04:6c:49:\
        26:74:8f:d2:73:eb:cc:30:a2:e6:ea:59:22:87:f8:97:f5:0e:\
        fd:ea:cc:92:a4:16:c4:52:18:ea:21:ce:b1:f1:e6:84:81:e5:\
        ba:a9:86:28:f2:43:5a:5d:12:9d:ac:1e:d9:a8:e5:0a:6a:a7:\
        7f:a0:87:29:cf:f2:89:4d:d4:ec:c5:e2:e6:7a:d0:36:23:8a:\
        4a:74:36:f9\
-----BEGIN CERTIFICATE-----\
MIIF0DCCBLigAwIBAgIEOrZQizANBgkqhkiG9w0BAQUFADB/MQswCQYDVQQGEwJC\
TTEZMBcGA1UEChMQUXVvVmFkaXMgTGltaXRlZDElMCMGA1UECxMcUm9vdCBDZXJ0\
aWZpY2F0aW9uIEF1dGhvcml0eTEuMCwGA1UEAxMlUXVvVmFkaXMgUm9vdCBDZXJ0\
aWZpY2F0aW9uIEF1dGhvcml0eTAeFw0wMTAzMTkxODMzMzNaFw0yMTAzMTcxODMz\
MzNaMH8xCzAJBgNVBAYTAkJNMRkwFwYDVQQKExBRdW9WYWRpcyBMaW1pdGVkMSUw\
IwYDVQQLExxSb290IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MS4wLAYDVQQDEyVR\
dW9WYWRpcyBSb290IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIIBIjANBgkqhkiG\
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAv2G1lVO6V/z68mcLOhrfEYBklbTRvM16z/Yp\
li4kVEAkOPcahdxYTMukJ0KX0J+DisPkBgNbAKVRHnAEdOLB1Dqr1607BxgFjv2D\
rOpm2RgbaIr1VxqYuvXtdj182d6UajtLF8HVj71lODqV0D1VNk7feVcxKh7YWWVJ\
WCCYfqtffp/p1k3sg3Spx2zY7ilKhSoGFPlU5tPaZQeLYzcS19Dsw3sgQUSj7cug\
F+FxZc4dZjH3dgEZyH0DWLaVSR2mEiboxgx24ONmy+pdpibu5cxfvWenAScOospU\
xbF6lR1xHkopigPcakXBpBlebzbNw6Kwt/5cOOJSvPhEQ+aQuwIDAQABo4ICUjCC\
Ak4wPQYIKwYBBQUHAQEEMTAvMC0GCCsGAQUFBzABhiFodHRwczovL29jc3AucXVv\
dmFkaXNvZmZzaG9yZS5jb20wDwYDVR0TAQH/BAUwAwEB/zCCARoGA1UdIASCAREw\
ggENMIIBCQYJKwYBBAG+WAABMIH7MIHUBggrBgEFBQcCAjCBxxqBxFJlbGlhbmNl\
IG9uIHRoZSBRdW9WYWRpcyBSb290IENlcnRpZmljYXRlIGJ5IGFueSBwYXJ0eSBh\
c3N1bWVzIGFjY2VwdGFuY2Ugb2YgdGhlIHRoZW4gYXBwbGljYWJsZSBzdGFuZGFy\
ZCB0ZXJtcyBhbmQgY29uZGl0aW9ucyBvZiB1c2UsIGNlcnRpZmljYXRpb24gcHJh\
Y3RpY2VzLCBhbmQgdGhlIFF1b1ZhZGlzIENlcnRpZmljYXRlIFBvbGljeS4wIgYI\
KwYBBQUHAgEWFmh0dHA6Ly93d3cucXVvdmFkaXMuYm0wHQYDVR0OBBYEFItLbe3T\
KbkGGew5Oanwl4Rqy+/fMIGuBgNVHSMEgaYwgaOAFItLbe3TKbkGGew5Oanwl4Rq\
y+/foYGEpIGBMH8xCzAJBgNVBAYTAkJNMRkwFwYDVQQKExBRdW9WYWRpcyBMaW1p\
dGVkMSUwIwYDVQQLExxSb290IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MS4wLAYD\
VQQDEyVRdW9WYWRpcyBSb290IENlcnRpZmljYXRpb24gQXV0aG9yaXR5ggQ6tlCL\
MA4GA1UdDwEB/wQEAwIBBjANBgkqhkiG9w0BAQUFAAOCAQEAitQUtf70mpKnGdSk\
fnIYj9lofFIk3WdvOXrEql494liwTXCYhGHoG+NpGA7O+0dQoE7/8CQfvbLO9Sf8\
7C9TqnN7Az10buYWnuulLsS/VidQK2K6vkscPFVcQR0kvoIgR13VRH56FmjffU1R\
cHhXHTMe/QKZnAzNCgVPx7uOpHX6Sm2xgI4JVrmcGmD+XcHXetwReNDWXcG31a0y\
mQM6isxUJTkxgXsTIlG6Rmyhu576BGxJJnSP0nPrzDCi5upZIof4l/UO/erMkqQW\
xFIY6iHOsfHmhIHluqmGKPJDWl0Snawe2ajlCmqnf6CHKc/yiU3U7MXi5nrQNiOK\
SnQ2+Q==\
-----END CERTIFICATE-----\
\
Security Communication Root CA\
==============================\
\
MD5 Fingerprint=F1:BC:63:6A:54:E0:B5:27:F5:CD:E7:1A:E3:4D:6E:4A\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=JP, O=SECOM Trust.net, OU=Security Communication RootCA1\
        Validity\
            Not Before: Sep 30 04:20:49 2003 GMT\
            Not After : Sep 30 04:20:49 2023 GMT\
        Subject: C=JP, O=SECOM Trust.net, OU=Security Communication RootCA1\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b3:b3:fe:7f:d3:6d:b1:ef:16:7c:57:a5:0c:6d:\
                    76:8a:2f:4b:bf:64:fb:4c:ee:8a:f0:f3:29:7c:f5:\
                    ff:ee:2a:e0:e9:e9:ba:5b:64:22:9a:9a:6f:2c:3a:\
                    26:69:51:05:99:26:dc:d5:1c:6a:71:c6:9a:7d:1e:\
                    9d:dd:7c:6c:c6:8c:67:67:4a:3e:f8:71:b0:19:27:\
                    a9:09:0c:a6:95:bf:4b:8c:0c:fa:55:98:3b:d8:e8:\
                    22:a1:4b:71:38:79:ac:97:92:69:b3:89:7e:ea:21:\
                    68:06:98:14:96:87:d2:61:36:bc:6d:27:56:9e:57:\
                    ee:c0:c0:56:fd:32:cf:a4:d9:8e:c2:23:d7:8d:a8:\
                    f3:d8:25:ac:97:e4:70:38:f4:b6:3a:b4:9d:3b:97:\
                    26:43:a3:a1:bc:49:59:72:4c:23:30:87:01:58:f6:\
                    4e:be:1c:68:56:66:af:cd:41:5d:c8:b3:4d:2a:55:\
                    46:ab:1f:da:1e:e2:40:3d:db:cd:7d:b9:92:80:9c:\
                    37:dd:0c:96:64:9d:dc:22:f7:64:8b:df:61:de:15:\
                    94:52:15:a0:7d:52:c9:4b:a8:21:c9:c6:b1:ed:cb:\
                    c3:95:60:d1:0f:f0:ab:70:f8:df:cb:4d:7e:ec:d6:\
                    fa:ab:d9:bd:7f:54:f2:a5:e9:79:fa:d9:d6:76:24:\
                    28:73\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                A0:73:49:99:68:DC:85:5B:65:E3:9B:28:2F:57:9F:BD:33:BC:07:48\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
    Signature Algorithm: sha1WithRSAEncryption\
        68:40:a9:a8:bb:e4:4f:5d:79:b3:05:b5:17:b3:60:13:eb:c6:\
        92:5d:e0:d1:d3:6a:fe:fb:be:9b:6d:bf:c7:05:6d:59:20:c4:\
        1c:f0:b7:da:84:58:02:63:fa:48:16:ef:4f:a5:0b:f7:4a:98:\
        f2:3f:9e:1b:ad:47:6b:63:ce:08:47:eb:52:3f:78:9c:af:4d:\
        ae:f8:d5:4f:cf:9a:98:2a:10:41:39:52:c4:dd:d9:9b:0e:ef:\
        93:01:ae:b2:2e:ca:68:42:24:42:6c:b0:b3:3a:3e:cd:e9:da:\
        48:c4:15:cb:e9:f9:07:0f:92:50:49:8a:dd:31:97:5f:c9:e9:\
        37:aa:3b:59:65:97:94:32:c9:b3:9f:3e:3a:62:58:c5:49:ad:\
        62:0e:71:a5:32:aa:2f:c6:89:76:43:40:13:13:67:3d:a2:54:\
        25:10:cb:f1:3a:f2:d9:fa:db:49:56:bb:a6:fe:a7:41:35:c3:\
        e0:88:61:c9:88:c7:df:36:10:22:98:59:ea:b0:4a:fb:56:16:\
        73:6e:ac:4d:f7:22:a1:4f:ad:1d:7a:2d:45:27:e5:30:c1:5e:\
        f2:da:13:cb:25:42:51:95:47:03:8c:6c:21:cc:74:42:ed:53:\
        ff:33:8b:8f:0f:57:01:16:2f:cf:a6:ee:c9:70:22:14:bd:fd:\
        be:6c:0b:03\
-----BEGIN CERTIFICATE-----\
MIIDWjCCAkKgAwIBAgIBADANBgkqhkiG9w0BAQUFADBQMQswCQYDVQQGEwJKUDEY\
MBYGA1UEChMPU0VDT00gVHJ1c3QubmV0MScwJQYDVQQLEx5TZWN1cml0eSBDb21t\
dW5pY2F0aW9uIFJvb3RDQTEwHhcNMDMwOTMwMDQyMDQ5WhcNMjMwOTMwMDQyMDQ5\
WjBQMQswCQYDVQQGEwJKUDEYMBYGA1UEChMPU0VDT00gVHJ1c3QubmV0MScwJQYD\
VQQLEx5TZWN1cml0eSBDb21tdW5pY2F0aW9uIFJvb3RDQTEwggEiMA0GCSqGSIb3\
DQEBAQUAA4IBDwAwggEKAoIBAQCzs/5/022x7xZ8V6UMbXaKL0u/ZPtM7orw8yl8\
9f/uKuDp6bpbZCKamm8sOiZpUQWZJtzVHGpxxpp9Hp3dfGzGjGdnSj74cbAZJ6kJ\
DKaVv0uMDPpVmDvY6CKhS3E4eayXkmmziX7qIWgGmBSWh9JhNrxtJ1aeV+7AwFb9\
Ms+k2Y7CI9eNqPPYJayX5HA49LY6tJ07lyZDo6G8SVlyTCMwhwFY9k6+HGhWZq/N\
QV3Is00qVUarH9oe4kA92819uZKAnDfdDJZkndwi92SL32HeFZRSFaB9UslLqCHJ\
xrHty8OVYNEP8Ktw+N/LTX7s1vqr2b1/VPKl6Xn62dZ2JChzAgMBAAGjPzA9MB0G\
A1UdDgQWBBSgc0mZaNyFW2XjmygvV5+9M7wHSDALBgNVHQ8EBAMCAQYwDwYDVR0T\
AQH/BAUwAwEB/zANBgkqhkiG9w0BAQUFAAOCAQEAaECpqLvkT115swW1F7NgE+vG\
kl3g0dNq/vu+m22/xwVtWSDEHPC32oRYAmP6SBbvT6UL90qY8j+eG61Ha2POCEfr\
Uj94nK9NrvjVT8+amCoQQTlSxN3Zmw7vkwGusi7KaEIkQmywszo+zenaSMQVy+n5\
Bw+SUEmK3TGXX8npN6o7WWWXlDLJs58+OmJYxUmtYg5xpTKqL8aJdkNAExNnPaJU\
JRDL8Try2frbSVa7pv6nQTXD4IhhyYjH3zYQIphZ6rBK+1YWc26sTfcioU+tHXot\
RSflMMFe8toTyyVCUZVHA4xsIcx0Qu1T/zOLjw9XARYvz6buyXAiFL39vmwLAw==\
-----END CERTIFICATE-----\
\
Sonera Class 1 Root CA\
======================\
\
MD5 Fingerprint=33:B7:84:F5:5F:27:D7:68:27:DE:14:DE:12:2A:ED:6F\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 36 (0x24)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=FI, O=Sonera, CN=Sonera Class1 CA\
        Validity\
            Not Before: Apr  6 10:49:13 2001 GMT\
            Not After : Apr  6 10:49:13 2021 GMT\
        Subject: C=FI, O=Sonera, CN=Sonera Class1 CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b5:89:1f:2b:4f:67:0a:79:ff:c5:1e:f8:7f:3c:\
                    ed:d1:7e:da:b0:cd:6d:2f:36:ac:34:c6:db:d9:64:\
                    17:08:63:30:33:22:8a:4c:ee:8e:bb:0f:0d:42:55:\
                    c9:9d:2e:a5:ef:f7:a7:8c:c3:ab:b9:97:cb:8e:ef:\
                    3f:15:67:a8:82:72:63:53:0f:41:8c:7d:10:95:24:\
                    a1:5a:a5:06:fa:92:57:9d:fa:a5:01:f2:75:e9:1f:\
                    bc:56:26:52:4e:78:19:65:58:55:03:58:c0:14:ae:\
                    8c:7c:55:5f:70:5b:77:23:06:36:97:f3:24:b5:9a:\
                    46:95:e4:df:0d:0b:05:45:e5:d1:f2:1d:82:bb:c6:\
                    13:e0:fe:aa:7a:fd:69:30:94:f3:d2:45:85:fc:f2:\
                    32:5b:32:de:e8:6c:5d:1f:cb:a4:22:74:b0:80:8e:\
                    5d:94:f7:06:00:4b:a9:d4:5e:2e:35:50:09:f3:80:\
                    97:f4:0c:17:ae:39:d8:5f:cd:33:c1:1c:ca:89:c2:\
                    22:f7:45:12:ed:5e:12:93:9d:63:ab:82:2e:b9:eb:\
                    42:41:44:cb:4a:1a:00:82:0d:9e:f9:8b:57:3e:4c:\
                    c7:17:ed:2c:8b:72:33:5f:72:7a:38:56:d5:e6:d9:\
                    ae:05:1a:1d:75:45:b1:cb:a5:25:1c:12:57:36:fd:\
                    22:37\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                47:E2:0C:8B:F6:53:88:52\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        8b:1a:b2:c9:5d:61:b4:e1:b9:2b:b9:53:d1:b2:85:9d:77:8e:\
        16:ee:11:3d:db:c2:63:d9:5b:97:65:fb:12:67:d8:2a:5c:b6:\
        ab:e5:5e:c3:b7:16:2f:c8:e8:ab:1d:8a:fd:ab:1a:7c:d5:5f:\
        63:cf:dc:b0:dd:77:b9:a8:e6:d2:22:38:87:07:14:d9:ff:be:\
        56:b5:fd:07:0e:3c:55:ca:16:cc:a7:a6:77:37:fb:db:5c:1f:\
        4e:59:06:87:a3:03:43:f5:16:ab:b7:84:bd:4e:ef:9f:31:37:\
        f0:46:f1:40:b6:d1:0c:a5:64:f8:63:5e:21:db:55:4e:4f:31:\
        76:9c:10:61:8e:b6:53:3a:a3:11:be:af:6d:7c:1e:bd:ae:2d:\
        e2:0c:69:c7:85:53:68:a2:61:ba:c5:3e:b4:79:54:78:9e:0a:\
        c7:02:be:62:d1:11:82:4b:65:2f:91:5a:c2:a8:87:b1:56:68:\
        94:79:f9:25:f7:c1:d5:ae:1a:b8:bb:3d:8f:a9:8a:38:15:f7:\
        73:d0:5a:60:d1:80:b0:f0:dc:d5:50:cd:4e:ee:92:48:69:ed:\
        b2:23:1e:30:cc:c8:94:c8:b6:f5:3b:86:7f:3f:a6:2e:9f:f6:\
        3e:2c:b5:92:96:3e:df:2c:93:8a:ff:81:8c:0f:0f:59:21:19:\
        57:bd:55:9a\
-----BEGIN CERTIFICATE-----\
MIIDIDCCAgigAwIBAgIBJDANBgkqhkiG9w0BAQUFADA5MQswCQYDVQQGEwJGSTEP\
MA0GA1UEChMGU29uZXJhMRkwFwYDVQQDExBTb25lcmEgQ2xhc3MxIENBMB4XDTAx\
MDQwNjEwNDkxM1oXDTIxMDQwNjEwNDkxM1owOTELMAkGA1UEBhMCRkkxDzANBgNV\
BAoTBlNvbmVyYTEZMBcGA1UEAxMQU29uZXJhIENsYXNzMSBDQTCCASIwDQYJKoZI\
hvcNAQEBBQADggEPADCCAQoCggEBALWJHytPZwp5/8Ue+H887dF+2rDNbS82rDTG\
29lkFwhjMDMiikzujrsPDUJVyZ0upe/3p4zDq7mXy47vPxVnqIJyY1MPQYx9EJUk\
oVqlBvqSV536pQHydekfvFYmUk54GWVYVQNYwBSujHxVX3BbdyMGNpfzJLWaRpXk\
3w0LBUXl0fIdgrvGE+D+qnr9aTCU89JFhfzyMlsy3uhsXR/LpCJ0sICOXZT3BgBL\
qdReLjVQCfOAl/QMF6452F/NM8EcyonCIvdFEu1eEpOdY6uCLrnrQkFEy0oaAIIN\
nvmLVz5MxxftLItyM19yejhW1ebZrgUaHXVFsculJRwSVzb9IjcCAwEAAaMzMDEw\
DwYDVR0TAQH/BAUwAwEB/zARBgNVHQ4ECgQIR+IMi/ZTiFIwCwYDVR0PBAQDAgEG\
MA0GCSqGSIb3DQEBBQUAA4IBAQCLGrLJXWG04bkruVPRsoWdd44W7hE928Jj2VuX\
ZfsSZ9gqXLar5V7DtxYvyOirHYr9qxp81V9jz9yw3Xe5qObSIjiHBxTZ/75Wtf0H\
DjxVyhbMp6Z3N/vbXB9OWQaHowND9Rart4S9Tu+fMTfwRvFAttEMpWT4Y14h21VO\
TzF2nBBhjrZTOqMRvq9tfB69ri3iDGnHhVNoomG6xT60eVR4ngrHAr5i0RGCS2Uv\
kVrCqIexVmiUefkl98HVrhq4uz2PqYo4Ffdz0Fpg0YCw8NzVUM1O7pJIae2yIx4w\
zMiUyLb1O4Z/P6Yun/Y+LLWSlj7fLJOK/4GMDw9ZIRlXvVWa\
-----END CERTIFICATE-----\
\
Sonera Class 2 Root CA\
======================\
\
MD5 Fingerprint=A3:EC:75:0F:2E:88:DF:FA:48:01:4E:0B:5C:48:6F:FB\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 29 (0x1d)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=FI, O=Sonera, CN=Sonera Class2 CA\
        Validity\
            Not Before: Apr  6 07:29:40 2001 GMT\
            Not After : Apr  6 07:29:40 2021 GMT\
        Subject: C=FI, O=Sonera, CN=Sonera Class2 CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:90:17:4a:35:9d:ca:f0:0d:96:c7:44:fa:16:37:\
                    fc:48:bd:bd:7f:80:2d:35:3b:e1:6f:a8:67:a9:bf:\
                    03:1c:4d:8c:6f:32:47:d5:41:68:a4:13:04:c1:35:\
                    0c:9a:84:43:fc:5c:1d:ff:89:b3:e8:17:18:cd:91:\
                    5f:fb:89:e3:ea:bf:4e:5d:7c:1b:26:d3:75:79:ed:\
                    e6:84:e3:57:e5:ad:29:c4:f4:3a:28:e7:a5:7b:84:\
                    36:69:b3:fd:5e:76:bd:a3:2d:99:d3:90:4e:23:28:\
                    7d:18:63:f1:54:3b:26:9d:76:5b:97:42:b2:ff:ae:\
                    f0:4e:ec:dd:39:95:4e:83:06:7f:e7:49:40:c8:c5:\
                    01:b2:54:5a:66:1d:3d:fc:f9:e9:3c:0a:9e:81:b8:\
                    70:f0:01:8b:e4:23:54:7c:c8:ae:f8:90:1e:00:96:\
                    72:d4:54:cf:61:23:bc:ea:fb:9d:02:95:d1:b6:b9:\
                    71:3a:69:08:3f:0f:b4:e1:42:c7:88:f5:3f:98:a8:\
                    a7:ba:1c:e0:71:71:ef:58:57:81:50:7a:5c:6b:74:\
                    46:0e:83:03:98:c3:8e:a8:6e:f2:76:32:6e:27:83:\
                    c2:73:f3:dc:18:e8:b4:93:ea:75:44:6b:04:60:20:\
                    71:57:87:9d:f3:be:a0:90:23:3d:8a:24:e1:da:21:\
                    db:c3\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                4A:A0:AA:58:84:D3:5E:3C\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
    Signature Algorithm: sha1WithRSAEncryption\
        5a:ce:87:f9:16:72:15:57:4b:1d:d9:9b:e7:a2:26:30:ec:93:\
        67:df:d6:2d:d2:34:af:f7:38:a5:ce:ab:16:b9:ab:2f:7c:35:\
        cb:ac:d0:0f:b4:4c:2b:fc:80:ef:6b:8c:91:5f:36:76:f7:db:\
        b3:1b:19:ea:f4:b2:11:fd:61:71:44:bf:28:b3:3a:1d:bf:b3:\
        43:e8:9f:bf:dc:31:08:71:b0:9d:8d:d6:34:47:32:90:c6:65:\
        24:f7:a0:4a:7c:04:73:8f:39:6f:17:8c:72:b5:bd:4b:c8:7a:\
        f8:7b:83:c3:28:4e:9c:09:ea:67:3f:b2:67:04:1b:c3:14:da:\
        f8:e7:49:24:91:d0:1d:6a:fa:61:39:ef:6b:e7:21:75:06:07:\
        d8:12:b4:21:20:70:42:71:81:da:3c:9a:36:be:a6:5b:0d:6a:\
        6c:9a:1f:91:7b:f9:f9:ef:42:ba:4e:4e:9e:cc:0c:8d:94:dc:\
        d9:45:9c:5e:ec:42:50:63:ae:f4:5d:c4:b1:12:dc:ca:3b:a8:\
        2e:9d:14:5a:05:75:b7:ec:d7:63:e2:ba:35:b6:04:08:91:e8:\
        da:9d:9c:f6:66:b5:18:ac:0a:a6:54:26:34:33:d2:1b:c1:d4:\
        7f:1a:3a:8e:0b:aa:32:6e:db:fc:4f:25:9f:d9:32:c7:96:5a:\
        70:ac:df:4c\
-----BEGIN CERTIFICATE-----\
MIIDIDCCAgigAwIBAgIBHTANBgkqhkiG9w0BAQUFADA5MQswCQYDVQQGEwJGSTEP\
MA0GA1UEChMGU29uZXJhMRkwFwYDVQQDExBTb25lcmEgQ2xhc3MyIENBMB4XDTAx\
MDQwNjA3Mjk0MFoXDTIxMDQwNjA3Mjk0MFowOTELMAkGA1UEBhMCRkkxDzANBgNV\
BAoTBlNvbmVyYTEZMBcGA1UEAxMQU29uZXJhIENsYXNzMiBDQTCCASIwDQYJKoZI\
hvcNAQEBBQADggEPADCCAQoCggEBAJAXSjWdyvANlsdE+hY3/Ei9vX+ALTU74W+o\
Z6m/AxxNjG8yR9VBaKQTBME1DJqEQ/xcHf+Js+gXGM2RX/uJ4+q/Tl18GybTdXnt\
5oTjV+WtKcT0OijnpXuENmmz/V52vaMtmdOQTiMofRhj8VQ7Jp12W5dCsv+u8E7s\
3TmVToMGf+dJQMjFAbJUWmYdPfz56TwKnoG4cPABi+QjVHzIrviQHgCWctRUz2Ej\
vOr7nQKV0ba5cTppCD8PtOFCx4j1P5iop7oc4HFx71hXgVB6XGt0Rg6DA5jDjqhu\
8nYybieDwnPz3BjotJPqdURrBGAgcVeHnfO+oJAjPYok4doh28MCAwEAAaMzMDEw\
DwYDVR0TAQH/BAUwAwEB/zARBgNVHQ4ECgQISqCqWITTXjwwCwYDVR0PBAQDAgEG\
MA0GCSqGSIb3DQEBBQUAA4IBAQBazof5FnIVV0sd2ZvnoiYw7JNn39Yt0jSv9zil\
zqsWuasvfDXLrNAPtEwr/IDva4yRXzZ299uzGxnq9LIR/WFxRL8oszodv7ND6J+/\
3DEIcbCdjdY0RzKQxmUk96BKfARzjzlvF4xytb1LyHr4e4PDKE6cCepnP7JnBBvD\
FNr450kkkdAdavphOe9r5yF1BgfYErQhIHBCcYHaPJo2vqZbDWpsmh+Re/n570K6\
Tk6ezAyNlNzZRZxe7EJQY670XcSxEtzKO6gunRRaBXW37Ndj4ro1tgQIkejanZz2\
ZrUYrAqmVCY0M9IbwdR/GjqOC6oybtv8TyWf2TLHllpwrN9M\
-----END CERTIFICATE-----\
\
Thawte Premium Server CA\
========================\
\
MD5 Fingerprint=06:9F:69:79:16:66:90:02:1B:8C:8C:A2:C3:07:6F:3A\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1 (0x1)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting cc, OU=Certification Services Division, CN=Thawte Premium Server CA/emailAddress=premium-server@thawte.com\
        Validity\
            Not Before: Aug  1 00:00:00 1996 GMT\
            Not After : Dec 31 23:59:59 2020 GMT\
        Subject: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting cc, OU=Certification Services Division, CN=Thawte Premium Server CA/emailAddress=premium-server@thawte.com\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:d2:36:36:6a:8b:d7:c2:5b:9e:da:81:41:62:8f:\
                    38:ee:49:04:55:d6:d0:ef:1c:1b:95:16:47:ef:18:\
                    48:35:3a:52:f4:2b:6a:06:8f:3b:2f:ea:56:e3:af:\
                    86:8d:9e:17:f7:9e:b4:65:75:02:4d:ef:cb:09:a2:\
                    21:51:d8:9b:d0:67:d0:ba:0d:92:06:14:73:d4:93:\
                    cb:97:2a:00:9c:5c:4e:0c:bc:fa:15:52:fc:f2:44:\
                    6e:da:11:4a:6e:08:9f:2f:2d:e3:f9:aa:3a:86:73:\
                    b6:46:53:58:c8:89:05:bd:83:11:b8:73:3f:aa:07:\
                    8d:f4:42:4d:e7:40:9d:1c:37\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
    Signature Algorithm: md5WithRSAEncryption\
        26:48:2c:16:c2:58:fa:e8:16:74:0c:aa:aa:5f:54:3f:f2:d7:\
        c9:78:60:5e:5e:6e:37:63:22:77:36:7e:b2:17:c4:34:b9:f5:\
        08:85:fc:c9:01:38:ff:4d:be:f2:16:42:43:e7:bb:5a:46:fb:\
        c1:c6:11:1f:f1:4a:b0:28:46:c9:c3:c4:42:7d:bc:fa:ab:59:\
        6e:d5:b7:51:88:11:e3:a4:85:19:6b:82:4c:a4:0c:12:ad:e9:\
        a4:ae:3f:f1:c3:49:65:9a:8c:c5:c8:3e:25:b7:94:99:bb:92:\
        32:71:07:f0:86:5e:ed:50:27:a6:0d:a6:23:f9:bb:cb:a6:07:\
        14:42\
-----BEGIN CERTIFICATE-----\
MIIDJzCCApCgAwIBAgIBATANBgkqhkiG9w0BAQQFADCBzjELMAkGA1UEBhMCWkEx\
FTATBgNVBAgTDFdlc3Rlcm4gQ2FwZTESMBAGA1UEBxMJQ2FwZSBUb3duMR0wGwYD\
VQQKExRUaGF3dGUgQ29uc3VsdGluZyBjYzEoMCYGA1UECxMfQ2VydGlmaWNhdGlv\
biBTZXJ2aWNlcyBEaXZpc2lvbjEhMB8GA1UEAxMYVGhhd3RlIFByZW1pdW0gU2Vy\
dmVyIENBMSgwJgYJKoZIhvcNAQkBFhlwcmVtaXVtLXNlcnZlckB0aGF3dGUuY29t\
MB4XDTk2MDgwMTAwMDAwMFoXDTIwMTIzMTIzNTk1OVowgc4xCzAJBgNVBAYTAlpB\
MRUwEwYDVQQIEwxXZXN0ZXJuIENhcGUxEjAQBgNVBAcTCUNhcGUgVG93bjEdMBsG\
A1UEChMUVGhhd3RlIENvbnN1bHRpbmcgY2MxKDAmBgNVBAsTH0NlcnRpZmljYXRp\
b24gU2VydmljZXMgRGl2aXNpb24xITAfBgNVBAMTGFRoYXd0ZSBQcmVtaXVtIFNl\
cnZlciBDQTEoMCYGCSqGSIb3DQEJARYZcHJlbWl1bS1zZXJ2ZXJAdGhhd3RlLmNv\
bTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA0jY2aovXwlue2oFBYo847kkE\
VdbQ7xwblRZH7xhINTpS9CtqBo87L+pW46+GjZ4X9560ZXUCTe/LCaIhUdib0GfQ\
ug2SBhRz1JPLlyoAnFxODLz6FVL88kRu2hFKbgifLy3j+ao6hnO2RlNYyIkFvYMR\
uHM/qgeN9EJN50CdHDcCAwEAAaMTMBEwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG\
9w0BAQQFAAOBgQAmSCwWwlj66BZ0DKqqX1Q/8tfJeGBeXm43YyJ3Nn6yF8Q0ufUI\
hfzJATj/Tb7yFkJD57taRvvBxhEf8UqwKEbJw8RCfbz6q1lu1bdRiBHjpIUZa4JM\
pAwSremkrj/xw0llmozFyD4lt5SZu5IycQfwhl7tUCemDaYj+bvLpgcUQg==\
-----END CERTIFICATE-----\
\
Staat der Nederlanden Root CA\
=============================\
\
MD5 Fingerprint=60:84:7C:5A:CE:DB:0C:D4:CB:A7:E9:FE:02:C6:A9:C0\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 10000010 (0x98968a)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=NL, O=Staat der Nederlanden, CN=Staat der Nederlanden Root CA\
        Validity\
            Not Before: Dec 17 09:23:49 2002 GMT\
            Not After : Dec 16 09:15:38 2015 GMT\
        Subject: C=NL, O=Staat der Nederlanden, CN=Staat der Nederlanden Root CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:98:d2:b5:51:11:7a:81:a6:14:98:71:6d:be:cc:\
                    e7:13:1b:d6:27:0e:7a:b3:6a:18:1c:b6:61:5a:d5:\
                    61:09:bf:de:90:13:c7:67:ee:dd:f3:da:c5:0c:12:\
                    9e:35:55:3e:2c:27:88:40:6b:f7:dc:dd:22:61:f5:\
                    c2:c7:0e:f5:f6:d5:76:53:4d:8f:8c:bc:18:76:37:\
                    85:9d:e8:ca:49:c7:d2:4f:98:13:09:a2:3e:22:88:\
                    9c:7f:d6:f2:10:65:b4:ee:5f:18:d5:17:e3:f8:c5:\
                    fd:e2:9d:a2:ef:53:0e:85:77:a2:0f:e1:30:47:ee:\
                    00:e7:33:7d:44:67:1a:0b:51:e8:8b:a0:9e:50:98:\
                    68:34:52:1f:2e:6d:01:f2:60:45:f2:31:eb:a9:31:\
                    68:29:bb:7a:41:9e:c6:19:7f:94:b4:51:39:03:7f:\
                    b2:de:a7:32:9b:b4:47:8e:6f:b4:4a:ae:e5:af:b1:\
                    dc:b0:1b:61:bc:99:72:de:e4:89:b7:7a:26:5d:da:\
                    33:49:5b:52:9c:0e:f5:8a:ad:c3:b8:3d:e8:06:6a:\
                    c2:d5:2a:0b:6c:7b:84:bd:56:05:cb:86:65:92:ec:\
                    44:2b:b0:8e:b9:dc:70:0b:46:da:ad:bc:63:88:39:\
                    fa:db:6a:fe:23:fa:bc:e4:48:f4:67:2b:6a:11:10:\
                    21:49\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: \
                CA:TRUE\
            X509v3 Certificate Policies: \
                Policy: 2.5.29.32.0\
                  CPS: http://www.pkioverheid.nl/policies/root-policy\
\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Subject Key Identifier: \
                A8:7D:EB:BC:63:A4:74:13:74:00:EC:96:E0:D3:34:C1:2C:BF:6C:F8\
    Signature Algorithm: sha1WithRSAEncryption\
        05:84:87:55:74:36:61:c1:bb:d1:d4:c6:15:a8:13:b4:9f:a4:\
        fe:bb:ee:15:b4:2f:06:0c:29:f2:a8:92:a4:61:0d:fc:ab:5c:\
        08:5b:51:13:2b:4d:c2:2a:61:c8:f8:09:58:fc:2d:02:b2:39:\
        7d:99:66:81:bf:6e:5c:95:45:20:6c:e6:79:a7:d1:d8:1c:29:\
        fc:c2:20:27:51:c8:f1:7c:5d:34:67:69:85:11:30:c6:00:d2:\
        d7:f3:d3:7c:b6:f0:31:57:28:12:82:73:e9:33:2f:a6:55:b4:\
        0b:91:94:47:9c:fa:bb:7a:42:32:e8:ae:7e:2d:c8:bc:ac:14:\
        bf:d9:0f:d9:5b:fc:c1:f9:7a:95:e1:7d:7e:96:fc:71:b0:c2:\
        4c:c8:df:45:34:c9:ce:0d:f2:9c:64:08:d0:3b:c3:29:c5:b2:\
        ed:90:04:c1:b1:29:91:c5:30:6f:c1:a9:72:33:cc:fe:5d:16:\
        17:2c:11:69:e7:7e:fe:c5:83:08:df:bc:dc:22:3a:2e:20:69:\
        23:39:56:60:67:90:8b:2e:76:39:fb:11:88:97:f6:7c:bd:4b:\
        b8:20:16:67:05:8d:e2:3b:c1:72:3f:94:95:37:c7:5d:b9:9e:\
        d8:93:a1:17:8f:ff:0c:66:15:c1:24:7c:32:7c:03:1d:3b:a1:\
        58:45:32:93\
-----BEGIN CERTIFICATE-----\
MIIDujCCAqKgAwIBAgIEAJiWijANBgkqhkiG9w0BAQUFADBVMQswCQYDVQQGEwJO\
TDEeMBwGA1UEChMVU3RhYXQgZGVyIE5lZGVybGFuZGVuMSYwJAYDVQQDEx1TdGFh\
dCBkZXIgTmVkZXJsYW5kZW4gUm9vdCBDQTAeFw0wMjEyMTcwOTIzNDlaFw0xNTEy\
MTYwOTE1MzhaMFUxCzAJBgNVBAYTAk5MMR4wHAYDVQQKExVTdGFhdCBkZXIgTmVk\
ZXJsYW5kZW4xJjAkBgNVBAMTHVN0YWF0IGRlciBOZWRlcmxhbmRlbiBSb290IENB\
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAmNK1URF6gaYUmHFtvszn\
ExvWJw56s2oYHLZhWtVhCb/ekBPHZ+7d89rFDBKeNVU+LCeIQGv33N0iYfXCxw71\
9tV2U02PjLwYdjeFnejKScfST5gTCaI+Ioicf9byEGW07l8Y1Rfj+MX94p2i71MO\
hXeiD+EwR+4A5zN9RGcaC1Hoi6CeUJhoNFIfLm0B8mBF8jHrqTFoKbt6QZ7GGX+U\
tFE5A3+y3qcym7RHjm+0Sq7lr7HcsBthvJly3uSJt3omXdozSVtSnA71iq3DuD3o\
BmrC1SoLbHuEvVYFy4ZlkuxEK7COudxwC0barbxjiDn622r+I/q85Ej0ZytqERAh\
SQIDAQABo4GRMIGOMAwGA1UdEwQFMAMBAf8wTwYDVR0gBEgwRjBEBgRVHSAAMDww\
OgYIKwYBBQUHAgEWLmh0dHA6Ly93d3cucGtpb3ZlcmhlaWQubmwvcG9saWNpZXMv\
cm9vdC1wb2xpY3kwDgYDVR0PAQH/BAQDAgEGMB0GA1UdDgQWBBSofeu8Y6R0E3QA\
7Jbg0zTBLL9s+DANBgkqhkiG9w0BAQUFAAOCAQEABYSHVXQ2YcG70dTGFagTtJ+k\
/rvuFbQvBgwp8qiSpGEN/KtcCFtREytNwiphyPgJWPwtArI5fZlmgb9uXJVFIGzm\
eafR2Bwp/MIgJ1HI8XxdNGdphREwxgDS1/PTfLbwMVcoEoJz6TMvplW0C5GUR5z6\
u3pCMuiufi3IvKwUv9kP2Vv8wfl6leF9fpb8cbDCTMjfRTTJzg3ynGQI0DvDKcWy\
7ZAEwbEpkcUwb8GpcjPM/l0WFywRaed+/sWDCN+83CI6LiBpIzlWYGeQiy52OfsR\
iJf2fL1LuCAWZwWN4jvBcj+UlTfHXbme2JOhF4//DGYVwSR8MnwDHTuhWEUykw==\
-----END CERTIFICATE-----\
\
TDC Internet Root CA\
====================\
\
MD5 Fingerprint=91:F4:03:55:20:A1:F8:63:2C:62:DE:AC:FB:61:1C:8E\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 986490188 (0x3acca54c)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=DK, O=TDC Internet, OU=TDC Internet Root CA\
        Validity\
            Not Before: Apr  5 16:33:17 2001 GMT\
            Not After : Apr  5 17:03:17 2021 GMT\
        Subject: C=DK, O=TDC Internet, OU=TDC Internet Root CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:c4:b8:40:bc:91:d5:63:1f:d7:99:a0:8b:0c:40:\
                    1e:74:b7:48:9d:46:8c:02:b2:e0:24:5f:f0:19:13:\
                    a7:37:83:6b:5d:c7:8e:f9:84:30:ce:1a:3b:fa:fb:\
                    ce:8b:6d:23:c6:c3:6e:66:9f:89:a5:df:e0:42:50:\
                    67:fa:1f:6c:1e:f4:d0:05:d6:bf:ca:d6:4e:e4:68:\
                    60:6c:46:aa:1c:5d:63:e1:07:86:0e:65:00:a7:2e:\
                    a6:71:c6:bc:b9:81:a8:3a:7d:1a:d2:f9:d1:ac:4b:\
                    cb:ce:75:af:dc:7b:fa:81:73:d4:fc:ba:bd:41:88:\
                    d4:74:b3:f9:5e:38:3a:3c:43:a8:d2:95:4e:77:6d:\
                    13:0c:9d:8f:78:01:b7:5a:20:1f:03:37:35:e2:2c:\
                    db:4b:2b:2c:78:b9:49:db:c4:d0:c7:9c:9c:e4:8a:\
                    20:09:21:16:56:66:ff:05:ec:5b:e3:f0:cf:ab:24:\
                    24:5e:c3:7f:70:7a:12:c4:d2:b5:10:a0:b6:21:e1:\
                    8d:78:69:55:44:69:f5:ca:96:1c:34:85:17:25:77:\
                    e2:f6:2f:27:98:78:fd:79:06:3a:a2:d6:5a:43:c1:\
                    ff:ec:04:3b:ee:13:ef:d3:58:5a:ff:92:eb:ec:ae:\
                    da:f2:37:03:47:41:b6:97:c9:2d:0a:41:22:bb:bb:\
                    e6:a7\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 CRL Distribution Points: \
                DirName:/C=DK/O=TDC Internet/OU=TDC Internet Root CA/CN=CRL1\
\
            X509v3 Private Key Usage Period: \
                Not Before: Apr  5 16:33:17 2001 GMT, Not After: Apr  5 17:03:17 2021 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:6C:64:01:C7:FD:85:6D:AC:C8:DA:9E:50:08:85:08:B5:3C:56:A8:50\
\
            X509v3 Subject Key Identifier: \
                6C:64:01:C7:FD:85:6D:AC:C8:DA:9E:50:08:85:08:B5:3C:56:A8:50\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0...V5.0:4.0....\
    Signature Algorithm: sha1WithRSAEncryption\
        4e:43:cc:d1:dd:1d:10:1b:06:7f:b7:a4:fa:d3:d9:4d:fb:23:\
        9f:23:54:5b:e6:8b:2f:04:28:8b:b5:27:6d:89:a1:ec:98:69:\
        dc:e7:8d:26:83:05:79:74:ec:b4:b9:a3:97:c1:35:00:fd:15:\
        da:39:81:3a:95:31:90:de:97:e9:86:a8:99:77:0c:e5:5a:a0:\
        84:ff:12:16:ac:6e:b8:8d:c3:7b:92:c2:ac:2e:d0:7d:28:ec:\
        b6:f3:60:38:69:6f:3e:d8:04:55:3e:9e:cc:55:d2:ba:fe:bb:\
        47:04:d7:0a:d9:16:0a:34:29:f5:58:13:d5:4f:cf:8f:56:4b:\
        b3:1e:ee:d3:98:79:da:08:1e:0c:6f:b8:f8:16:27:ef:c2:6f:\
        3d:f6:a3:4b:3e:0e:e4:6d:6c:db:3b:41:12:9b:bd:0d:47:23:\
        7f:3c:4a:d0:af:c0:af:f6:ef:1b:b5:15:c4:eb:83:c4:09:5f:\
        74:8b:d9:11:fb:c2:56:b1:3c:f8:70:ca:34:8d:43:40:13:8c:\
        fd:99:03:54:79:c6:2e:ea:86:a1:f6:3a:d4:09:bc:f4:bc:66:\
        cc:3d:58:d0:57:49:0a:ee:25:e2:41:ee:13:f9:9b:38:34:d1:\
        00:f5:7e:e7:94:1d:fc:69:03:62:b8:99:05:05:3d:6b:78:12:\
        bd:b0:6f:65\
-----BEGIN CERTIFICATE-----\
MIIEKzCCAxOgAwIBAgIEOsylTDANBgkqhkiG9w0BAQUFADBDMQswCQYDVQQGEwJE\
SzEVMBMGA1UEChMMVERDIEludGVybmV0MR0wGwYDVQQLExRUREMgSW50ZXJuZXQg\
Um9vdCBDQTAeFw0wMTA0MDUxNjMzMTdaFw0yMTA0MDUxNzAzMTdaMEMxCzAJBgNV\
BAYTAkRLMRUwEwYDVQQKEwxUREMgSW50ZXJuZXQxHTAbBgNVBAsTFFREQyBJbnRl\
cm5ldCBSb290IENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxLhA\
vJHVYx/XmaCLDEAedLdInUaMArLgJF/wGROnN4NrXceO+YQwzho7+vvOi20jxsNu\
Zp+Jpd/gQlBn+h9sHvTQBda/ytZO5GhgbEaqHF1j4QeGDmUApy6mcca8uYGoOn0a\
0vnRrEvLznWv3Hv6gXPU/Lq9QYjUdLP5Xjg6PEOo0pVOd20TDJ2PeAG3WiAfAzc1\
4izbSysseLlJ28TQx5yc5IogCSEWVmb/Bexb4/DPqyQkXsN/cHoSxNK1EKC2IeGN\
eGlVRGn1ypYcNIUXJXfi9i8nmHj9eQY6otZaQ8H/7AQ77hPv01ha/5Lr7K7a8jcD\
R0G2l8ktCkEiu7vmpwIDAQABo4IBJTCCASEwEQYJYIZIAYb4QgEBBAQDAgAHMGUG\
A1UdHwReMFwwWqBYoFakVDBSMQswCQYDVQQGEwJESzEVMBMGA1UEChMMVERDIElu\
dGVybmV0MR0wGwYDVQQLExRUREMgSW50ZXJuZXQgUm9vdCBDQTENMAsGA1UEAxME\
Q1JMMTArBgNVHRAEJDAigA8yMDAxMDQwNTE2MzMxN1qBDzIwMjEwNDA1MTcwMzE3\
WjALBgNVHQ8EBAMCAQYwHwYDVR0jBBgwFoAUbGQBx/2FbazI2p5QCIUItTxWqFAw\
HQYDVR0OBBYEFGxkAcf9hW2syNqeUAiFCLU8VqhQMAwGA1UdEwQFMAMBAf8wHQYJ\
KoZIhvZ9B0EABBAwDhsIVjUuMDo0LjADAgSQMA0GCSqGSIb3DQEBBQUAA4IBAQBO\
Q8zR3R0QGwZ/t6T609lN+yOfI1Rb5osvBCiLtSdtiaHsmGnc540mgwV5dOy0uaOX\
wTUA/RXaOYE6lTGQ3pfphqiZdwzlWqCE/xIWrG64jcN7ksKsLtB9KOy282A4aW8+\
2ARVPp7MVdK6/rtHBNcK2RYKNCn1WBPVT8+PVkuzHu7TmHnaCB4Mb7j4Fifvwm89\
9qNLPg7kbWzbO0ESm70NRyN/PErQr8Cv9u8btRXE64PECV90i9kR+8JWsTz4cMo0\
jUNAE4z9mQNUecYu6oah9jrUCbz0vGbMPVjQV0kK7iXiQe4T+Zs4NNEA9X7nlB38\
aQNiuJkFBT1reBK9sG9l\
-----END CERTIFICATE-----\
\
TDC OCES Root CA\
================\
\
MD5 Fingerprint=93:7F:90:1C:ED:84:67:17:A4:65:5F:9B:CB:30:02:97\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 1044954564 (0x3e48bdc4)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=DK, O=TDC, CN=TDC OCES CA\
        Validity\
            Not Before: Feb 11 08:39:30 2003 GMT\
            Not After : Feb 11 09:09:30 2037 GMT\
        Subject: C=DK, O=TDC, CN=TDC OCES CA\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:ac:62:f6:61:20:b2:cf:c0:c6:85:d7:e3:79:e6:\
                    cc:ed:f2:39:92:a4:97:2e:64:a3:84:5b:87:9c:4c:\
                    fd:a4:f3:c4:5f:21:bd:56:10:eb:db:2e:61:ec:93:\
                    69:e3:a3:cc:bd:99:c3:05:fc:06:b8:ca:36:1c:fe:\
                    90:8e:49:4c:c4:56:9a:2f:56:bc:cf:7b:0c:f1:6f:\
                    47:a6:0d:43:4d:e2:e9:1d:39:34:cd:8d:2c:d9:12:\
                    98:f9:e3:e1:c1:4a:7c:86:38:c4:a9:c4:61:88:d2:\
                    5e:af:1a:26:4d:d5:e4:a0:22:47:84:d9:64:b7:19:\
                    96:fc:ec:19:e4:b2:97:26:4e:4a:4c:cb:8f:24:8b:\
                    54:18:1c:48:61:7b:d5:88:68:da:5d:b5:ea:cd:1a:\
                    30:c1:80:83:76:50:aa:4f:d1:d4:dd:38:f0:ef:16:\
                    f4:e1:0c:50:06:bf:ea:fb:7a:49:a1:28:2b:1c:f6:\
                    fc:15:32:a3:74:6a:8f:a9:c3:62:29:71:31:e5:3b:\
                    a4:60:17:5e:74:e6:da:13:ed:e9:1f:1f:1b:d1:b2:\
                    68:73:c6:10:34:75:46:10:10:e3:90:00:76:40:cb:\
                    8b:b7:43:09:21:ff:ab:4e:93:c6:58:e9:a5:82:db:\
                    77:c4:3a:99:b1:72:95:49:04:f0:b7:2b:fa:7b:59:\
                    8e:dd\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Certificate Policies: \
                Policy: 1.2.208.169.1.1.1\
                  CPS: http://www.certifikat.dk/repository\
                  User Notice:\
                    Organization: TDC\
                    Number: 1\
                    Explicit Text: Certifikater fra denne CA udstedes under OID 1.2.208.169.1.1.1. Certificates from this CA are issued under OID 1.2.208.169.1.1.1.\
\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 CRL Distribution Points: \
                DirName:/C=DK/O=TDC/CN=TDC OCES CA/CN=CRL1\
                URI:http://crl.oces.certifikat.dk/oces.crl\
\
            X509v3 Private Key Usage Period: \
                Not Before: Feb 11 08:39:30 2003 GMT, Not After: Feb 11 09:09:30 2037 GMT\
            X509v3 Authority Key Identifier: \
                keyid:60:B5:85:EC:56:64:7E:12:19:27:67:1D:50:15:4B:73:AE:3B:F9:12\
\
            X509v3 Subject Key Identifier: \
                60:B5:85:EC:56:64:7E:12:19:27:67:1D:50:15:4B:73:AE:3B:F9:12\
            1.2.840.113533.7.65.0: \
                0...V6.0:4.0....\
    Signature Algorithm: sha1WithRSAEncryption\
        0a:ba:26:26:46:d3:73:a8:09:f3:6b:0b:30:99:fd:8a:e1:57:\
        7a:11:d3:b8:94:d7:09:10:6e:a3:b1:38:03:d1:b6:f2:43:41:\
        29:62:a7:72:d8:fb:7c:05:e6:31:70:27:54:18:4e:8a:7c:4e:\
        e5:d1:ca:8c:78:88:cf:1b:d3:90:8b:e6:23:f8:0b:0e:33:43:\
        7d:9c:e2:0a:19:8f:c9:01:3e:74:5d:74:c9:8b:1c:03:e5:18:\
        c8:01:4c:3f:cb:97:05:5d:98:71:a6:98:6f:b6:7c:bd:37:7f:\
        be:e1:93:25:6d:6f:f0:0a:ad:17:18:e1:03:bc:07:29:c8:ad:\
        26:e8:f8:61:f0:fd:21:09:7e:9a:8e:a9:68:7d:48:62:72:bd:\
        00:ea:01:99:b8:06:82:51:81:4e:f1:f5:b4:91:54:b9:23:7a:\
        00:9a:9f:5d:8d:e0:3c:64:b9:1a:12:92:2a:c7:82:44:72:39:\
        dc:e2:3c:c6:d8:55:f5:15:4e:c8:05:0e:db:c6:d0:62:a6:ec:\
        15:b4:b5:02:82:db:ac:8c:a2:81:f0:9b:99:31:f5:20:20:a8:\
        88:61:0a:07:9f:94:fc:d0:d7:1b:cc:2e:17:f3:04:27:76:67:\
        eb:54:83:fd:a4:90:7e:06:3d:04:a3:43:2d:da:fc:0b:62:ea:\
        2f:5f:62:53\
-----BEGIN CERTIFICATE-----\
MIIFGTCCBAGgAwIBAgIEPki9xDANBgkqhkiG9w0BAQUFADAxMQswCQYDVQQGEwJE\
SzEMMAoGA1UEChMDVERDMRQwEgYDVQQDEwtUREMgT0NFUyBDQTAeFw0wMzAyMTEw\
ODM5MzBaFw0zNzAyMTEwOTA5MzBaMDExCzAJBgNVBAYTAkRLMQwwCgYDVQQKEwNU\
REMxFDASBgNVBAMTC1REQyBPQ0VTIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A\
MIIBCgKCAQEArGL2YSCyz8DGhdfjeebM7fI5kqSXLmSjhFuHnEz9pPPEXyG9VhDr\
2y5h7JNp46PMvZnDBfwGuMo2HP6QjklMxFaaL1a8z3sM8W9Hpg1DTeLpHTk0zY0s\
2RKY+ePhwUp8hjjEqcRhiNJerxomTdXkoCJHhNlktxmW/OwZ5LKXJk5KTMuPJItU\
GBxIYXvViGjaXbXqzRowwYCDdlCqT9HU3Tjw7xb04QxQBr/q+3pJoSgrHPb8FTKj\
dGqPqcNiKXEx5TukYBdedObaE+3pHx8b0bJoc8YQNHVGEBDjkAB2QMuLt0MJIf+r\
TpPGWOmlgtt3xDqZsXKVSQTwtyv6e1mO3QIDAQABo4ICNzCCAjMwDwYDVR0TAQH/\
BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwgewGA1UdIASB5DCB4TCB3gYIKoFQgSkB\
AQEwgdEwLwYIKwYBBQUHAgEWI2h0dHA6Ly93d3cuY2VydGlmaWthdC5kay9yZXBv\
c2l0b3J5MIGdBggrBgEFBQcCAjCBkDAKFgNUREMwAwIBARqBgUNlcnRpZmlrYXRl\
ciBmcmEgZGVubmUgQ0EgdWRzdGVkZXMgdW5kZXIgT0lEIDEuMi4yMDguMTY5LjEu\
MS4xLiBDZXJ0aWZpY2F0ZXMgZnJvbSB0aGlzIENBIGFyZSBpc3N1ZWQgdW5kZXIg\
T0lEIDEuMi4yMDguMTY5LjEuMS4xLjARBglghkgBhvhCAQEEBAMCAAcwgYEGA1Ud\
HwR6MHgwSKBGoESkQjBAMQswCQYDVQQGEwJESzEMMAoGA1UEChMDVERDMRQwEgYD\
VQQDEwtUREMgT0NFUyBDQTENMAsGA1UEAxMEQ1JMMTAsoCqgKIYmaHR0cDovL2Ny\
bC5vY2VzLmNlcnRpZmlrYXQuZGsvb2Nlcy5jcmwwKwYDVR0QBCQwIoAPMjAwMzAy\
MTEwODM5MzBagQ8yMDM3MDIxMTA5MDkzMFowHwYDVR0jBBgwFoAUYLWF7FZkfhIZ\
J2cdUBVLc647+RIwHQYDVR0OBBYEFGC1hexWZH4SGSdnHVAVS3OuO/kSMB0GCSqG\
SIb2fQdBAAQQMA4bCFY2LjA6NC4wAwIEkDANBgkqhkiG9w0BAQUFAAOCAQEACrom\
JkbTc6gJ82sLMJn9iuFXehHTuJTXCRBuo7E4A9G28kNBKWKnctj7fAXmMXAnVBhO\
inxO5dHKjHiIzxvTkIvmI/gLDjNDfZziChmPyQE+dF10yYscA+UYyAFMP8uXBV2Y\
caaYb7Z8vTd/vuGTJW1v8AqtFxjhA7wHKcitJuj4YfD9IQl+mo6paH1IYnK9AOoB\
mbgGglGBTvH1tJFUuSN6AJqfXY3gPGS5GhKSKseCRHI53OI8xthV9RVOyAUO28bQ\
YqbsFbS1AoLbrIyigfCbmTH1ICCoiGEKB5+U/NDXG8wuF/MEJ3Zn61SD/aSQfgY9\
BKNDLdr8C2LqL19iUw==\
-----END CERTIFICATE-----\
\
UTN DATACorp SGC Root CA\
========================\
\
MD5 Fingerprint=B3:A5:3E:77:21:6D:AC:4A:C0:C9:FB:D5:41:3D:CA:06\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            44:be:0c:8b:50:00:21:b4:11:d3:2a:68:06:a9:ad:69\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN - DATACorp SGC\
        Validity\
            Not Before: Jun 24 18:57:21 1999 GMT\
            Not After : Jun 24 19:06:30 2019 GMT\
        Subject: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN - DATACorp SGC\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:df:ee:58:10:a2:2b:6e:55:c4:8e:bf:2e:46:09:\
                    e7:e0:08:0f:2e:2b:7a:13:94:1b:bd:f6:b6:80:8e:\
                    65:05:93:00:1e:bc:af:e2:0f:8e:19:0d:12:47:ec:\
                    ac:ad:a3:fa:2e:70:f8:de:6e:fb:56:42:15:9e:2e:\
                    5c:ef:23:de:21:b9:05:76:27:19:0f:4f:d6:c3:9c:\
                    b4:be:94:19:63:f2:a6:11:0a:eb:53:48:9c:be:f2:\
                    29:3b:16:e8:1a:a0:4c:a6:c9:f4:18:59:68:c0:70:\
                    f2:53:00:c0:5e:50:82:a5:56:6f:36:f9:4a:e0:44:\
                    86:a0:4d:4e:d6:47:6e:49:4a:cb:67:d7:a6:c4:05:\
                    b9:8e:1e:f4:fc:ff:cd:e7:36:e0:9c:05:6c:b2:33:\
                    22:15:d0:b4:e0:cc:17:c0:b2:c0:f4:fe:32:3f:29:\
                    2a:95:7b:d8:f2:a7:4e:0f:54:7c:a1:0d:80:b3:09:\
                    03:c1:ff:5c:dd:5e:9a:3e:bc:ae:bc:47:8a:6a:ae:\
                    71:ca:1f:b1:2a:b8:5f:42:05:0b:ec:46:30:d1:72:\
                    0b:ca:e9:56:6d:f5:ef:df:78:be:61:ba:b2:a5:ae:\
                    04:4c:bc:a8:ac:69:15:97:bd:ef:eb:b4:8c:bf:35:\
                    f8:d4:c3:d1:28:0e:5c:3a:9f:70:18:33:20:77:c4:\
                    a2:af\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                53:32:D1:B3:CF:7F:FA:E0:F1:A0:5D:85:4E:92:D2:9E:45:1D:B4:4F\
            X509v3 CRL Distribution Points: \
                URI:http://crl.usertrust.com/UTN-DATACorpSGC.crl\
\
            X509v3 Extended Key Usage: \
                TLS Web Server Authentication, Microsoft Server Gated Crypto, Netscape Server Gated Crypto\
    Signature Algorithm: sha1WithRSAEncryption\
        27:35:97:00:8a:8b:28:bd:c6:33:30:1e:29:fc:e2:f7:d5:98:\
        d4:40:bb:60:ca:bf:ab:17:2c:09:36:7f:50:fa:41:dc:ae:96:\
        3a:0a:23:3e:89:59:c9:a3:07:ed:1b:37:ad:fc:7c:be:51:49:\
        5a:de:3a:0a:54:08:16:45:c2:99:b1:87:cd:8c:68:e0:69:03:\
        e9:c4:4e:98:b2:3b:8c:16:b3:0e:a0:0c:98:50:9b:93:a9:70:\
        09:c8:2c:a3:8f:df:02:e4:e0:71:3a:f1:b4:23:72:a0:aa:01:\
        df:df:98:3e:14:50:a0:31:26:bd:28:e9:5a:30:26:75:f9:7b:\
        60:1c:8d:f3:cd:50:26:6d:04:27:9a:df:d5:0d:45:47:29:6b:\
        2c:e6:76:d9:a9:29:7d:32:dd:c9:36:3c:bd:ae:35:f1:11:9e:\
        1d:bb:90:3f:12:47:4e:8e:d7:7e:0f:62:73:1d:52:26:38:1c:\
        18:49:fd:30:74:9a:c4:e5:22:2f:d8:c0:8d:ed:91:7a:4c:00:\
        8f:72:7f:5d:da:dd:1b:8b:45:6b:e7:dd:69:97:a8:c5:56:4c:\
        0f:0c:f6:9f:7a:91:37:f6:97:82:e0:dd:71:69:ff:76:3f:60:\
        4d:3c:cf:f7:99:f9:c6:57:f4:c9:55:39:78:ba:2c:79:c9:a6:\
        88:2b:f4:08\
-----BEGIN CERTIFICATE-----\
MIIEXjCCA0agAwIBAgIQRL4Mi1AAIbQR0ypoBqmtaTANBgkqhkiG9w0BAQUFADCB\
kzELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAlVUMRcwFQYDVQQHEw5TYWx0IExha2Ug\
Q2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMSEwHwYDVQQLExho\
dHRwOi8vd3d3LnVzZXJ0cnVzdC5jb20xGzAZBgNVBAMTElVUTiAtIERBVEFDb3Jw\
IFNHQzAeFw05OTA2MjQxODU3MjFaFw0xOTA2MjQxOTA2MzBaMIGTMQswCQYDVQQG\
EwJVUzELMAkGA1UECBMCVVQxFzAVBgNVBAcTDlNhbHQgTGFrZSBDaXR5MR4wHAYD\
VQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxITAfBgNVBAsTGGh0dHA6Ly93d3cu\
dXNlcnRydXN0LmNvbTEbMBkGA1UEAxMSVVROIC0gREFUQUNvcnAgU0dDMIIBIjAN\
BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA3+5YEKIrblXEjr8uRgnn4AgPLit6\
E5Qbvfa2gI5lBZMAHryv4g+OGQ0SR+ysraP6LnD43m77VkIVni5c7yPeIbkFdicZ\
D0/Ww5y0vpQZY/KmEQrrU0icvvIpOxboGqBMpsn0GFlowHDyUwDAXlCCpVZvNvlK\
4ESGoE1O1kduSUrLZ9emxAW5jh70/P/N5zbgnAVssjMiFdC04MwXwLLA9P4yPykq\
lXvY8qdOD1R8oQ2AswkDwf9c3V6aPryuvEeKaq5xyh+xKrhfQgUL7EYw0XILyulW\
bfXv33i+Ybqypa4ETLyorGkVl73v67SMvzX41MPRKA5cOp9wGDMgd8SirwIDAQAB\
o4GrMIGoMAsGA1UdDwQEAwIBxjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBRT\
MtGzz3/64PGgXYVOktKeRR20TzA9BgNVHR8ENjA0MDKgMKAuhixodHRwOi8vY3Js\
LnVzZXJ0cnVzdC5jb20vVVROLURBVEFDb3JwU0dDLmNybDAqBgNVHSUEIzAhBggr\
BgEFBQcDAQYKKwYBBAGCNwoDAwYJYIZIAYb4QgQBMA0GCSqGSIb3DQEBBQUAA4IB\
AQAnNZcAiosovcYzMB4p/OL31ZjUQLtgyr+rFywJNn9Q+kHcrpY6CiM+iVnJowft\
Gzet/Hy+UUla3joKVAgWRcKZsYfNjGjgaQPpxE6YsjuMFrMOoAyYUJuTqXAJyCyj\
j98C5OBxOvG0I3KgqgHf35g+FFCgMSa9KOlaMCZ1+XtgHI3zzVAmbQQnmt/VDUVH\
KWss5nbZqSl9Mt3JNjy9rjXxEZ4du5A/EkdOjtd+D2JzHVImOBwYSf0wdJrE5SIv\
2MCN7ZF6TACPcn9d2t0bi0Vr591pl6jFVkwPDPafepE39peC4N1xaf92P2BNPM/3\
mfnGV/TJVTl4uix5yaaIK/QI\
-----END CERTIFICATE-----\
\
UTN USERFirst Email Root CA\
===========================\
\
MD5 Fingerprint=D7:34:3D:EF:1D:27:09:28:E1:31:02:5B:13:2B:DD:F7\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            44:be:0c:8b:50:00:24:b4:11:d3:36:25:25:67:c9:89\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN-USERFirst-Client Authentication and Email\
        Validity\
            Not Before: Jul  9 17:28:50 1999 GMT\
            Not After : Jul  9 17:36:58 2019 GMT\
        Subject: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN-USERFirst-Client Authentication and Email\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b2:39:85:a4:f2:7d:ab:41:3b:62:46:37:ae:cd:\
                    c1:60:75:bc:39:65:f9:4a:1a:47:a2:b9:cc:48:cc:\
                    6a:98:d5:4d:35:19:b9:a4:42:e5:ce:49:e2:8a:2f:\
                    1e:7c:d2:31:07:c7:4e:b4:83:64:9d:2e:29:d5:a2:\
                    64:c4:85:bd:85:51:35:79:a4:4e:68:90:7b:1c:7a:\
                    a4:92:a8:17:f2:98:15:f2:93:cc:c9:a4:32:95:bb:\
                    0c:4f:30:bd:98:a0:0b:8b:e5:6e:1b:a2:46:fa:78:\
                    bc:a2:6f:ab:59:5e:a5:2f:cf:ca:da:6d:aa:2f:eb:\
                    ac:a1:b3:6a:aa:b7:2e:67:35:8b:79:e1:1e:69:88:\
                    e2:e6:46:cd:a0:a5:ea:be:0b:ce:76:3a:7a:0e:9b:\
                    ea:fc:da:27:5b:3d:73:1f:22:e6:48:61:c6:4c:f3:\
                    69:b1:a8:2e:1b:b6:d4:31:20:2c:bc:82:8a:8e:a4:\
                    0e:a5:d7:89:43:fc:16:5a:af:1d:71:d7:11:59:da:\
                    ba:87:0d:af:fa:f3:e1:c2:f0:a4:c5:67:8c:d6:d6:\
                    54:3a:de:0a:a4:ba:03:77:b3:65:c8:fd:1e:d3:74:\
                    62:aa:18:ca:68:93:1e:a1:85:7e:f5:47:65:cb:f8:\
                    4d:57:28:74:d2:34:ff:30:b6:ee:f6:62:30:14:8c:\
                    2c:eb\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                89:82:67:7D:C4:9D:26:70:00:4B:B4:50:48:7C:DE:3D:AE:04:6E:7D\
            X509v3 CRL Distribution Points: \
                URI:http://crl.usertrust.com/UTN-USERFirst-ClientAuthenticationandEmail.crl\
\
            X509v3 Extended Key Usage: \
                TLS Web Client Authentication, E-mail Protection\
    Signature Algorithm: sha1WithRSAEncryption\
        b1:6d:61:5d:a6:1a:7f:7c:ab:4a:e4:30:fc:53:6f:25:24:c6:\
        ca:ed:e2:31:5c:2b:0e:ee:ee:61:55:6f:04:3e:cf:39:de:c5:\
        1b:49:94:e4:eb:20:4c:b4:e6:9e:50:2e:72:d9:8d:f5:aa:a3:\
        b3:4a:da:56:1c:60:97:80:dc:82:a2:ad:4a:bd:8a:2b:ff:0b:\
        09:b4:c6:d7:20:04:45:e4:cd:80:01:ba:ba:2b:6e:ce:aa:d7:\
        92:fe:e4:af:eb:f4:26:1d:16:2a:7f:6c:30:95:37:2f:33:12:\
        ac:7f:dd:c7:d1:11:8c:51:98:b2:d0:a3:91:d0:ad:f6:9f:9e:\
        83:93:1e:1d:42:b8:46:af:6b:66:f0:9b:7f:ea:e3:03:02:e5:\
        02:51:c1:aa:d5:35:9d:72:40:03:89:ba:31:1d:c5:10:68:52:\
        9e:df:a2:85:c5:5c:08:a6:78:e6:53:4f:b1:e8:b7:d3:14:9e:\
        93:a6:c3:64:e3:ac:7e:71:cd:bc:9f:e9:03:1b:cc:fb:e9:ac:\
        31:c1:af:7c:15:74:02:99:c3:b2:47:a6:c2:32:61:d7:c7:6f:\
        48:24:51:27:a1:d5:87:55:f2:7b:8f:98:3d:16:9e:ee:75:b6:\
        f8:d0:8e:f2:f3:c6:ae:28:5b:a7:f0:f3:36:17:fc:c3:05:d3:\
        ca:03:4a:54\
-----BEGIN CERTIFICATE-----\
MIIEojCCA4qgAwIBAgIQRL4Mi1AAJLQR0zYlJWfJiTANBgkqhkiG9w0BAQUFADCB\
rjELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAlVUMRcwFQYDVQQHEw5TYWx0IExha2Ug\
Q2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMSEwHwYDVQQLExho\
dHRwOi8vd3d3LnVzZXJ0cnVzdC5jb20xNjA0BgNVBAMTLVVUTi1VU0VSRmlyc3Qt\
Q2xpZW50IEF1dGhlbnRpY2F0aW9uIGFuZCBFbWFpbDAeFw05OTA3MDkxNzI4NTBa\
Fw0xOTA3MDkxNzM2NThaMIGuMQswCQYDVQQGEwJVUzELMAkGA1UECBMCVVQxFzAV\
BgNVBAcTDlNhbHQgTGFrZSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5l\
dHdvcmsxITAfBgNVBAsTGGh0dHA6Ly93d3cudXNlcnRydXN0LmNvbTE2MDQGA1UE\
AxMtVVROLVVTRVJGaXJzdC1DbGllbnQgQXV0aGVudGljYXRpb24gYW5kIEVtYWls\
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsjmFpPJ9q0E7YkY3rs3B\
YHW8OWX5ShpHornMSMxqmNVNNRm5pELlzkniii8efNIxB8dOtINknS4p1aJkxIW9\
hVE1eaROaJB7HHqkkqgX8pgV8pPMyaQylbsMTzC9mKALi+VuG6JG+ni8om+rWV6l\
L8/K2m2qL+usobNqqrcuZzWLeeEeaYji5kbNoKXqvgvOdjp6Dpvq/NonWz1zHyLm\
SGHGTPNpsaguG7bUMSAsvIKKjqQOpdeJQ/wWWq8dcdcRWdq6hw2v+vPhwvCkxWeM\
1tZUOt4KpLoDd7NlyP0e03RiqhjKaJMeoYV+9Udly/hNVyh00jT/MLbu9mIwFIws\
6wIDAQABo4G5MIG2MAsGA1UdDwQEAwIBxjAPBgNVHRMBAf8EBTADAQH/MB0GA1Ud\
DgQWBBSJgmd9xJ0mcABLtFBIfN49rgRufTBYBgNVHR8EUTBPME2gS6BJhkdodHRw\
Oi8vY3JsLnVzZXJ0cnVzdC5jb20vVVROLVVTRVJGaXJzdC1DbGllbnRBdXRoZW50\
aWNhdGlvbmFuZEVtYWlsLmNybDAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUH\
AwQwDQYJKoZIhvcNAQEFBQADggEBALFtYV2mGn98q0rkMPxTbyUkxsrt4jFcKw7u\
7mFVbwQ+zznexRtJlOTrIEy05p5QLnLZjfWqo7NK2lYcYJeA3IKirUq9iiv/Cwm0\
xtcgBEXkzYABurorbs6q15L+5K/r9CYdFip/bDCVNy8zEqx/3cfREYxRmLLQo5HQ\
rfafnoOTHh1CuEava2bwm3/q4wMC5QJRwarVNZ1yQAOJujEdxRBoUp7fooXFXAim\
eOZTT7Hot9MUnpOmw2TjrH5xzbyf6QMbzPvprDHBr3wVdAKZw7JHpsIyYdfHb0gk\
USeh1YdV8nuPmD0Wnu51tvjQjvLzxq4oW6fw8zYX/MMF08oDSlQ=\
-----END CERTIFICATE-----\
\
UTN USERFirst Hardware Root CA\
==============================\
\
MD5 Fingerprint=4C:56:41:E5:0D:BB:2B:E8:CA:A3:ED:18:08:AD:43:39\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            44:be:0c:8b:50:00:24:b4:11:d3:36:2a:fe:65:0a:fd\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN-USERFirst-Hardware\
        Validity\
            Not Before: Jul  9 18:10:42 1999 GMT\
            Not After : Jul  9 18:19:22 2019 GMT\
        Subject: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN-USERFirst-Hardware\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b1:f7:c3:38:3f:b4:a8:7f:cf:39:82:51:67:d0:\
                    6d:9f:d2:ff:58:f3:e7:9f:2b:ec:0d:89:54:99:b9:\
                    38:99:16:f7:e0:21:79:48:c2:bb:61:74:12:96:1d:\
                    3c:6a:72:d5:3c:10:67:3a:39:ed:2b:13:cd:66:eb:\
                    95:09:33:a4:6c:97:b1:e8:c6:ec:c1:75:79:9c:46:\
                    5e:8d:ab:d0:6a:fd:b9:2a:55:17:10:54:b3:19:f0:\
                    9a:f6:f1:b1:5d:b6:a7:6d:fb:e0:71:17:6b:a2:88:\
                    fb:00:df:fe:1a:31:77:0c:9a:01:7a:b1:32:e3:2b:\
                    01:07:38:6e:c3:a5:5e:23:bc:45:9b:7b:50:c1:c9:\
                    30:8f:db:e5:2b:7a:d3:5b:fb:33:40:1e:a0:d5:98:\
                    17:bc:8b:87:c3:89:d3:5d:a0:8e:b2:aa:aa:f6:8e:\
                    69:88:06:c5:fa:89:21:f3:08:9d:69:2e:09:33:9b:\
                    29:0d:46:0f:8c:cc:49:34:b0:69:51:bd:f9:06:cd:\
                    68:ad:66:4c:bc:3e:ac:61:bd:0a:88:0e:c8:df:3d:\
                    ee:7c:04:4c:9d:0a:5e:6b:91:d6:ee:c7:ed:28:8d:\
                    ab:4d:87:89:73:d0:6e:a4:d0:1e:16:8b:14:e1:76:\
                    44:03:7f:63:ac:e4:cd:49:9c:c5:92:f4:ab:32:a1:\
                    48:5b\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                A1:72:5F:26:1B:28:98:43:95:5D:07:37:D5:85:96:9D:4B:D2:C3:45\
            X509v3 CRL Distribution Points: \
                URI:http://crl.usertrust.com/UTN-USERFirst-Hardware.crl\
\
            X509v3 Extended Key Usage: \
                TLS Web Server Authentication, IPSec End System, IPSec Tunnel, IPSec User\
    Signature Algorithm: sha1WithRSAEncryption\
        47:19:0f:de:74:c6:99:97:af:fc:ad:28:5e:75:8e:eb:2d:67:\
        ee:4e:7b:2b:d7:0c:ff:f6:de:cb:55:a2:0a:e1:4c:54:65:93:\
        60:6b:9f:12:9c:ad:5e:83:2c:eb:5a:ae:c0:e4:2d:f4:00:63:\
        1d:b8:c0:6c:f2:cf:49:bb:4d:93:6f:06:a6:0a:22:b2:49:62:\
        08:4e:ff:c8:c8:14:b2:88:16:5d:e7:01:e4:12:95:e5:45:34:\
        b3:8b:69:bd:cf:b4:85:8f:75:51:9e:7d:3a:38:3a:14:48:12:\
        c6:fb:a7:3b:1a:8d:0d:82:40:07:e8:04:08:90:a1:89:cb:19:\
        50:df:ca:1c:01:bc:1d:04:19:7b:10:76:97:3b:ee:90:90:ca:\
        c4:0e:1f:16:6e:75:ef:33:f8:d3:6f:5b:1e:96:e3:e0:74:77:\
        74:7b:8a:a2:6e:2d:dd:76:d6:39:30:82:f0:ab:9c:52:f2:2a:\
        c7:af:49:5e:7e:c7:68:e5:82:81:c8:6a:27:f9:27:88:2a:d5:\
        58:50:95:1f:f0:3b:1c:57:bb:7d:14:39:62:2b:9a:c9:94:92:\
        2a:a3:22:0c:ff:89:26:7d:5f:23:2b:47:d7:15:1d:a9:6a:9e:\
        51:0d:2a:51:9e:81:f9:d4:3b:5e:70:12:7f:10:32:9c:1e:bb:\
        9d:f8:66:a8\
-----BEGIN CERTIFICATE-----\
MIIEdDCCA1ygAwIBAgIQRL4Mi1AAJLQR0zYq/mUK/TANBgkqhkiG9w0BAQUFADCB\
lzELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAlVUMRcwFQYDVQQHEw5TYWx0IExha2Ug\
Q2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMSEwHwYDVQQLExho\
dHRwOi8vd3d3LnVzZXJ0cnVzdC5jb20xHzAdBgNVBAMTFlVUTi1VU0VSRmlyc3Qt\
SGFyZHdhcmUwHhcNOTkwNzA5MTgxMDQyWhcNMTkwNzA5MTgxOTIyWjCBlzELMAkG\
A1UEBhMCVVMxCzAJBgNVBAgTAlVUMRcwFQYDVQQHEw5TYWx0IExha2UgQ2l0eTEe\
MBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMSEwHwYDVQQLExhodHRwOi8v\
d3d3LnVzZXJ0cnVzdC5jb20xHzAdBgNVBAMTFlVUTi1VU0VSRmlyc3QtSGFyZHdh\
cmUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCx98M4P7Sof885glFn\
0G2f0v9Y8+efK+wNiVSZuTiZFvfgIXlIwrthdBKWHTxqctU8EGc6Oe0rE81m65UJ\
M6Rsl7HoxuzBdXmcRl6Nq9Bq/bkqVRcQVLMZ8Jr28bFdtqdt++BxF2uiiPsA3/4a\
MXcMmgF6sTLjKwEHOG7DpV4jvEWbe1DByTCP2+UretNb+zNAHqDVmBe8i4fDidNd\
oI6yqqr2jmmIBsX6iSHzCJ1pLgkzmykNRg+MzEk0sGlRvfkGzWitZky8PqxhvQqI\
DsjfPe58BEydCl5rkdbux+0ojatNh4lz0G6k0B4WixThdkQDf2Os5M1JnMWS9Ksy\
oUhbAgMBAAGjgbkwgbYwCwYDVR0PBAQDAgHGMA8GA1UdEwEB/wQFMAMBAf8wHQYD\
VR0OBBYEFKFyXyYbKJhDlV0HN9WFlp1L0sNFMEQGA1UdHwQ9MDswOaA3oDWGM2h0\
dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9VVE4tVVNFUkZpcnN0LUhhcmR3YXJlLmNy\
bDAxBgNVHSUEKjAoBggrBgEFBQcDAQYIKwYBBQUHAwUGCCsGAQUFBwMGBggrBgEF\
BQcDBzANBgkqhkiG9w0BAQUFAAOCAQEARxkP3nTGmZev/K0oXnWO6y1n7k57K9cM\
//bey1WiCuFMVGWTYGufEpytXoMs61quwOQt9ABjHbjAbPLPSbtNk28Gpgoiskli\
CE7/yMgUsogWXecB5BKV5UU0s4tpvc+0hY91UZ59Ojg6FEgSxvunOxqNDYJAB+gE\
CJChicsZUN/KHAG8HQQZexB2lzvukJDKxA4fFm517zP4029bHpbj4HR3dHuKom4t\
3XbWOTCC8KucUvIqx69JXn7HaOWCgchqJ/kniCrVWFCVH/A7HFe7fRQ5YiuayZSS\
KqMiDP+JJn1fIytH1xUdqWqeUQ0qUZ6B+dQ7XnASfxAynB67nfhmqA==\
-----END CERTIFICATE-----\
\
UTN USERFirst Object Root CA\
============================\
\
MD5 Fingerprint=A7:F2:E4:16:06:41:11:50:30:6B:9C:E3:B4:9C:B0:C9\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            44:be:0c:8b:50:00:24:b4:11:d3:36:2d:e0:b3:5f:1b\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN-USERFirst-Object\
        Validity\
            Not Before: Jul  9 18:31:20 1999 GMT\
            Not After : Jul  9 18:40:36 2019 GMT\
        Subject: C=US, ST=UT, L=Salt Lake City, O=The USERTRUST Network, OU=http://www.usertrust.com, CN=UTN-USERFirst-Object\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:ce:aa:81:3f:a3:a3:61:78:aa:31:00:55:95:11:\
                    9e:27:0f:1f:1c:df:3a:9b:82:68:30:c0:4a:61:1d:\
                    f1:2f:0e:fa:be:79:f7:a5:23:ef:55:51:96:84:cd:\
                    db:e3:b9:6e:3e:31:d8:0a:20:67:c7:f4:d9:bf:94:\
                    eb:47:04:3e:02:ce:2a:a2:5d:87:04:09:f6:30:9d:\
                    18:8a:97:b2:aa:1c:fc:41:d2:a1:36:cb:fb:3d:91:\
                    ba:e7:d9:70:35:fa:e4:e7:90:c3:9b:a3:9b:d3:3c:\
                    f5:12:99:77:b1:b7:09:e0:68:e6:1c:b8:f3:94:63:\
                    88:6a:6a:fe:0b:76:c9:be:f4:22:e4:67:b9:ab:1a:\
                    5e:77:c1:85:07:dd:0d:6c:bf:ee:06:c7:77:6a:41:\
                    9e:a7:0f:d7:fb:ee:94:17:b7:fc:85:be:a4:ab:c4:\
                    1c:31:dd:d7:b6:d1:e4:f0:ef:df:16:8f:b2:52:93:\
                    d7:a1:d4:89:a1:07:2e:bf:e1:01:12:42:1e:1a:e1:\
                    d8:95:34:db:64:79:28:ff:ba:2e:11:c2:e5:e8:5b:\
                    92:48:fb:47:0b:c2:6c:da:ad:32:83:41:f3:a5:e5:\
                    41:70:fd:65:90:6d:fa:fa:51:c4:f9:bd:96:2b:19:\
                    04:2c:d3:6d:a7:dc:f0:7f:6f:83:65:e2:6a:ab:87:\
                    86:75\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Key Usage: \
                Digital Signature, Non Repudiation, Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                DA:ED:64:74:14:9C:14:3C:AB:DD:99:A9:BD:5B:28:4D:8B:3C:C9:D8\
            X509v3 CRL Distribution Points: \
                URI:http://crl.usertrust.com/UTN-USERFirst-Object.crl\
\
            X509v3 Extended Key Usage: \
                Code Signing, Time Stamping, Microsoft Encrypted File System\
    Signature Algorithm: sha1WithRSAEncryption\
        08:1f:52:b1:37:44:78:db:fd:ce:b9:da:95:96:98:aa:55:64:\
        80:b5:5a:40:dd:21:a5:c5:c1:f3:5f:2c:4c:c8:47:5a:69:ea:\
        e8:f0:35:35:f4:d0:25:f3:c8:a6:a4:87:4a:bd:1b:b1:73:08:\
        bd:d4:c3:ca:b6:35:bb:59:86:77:31:cd:a7:80:14:ae:13:ef:\
        fc:b1:48:f9:6b:25:25:2d:51:b6:2c:6d:45:c1:98:c8:8a:56:\
        5d:3e:ee:43:4e:3e:6b:27:8e:d0:3a:4b:85:0b:5f:d3:ed:6a:\
        a7:75:cb:d1:5a:87:2f:39:75:13:5a:72:b0:02:81:9f:be:f0:\
        0f:84:54:20:62:6c:69:d4:e1:4d:c6:0d:99:43:01:0d:12:96:\
        8c:78:9d:bf:50:a2:b1:44:aa:6a:cf:17:7a:cf:6f:0f:d4:f8:\
        24:55:5f:f0:34:16:49:66:3e:50:46:c9:63:71:38:31:62:b8:\
        62:b9:f3:53:ad:6c:b5:2b:a2:12:aa:19:4f:09:da:5e:e7:93:\
        c6:8e:14:08:fe:f0:30:80:18:a0:86:85:4d:c8:7d:d7:8b:03:\
        fe:6e:d5:f7:9d:16:ac:92:2c:a0:23:e5:9c:91:52:1f:94:df:\
        17:94:73:c3:b3:c1:c1:71:05:20:00:78:bd:13:52:1d:a8:3e:\
        cd:00:1f:c8\
-----BEGIN CERTIFICATE-----\
MIIEZjCCA06gAwIBAgIQRL4Mi1AAJLQR0zYt4LNfGzANBgkqhkiG9w0BAQUFADCB\
lTELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAlVUMRcwFQYDVQQHEw5TYWx0IExha2Ug\
Q2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMSEwHwYDVQQLExho\
dHRwOi8vd3d3LnVzZXJ0cnVzdC5jb20xHTAbBgNVBAMTFFVUTi1VU0VSRmlyc3Qt\
T2JqZWN0MB4XDTk5MDcwOTE4MzEyMFoXDTE5MDcwOTE4NDAzNlowgZUxCzAJBgNV\
BAYTAlVTMQswCQYDVQQIEwJVVDEXMBUGA1UEBxMOU2FsdCBMYWtlIENpdHkxHjAc\
BgNVBAoTFVRoZSBVU0VSVFJVU1QgTmV0d29yazEhMB8GA1UECxMYaHR0cDovL3d3\
dy51c2VydHJ1c3QuY29tMR0wGwYDVQQDExRVVE4tVVNFUkZpcnN0LU9iamVjdDCC\
ASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAM6qgT+jo2F4qjEAVZURnicP\
HxzfOpuCaDDASmEd8S8O+r5596Uj71VRloTN2+O5bj4x2AogZ8f02b+U60cEPgLO\
KqJdhwQJ9jCdGIqXsqoc/EHSoTbL+z2RuufZcDX65OeQw5ujm9M89RKZd7G3CeBo\
5hy485RjiGpq/gt2yb70IuRnuasaXnfBhQfdDWy/7gbHd2pBnqcP1/vulBe3/IW+\
pKvEHDHd17bR5PDv3xaPslKT16HUiaEHLr/hARJCHhrh2JU022R5KP+6LhHC5ehb\
kkj7RwvCbNqtMoNB86XlQXD9ZZBt+vpRxPm9lisZBCzTbafc8H9vg2XiaquHhnUC\
AwEAAaOBrzCBrDALBgNVHQ8EBAMCAcYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E\
FgQU2u1kdBScFDyr3ZmpvVsoTYs8ydgwQgYDVR0fBDswOTA3oDWgM4YxaHR0cDov\
L2NybC51c2VydHJ1c3QuY29tL1VUTi1VU0VSRmlyc3QtT2JqZWN0LmNybDApBgNV\
HSUEIjAgBggrBgEFBQcDAwYIKwYBBQUHAwgGCisGAQQBgjcKAwQwDQYJKoZIhvcN\
AQEFBQADggEBAAgfUrE3RHjb/c652pWWmKpVZIC1WkDdIaXFwfNfLEzIR1pp6ujw\
NTX00CXzyKakh0q9G7FzCL3Uw8q2NbtZhncxzaeAFK4T7/yxSPlrJSUtUbYsbUXB\
mMiKVl0+7kNOPmsnjtA6S4ULX9Ptaqd1y9Fahy85dRNacrACgZ++8A+EVCBibGnU\
4U3GDZlDAQ0Slox4nb9QorFEqmrPF3rPbw/U+CRVX/A0FklmPlBGyWNxODFiuGK5\
81OtbLUrohKqGU8J2l7nk8aOFAj+8DCAGKCGhU3IfdeLA/5u1fedFqySLKAj5ZyR\
Uh+U3xeUc8OzwcFxBSAAeL0TUh2oPs0AH8g=\
-----END CERTIFICATE-----\
\
Camerfirma Chambers of Commerce Root\
====================================\
\
MD5 Fingerprint=B0:01:EE:14:D9:AF:29:18:94:76:8E:F1:69:33:2A:84\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=EU, O=AC Camerfirma SA CIF A82743287, OU=http://www.chambersign.org, CN=Chambers of Commerce Root\
        Validity\
            Not Before: Sep 30 16:13:43 2003 GMT\
            Not After : Sep 30 16:13:44 2037 GMT\
        Subject: C=EU, O=AC Camerfirma SA CIF A82743287, OU=http://www.chambersign.org, CN=Chambers of Commerce Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b7:36:55:e5:a5:5d:18:30:e0:da:89:54:91:fc:\
                    c8:c7:52:f8:2f:50:d9:ef:b1:75:73:65:47:7d:1b:\
                    5b:ba:75:c5:fc:a1:88:24:fa:2f:ed:ca:08:4a:39:\
                    54:c4:51:7a:b5:da:60:ea:38:3c:81:b2:cb:f1:bb:\
                    d9:91:23:3f:48:01:70:75:a9:05:2a:ad:1f:71:f3:\
                    c9:54:3d:1d:06:6a:40:3e:b3:0c:85:ee:5c:1b:79:\
                    c2:62:c4:b8:36:8e:35:5d:01:0c:23:04:47:35:aa:\
                    9b:60:4e:a0:66:3d:cb:26:0a:9c:40:a1:f4:5d:98:\
                    bf:71:ab:a5:00:68:2a:ed:83:7a:0f:a2:14:b5:d4:\
                    22:b3:80:b0:3c:0c:5a:51:69:2d:58:18:8f:ed:99:\
                    9e:f1:ae:e2:95:e6:f6:47:a8:d6:0c:0f:b0:58:58:\
                    db:c3:66:37:9e:9b:91:54:33:37:d2:94:1c:6a:48:\
                    c9:c9:f2:a5:da:a5:0c:23:f7:23:0e:9c:32:55:5e:\
                    71:9c:84:05:51:9a:2d:fd:e6:4e:2a:34:5a:de:ca:\
                    40:37:67:0c:54:21:55:77:da:0a:0c:cc:97:ae:80:\
                    dc:94:36:4a:f4:3e:ce:36:13:1e:53:e4:ac:4e:3a:\
                    05:ec:db:ae:72:9c:38:8b:d0:39:3b:89:0a:3e:77:\
                    fe:75\
                Exponent: 3 (0x3)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE, pathlen:12\
            X509v3 CRL Distribution Points: \
                URI:http://crl.chambersign.org/chambersroot.crl\
\
            X509v3 Subject Key Identifier: \
                E3:94:F5:B1:4D:E9:DB:A1:29:5B:57:8B:4D:76:06:76:E1:D1:A2:8A\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Subject Alternative Name: \
                email:chambersroot@chambersign.org\
            X509v3 Issuer Alternative Name: \
                email:chambersroot@chambersign.org\
            X509v3 Certificate Policies: \
                Policy: 1.3.6.1.4.1.17326.10.3.1\
                  CPS: http://cps.chambersign.org/cps/chambersroot.html\
\
    Signature Algorithm: sha1WithRSAEncryption\
        0c:41:97:c2:1a:86:c0:22:7c:9f:fb:90:f3:1a:d1:03:b1:ef:\
        13:f9:21:5f:04:9c:da:c9:a5:8d:27:6c:96:87:91:be:41:90:\
        01:72:93:e7:1e:7d:5f:f6:89:c6:5d:a7:40:09:3d:ac:49:45:\
        45:dc:2e:8d:30:68:b2:09:ba:fb:c3:2f:cc:ba:0b:df:3f:77:\
        7b:46:7d:3a:12:24:8e:96:8f:3c:05:0a:6f:d2:94:28:1d:6d:\
        0c:c0:2e:88:22:d5:d8:cf:1d:13:c7:f0:48:d7:d7:05:a7:cf:\
        c7:47:9e:3b:3c:34:c8:80:4f:d4:14:bb:fc:0d:50:f7:fa:b3:\
        ec:42:5f:a9:dd:6d:c8:f4:75:cf:7b:c1:72:26:b1:01:1c:5c:\
        2c:fd:7a:4e:b4:01:c5:05:57:b9:e7:3c:aa:05:d9:88:e9:07:\
        46:41:ce:ef:41:81:ae:58:df:83:a2:ae:ca:d7:77:1f:e7:00:\
        3c:9d:6f:8e:e4:32:09:1d:4d:78:34:78:34:3c:94:9b:26:ed:\
        4f:71:c6:19:7a:bd:20:22:48:5a:fe:4b:7d:03:b7:e7:58:be:\
        c6:32:4e:74:1e:68:dd:a8:68:5b:b3:3e:ee:62:7d:d9:80:e8:\
        0a:75:7a:b7:ee:b4:65:9a:21:90:e0:aa:d0:98:bc:38:b5:73:\
        3c:8b:f8:dc\
-----BEGIN CERTIFICATE-----\
MIIEvTCCA6WgAwIBAgIBADANBgkqhkiG9w0BAQUFADB/MQswCQYDVQQGEwJFVTEn\
MCUGA1UEChMeQUMgQ2FtZXJmaXJtYSBTQSBDSUYgQTgyNzQzMjg3MSMwIQYDVQQL\
ExpodHRwOi8vd3d3LmNoYW1iZXJzaWduLm9yZzEiMCAGA1UEAxMZQ2hhbWJlcnMg\
b2YgQ29tbWVyY2UgUm9vdDAeFw0wMzA5MzAxNjEzNDNaFw0zNzA5MzAxNjEzNDRa\
MH8xCzAJBgNVBAYTAkVVMScwJQYDVQQKEx5BQyBDYW1lcmZpcm1hIFNBIENJRiBB\
ODI3NDMyODcxIzAhBgNVBAsTGmh0dHA6Ly93d3cuY2hhbWJlcnNpZ24ub3JnMSIw\
IAYDVQQDExlDaGFtYmVycyBvZiBDb21tZXJjZSBSb290MIIBIDANBgkqhkiG9w0B\
AQEFAAOCAQ0AMIIBCAKCAQEAtzZV5aVdGDDg2olUkfzIx1L4L1DZ77F1c2VHfRtb\
unXF/KGIJPov7coISjlUxFF6tdpg6jg8gbLL8bvZkSM/SAFwdakFKq0fcfPJVD0d\
BmpAPrMMhe5cG3nCYsS4No41XQEMIwRHNaqbYE6gZj3LJgqcQKH0XZi/caulAGgq\
7YN6D6IUtdQis4CwPAxaUWktWBiP7Zme8a7ileb2R6jWDA+wWFjbw2Y3npuRVDM3\
0pQcakjJyfKl2qUMI/cjDpwyVV5xnIQFUZot/eZOKjRa3spAN2cMVCFVd9oKDMyX\
roDclDZK9D7ONhMeU+SsTjoF7Nuucpw4i9A5O4kKPnf+dQIBA6OCAUQwggFAMBIG\
A1UdEwEB/wQIMAYBAf8CAQwwPAYDVR0fBDUwMzAxoC+gLYYraHR0cDovL2NybC5j\
aGFtYmVyc2lnbi5vcmcvY2hhbWJlcnNyb290LmNybDAdBgNVHQ4EFgQU45T1sU3p\
26EpW1eLTXYGduHRooowDgYDVR0PAQH/BAQDAgEGMBEGCWCGSAGG+EIBAQQEAwIA\
BzAnBgNVHREEIDAegRxjaGFtYmVyc3Jvb3RAY2hhbWJlcnNpZ24ub3JnMCcGA1Ud\
EgQgMB6BHGNoYW1iZXJzcm9vdEBjaGFtYmVyc2lnbi5vcmcwWAYDVR0gBFEwTzBN\
BgsrBgEEAYGHLgoDATA+MDwGCCsGAQUFBwIBFjBodHRwOi8vY3BzLmNoYW1iZXJz\
aWduLm9yZy9jcHMvY2hhbWJlcnNyb290Lmh0bWwwDQYJKoZIhvcNAQEFBQADggEB\
AAxBl8IahsAifJ/7kPMa0QOx7xP5IV8EnNrJpY0nbJaHkb5BkAFyk+cefV/2icZd\
p0AJPaxJRUXcLo0waLIJuvvDL8y6C98/d3tGfToSJI6WjzwFCm/SlCgdbQzALogi\
1djPHRPH8EjX1wWnz8dHnjs8NMiAT9QUu/wNUPf6s+xCX6ndbcj0dc97wXImsQEc\
XCz9ek60AcUFV7nnPKoF2YjpB0ZBzu9Bga5Y34OirsrXdx/nADydb47kMgkdTXg0\
eDQ8lJsm7U9xxhl6vSAiSFr+S30Dt+dYvsYyTnQeaN2oaFuzPu5ifdmA6Ap1erfu\
tGWaIZDgqtCYvDi1czyL+Nw=\
-----END CERTIFICATE-----\
\
Camerfirma Global Chambersign Root\
==================================\
\
MD5 Fingerprint=C5:E6:7B:BF:06:D0:4F:43:ED:C4:7A:65:8A:FB:6B:19\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=EU, O=AC Camerfirma SA CIF A82743287, OU=http://www.chambersign.org, CN=Global Chambersign Root\
        Validity\
            Not Before: Sep 30 16:14:18 2003 GMT\
            Not After : Sep 30 16:14:18 2037 GMT\
        Subject: C=EU, O=AC Camerfirma SA CIF A82743287, OU=http://www.chambersign.org, CN=Global Chambersign Root\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:a2:70:a2:d0:9f:42:ae:5b:17:c7:d8:7d:cf:14:\
                    83:fc:4f:c9:a1:b7:13:af:8a:d7:9e:3e:04:0a:92:\
                    8b:60:56:fa:b4:32:2f:88:4d:a1:60:08:f4:b7:09:\
                    4e:a0:49:2f:49:d6:d3:df:9d:97:5a:9f:94:04:70:\
                    ec:3f:59:d9:b7:cc:66:8b:98:52:28:09:02:df:c5:\
                    2f:84:8d:7a:97:77:bf:ec:40:9d:25:72:ab:b5:3f:\
                    32:98:fb:b7:b7:fc:72:84:e5:35:87:f9:55:fa:a3:\
                    1f:0e:6f:2e:28:dd:69:a0:d9:42:10:c6:f8:b5:44:\
                    c2:d0:43:7f:db:bc:e4:a2:3c:6a:55:78:0a:77:a9:\
                    d8:ea:19:32:b7:2f:fe:5c:3f:1b:ee:b1:98:ec:ca:\
                    ad:7a:69:45:e3:96:0f:55:f6:e6:ed:75:ea:65:e8:\
                    32:56:93:46:89:a8:25:8a:65:06:ee:6b:bf:79:07:\
                    d0:f1:b7:af:ed:2c:4d:92:bb:c0:a8:5f:a7:67:7d:\
                    04:f2:15:08:70:ac:92:d6:7d:04:d2:33:fb:4c:b6:\
                    0b:0b:fb:1a:c9:c4:8d:03:a9:7e:5c:f2:50:ab:12:\
                    a5:a1:cf:48:50:a5:ef:d2:c8:1a:13:fa:b0:7f:b1:\
                    82:1c:77:6a:0f:5f:dc:0b:95:8f:ef:43:7e:e6:45:\
                    09:25\
                Exponent: 3 (0x3)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE, pathlen:12\
            X509v3 CRL Distribution Points: \
                URI:http://crl.chambersign.org/chambersignroot.crl\
\
            X509v3 Subject Key Identifier: \
                43:9C:36:9F:B0:9E:30:4D:C6:CE:5F:AD:10:AB:E5:03:A5:FA:A9:14\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            X509v3 Subject Alternative Name: \
                email:chambersignroot@chambersign.org\
            X509v3 Issuer Alternative Name: \
                email:chambersignroot@chambersign.org\
            X509v3 Certificate Policies: \
                Policy: 1.3.6.1.4.1.17326.10.1.1\
                  CPS: http://cps.chambersign.org/cps/chambersignroot.html\
\
    Signature Algorithm: sha1WithRSAEncryption\
        3c:3b:70:91:f9:04:54:27:91:e1:ed:ed:fe:68:7f:61:5d:e5:\
        41:65:4f:32:f1:18:05:94:6a:1c:de:1f:70:db:3e:7b:32:02:\
        34:b5:0c:6c:a1:8a:7c:a5:f4:8f:ff:d4:d8:ad:17:d5:2d:04:\
        d1:3f:58:80:e2:81:59:88:be:c0:e3:46:93:24:fe:90:bd:26:\
        a2:30:2d:e8:97:26:57:35:89:74:96:18:f6:15:e2:af:24:19:\
        56:02:02:b2:ba:0f:14:ea:c6:8a:66:c1:86:45:55:8b:be:92:\
        be:9c:a4:04:c7:49:3c:9e:e8:29:7a:89:d7:fe:af:ff:68:f5:\
        a5:17:90:bd:ac:99:cc:a5:86:57:09:67:46:db:d6:16:c2:46:\
        f1:e4:a9:50:f5:8f:d1:92:15:d3:5f:3e:c6:00:49:3a:6e:58:\
        b2:d1:d1:27:0d:25:c8:32:f8:20:11:cd:7d:32:33:48:94:54:\
        4c:dd:dc:79:c4:30:9f:eb:8e:b8:55:b5:d7:88:5c:c5:6a:24:\
        3d:b2:d3:05:03:51:c6:07:ef:cc:14:72:74:3d:6e:72:ce:18:\
        28:8c:4a:a0:77:e5:09:2b:45:44:47:ac:b7:67:7f:01:8a:05:\
        5a:93:be:a1:c1:ff:f8:e7:0e:67:a4:47:49:76:5d:75:90:1a:\
        f5:26:8f:f0\
-----BEGIN CERTIFICATE-----\
MIIExTCCA62gAwIBAgIBADANBgkqhkiG9w0BAQUFADB9MQswCQYDVQQGEwJFVTEn\
MCUGA1UEChMeQUMgQ2FtZXJmaXJtYSBTQSBDSUYgQTgyNzQzMjg3MSMwIQYDVQQL\
ExpodHRwOi8vd3d3LmNoYW1iZXJzaWduLm9yZzEgMB4GA1UEAxMXR2xvYmFsIENo\
YW1iZXJzaWduIFJvb3QwHhcNMDMwOTMwMTYxNDE4WhcNMzcwOTMwMTYxNDE4WjB9\
MQswCQYDVQQGEwJFVTEnMCUGA1UEChMeQUMgQ2FtZXJmaXJtYSBTQSBDSUYgQTgy\
NzQzMjg3MSMwIQYDVQQLExpodHRwOi8vd3d3LmNoYW1iZXJzaWduLm9yZzEgMB4G\
A1UEAxMXR2xvYmFsIENoYW1iZXJzaWduIFJvb3QwggEgMA0GCSqGSIb3DQEBAQUA\
A4IBDQAwggEIAoIBAQCicKLQn0KuWxfH2H3PFIP8T8mhtxOviteePgQKkotgVvq0\
Mi+ITaFgCPS3CU6gSS9J1tPfnZdan5QEcOw/Wdm3zGaLmFIoCQLfxS+EjXqXd7/s\
QJ0lcqu1PzKY+7e3/HKE5TWH+VX6ox8Oby4o3Wmg2UIQxvi1RMLQQ3/bvOSiPGpV\
eAp3qdjqGTK3L/5cPxvusZjsyq16aUXjlg9V9ubtdepl6DJWk0aJqCWKZQbua795\
B9Dxt6/tLE2Su8CoX6dnfQTyFQhwrJLWfQTSM/tMtgsL+xrJxI0DqX5c8lCrEqWh\
z0hQpe/SyBoT+rB/sYIcd2oPX9wLlY/vQ37mRQklAgEDo4IBUDCCAUwwEgYDVR0T\
AQH/BAgwBgEB/wIBDDA/BgNVHR8EODA2MDSgMqAwhi5odHRwOi8vY3JsLmNoYW1i\
ZXJzaWduLm9yZy9jaGFtYmVyc2lnbnJvb3QuY3JsMB0GA1UdDgQWBBRDnDafsJ4w\
TcbOX60Qq+UDpfqpFDAOBgNVHQ8BAf8EBAMCAQYwEQYJYIZIAYb4QgEBBAQDAgAH\
MCoGA1UdEQQjMCGBH2NoYW1iZXJzaWducm9vdEBjaGFtYmVyc2lnbi5vcmcwKgYD\
VR0SBCMwIYEfY2hhbWJlcnNpZ25yb290QGNoYW1iZXJzaWduLm9yZzBbBgNVHSAE\
VDBSMFAGCysGAQQBgYcuCgEBMEEwPwYIKwYBBQUHAgEWM2h0dHA6Ly9jcHMuY2hh\
bWJlcnNpZ24ub3JnL2Nwcy9jaGFtYmVyc2lnbnJvb3QuaHRtbDANBgkqhkiG9w0B\
AQUFAAOCAQEAPDtwkfkEVCeR4e3t/mh/YV3lQWVPMvEYBZRqHN4fcNs+ezICNLUM\
bKGKfKX0j//U2K0X1S0E0T9YgOKBWYi+wONGkyT+kL0mojAt6JcmVzWJdJYY9hXi\
ryQZVgICsroPFOrGimbBhkVVi76SvpykBMdJPJ7oKXqJ1/6v/2j1pReQvayZzKWG\
VwlnRtvWFsJG8eSpUPWP0ZIV018+xgBJOm5YstHRJw0lyDL4IBHNfTIzSJRUTN3c\
ecQwn+uOuFW114hcxWokPbLTBQNRxgfvzBRydD1ucs4YKIxKoHflCStFREest2d/\
AYoFWpO+ocH/+OcOZ6RHSXZddZAa9SaP8A==\
-----END CERTIFICATE-----\
\
NetLock Notary (Class A) Root\
=============================\
\
MD5 Fingerprint=86:38:6D:5E:49:63:6C:85:5C:DB:6D:DC:94:B7:D0:F7\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 259 (0x103)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=HU, ST=Hungary, L=Budapest, O=NetLock Halozatbiztonsagi Kft., OU=Tanusitvanykiadok, CN=NetLock Kozjegyzoi (Class A) Tanusitvanykiado\
        Validity\
            Not Before: Feb 24 23:14:47 1999 GMT\
            Not After : Feb 19 23:14:47 2019 GMT\
        Subject: C=HU, ST=Hungary, L=Budapest, O=NetLock Halozatbiztonsagi Kft., OU=Tanusitvanykiadok, CN=NetLock Kozjegyzoi (Class A) Tanusitvanykiado\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:bc:74:8c:0f:bb:4c:f4:37:1e:a9:05:82:d8:e6:\
                    e1:6c:70:ea:78:b5:6e:d1:38:44:0d:a8:83:ce:5d:\
                    d2:d6:d5:81:c5:d4:4b:e7:5b:94:70:26:db:3b:9d:\
                    6a:4c:62:f7:71:f3:64:d6:61:3b:3d:eb:73:a3:37:\
                    d9:cf:ea:8c:92:3b:cd:f7:07:dc:66:74:97:f4:45:\
                    22:dd:f4:5c:e0:bf:6d:f3:be:65:33:e4:15:3a:bf:\
                    db:98:90:55:38:c4:ed:a6:55:63:0b:b0:78:04:f4:\
                    e3:6e:c1:3f:8e:fc:51:78:1f:92:9e:83:c2:fe:d9:\
                    b0:a9:c9:bc:5a:00:ff:a9:a8:98:74:fb:f6:2c:3e:\
                    15:39:0d:b6:04:55:a8:0e:98:20:42:b3:b1:25:ad:\
                    7e:9a:6f:5d:53:b1:ab:0c:fc:eb:e0:f3:7a:b3:a8:\
                    b3:ff:46:f6:63:a2:d8:3a:98:7b:b6:ac:85:ff:b0:\
                    25:4f:74:63:e7:13:07:a5:0a:8f:05:f7:c0:64:6f:\
                    7e:a7:27:80:96:de:d4:2e:86:60:c7:6b:2b:5e:73:\
                    7b:17:e7:91:3f:64:0c:d8:4b:22:34:2b:9b:32:f2:\
                    48:1f:9f:a1:0a:84:7a:e2:c2:ad:97:3d:8e:d5:c1:\
                    f9:56:a3:50:e9:c6:b4:fa:98:a2:ee:95:e6:2a:03:\
                    8c:df\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE, pathlen:4\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            Netscape Comment: \
                FIGYELEM! Ezen tanusitvany a NetLock Kft. Altalanos Szolgaltatasi Felteteleiben leirt eljarasok alapjan keszult. A hitelesites folyamatat a NetLock Kft. termekfelelosseg-biztositasa vedi. A digitalis alairas elfogadasanak feltetele az eloirt ellenorzesi eljaras megtetele. Az eljaras leirasa megtalalhato a NetLock Kft. Internet honlapjan a https://www.netlock.net/docs cimen vagy kerheto az ellenorzes@netlock.net e-mail cimen. IMPORTANT! The issuance and the use of this certificate is subject to the NetLock CPS available at https://www.netlock.net/docs or by e-mail at cps@netlock.net.\
    Signature Algorithm: md5WithRSAEncryption\
        48:24:46:f7:ba:56:6f:fa:c8:28:03:40:4e:e5:31:39:6b:26:\
        6b:53:7f:db:df:df:f3:71:3d:26:c0:14:0e:c6:67:7b:23:a8:\
        0c:73:dd:01:bb:c6:ca:6e:37:39:55:d5:c7:8c:56:20:0e:28:\
        0a:0e:d2:2a:a4:b0:49:52:c6:38:07:fe:be:0a:09:8c:d1:98:\
        cf:ca:da:14:31:a1:4f:d2:39:fc:0f:11:2c:43:c3:dd:ab:93:\
        c7:55:3e:47:7c:18:1a:00:dc:f3:7b:d8:f2:7f:52:6c:20:f4:\
        0b:5f:69:52:f4:ee:f8:b2:29:60:eb:e3:49:31:21:0d:d6:b5:\
        10:41:e2:41:09:6c:e2:1a:9a:56:4b:77:02:f6:a0:9b:9a:27:\
        87:e8:55:29:71:c2:90:9f:45:78:1a:e1:15:64:3d:d0:0e:d8:\
        a0:76:9f:ae:c5:d0:2e:ea:d6:0f:56:ec:64:7f:5a:9b:14:58:\
        01:27:7e:13:50:c7:6b:2a:e6:68:3c:bf:5c:a0:0a:1b:e1:0e:\
        7a:e9:e2:80:c3:e9:e9:f6:fd:6c:11:9e:d0:e5:28:27:2b:54:\
        32:42:14:82:75:e6:4a:f0:2b:66:75:63:8c:a2:fb:04:3e:83:\
        0e:9b:36:f0:18:e4:26:20:c3:8c:f0:28:07:ad:3c:17:66:88:\
        b5:fd:b6:88\
-----BEGIN CERTIFICATE-----\
MIIGfTCCBWWgAwIBAgICAQMwDQYJKoZIhvcNAQEEBQAwga8xCzAJBgNVBAYTAkhV\
MRAwDgYDVQQIEwdIdW5nYXJ5MREwDwYDVQQHEwhCdWRhcGVzdDEnMCUGA1UEChMe\
TmV0TG9jayBIYWxvemF0Yml6dG9uc2FnaSBLZnQuMRowGAYDVQQLExFUYW51c2l0\
dmFueWtpYWRvazE2MDQGA1UEAxMtTmV0TG9jayBLb3pqZWd5em9pIChDbGFzcyBB\
KSBUYW51c2l0dmFueWtpYWRvMB4XDTk5MDIyNDIzMTQ0N1oXDTE5MDIxOTIzMTQ0\
N1owga8xCzAJBgNVBAYTAkhVMRAwDgYDVQQIEwdIdW5nYXJ5MREwDwYDVQQHEwhC\
dWRhcGVzdDEnMCUGA1UEChMeTmV0TG9jayBIYWxvemF0Yml6dG9uc2FnaSBLZnQu\
MRowGAYDVQQLExFUYW51c2l0dmFueWtpYWRvazE2MDQGA1UEAxMtTmV0TG9jayBL\
b3pqZWd5em9pIChDbGFzcyBBKSBUYW51c2l0dmFueWtpYWRvMIIBIjANBgkqhkiG\
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvHSMD7tM9DceqQWC2ObhbHDqeLVu0ThEDaiD\
zl3S1tWBxdRL51uUcCbbO51qTGL3cfNk1mE7PetzozfZz+qMkjvN9wfcZnSX9EUi\
3fRc4L9t875lM+QVOr/bmJBVOMTtplVjC7B4BPTjbsE/jvxReB+SnoPC/tmwqcm8\
WgD/qaiYdPv2LD4VOQ22BFWoDpggQrOxJa1+mm9dU7GrDPzr4PN6s6iz/0b2Y6LY\
Oph7tqyF/7AlT3Rj5xMHpQqPBffAZG9+pyeAlt7ULoZgx2srXnN7F+eRP2QM2Esi\
NCubMvJIH5+hCoR64sKtlz2O1cH5VqNQ6ca0+pii7pXmKgOM3wIDAQABo4ICnzCC\
ApswDgYDVR0PAQH/BAQDAgAGMBIGA1UdEwEB/wQIMAYBAf8CAQQwEQYJYIZIAYb4\
QgEBBAQDAgAHMIICYAYJYIZIAYb4QgENBIICURaCAk1GSUdZRUxFTSEgRXplbiB0\
YW51c2l0dmFueSBhIE5ldExvY2sgS2Z0LiBBbHRhbGFub3MgU3pvbGdhbHRhdGFz\
aSBGZWx0ZXRlbGVpYmVuIGxlaXJ0IGVsamFyYXNvayBhbGFwamFuIGtlc3p1bHQu\
IEEgaGl0ZWxlc2l0ZXMgZm9seWFtYXRhdCBhIE5ldExvY2sgS2Z0LiB0ZXJtZWtm\
ZWxlbG9zc2VnLWJpenRvc2l0YXNhIHZlZGkuIEEgZGlnaXRhbGlzIGFsYWlyYXMg\
ZWxmb2dhZGFzYW5hayBmZWx0ZXRlbGUgYXogZWxvaXJ0IGVsbGVub3J6ZXNpIGVs\
amFyYXMgbWVndGV0ZWxlLiBBeiBlbGphcmFzIGxlaXJhc2EgbWVndGFsYWxoYXRv\
IGEgTmV0TG9jayBLZnQuIEludGVybmV0IGhvbmxhcGphbiBhIGh0dHBzOi8vd3d3\
Lm5ldGxvY2submV0L2RvY3MgY2ltZW4gdmFneSBrZXJoZXRvIGF6IGVsbGVub3J6\
ZXNAbmV0bG9jay5uZXQgZS1tYWlsIGNpbWVuLiBJTVBPUlRBTlQhIFRoZSBpc3N1\
YW5jZSBhbmQgdGhlIHVzZSBvZiB0aGlzIGNlcnRpZmljYXRlIGlzIHN1YmplY3Qg\
dG8gdGhlIE5ldExvY2sgQ1BTIGF2YWlsYWJsZSBhdCBodHRwczovL3d3dy5uZXRs\
b2NrLm5ldC9kb2NzIG9yIGJ5IGUtbWFpbCBhdCBjcHNAbmV0bG9jay5uZXQuMA0G\
CSqGSIb3DQEBBAUAA4IBAQBIJEb3ulZv+sgoA0BO5TE5ayZrU3/b39/zcT0mwBQO\
xmd7I6gMc90Bu8bKbjc5VdXHjFYgDigKDtIqpLBJUsY4B/6+CgmM0ZjPytoUMaFP\
0jn8DxEsQ8Pdq5PHVT5HfBgaANzze9jyf1JsIPQLX2lS9O74silg6+NJMSEN1rUQ\
QeJBCWziGppWS3cC9qCbmieH6FUpccKQn0V4GuEVZD3QDtigdp+uxdAu6tYPVuxk\
f1qbFFgBJ34TUMdrKuZoPL9coAob4Q566eKAw+np9v1sEZ7Q5SgnK1QyQhSCdeZK\
8CtmdWOMovsEPoMOmzbwGOQmIMOM8CgHrTwXZoi1/baI\
-----END CERTIFICATE-----\
\
Equifax Secure CA\
=================\
\
MD5 Fingerprint=67:CB:9D:C0:13:24:8A:82:9B:B2:17:1E:D1:1B:EC:D4\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 903804111 (0x35def4cf)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=Equifax, OU=Equifax Secure Certificate Authority\
        Validity\
            Not Before: Aug 22 16:41:51 1998 GMT\
            Not After : Aug 22 16:41:51 2018 GMT\
        Subject: C=US, O=Equifax, OU=Equifax Secure Certificate Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:c1:5d:b1:58:67:08:62:ee:a0:9a:2d:1f:08:6d:\
                    91:14:68:98:0a:1e:fe:da:04:6f:13:84:62:21:c3:\
                    d1:7c:ce:9f:05:e0:b8:01:f0:4e:34:ec:e2:8a:95:\
                    04:64:ac:f1:6b:53:5f:05:b3:cb:67:80:bf:42:02:\
                    8e:fe:dd:01:09:ec:e1:00:14:4f:fc:fb:f0:0c:dd:\
                    43:ba:5b:2b:e1:1f:80:70:99:15:57:93:16:f1:0f:\
                    97:6a:b7:c2:68:23:1c:cc:4d:59:30:ac:51:1e:3b:\
                    af:2b:d6:ee:63:45:7b:c5:d9:5f:50:d2:e3:50:0f:\
                    3a:88:e7:bf:14:fd:e0:c7:b9\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 CRL Distribution Points: \
                DirName:/C=US/O=Equifax/OU=Equifax Secure Certificate Authority/CN=CRL1\
\
            X509v3 Private Key Usage Period: \
                Not After: Aug 22 16:41:51 2018 GMT\
            X509v3 Key Usage: \
                Certificate Sign, CRL Sign\
            X509v3 Authority Key Identifier: \
                keyid:48:E6:68:F9:2B:D2:B2:95:D7:47:D8:23:20:10:4F:33:98:90:9F:D4\
\
            X509v3 Subject Key Identifier: \
                48:E6:68:F9:2B:D2:B2:95:D7:47:D8:23:20:10:4F:33:98:90:9F:D4\
            X509v3 Basic Constraints: \
                CA:TRUE\
            1.2.840.113533.7.65.0: \
                0...V3.0c....\
    Signature Algorithm: sha1WithRSAEncryption\
        58:ce:29:ea:fc:f7:de:b5:ce:02:b9:17:b5:85:d1:b9:e3:e0:\
        95:cc:25:31:0d:00:a6:92:6e:7f:b6:92:63:9e:50:95:d1:9a:\
        6f:e4:11:de:63:85:6e:98:ee:a8:ff:5a:c8:d3:55:b2:66:71:\
        57:de:c0:21:eb:3d:2a:a7:23:49:01:04:86:42:7b:fc:ee:7f:\
        a2:16:52:b5:67:67:d3:40:db:3b:26:58:b2:28:77:3d:ae:14:\
        77:61:d6:fa:2a:66:27:a0:0d:fa:a7:73:5c:ea:70:f1:94:21:\
        65:44:5f:fa:fc:ef:29:68:a9:a2:87:79:ef:79:ef:4f:ac:07:\
        77:38\
-----BEGIN CERTIFICATE-----\
MIIDIDCCAomgAwIBAgIENd70zzANBgkqhkiG9w0BAQUFADBOMQswCQYDVQQGEwJV\
UzEQMA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2Vy\
dGlmaWNhdGUgQXV0aG9yaXR5MB4XDTk4MDgyMjE2NDE1MVoXDTE4MDgyMjE2NDE1\
MVowTjELMAkGA1UEBhMCVVMxEDAOBgNVBAoTB0VxdWlmYXgxLTArBgNVBAsTJEVx\
dWlmYXggU2VjdXJlIENlcnRpZmljYXRlIEF1dGhvcml0eTCBnzANBgkqhkiG9w0B\
AQEFAAOBjQAwgYkCgYEAwV2xWGcIYu6gmi0fCG2RFGiYCh7+2gRvE4RiIcPRfM6f\
BeC4AfBONOziipUEZKzxa1NfBbPLZ4C/QgKO/t0BCezhABRP/PvwDN1Dulsr4R+A\
cJkVV5MW8Q+XarfCaCMczE1ZMKxRHjuvK9buY0V7xdlfUNLjUA86iOe/FP3gx7kC\
AwEAAaOCAQkwggEFMHAGA1UdHwRpMGcwZaBjoGGkXzBdMQswCQYDVQQGEwJVUzEQ\
MA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2VydGlm\
aWNhdGUgQXV0aG9yaXR5MQ0wCwYDVQQDEwRDUkwxMBoGA1UdEAQTMBGBDzIwMTgw\
ODIyMTY0MTUxWjALBgNVHQ8EBAMCAQYwHwYDVR0jBBgwFoAUSOZo+SvSspXXR9gj\
IBBPM5iQn9QwHQYDVR0OBBYEFEjmaPkr0rKV10fYIyAQTzOYkJ/UMAwGA1UdEwQF\
MAMBAf8wGgYJKoZIhvZ9B0EABA0wCxsFVjMuMGMDAgbAMA0GCSqGSIb3DQEBBQUA\
A4GBAFjOKer89961zgK5F7WF0bnj4JXMJTENAKaSbn+2kmOeUJXRmm/kEd5jhW6Y\
7qj/WsjTVbJmcVfewCHrPSqnI0kBBIZCe/zuf6IWUrVnZ9NA2zsmWLIodz2uFHdh\
1voqZiegDfqnc1zqcPGUIWVEX/r87yloqaKHee9570+sB3c4\
-----END CERTIFICATE-----\
\
NetLock Business (Class B) Root\
===============================\
\
MD5 Fingerprint=39:16:AA:B9:6A:41:E1:14:69:DF:9E:6C:3B:72:DC:B6\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 105 (0x69)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=HU, L=Budapest, O=NetLock Halozatbiztonsagi Kft., OU=Tanusitvanykiadok, CN=NetLock Uzleti (Class B) Tanusitvanykiado\
        Validity\
            Not Before: Feb 25 14:10:22 1999 GMT\
            Not After : Feb 20 14:10:22 2019 GMT\
        Subject: C=HU, L=Budapest, O=NetLock Halozatbiztonsagi Kft., OU=Tanusitvanykiadok, CN=NetLock Uzleti (Class B) Tanusitvanykiado\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:b1:ea:04:ec:20:a0:23:c2:8f:38:60:cf:c7:46:\
                    b3:d5:1b:fe:fb:b9:99:9e:04:dc:1c:7f:8c:4a:81:\
                    98:ee:a4:d4:ca:8a:17:b9:22:7f:83:0a:75:4c:9b:\
                    c0:69:d8:64:39:a3:ed:92:a3:fd:5b:5c:74:1a:c0:\
                    47:ca:3a:69:76:9a:ba:e2:44:17:fc:4c:a3:d5:fe:\
                    b8:97:88:af:88:03:89:1f:a4:f2:04:3e:c8:07:0b:\
                    e6:f9:b3:2f:7a:62:14:09:46:14:ca:64:f5:8b:80:\
                    b5:62:a8:d8:6b:d6:71:93:2d:b3:bf:09:54:58:ed:\
                    06:eb:a8:7b:dc:43:b1:a1:69\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE, pathlen:4\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            Netscape Comment: \
                FIGYELEM! Ezen tanusitvany a NetLock Kft. Altalanos Szolgaltatasi Felteteleiben leirt eljarasok alapjan keszult. A hitelesites folyamatat a NetLock Kft. termekfelelosseg-biztositasa vedi. A digitalis alairas elfogadasanak feltetele az eloirt ellenorzesi eljaras megtetele. Az eljaras leirasa megtalalhato a NetLock Kft. Internet honlapjan a https://www.netlock.net/docs cimen vagy kerheto az ellenorzes@netlock.net e-mail cimen. IMPORTANT! The issuance and the use of this certificate is subject to the NetLock CPS available at https://www.netlock.net/docs or by e-mail at cps@netlock.net.\
    Signature Algorithm: md5WithRSAEncryption\
        04:db:ae:8c:17:af:f8:0e:90:31:4e:cd:3e:09:c0:6d:3a:b0:\
        f8:33:4c:47:4c:e3:75:88:10:97:ac:b0:38:15:91:c6:29:96:\
        cc:21:c0:6d:3c:a5:74:cf:d8:82:a5:39:c3:65:e3:42:70:bb:\
        22:90:e3:7d:db:35:76:e1:a0:b5:da:9f:70:6e:93:1a:30:39:\
        1d:30:db:2e:e3:7c:b2:91:b2:d1:37:29:fa:b9:d6:17:5c:47:\
        4f:e3:1d:38:eb:9f:d5:7b:95:a8:28:9e:15:4a:d1:d1:d0:2b:\
        00:97:a0:e2:92:36:2b:63:ac:58:01:6b:33:29:50:86:83:f1:\
        01:48\
-----BEGIN CERTIFICATE-----\
MIIFSzCCBLSgAwIBAgIBaTANBgkqhkiG9w0BAQQFADCBmTELMAkGA1UEBhMCSFUx\
ETAPBgNVBAcTCEJ1ZGFwZXN0MScwJQYDVQQKEx5OZXRMb2NrIEhhbG96YXRiaXp0\
b25zYWdpIEtmdC4xGjAYBgNVBAsTEVRhbnVzaXR2YW55a2lhZG9rMTIwMAYDVQQD\
EylOZXRMb2NrIFV6bGV0aSAoQ2xhc3MgQikgVGFudXNpdHZhbnlraWFkbzAeFw05\
OTAyMjUxNDEwMjJaFw0xOTAyMjAxNDEwMjJaMIGZMQswCQYDVQQGEwJIVTERMA8G\
A1UEBxMIQnVkYXBlc3QxJzAlBgNVBAoTHk5ldExvY2sgSGFsb3phdGJpenRvbnNh\
Z2kgS2Z0LjEaMBgGA1UECxMRVGFudXNpdHZhbnlraWFkb2sxMjAwBgNVBAMTKU5l\
dExvY2sgVXpsZXRpIChDbGFzcyBCKSBUYW51c2l0dmFueWtpYWRvMIGfMA0GCSqG\
SIb3DQEBAQUAA4GNADCBiQKBgQCx6gTsIKAjwo84YM/HRrPVG/77uZmeBNwcf4xK\
gZjupNTKihe5In+DCnVMm8Bp2GQ5o+2So/1bXHQawEfKOml2mrriRBf8TKPV/riX\
iK+IA4kfpPIEPsgHC+b5sy96YhQJRhTKZPWLgLViqNhr1nGTLbO/CVRY7QbrqHvc\
Q7GhaQIDAQABo4ICnzCCApswEgYDVR0TAQH/BAgwBgEB/wIBBDAOBgNVHQ8BAf8E\
BAMCAAYwEQYJYIZIAYb4QgEBBAQDAgAHMIICYAYJYIZIAYb4QgENBIICURaCAk1G\
SUdZRUxFTSEgRXplbiB0YW51c2l0dmFueSBhIE5ldExvY2sgS2Z0LiBBbHRhbGFu\
b3MgU3pvbGdhbHRhdGFzaSBGZWx0ZXRlbGVpYmVuIGxlaXJ0IGVsamFyYXNvayBh\
bGFwamFuIGtlc3p1bHQuIEEgaGl0ZWxlc2l0ZXMgZm9seWFtYXRhdCBhIE5ldExv\
Y2sgS2Z0LiB0ZXJtZWtmZWxlbG9zc2VnLWJpenRvc2l0YXNhIHZlZGkuIEEgZGln\
aXRhbGlzIGFsYWlyYXMgZWxmb2dhZGFzYW5hayBmZWx0ZXRlbGUgYXogZWxvaXJ0\
IGVsbGVub3J6ZXNpIGVsamFyYXMgbWVndGV0ZWxlLiBBeiBlbGphcmFzIGxlaXJh\
c2EgbWVndGFsYWxoYXRvIGEgTmV0TG9jayBLZnQuIEludGVybmV0IGhvbmxhcGph\
biBhIGh0dHBzOi8vd3d3Lm5ldGxvY2submV0L2RvY3MgY2ltZW4gdmFneSBrZXJo\
ZXRvIGF6IGVsbGVub3J6ZXNAbmV0bG9jay5uZXQgZS1tYWlsIGNpbWVuLiBJTVBP\
UlRBTlQhIFRoZSBpc3N1YW5jZSBhbmQgdGhlIHVzZSBvZiB0aGlzIGNlcnRpZmlj\
YXRlIGlzIHN1YmplY3QgdG8gdGhlIE5ldExvY2sgQ1BTIGF2YWlsYWJsZSBhdCBo\
dHRwczovL3d3dy5uZXRsb2NrLm5ldC9kb2NzIG9yIGJ5IGUtbWFpbCBhdCBjcHNA\
bmV0bG9jay5uZXQuMA0GCSqGSIb3DQEBBAUAA4GBAATbrowXr/gOkDFOzT4JwG06\
sPgzTEdM43WIEJessDgVkcYplswhwG08pXTP2IKlOcNl40JwuyKQ433bNXbhoLXa\
n3BukxowOR0w2y7jfLKRstE3Kfq51hdcR0/jHTjrn9V7lagonhVK0dHQKwCXoOKS\
NitjrFgBazMpUIaD8QFI\
-----END CERTIFICATE-----\
\
NetLock Express (Class C) Root\
==============================\
\
MD5 Fingerprint=4F:EB:F1:F0:70:C2:80:63:5D:58:9F:DA:12:3C:A9:C4\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 104 (0x68)\
        Signature Algorithm: md5WithRSAEncryption\
        Issuer: C=HU, L=Budapest, O=NetLock Halozatbiztonsagi Kft., OU=Tanusitvanykiadok, CN=NetLock Expressz (Class C) Tanusitvanykiado\
        Validity\
            Not Before: Feb 25 14:08:11 1999 GMT\
            Not After : Feb 20 14:08:11 2019 GMT\
        Subject: C=HU, L=Budapest, O=NetLock Halozatbiztonsagi Kft., OU=Tanusitvanykiadok, CN=NetLock Expressz (Class C) Tanusitvanykiado\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (1024 bit)\
                Modulus (1024 bit):\
                    00:eb:ec:b0:6c:61:8a:23:25:af:60:20:e3:d9:9f:\
                    fc:93:0b:db:5d:8d:b0:a1:b3:40:3a:82:ce:fd:75:\
                    e0:78:32:03:86:5a:86:95:91:ed:53:fa:9d:40:fc:\
                    e6:e8:dd:d9:5b:7a:03:bd:5d:f3:3b:0c:c3:51:79:\
                    9b:ad:55:a0:e9:d0:03:10:af:0a:ba:14:42:d9:52:\
                    26:11:22:c7:d2:20:cc:82:a4:9a:a9:fe:b8:81:76:\
                    9d:6a:b7:d2:36:75:3e:b1:86:09:f6:6e:6d:7e:4e:\
                    b7:7a:ec:ae:71:84:f6:04:33:08:25:32:eb:74:ac:\
                    16:44:c6:e4:40:93:1d:7f:ad\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            X509v3 Basic Constraints: critical\
                CA:TRUE, pathlen:4\
            X509v3 Key Usage: critical\
                Certificate Sign, CRL Sign\
            Netscape Cert Type: \
                SSL CA, S/MIME CA, Object Signing CA\
            Netscape Comment: \
                FIGYELEM! Ezen tanusitvany a NetLock Kft. Altalanos Szolgaltatasi Felteteleiben leirt eljarasok alapjan keszult. A hitelesites folyamatat a NetLock Kft. termekfelelosseg-biztositasa vedi. A digitalis alairas elfogadasanak feltetele az eloirt ellenorzesi eljaras megtetele. Az eljaras leirasa megtalalhato a NetLock Kft. Internet honlapjan a https://www.netlock.net/docs cimen vagy kerheto az ellenorzes@netlock.net e-mail cimen. IMPORTANT! The issuance and the use of this certificate is subject to the NetLock CPS available at https://www.netlock.net/docs or by e-mail at cps@netlock.net.\
    Signature Algorithm: md5WithRSAEncryption\
        10:ad:7f:d7:0c:32:80:0a:d8:86:f1:79:98:b5:ad:d4:cd:b3:\
        36:c4:96:48:c1:5c:cd:9a:d9:05:2e:9f:be:50:eb:f4:26:14:\
        10:2d:d4:66:17:f8:9e:c1:27:fd:f1:ed:e4:7b:4b:a0:6c:b5:\
        ab:9a:57:70:a6:ed:a0:a4:ed:2e:f5:fd:fc:bd:fe:4d:37:08:\
        0c:bc:e3:96:83:22:f5:49:1b:7f:4b:2b:b4:54:c1:80:7c:99:\
        4e:1d:d0:8c:ee:d0:ac:e5:92:fa:75:56:fe:64:a0:13:8f:b8:\
        b8:16:9d:61:05:67:80:c8:d0:d8:a5:07:02:34:98:04:8d:33:\
        04:d4\
-----BEGIN CERTIFICATE-----\
MIIFTzCCBLigAwIBAgIBaDANBgkqhkiG9w0BAQQFADCBmzELMAkGA1UEBhMCSFUx\
ETAPBgNVBAcTCEJ1ZGFwZXN0MScwJQYDVQQKEx5OZXRMb2NrIEhhbG96YXRiaXp0\
b25zYWdpIEtmdC4xGjAYBgNVBAsTEVRhbnVzaXR2YW55a2lhZG9rMTQwMgYDVQQD\
EytOZXRMb2NrIEV4cHJlc3N6IChDbGFzcyBDKSBUYW51c2l0dmFueWtpYWRvMB4X\
DTk5MDIyNTE0MDgxMVoXDTE5MDIyMDE0MDgxMVowgZsxCzAJBgNVBAYTAkhVMREw\
DwYDVQQHEwhCdWRhcGVzdDEnMCUGA1UEChMeTmV0TG9jayBIYWxvemF0Yml6dG9u\
c2FnaSBLZnQuMRowGAYDVQQLExFUYW51c2l0dmFueWtpYWRvazE0MDIGA1UEAxMr\
TmV0TG9jayBFeHByZXNzeiAoQ2xhc3MgQykgVGFudXNpdHZhbnlraWFkbzCBnzAN\
BgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA6+ywbGGKIyWvYCDj2Z/8kwvbXY2wobNA\
OoLO/XXgeDIDhlqGlZHtU/qdQPzm6N3ZW3oDvV3zOwzDUXmbrVWg6dADEK8KuhRC\
2VImESLH0iDMgqSaqf64gXadarfSNnU+sYYJ9m5tfk63euyucYT2BDMIJTLrdKwW\
RMbkQJMdf60CAwEAAaOCAp8wggKbMBIGA1UdEwEB/wQIMAYBAf8CAQQwDgYDVR0P\
AQH/BAQDAgAGMBEGCWCGSAGG+EIBAQQEAwIABzCCAmAGCWCGSAGG+EIBDQSCAlEW\
ggJNRklHWUVMRU0hIEV6ZW4gdGFudXNpdHZhbnkgYSBOZXRMb2NrIEtmdC4gQWx0\
YWxhbm9zIFN6b2xnYWx0YXRhc2kgRmVsdGV0ZWxlaWJlbiBsZWlydCBlbGphcmFz\
b2sgYWxhcGphbiBrZXN6dWx0LiBBIGhpdGVsZXNpdGVzIGZvbHlhbWF0YXQgYSBO\
ZXRMb2NrIEtmdC4gdGVybWVrZmVsZWxvc3NlZy1iaXp0b3NpdGFzYSB2ZWRpLiBB\
IGRpZ2l0YWxpcyBhbGFpcmFzIGVsZm9nYWRhc2FuYWsgZmVsdGV0ZWxlIGF6IGVs\
b2lydCBlbGxlbm9yemVzaSBlbGphcmFzIG1lZ3RldGVsZS4gQXogZWxqYXJhcyBs\
ZWlyYXNhIG1lZ3RhbGFsaGF0byBhIE5ldExvY2sgS2Z0LiBJbnRlcm5ldCBob25s\
YXBqYW4gYSBodHRwczovL3d3dy5uZXRsb2NrLm5ldC9kb2NzIGNpbWVuIHZhZ3kg\
a2VyaGV0byBheiBlbGxlbm9yemVzQG5ldGxvY2submV0IGUtbWFpbCBjaW1lbi4g\
SU1QT1JUQU5UISBUaGUgaXNzdWFuY2UgYW5kIHRoZSB1c2Ugb2YgdGhpcyBjZXJ0\
aWZpY2F0ZSBpcyBzdWJqZWN0IHRvIHRoZSBOZXRMb2NrIENQUyBhdmFpbGFibGUg\
YXQgaHR0cHM6Ly93d3cubmV0bG9jay5uZXQvZG9jcyBvciBieSBlLW1haWwgYXQg\
Y3BzQG5ldGxvY2submV0LjANBgkqhkiG9w0BAQQFAAOBgQAQrX/XDDKACtiG8XmY\
ta3UzbM2xJZIwVzNmtkFLp++UOv0JhQQLdRmF/iewSf98e3ke0ugbLWrmldwpu2g\
pO0u9f38vf5NNwgMvOOWgyL1SRt/Syu0VMGAfJlOHdCM7tCs5ZL6dVb+ZKATj7i4\
Fp1hBWeAyNDYpQcCNJgEjTME1A==\
-----END CERTIFICATE-----\
\
XRamp Global CA Root\
====================\
\
MD5 Fingerprint=A1:0B:44:B3:CA:10:D8:00:6E:9D:0F:D8:0F:92:0A:D1\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number:\
            50:94:6c:ec:18:ea:d5:9c:4d:d5:97:ef:75:8f:a0:ad\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, OU=www.xrampsecurity.com, O=XRamp Security Services Inc, CN=XRamp Global Certification Authority\
        Validity\
            Not Before: Nov  1 17:14:04 2004 GMT\
            Not After : Jan  1 05:37:19 2035 GMT\
        Subject: C=US, OU=www.xrampsecurity.com, O=XRamp Security Services Inc, CN=XRamp Global Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:98:24:1e:bd:15:b4:ba:df:c7:8c:a5:27:b6:38:\
                    0b:69:f3:b6:4e:a8:2c:2e:21:1d:5c:44:df:21:5d:\
                    7e:23:74:fe:5e:7e:b4:4a:b7:a6:ad:1f:ae:e0:06:\
                    16:e2:9b:5b:d9:67:74:6b:5d:80:8f:29:9d:86:1b:\
                    d9:9c:0d:98:6d:76:10:28:58:e4:65:b0:7f:4a:98:\
                    79:9f:e0:c3:31:7e:80:2b:b5:8c:c0:40:3b:11:86:\
                    d0:cb:a2:86:36:60:a4:d5:30:82:6d:d9:6e:d0:0f:\
                    12:04:33:97:5f:4f:61:5a:f0:e4:f9:91:ab:e7:1d:\
                    3b:bc:e8:cf:f4:6b:2d:34:7c:e2:48:61:1c:8e:f3:\
                    61:44:cc:6f:a0:4a:a9:94:b0:4d:da:e7:a9:34:7a:\
                    72:38:a8:41:cc:3c:94:11:7d:eb:c8:a6:8c:b7:86:\
                    cb:ca:33:3b:d9:3d:37:8b:fb:7a:3e:86:2c:e7:73:\
                    d7:0a:57:ac:64:9b:19:eb:f4:0f:04:08:8a:ac:03:\
                    17:19:64:f4:5a:25:22:8d:34:2c:b2:f6:68:1d:12:\
                    6d:d3:8a:1e:14:da:c4:8f:a6:e2:23:85:d5:7a:0d:\
                    bd:6a:e0:e9:ec:ec:17:bb:42:1b:67:aa:25:ed:45:\
                    83:21:fc:c1:c9:7c:d5:62:3e:fa:f2:c5:2d:d3:fd:\
                    d4:65\
                Exponent: 65537 (0x10001)\
        X509v3 extensions:\
            1.3.6.1.4.1.311.20.2: \
                ...C.A\
            X509v3 Key Usage: \
                Digital Signature, Certificate Sign, CRL Sign\
            X509v3 Basic Constraints: critical\
                CA:TRUE\
            X509v3 Subject Key Identifier: \
                C6:4F:A2:3D:06:63:84:09:9C:CE:62:E4:04:AC:8D:5C:B5:E9:B6:1B\
            X509v3 CRL Distribution Points: \
                URI:http://crl.xrampsecurity.com/XGCA.crl\
\
            1.3.6.1.4.1.311.21.1: \
                ...\
    Signature Algorithm: sha1WithRSAEncryption\
        91:15:39:03:01:1b:67:fb:4a:1c:f9:0a:60:5b:a1:da:4d:97:\
        62:f9:24:53:27:d7:82:64:4e:90:2e:c3:49:1b:2b:9a:dc:fc:\
        a8:78:67:35:f1:1d:f0:11:bd:b7:48:e3:10:f6:0d:df:3f:d2:\
        c9:b6:aa:55:a4:48:ba:02:db:de:59:2e:15:5b:3b:9d:16:7d:\
        47:d7:37:ea:5f:4d:76:12:36:bb:1f:d7:a1:81:04:46:20:a3:\
        2c:6d:a9:9e:01:7e:3f:29:ce:00:93:df:fd:c9:92:73:89:89:\
        64:9e:e7:2b:e4:1c:91:2c:d2:b9:ce:7d:ce:6f:31:99:d3:e6:\
        be:d2:1e:90:f0:09:14:79:5c:23:ab:4d:d2:da:21:1f:4d:99:\
        79:9d:e1:cf:27:9f:10:9b:1c:88:0d:b0:8a:64:41:31:b8:0e:\
        6c:90:24:a4:9b:5c:71:8f:ba:bb:7e:1c:1b:db:6a:80:0f:21:\
        bc:e9:db:a6:b7:40:f4:b2:8b:a9:b1:e4:ef:9a:1a:d0:3d:69:\
        99:ee:a8:28:a3:e1:3c:b3:f0:b2:11:9c:cf:7c:40:e6:dd:e7:\
        43:7d:a2:d8:3a:b5:a9:8d:f2:34:99:c4:d4:10:e1:06:fd:09:\
        84:10:3b:ee:c4:4c:f4:ec:27:7c:42:c2:74:7c:82:8a:09:c9:\
        b4:03:25:bc\
-----BEGIN CERTIFICATE-----\
MIIEMDCCAxigAwIBAgIQUJRs7Bjq1ZxN1ZfvdY+grTANBgkqhkiG9w0BAQUFADCB\
gjELMAkGA1UEBhMCVVMxHjAcBgNVBAsTFXd3dy54cmFtcHNlY3VyaXR5LmNvbTEk\
MCIGA1UEChMbWFJhbXAgU2VjdXJpdHkgU2VydmljZXMgSW5jMS0wKwYDVQQDEyRY\
UmFtcCBHbG9iYWwgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDQxMTAxMTcx\
NDA0WhcNMzUwMTAxMDUzNzE5WjCBgjELMAkGA1UEBhMCVVMxHjAcBgNVBAsTFXd3\
dy54cmFtcHNlY3VyaXR5LmNvbTEkMCIGA1UEChMbWFJhbXAgU2VjdXJpdHkgU2Vy\
dmljZXMgSW5jMS0wKwYDVQQDEyRYUmFtcCBHbG9iYWwgQ2VydGlmaWNhdGlvbiBB\
dXRob3JpdHkwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCYJB69FbS6\
38eMpSe2OAtp87ZOqCwuIR1cRN8hXX4jdP5efrRKt6atH67gBhbim1vZZ3RrXYCP\
KZ2GG9mcDZhtdhAoWORlsH9KmHmf4MMxfoArtYzAQDsRhtDLooY2YKTVMIJt2W7Q\
DxIEM5dfT2Fa8OT5kavnHTu86M/0ay00fOJIYRyO82FEzG+gSqmUsE3a56k0enI4\
qEHMPJQRfevIpoy3hsvKMzvZPTeL+3o+hiznc9cKV6xkmxnr9A8ECIqsAxcZZPRa\
JSKNNCyy9mgdEm3Tih4U2sSPpuIjhdV6Db1q4Ons7Be7QhtnqiXtRYMh/MHJfNVi\
PvryxS3T/dRlAgMBAAGjgZ8wgZwwEwYJKwYBBAGCNxQCBAYeBABDAEEwCwYDVR0P\
BAQDAgGGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFMZPoj0GY4QJnM5i5ASs\
jVy16bYbMDYGA1UdHwQvMC0wK6ApoCeGJWh0dHA6Ly9jcmwueHJhbXBzZWN1cml0\
eS5jb20vWEdDQS5jcmwwEAYJKwYBBAGCNxUBBAMCAQEwDQYJKoZIhvcNAQEFBQAD\
ggEBAJEVOQMBG2f7Shz5CmBbodpNl2L5JFMn14JkTpAuw0kbK5rc/Kh4ZzXxHfAR\
vbdI4xD2Dd8/0sm2qlWkSLoC295ZLhVbO50WfUfXN+pfTXYSNrsf16GBBEYgoyxt\
qZ4Bfj8pzgCT3/3JknOJiWSe5yvkHJEs0rnOfc5vMZnT5r7SHpDwCRR5XCOrTdLa\
IR9NmXmd4c8nnxCbHIgNsIpkQTG4DmyQJKSbXHGPurt+HBvbaoAPIbzp26a3QPSy\
i6mx5O+aGtA9aZnuqCij4Tyz8LIRnM98QObd50N9otg6tamN8jSZxNQQ4Qb9CYQQ\
O+7ETPTsJ3xCwnR8gooJybQDJbw=\
-----END CERTIFICATE-----\
\
Go Daddy Class 2 CA\
===================\
\
MD5 Fingerprint=91:DE:06:25:AB:DA:FD:32:17:0C:BB:25:17:2A:84:67\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=The Go Daddy Group, Inc., OU=Go Daddy Class 2 Certification Authority\
        Validity\
            Not Before: Jun 29 17:06:20 2004 GMT\
            Not After : Jun 29 17:06:20 2034 GMT\
        Subject: C=US, O=The Go Daddy Group, Inc., OU=Go Daddy Class 2 Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:de:9d:d7:ea:57:18:49:a1:5b:eb:d7:5f:48:86:\
                    ea:be:dd:ff:e4:ef:67:1c:f4:65:68:b3:57:71:a0:\
                    5e:77:bb:ed:9b:49:e9:70:80:3d:56:18:63:08:6f:\
                    da:f2:cc:d0:3f:7f:02:54:22:54:10:d8:b2:81:d4:\
                    c0:75:3d:4b:7f:c7:77:c3:3e:78:ab:1a:03:b5:20:\
                    6b:2f:6a:2b:b1:c5:88:7e:c4:bb:1e:b0:c1:d8:45:\
                    27:6f:aa:37:58:f7:87:26:d7:d8:2d:f6:a9:17:b7:\
                    1f:72:36:4e:a6:17:3f:65:98:92:db:2a:6e:5d:a2:\
                    fe:88:e0:0b:de:7f:e5:8d:15:e1:eb:cb:3a:d5:e2:\
                    12:a2:13:2d:d8:8e:af:5f:12:3d:a0:08:05:08:b6:\
                    5c:a5:65:38:04:45:99:1e:a3:60:60:74:c5:41:a5:\
                    72:62:1b:62:c5:1f:6f:5f:1a:42:be:02:51:65:a8:\
                    ae:23:18:6a:fc:78:03:a9:4d:7f:80:c3:fa:ab:5a:\
                    fc:a1:40:a4:ca:19:16:fe:b2:c8:ef:5e:73:0d:ee:\
                    77:bd:9a:f6:79:98:bc:b1:07:67:a2:15:0d:dd:a0:\
                    58:c6:44:7b:0a:3e:62:28:5f:ba:41:07:53:58:cf:\
                    11:7e:38:74:c5:f8:ff:b5:69:90:8f:84:74:ea:97:\
                    1b:af\
                Exponent: 3 (0x3)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                D2:C4:B0:D2:91:D4:4C:11:71:B3:61:CB:3D:A1:FE:DD:A8:6A:D4:E3\
            X509v3 Authority Key Identifier: \
                keyid:D2:C4:B0:D2:91:D4:4C:11:71:B3:61:CB:3D:A1:FE:DD:A8:6A:D4:E3\
                DirName:/C=US/O=The Go Daddy Group, Inc./OU=Go Daddy Class 2 Certification Authority\
                serial:00\
\
            X509v3 Basic Constraints: \
                CA:TRUE\
    Signature Algorithm: sha1WithRSAEncryption\
        32:4b:f3:b2:ca:3e:91:fc:12:c6:a1:07:8c:8e:77:a0:33:06:\
        14:5c:90:1e:18:f7:08:a6:3d:0a:19:f9:87:80:11:6e:69:e4:\
        96:17:30:ff:34:91:63:72:38:ee:cc:1c:01:a3:1d:94:28:a4:\
        31:f6:7a:c4:54:d7:f6:e5:31:58:03:a2:cc:ce:62:db:94:45:\
        73:b5:bf:45:c9:24:b5:d5:82:02:ad:23:79:69:8d:b8:b6:4d:\
        ce:cf:4c:ca:33:23:e8:1c:88:aa:9d:8b:41:6e:16:c9:20:e5:\
        89:9e:cd:3b:da:70:f7:7e:99:26:20:14:54:25:ab:6e:73:85:\
        e6:9b:21:9d:0a:6c:82:0e:a8:f8:c2:0c:fa:10:1e:6c:96:ef:\
        87:0d:c4:0f:61:8b:ad:ee:83:2b:95:f8:8e:92:84:72:39:eb:\
        20:ea:83:ed:83:cd:97:6e:08:bc:eb:4e:26:b6:73:2b:e4:d3:\
        f6:4c:fe:26:71:e2:61:11:74:4a:ff:57:1a:87:0f:75:48:2e:\
        cf:51:69:17:a0:02:12:61:95:d5:d1:40:b2:10:4c:ee:c4:ac:\
        10:43:a6:a5:9e:0a:d5:95:62:9a:0d:cf:88:82:c5:32:0c:e4:\
        2b:9f:45:e6:0d:9f:28:9c:b1:b9:2a:5a:57:ad:37:0f:af:1d:\
        7f:db:bd:9f\
-----BEGIN CERTIFICATE-----\
MIIEADCCAuigAwIBAgIBADANBgkqhkiG9w0BAQUFADBjMQswCQYDVQQGEwJVUzEh\
MB8GA1UEChMYVGhlIEdvIERhZGR5IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBE\
YWRkeSBDbGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTA0MDYyOTE3\
MDYyMFoXDTM0MDYyOTE3MDYyMFowYzELMAkGA1UEBhMCVVMxITAfBgNVBAoTGFRo\
ZSBHbyBEYWRkeSBHcm91cCwgSW5jLjExMC8GA1UECxMoR28gRGFkZHkgQ2xhc3Mg\
MiBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTCCASAwDQYJKoZIhvcNAQEBBQADggEN\
ADCCAQgCggEBAN6d1+pXGEmhW+vXX0iG6r7d/+TvZxz0ZWizV3GgXne77ZtJ6XCA\
PVYYYwhv2vLM0D9/AlQiVBDYsoHUwHU9S3/Hd8M+eKsaA7Ugay9qK7HFiH7Eux6w\
wdhFJ2+qN1j3hybX2C32qRe3H3I2TqYXP2WYktsqbl2i/ojgC95/5Y0V4evLOtXi\
EqITLdiOr18SPaAIBQi2XKVlOARFmR6jYGB0xUGlcmIbYsUfb18aQr4CUWWoriMY\
avx4A6lNf4DD+qta/KFApMoZFv6yyO9ecw3ud72a9nmYvLEHZ6IVDd2gWMZEewo+\
YihfukEHU1jPEX44dMX4/7VpkI+EdOqXG68CAQOjgcAwgb0wHQYDVR0OBBYEFNLE\
sNKR1EwRcbNhyz2h/t2oatTjMIGNBgNVHSMEgYUwgYKAFNLEsNKR1EwRcbNhyz2h\
/t2oatTjoWekZTBjMQswCQYDVQQGEwJVUzEhMB8GA1UEChMYVGhlIEdvIERhZGR5\
IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBEYWRkeSBDbGFzcyAyIENlcnRpZmlj\
YXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEFBQAD\
ggEBADJL87LKPpH8EsahB4yOd6AzBhRckB4Y9wimPQoZ+YeAEW5p5JYXMP80kWNy\
OO7MHAGjHZQopDH2esRU1/blMVgDoszOYtuURXO1v0XJJLXVggKtI3lpjbi2Tc7P\
TMozI+gciKqdi0FuFskg5YmezTvacPd+mSYgFFQlq25zheabIZ0KbIIOqPjCDPoQ\
HmyW74cNxA9hi63ugyuV+I6ShHI56yDqg+2DzZduCLzrTia2cyvk0/ZM/iZx4mER\
dEr/VxqHD3VILs9RaRegAhJhldXRQLIQTO7ErBBDpqWeCtWVYpoNz4iCxTIM5Cuf\
ReYNnyicsbkqWletNw+vHX/bvZ8=\
-----END CERTIFICATE-----\
\
Starfield Class 2 CA\
====================\
\
MD5 Fingerprint=32:4A:4B:BB:C8:63:69:9B:BE:74:9A:C6:DD:1D:46:24\
Certificate:\
    Data:\
        Version: 3 (0x2)\
        Serial Number: 0 (0x0)\
        Signature Algorithm: sha1WithRSAEncryption\
        Issuer: C=US, O=Starfield Technologies, Inc., OU=Starfield Class 2 Certification Authority\
        Validity\
            Not Before: Jun 29 17:39:16 2004 GMT\
            Not After : Jun 29 17:39:16 2034 GMT\
        Subject: C=US, O=Starfield Technologies, Inc., OU=Starfield Class 2 Certification Authority\
        Subject Public Key Info:\
            Public Key Algorithm: rsaEncryption\
            RSA Public Key: (2048 bit)\
                Modulus (2048 bit):\
                    00:b7:32:c8:fe:e9:71:a6:04:85:ad:0c:11:64:df:\
                    ce:4d:ef:c8:03:18:87:3f:a1:ab:fb:3c:a6:9f:f0:\
                    c3:a1:da:d4:d8:6e:2b:53:90:fb:24:a4:3e:84:f0:\
                    9e:e8:5f:ec:e5:27:44:f5:28:a6:3f:7b:de:e0:2a:\
                    f0:c8:af:53:2f:9e:ca:05:01:93:1e:8f:66:1c:39:\
                    a7:4d:fa:5a:b6:73:04:25:66:eb:77:7f:e7:59:c6:\
                    4a:99:25:14:54:eb:26:c7:f3:7f:19:d5:30:70:8f:\
                    af:b0:46:2a:ff:ad:eb:29:ed:d7:9f:aa:04:87:a3:\
                    d4:f9:89:a5:34:5f:db:43:91:82:36:d9:66:3c:b1:\
                    b8:b9:82:fd:9c:3a:3e:10:c8:3b:ef:06:65:66:7a:\
                    9b:19:18:3d:ff:71:51:3c:30:2e:5f:be:3d:77:73:\
                    b2:5d:06:6c:c3:23:56:9a:2b:85:26:92:1c:a7:02:\
                    b3:e4:3f:0d:af:08:79:82:b8:36:3d:ea:9c:d3:35:\
                    b3:bc:69:ca:f5:cc:9d:e8:fd:64:8d:17:80:33:6e:\
                    5e:4a:5d:99:c9:1e:87:b4:9d:1a:c0:d5:6e:13:35:\
                    23:5e:df:9b:5f:3d:ef:d6:f7:76:c2:ea:3e:bb:78:\
                    0d:1c:42:67:6b:04:d8:f8:d6:da:6f:8b:f2:44:a0:\
                    01:ab\
                Exponent: 3 (0x3)\
        X509v3 extensions:\
            X509v3 Subject Key Identifier: \
                BF:5F:B7:D1:CE:DD:1F:86:F4:5B:55:AC:DC:D7:10:C2:0E:A9:88:E7\
            X509v3 Authority Key Identifier: \
                keyid:BF:5F:B7:D1:CE:DD:1F:86:F4:5B:55:AC:DC:D7:10:C2:0E:A9:88:E7\
                DirName:/C=US/O=Starfield Technologies, Inc./OU=Starfield Class 2 Certification Authority\
                serial:00\
\
            X509v3 Basic Constraints: \
                CA:TRUE\
    Signature Algorithm: sha1WithRSAEncryption\
        05:9d:3f:88:9d:d1:c9:1a:55:a1:ac:69:f3:f3:59:da:9b:01:\
        87:1a:4f:57:a9:a1:79:09:2a:db:f7:2f:b2:1e:cc:c7:5e:6a:\
        d8:83:87:a1:97:ef:49:35:3e:77:06:41:58:62:bf:8e:58:b8:\
        0a:67:3f:ec:b3:dd:21:66:1f:c9:54:fa:72:cc:3d:4c:40:d8:\
        81:af:77:9e:83:7a:bb:a2:c7:f5:34:17:8e:d9:11:40:f4:fc:\
        2c:2a:4d:15:7f:a7:62:5d:2e:25:d3:00:0b:20:1a:1d:68:f9:\
        17:b8:f4:bd:8b:ed:28:59:dd:4d:16:8b:17:83:c8:b2:65:c7:\
        2d:7a:a5:aa:bc:53:86:6d:dd:57:a4:ca:f8:20:41:0b:68:f0:\
        f4:fb:74:be:56:5d:7a:79:f5:f9:1d:85:e3:2d:95:be:f5:71:\
        90:43:cc:8d:1f:9a:00:0a:87:29:e9:55:22:58:00:23:ea:e3:\
        12:43:29:5b:47:08:dd:8c:41:6a:65:06:a8:e5:21:aa:41:b4:\
        95:21:95:b9:7d:d1:34:ab:13:d6:ad:bc:dc:e2:3d:39:cd:bd:\
        3e:75:70:a1:18:59:03:c9:22:b4:8f:9c:d5:5e:2a:d7:a5:b6:\
        d4:0a:6d:f8:b7:40:11:46:9a:1f:79:0e:62:bf:0f:97:ec:e0:\
        2f:1f:17:94\
-----BEGIN CERTIFICATE-----\
MIIEDzCCAvegAwIBAgIBADANBgkqhkiG9w0BAQUFADBoMQswCQYDVQQGEwJVUzEl\
MCMGA1UEChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMp\
U3RhcmZpZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDQw\
NjI5MTczOTE2WhcNMzQwNjI5MTczOTE2WjBoMQswCQYDVQQGEwJVUzElMCMGA1UE\
ChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMpU3RhcmZp\
ZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwggEgMA0GCSqGSIb3\
DQEBAQUAA4IBDQAwggEIAoIBAQC3Msj+6XGmBIWtDBFk385N78gDGIc/oav7PKaf\
8MOh2tTYbitTkPskpD6E8J7oX+zlJ0T1KKY/e97gKvDIr1MvnsoFAZMej2YcOadN\
+lq2cwQlZut3f+dZxkqZJRRU6ybH838Z1TBwj6+wRir/resp7defqgSHo9T5iaU0\
X9tDkYI22WY8sbi5gv2cOj4QyDvvBmVmepsZGD3/cVE8MC5fvj13c7JdBmzDI1aa\
K4UmkhynArPkPw2vCHmCuDY96pzTNbO8acr1zJ3o/WSNF4Azbl5KXZnJHoe0nRrA\
1W4TNSNe35tfPe/W93bC6j67eA0cQmdrBNj41tpvi/JEoAGrAgEDo4HFMIHCMB0G\
A1UdDgQWBBS/X7fRzt0fhvRbVazc1xDCDqmI5zCBkgYDVR0jBIGKMIGHgBS/X7fR\
zt0fhvRbVazc1xDCDqmI56FspGowaDELMAkGA1UEBhMCVVMxJTAjBgNVBAoTHFN0\
YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xMjAwBgNVBAsTKVN0YXJmaWVsZCBD\
bGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8w\
DQYJKoZIhvcNAQEFBQADggEBAAWdP4id0ckaVaGsafPzWdqbAYcaT1epoXkJKtv3\
L7IezMdeatiDh6GX70k1PncGQVhiv45YuApnP+yz3SFmH8lU+nLMPUxA2IGvd56D\
eruix/U0F47ZEUD0/CwqTRV/p2JdLiXTAAsgGh1o+Re49L2L7ShZ3U0WixeDyLJl\
xy16paq8U4Zt3VekyvggQQto8PT7dL5WXXp59fkdheMtlb71cZBDzI0fmgAKhynp\
VSJYACPq4xJDKVtHCN2MQWplBqjlIapBtJUhlbl90TSrE9atvNziPTnNvT51cKEY\
WQPJIrSPnNVeKtelttQKbfi3QBFGmh95DmK/D5fs4C8fF5Q=\
-----END CERTIFICATE-----\
";

#ifdef __cplusplus
}
#endif
