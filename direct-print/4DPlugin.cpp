/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : Direct Print
 #	author : miyako
 #	2016/08/29
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

void PluginMain(PA_long32 selector, PA_PluginParameters params)
{
	try
	{
		PA_long32 pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (PA_long32 pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
// --- Direct Print

		case 1 :
			PRINT_BLOB(pResult, pParams);
			break;

		case 2 :
			PRINT_BLOB_ARRAY(pResult, pParams);
			break;

	}
}

// --------------------------------- Direct Print ---------------------------------


void PRINT_BLOB(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;
	C_LONGINT Param3;
	C_TEXT Param4;
	
	PA_Handle h = *(PA_Handle *)(pParams[1]);
	if(h)
	{
		Param1.fromParamAtIndex(pParams, 1);
		Param4.fromParamAtIndex(pParams, 4);
		
#if VERSIONWIN
		
		LPTSTR printerName = Param1.getUTF16Length() ? (LPTSTR)Param1.getUTF16StringPtr() : NULL;
		HANDLE printer = NULL;
		
		BOOL success = OpenPrinter(printerName,
															 &printer,
															 NULL);
		if(success)
		{
			DOC_INFO_1 docInfo;
			docInfo.pDocName = L"DOCUMENT";
			docInfo.pOutputFile = NULL;
			if(!Param4.getUTF16Length())
			{
				docInfo.pDatatype = L"RAW";
			}else{
				docInfo.pDatatype = (LPTSTR)Param4.getUTF16StringPtr();
			}
			
			DWORD printJob = StartDocPrinter(&printer, (DWORD)1, (LPBYTE)&docInfo);
			
			if(printJob)
			{				
					LPVOID buf = (LPVOID)PA_LockHandle(h);
					DWORD len = PA_GetHandleSize(h);
					DWORD written = 0;
					success = WritePrinter(&printer,
																 buf,
																 len,
																 &written);
					
					if(!success)
					{
						Param3.setIntValue(ERR_WRITE_PRINTER);
					}
					
					PA_UnlockHandle(h);
				
				success = EndDocPrinter(&printer);
				
				if(!success)
				{
					Param3.setIntValue(ERR_END_DOC_PRINTER);
				}
				
			}else{
				Param3.setIntValue(ERR_START_DOC_PRINTER);
			}
			
		}else{
			Param3.setIntValue(ERR_OPEN_PRINTER);
		}
#else
		
		NSString *mimeType = @"application/vnd.cups-raw";
		
		if(Param4.getUTF16Length())
		{
			mimeType = Param4.copyUTF16String();
		}
		
		NSString *printerName = Param1.copyUTF16String();
		CFArrayRef printers;
		
		PMServerCreatePrinterList(kPMServerLocal, &printers);
		
		if(printers)
		{
			for(NSUInteger i = 0; i < CFArrayGetCount(printers); ++i)
			{
				PMPrinter _printer = (PMPrinter)CFArrayGetValueAtIndex(printers, i);
				NSString *_printerName = (NSString *)PMPrinterGetName(_printer);
				NSString *_printerId = (NSString *)PMPrinterGetID(_printer);
				
				if(([_printerName localizedCaseInsensitiveCompare:printerName] == NSOrderedSame)
				||([_printerId localizedCaseInsensitiveCompare:printerName] == NSOrderedSame))
				{
					PMPrintSession session;
					OSStatus status = PMCreateSession(&session);
					if(!status)
					{
						PMPrintSettings settings;
						status = PMCreatePrintSettings(&settings);
						if(!status)
						{
							PMPrinter printer = _printer;
							status = PMSessionSetCurrentPMPrinter(session, printer);
							if(!status)
							{
								PMPageFormat format;
								status = PMCreatePageFormat(&format);
								if(!status)
								{
									status = PMSessionDefaultPageFormat(session, format);
									if(!status)
									{
										status = PMSessionDefaultPrintSettings(session, settings);
										if(!status)
										{
											CFArrayRef mimeTypes;
											status = PMPrinterGetMimeTypes(printer, settings, &mimeTypes);
											if (status == noErr && mimeTypes)
											{
												if (CFArrayContainsValue(mimeTypes, CFRangeMake(0, CFArrayGetCount(mimeTypes)), mimeType))
												{
													char* buf = PA_LockHandle(h);
													PA_long32 len = PA_GetHandleSize(h);
													
													NSData *data = [[NSData alloc]initWithBytes:(const void *)buf length:len];
													
//													NSString *temporaryFolder = NSTemporaryDirectory();
									
//#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1080
//													NSString *uuid = [[[NSUUID UUID]UUIDString]stringByReplacingOccurrencesOfString:@"-" withString:@""];
//#else
//													CFUUIDRef _uuid = CFUUIDCreate(kCFAllocatorDefault);
//													NSString *uuid = [(NSString *)CFUUIDCreateString(kCFAllocatorDefault, _uuid) stringByReplacingOccurrencesOfString:@"-" withString:@""];
//													CFRelease(_uuid);
//#endif
//													NSString *path = [[temporaryFolder stringByAppendingPathComponent:uuid]stringByAppendingPathExtension:@"zpl"];
													
//													if([data writeToFile:path atomically:YES])
//													{
//														NSURL *url = [[NSURL alloc]initFileURLWithPath:path isDirectory:NO];
//														if(url)
//														{
													
													CGDataProviderRef provider = CGDataProviderCreateWithCFData((CFDataRef)data);
													status = PMPrinterPrintWithProvider(printer, settings, format, (CFStringRef)mimeType, provider);
													CGDataProviderRelease(provider);
													
//															status = PMPrinterPrintWithFile(printer, settings, format, (CFStringRef)mimeType, (CFURLRef)url);
//															[url release];
													if(status)
													{
														Param3.setIntValue(ERR_PM_PRINTER_PRINT_WITH_FILE);
													}
//														}
//														[[NSFileManager defaultManager]removeItemAtPath:path error:NULL];
//													}
													[data release];
												}
											}
										}else{
											Param3.setIntValue(ERR_PM_SESSION_DEFAULT_PRINT_SETTINGS);
										}
									}else{
										Param3.setIntValue(ERR_PM_SESSION_DEFAULT_PAGE_FORMAT);
									}
									PMRelease(format);
								}else{
									Param3.setIntValue(ERR_PM_CREATE_PAGE_FORMAT);
								}
							}else{
								Param3.setIntValue(ERR_PM_SET_CURRENT_PM_PRINTER);
							}
							PMRelease(settings);
						}else{
							Param3.setIntValue(ERR_PM_CREATE_PRINT_SETTINGS);
						}
						PMRelease(session);
					}else{
						Param3.setIntValue(ERR_PM_CREATE_SESSION);
					}
					break;
				}
			}
			[printerName release];
			CFRelease(printers);
		}
		
		if(Param4.getUTF16Length())
		{
			[mimeType release];
		}
#endif
	}

	Param3.toParamAtIndex(pParams, 3);
}

void PRINT_BLOB_ARRAY(sLONG_PTR *pResult, PackagePtr pParams)
{
#ifndef PLUGIN_API_V11
	C_TEXT Param1;
	C_LONGINT Param3;
	C_TEXT Param4;
	
	PA_Variable arr = *((PA_Variable*) pParams[1]);
	
	if(&arr)
	{
		if(PA_IsArrayVariable(&arr))
		{
			if(arr.fType == eVK_ArrayBlob)
			{
				Param1.fromParamAtIndex(pParams, 1);
				Param4.fromParamAtIndex(pParams, 4);
				
#if VERSIONWIN
				
				LPTSTR printerName = Param1.getUTF16Length() ? (LPTSTR)Param1.getUTF16StringPtr() : NULL;
				HANDLE printer = NULL;
				
				BOOL success = OpenPrinter(printerName,
																	 &printer,
																	 NULL);
				if(success)
				{
					DOC_INFO_1 docInfo;
					docInfo.pDocName = L"DOCUMENT";
					docInfo.pOutputFile = NULL;
					
					if(!Param4.getUTF16Length())
					{
						docInfo.pDatatype = L"RAW";
					}else{
						docInfo.pDatatype = (LPTSTR)Param4.getUTF16StringPtr();
					}
					
					DWORD printJob = StartDocPrinter(&printer, (DWORD)1, (LPBYTE)&docInfo);
					
					if(printJob)
					{
						PA_long32 size = PA_GetArrayNbElements(arr);
						for(PA_long32 i = 1; i <= size; ++i)
						{
							PA_Blob blob = PA_GetBlobInArray(arr, i);
							LPVOID buf = (LPVOID)PA_LockHandle(blob.fHandle);
							
							DWORD len = blob.fSize;
							DWORD written = 0;
							success = WritePrinter(&printer,
																		 buf,
																		 len,
																		 &written);
							
							if(!success)
							{
								Param3.setIntValue(ERR_WRITE_PRINTER);
							}
							
							PA_UnlockHandle(blob.fHandle);
						}
						
						success = EndDocPrinter(&printer);
						
						if(!success)
						{
							Param3.setIntValue(ERR_END_DOC_PRINTER);
						}
						
					}else{
						Param3.setIntValue(ERR_START_DOC_PRINTER);
					}
					
				}else{
					Param3.setIntValue(ERR_OPEN_PRINTER);
				}
#else
				
				NSString *mimeType = @"application/vnd.cups-raw";
				
				if(Param4.getUTF16Length())
				{
					mimeType = Param4.copyUTF16String();
				}
				
				NSString *printerName = Param1.copyUTF16String();
				CFArrayRef printers;
				
				PMServerCreatePrinterList(kPMServerLocal, &printers);
				
				if(printers)
				{
					for(NSUInteger i = 0; i < CFArrayGetCount(printers); ++i)
					{
						PMPrinter _printer = (PMPrinter)CFArrayGetValueAtIndex(printers, i);
						NSString *_printerName = (NSString *)PMPrinterGetName(_printer);
						NSString *_printerId = (NSString *)PMPrinterGetID(_printer);
						
						if(([_printerName localizedCaseInsensitiveCompare:printerName] == NSOrderedSame)
							 ||([_printerId localizedCaseInsensitiveCompare:printerName] == NSOrderedSame))
						{
							PMPrintSession session;
							OSStatus status = PMCreateSession(&session);
							if(!status)
							{
								PMPrintSettings settings;
								status = PMCreatePrintSettings(&settings);
								if(!status)
								{
									PMPrinter printer = _printer;
									status = PMSessionSetCurrentPMPrinter(session, printer);
									if(!status)
									{
										PMPageFormat format;
										status = PMCreatePageFormat(&format);
										if(!status)
										{
											status = PMSessionDefaultPageFormat(session, format);
											if(!status)
											{
												status = PMSessionDefaultPrintSettings(session, settings);
												if(!status)
												{
													CFArrayRef mimeTypes;
													status = PMPrinterGetMimeTypes(printer, settings, &mimeTypes);
													if (status == noErr && mimeTypes)
													{
														if (CFArrayContainsValue(mimeTypes, CFRangeMake(0, CFArrayGetCount(mimeTypes)), mimeType))
														{
															
															PA_long32 size = PA_GetArrayNbElements(arr);
															for(PA_long32 i = 1; i <= size; ++i)
															{
																PA_Blob blob = PA_GetBlobInArray(arr, i);
																char* buf = PA_LockHandle(blob.fHandle);
																PA_long32 len = blob.fSize;
																
																NSData *data = [[NSData alloc]initWithBytes:(const void *)buf length:len];
																
//																NSString *temporaryFolder = NSTemporaryDirectory();
																
//#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1080
//																NSString *uuid = [[[NSUUID UUID]UUIDString]stringByReplacingOccurrencesOfString:@"-" withString:@""];
//#else
//																CFUUIDRef _uuid = CFUUIDCreate(kCFAllocatorDefault);
//																NSString *uuid = [(NSString *)CFUUIDCreateString(kCFAllocatorDefault, _uuid) stringByReplacingOccurrencesOfString:@"-" withString:@""];
//																CFRelease(_uuid)
//#endif
//																NSString *path = [[temporaryFolder stringByAppendingPathComponent:uuid]stringByAppendingPathExtension:@"zpl"];
																
//																if([data writeToFile:path atomically:YES])
//																{
//																	NSURL *url = [[NSURL alloc]initFileURLWithPath:path isDirectory:NO];
//																	if(url)
//																	{
																CGDataProviderRef provider = CGDataProviderCreateWithCFData((CFDataRef)data);
																status = PMPrinterPrintWithProvider(printer, settings, format, (CFStringRef)mimeType, provider);
																CGDataProviderRelease(provider);
																
//																status = PMPrinterPrintWithFile(printer, settings, format, CFSTR("application/vnd.cups-raw"), (CFURLRef)url);
//																		[url release];
																		if(status)
																		{
																			Param3.setIntValue(ERR_PM_PRINTER_PRINT_WITH_FILE);
																		}
//																	}
//																	[[NSFileManager defaultManager]removeItemAtPath:path error:NULL];
//																}
																[data release];

																PA_UnlockHandle(blob.fHandle);
															}
														}
													}
												}else{
													Param3.setIntValue(ERR_PM_SESSION_DEFAULT_PRINT_SETTINGS);
												}
											}else{
												Param3.setIntValue(ERR_PM_SESSION_DEFAULT_PAGE_FORMAT);
											}
											PMRelease(format);
										}else{
											Param3.setIntValue(ERR_PM_CREATE_PAGE_FORMAT);
										}
									}else{
										Param3.setIntValue(ERR_PM_SET_CURRENT_PM_PRINTER);
									}
									PMRelease(settings);
								}else{
									Param3.setIntValue(ERR_PM_CREATE_PRINT_SETTINGS);
								}
								PMRelease(session);
							}else{
								Param3.setIntValue(ERR_PM_CREATE_SESSION);
							}
							break;
						}
					}
					[printerName release];
					CFRelease(printers);
				}
				
				if(Param4.getUTF16Length())
				{
					[mimeType release];
				}
#endif
			}
		}
	}

	Param3.toParamAtIndex(pParams, 3);
#endif
}

