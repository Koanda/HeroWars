#ifndef _UPX_H_
#define _UPX_H_

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <psapi.h>

#define JMP(frm, to) (int)(((int)to - (int)frm) - 5);

extern unsigned long returnStart, returnJump, returnEnd;
extern unsigned long iatStart, iatEnd;

/**
 * Upx loader library
 *
 * @author		Intline9 <Intline9@gmail.com>
 * @version		2010.0801
 */
class Upx
{
	public:
		Upx();
		bool initialize();
		unsigned long waitForUnpacked(int timeOut);
		unsigned long hookFunction(const char* libraryName, const char* functionName, unsigned long newFunction);
		unsigned long hookIatFunction(const char* moduleName, const char* functionName, unsigned long newFunction);
		unsigned long aob(const unsigned char AoB[], const char mask[], unsigned long aStart, unsigned long aMax);

	private:
		//Some custom functions
		static void storeIat();
		static void unpackedHook();

		//Static this
		static Upx *_this;

		//Data
		bool _isUnpacked;
};
#endif
