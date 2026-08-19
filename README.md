![version](https://img.shields.io/badge/version-17%2B-3E8B93)
![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm20|%20win-64&color=blue)
[![license](https://img.shields.io/github/license/miyako/4d-plugin-direct-print)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/4d-plugin-direct-print/total)

# 4d-plugin-direct-print

Sends raw, pre-formatted data straight to a printer, bypassing 4D's own print engine and page layout entirely. On Windows it writes the bytes through the print spooler (`OpenPrinter`/`StartDocPrinter`/`WritePrinter`); on macOS it goes through Core Printing (`PMPrinterPrintWithProvider`), handing the raw bytes to the printer's driver tagged with a MIME type so the driver treats them as opaque data rather than something to rasterize. This is the mechanism you want for label/receipt printers that consume a command language directly — ZPL (Zebra), ESC/POS, raw PDF/PostScript passthrough — not for printing a normal 4D form or report.

Both commands return their result through an `error` output parameter, never as a 4D language error — always check it after the call.

## Summary

Command | Returns | Purpose
---|---|---
[`PRINT BLOB`](#print-blob) | — (error via param 3) | Send one BLOB of raw data to a named printer
[`PRINT BLOB ARRAY`](#print-blob-array) | — (error via param 3) | Send an array of BLOBs to a named printer, one per page

**Platforms:** macOS (Intel & Apple Silicon) and Windows (64-bit). 4D has no Linux runtime, so there is no Linux build.

---

## Requirements & platform notes

- **Both commands take 3 mandatory parameters and one optional parameter** — `printer`, `data`, `error`, and an optional `type`. There is no shorter form.
- **Errors never raise a 4D error** — they're written back into the `error` (LONGINT) parameter you pass by reference. A value of `0` means success. See [Error handling](#error-handling--troubleshooting) for the full list.
- **`printer` must be an exact printer name or ID** (case-insensitive). Passing an empty string, or a name that doesn't match any installed printer, is treated as a failure (see `ERR_PM_PRINTER_NOT_FOUND` on macOS / `ERR_OPEN_PRINTER` on Windows) — it does **not** fall back to a system default printer. Use 4D's own `Get current printer` command to get the current default printer's name, as shown in the examples below.
- **The optional `type` parameter means different things on each platform:**
  - **On Windows**, it's the `pDatatype` member of the spooler's `DOC_INFO_1` structure. Default when omitted: `"RAW"`.
  - **On macOS**, it's the MIME type passed to `PMPrinterPrintWithProvider`. Default when omitted: `"application/vnd.cups-raw"`.
  - **On macOS specifically**, the printer's driver must actually advertise support for whichever MIME type you end up using (default or explicit) — see the troubleshooting note below; this isn't checked on Windows.
- No minimum OS version is asserted here beyond "a supported 4D 17+ install": the Windows side uses the standard spooler API (`winspool.h`), and the macOS side uses Core Printing (Print Core/`ApplicationServices`), both long-stable, unversioned system frameworks — if your 4D version and OS combination is supported at all, these APIs are present.

---

## PRINT BLOB

### Syntax

```4d
PRINT BLOB(printer;data;error{;type})
```

Parameter | Type | Description
---|---|---
`printer` | TEXT | Exact name or ID of the target printer
`data` | BLOB | Raw bytes to send to the printer
`error` | LONGINT | Result code, written back by reference — `0` on success
`type` | TEXT | Optional. MIME type (macOS) / spooler datatype (Windows) of `data`. See platform defaults above
Result | — | This command has no function result; use the `error` parameter

### Description

The command opens the named printer, starts a single print job and a single page, writes the entire contents of `data` in one call, and closes out the page/job/printer handle — all before returning. There's no streaming or chunking: the whole BLOB is locked into memory and written in one `WritePrinter`/`PMPrinterPrintWithProvider` call, so very large BLOBs will hold that memory for the duration of the call.

If `data` is an empty BLOB (no handle), the command returns immediately without touching `error` at all — since `error` isn't reset to `0` by the command itself, make sure your calling code initializes it before the call if you want to distinguish "not attempted" from "succeeded."

### Example

From the plugin's own `README.md`:

```4d
$printerName:="ZEBRA GK420D"
C_BLOB($ZPL)

  // Command to be sent to the printer
$code:="^XA^FO10,10,^AO,30,20^FDFDTesting^FS^FO10,30^BY3^BCN,100,Y,N,N^FDTesting^FS^XZ"

CONVERT FROM TEXT($code;"us-ascii";$ZPL)

C_LONGINT($ERR)
$ERR:=0

PRINT BLOB($printerName;$ZPL;$ERR)

If($ERR#0)
  ALERT("Print failed with error: "+String($ERR))
End if
```

Printing to the current default printer instead of a named one, and sending a PDF directly on macOS:

```4d
C_BLOB($PDF)
  // ... load $PDF with PDF document bytes ...

C_LONGINT($ERR)
$ERR:=0

PRINT BLOB(Get current printer;$PDF;$ERR;"application/pdf")

If($ERR#0)
  ALERT("Print failed with error: "+String($ERR))
End if
```

---

## PRINT BLOB ARRAY

### Syntax

```4d
PRINT BLOB ARRAY(printer;data;error{;type})
```

Parameter | Type | Description
---|---|---
`printer` | TEXT | Exact name or ID of the target printer
`data` | ARRAY BLOB | One element per page/label to print, in array order
`error` | LONGINT | Result code, written back by reference — `0` on success
`type` | TEXT | Optional. MIME type (macOS) / spooler datatype (Windows) of every element in `data`. See platform defaults above
Result | — | This command has no function result; use the `error` parameter

### Description

Opens the printer and starts a single print job (not one job per element), then loops over `data` starting a new page for each element and writing that element's bytes before ending the page. The job is ended once, after every element has been processed. Every element in the array is sent with the **same** `type` — you can't mix datatypes within one call.

If a failure occurs partway through the array (e.g. one element's `WritePrinter` call fails), the command does **not** stop early — it continues through the remaining elements, and `error` reflects the **first** failure encountered across the whole batch, not necessarily the last or "worst" one. There's no per-element error reporting; if you need to know exactly which element failed, print elements one at a time with repeated [`PRINT BLOB`](#print-blob) calls instead.

If `data` isn't actually an array-of-BLOB variable, the command returns immediately without touching `error` — same caveat as [`PRINT BLOB`](#print-blob) about initializing `error` yourself beforehand.

### Example

Printing three copies of the same label, adapted from the plugin's own `README.md`:

```4d
$printerName:="ZEBRA GK420D"
C_BLOB($ZPL)

$code:="^XA^FO10,10,^AO,30,20^FDFDTesting^FS^FO10,30^BY3^BCN,100,Y,N,N^FDTesting^FS^XZ"
CONVERT FROM TEXT($code;"us-ascii";$ZPL)

ARRAY BLOB($ZPLs;3)
$ZPLs{1}:=$ZPL
$ZPLs{2}:=$ZPL
$ZPLs{3}:=$ZPL

C_LONGINT($ERR)
$ERR:=0

PRINT BLOB ARRAY($printerName;$ZPLs;$ERR)

If($ERR#0)
  ALERT("Print failed with error: "+String($ERR))
End if
```

Printing a variable number of different labels built in a loop:

```4d
C_LONGINT($i;$ERR)
ARRAY BLOB($labels;0)

For($i;1;10)
  APPEND TO ARRAY($labels;$labelBlobForRow($i))
End for

$ERR:=0
PRINT BLOB ARRAY(Get current printer;$labels;$ERR;"RAW")

If($ERR#0)
  ALERT("Print failed with error: "+String($ERR))
End if
```

---

## Error handling & troubleshooting

- **Always check `error` after every call — nothing is raised as a 4D error.** Both commands communicate failure exclusively through the `error` output parameter; a script that ignores it will never learn that a print silently failed.
- **`error` is only ever written to when the printer/BLOB check at the top of the command passes.** If you pass an empty BLOB (or, for the array command, a variable that isn't an array of BLOB), the command returns without writing `error` at all. Initialize `error` to `0` yourself before the call so you can tell "didn't run" apart from "ran and succeeded."
- **An unmatched or empty `printer` name is a reportable failure, not a fallback to the default printer.** On Windows this comes back as `ERR_OPEN_PRINTER` (`-1`); on macOS as `ERR_PM_PRINTER_NOT_FOUND` (`-18`). Use `Get current printer` if you want "whatever the user's default printer is," rather than leaving `printer` blank.
- **On macOS, an unsupported MIME type produces no error at all.** The command queries the printer driver's supported MIME types (`PMPrinterGetMimeTypes`) and only proceeds to print if your `type` (or the default `application/vnd.cups-raw`) is in that list. If it isn't — or if the driver query itself fails — nothing is printed and `error` is left untouched. This is a genuine gap in the current implementation: if you're getting no output and no error on macOS, suspect the MIME type first, and confirm the driver's supported types independently (e.g. via CUPS' own tooling) rather than assuming the plugin will tell you.
- **[`PRINT BLOB ARRAY`](#print-blob-array) only reports the first failure in the batch.** If elements 2 and 7 both fail to write, `error` reflects element 2's failure code; elements 3–10 are still attempted. Don't assume `error#0` means the whole batch failed — some elements may have printed successfully.
- **Very large BLOBs are held entirely in memory for the duration of the call.** There's no streaming; a multi-hundred-megabyte BLOB will be locked and copied into an `NSData`/`WritePrinter` buffer in one piece. Keep payloads sized to what your target printer's command language actually needs (labels/receipts are normally tiny; a raw PDF passthrough could be larger — plan accordingly).

### Error codes

Code | Constant | Meaning
---|---|---
`-1` | `ERR_OPEN_PRINTER` | *(Windows)* Couldn't open a handle to the named printer — usually means the name doesn't match an installed printer
`-2` | `ERR_START_DOC_PRINTER` | *(Windows)* `StartDocPrinter` failed
`-3` | `ERR_WRITE_PRINTER` | *(Windows)* `WritePrinter` failed — data not (fully) sent
`-4` | `ERR_END_DOC_PRINTER` | *(Windows)* `EndDocPrinter` failed
`-5` | `ERR_END_PAGE_PRINTER` | *(Windows)* `EndPagePrinter` failed
`-6` | `ERR_CLOSE_PRINTER` | *(Windows)* `ClosePrinter` failed
`-7` | `ERR_START_PAGE_PRINTER` | *(Windows)* `StartPagePrinter` failed — no data was written for that page
`-11` | `ERR_PM_CREATE_SESSION` | *(macOS)* Couldn't create a Print Manager session
`-12` | `ERR_PM_CREATE_PRINT_SETTINGS` | *(macOS)* Couldn't create print settings
`-13` | `ERR_PM_SET_CURRENT_PM_PRINTER` | *(macOS)* Couldn't set the session's current printer
`-14` | `ERR_PM_CREATE_PAGE_FORMAT` | *(macOS)* Couldn't create a page format
`-15` | `ERR_PM_SESSION_DEFAULT_PAGE_FORMAT` | *(macOS)* Couldn't get the session's default page format
`-16` | `ERR_PM_SESSION_DEFAULT_PRINT_SETTINGS` | *(macOS)* Couldn't get the session's default print settings
`-17` | `ERR_PM_PRINTER_PRINT_WITH_FILE` | *(macOS)* `PMPrinterPrintWithProvider` itself returned a non-zero status
`-18` | `ERR_PM_PRINTER_NOT_FOUND` | *(macOS)* No installed printer's name or ID matched `printer`

`-7` and `-18` reflect failure modes that previously returned no error at all (a `StartPagePrinter` failure on Windows, and an unmatched printer name on macOS) — if you're testing against a binary built before this fix, those two specific failures will still silently report success (`error=0`) even though nothing was printed.

---

## Quick reference

```4d
  // Single BLOB, named printer
C_TEXT($printer)
C_BLOB($data)
C_LONGINT($error)
$printer:="ZEBRA GK420D"
$error:=0
PRINT BLOB($printer;$data;$error)
If($error#0)
  ALERT("Print error: "+String($error))
End if
```

```4d
  // Array of BLOBs, current default printer, explicit type
ARRAY BLOB($data;0)
C_LONGINT($error)
$error:=0
PRINT BLOB ARRAY(Get current printer;$data;$error;"RAW")
If($error#0)
  ALERT("Print error: "+String($error))
End if
```
