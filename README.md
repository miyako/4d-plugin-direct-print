# 4d-plugin-direct-print
Send raw data to printer on windows

based on 

http://stackoverflow.com/questions/2044676/net-code-to-send-zpl-to-zebra-printers
http://stackoverflow.com/questions/4442122/send-raw-zpl-to-zebra-printer-via-usb

##Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🚫|🚫|🆗|🆗|

Commands
---
PRINT BLOB
PRINT BLOB ARRAY
---

Examples
---
$printerName:="ZEBRA GK420D"
C_BLOB($ZPL)

  // Command to be sent to the printer
$code:="^XA^FO10,10,^AO,30,20^FDFDTesting^FS^FO10,30^BY3^BCN,100,Y,N,N^FDTesting^FS^XZ"

CONVERT FROM TEXT($code;"us-ascii";$ZPL)

C_LONGINT($ERR)
  //ERR_OPEN_PRINTER: -1
  //ERR_START_DOC_PRINTER: -2
  //ERR_WRITE_PRINTER: -3
  //ERR_END_DOC_PRINTER: -4

PRINT BLOB ($printerName;$ZPL;$ERR)

ARRAY BLOB($ZPLs;3)
$ZPLs{1}:=$ZPL
$ZPLs{2}:=$ZPL
$ZPLs{3}:=$ZPL

PRINT BLOB ARRAY ($printerName;$ZPLs;$ERR)
---
