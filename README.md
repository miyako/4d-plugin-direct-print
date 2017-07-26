# 4d-plugin-direct-print
Send raw data to printer

based on 

http://stackoverflow.com/questions/2044676/net-code-to-send-zpl-to-zebra-printers
http://stackoverflow.com/questions/4442122/send-raw-zpl-to-zebra-printer-via-usb

### Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|

### Version

<img src="https://cloud.githubusercontent.com/assets/1725068/18940649/21945000-8645-11e6-86ed-4a0f800e5a73.png" width="32" height="32" /> <img src="https://cloud.githubusercontent.com/assets/1725068/18940648/2192ddba-8645-11e6-864d-6d5692d55717.png" width="32" height="32" />

## Syntax

```
PRINT BLOB(printer;data;error{;type})
```

Parameter|Type|Description
------------|------------|----
printer|TEXT|
data|BLOB|
error|LONGINT|
type|TEXT|

```
PRINT BLOB ARRAY(printer;data;error{;type})
```

Parameter|Type|Description
------------|------------|----
printer|TEXT|
data|ARRAY BLOB|
error|LONGINT|
type|TEXT|

## Examples

```
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
```

Both commands accept an optional ``$4``, which is a MIME type on Mac (passed to [PMPrinterPrintWithProvider](https://developer.apple.com/reference/applicationservices/1461110-pmprinterprintwithprovider?language=objc)), or ``pDatatype`` of the [DOC_INFO_1](https://msdn.microsoft.com/en-us/library/windows/desktop/dd162471(v=vs.85).aspx) structure on Windows.

By default, the value is ``application/vnd.cups-raw `` on Mac, ``RAW`` on Windows.

To print PDF direcrtly on Mac
```
PRINT BLOB(Get current printer;$PDF;$ERR;"application/pdf")
```
