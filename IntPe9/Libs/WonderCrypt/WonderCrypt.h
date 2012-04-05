#ifndef _WONDER_CRYPT_H
#define _WONDER_CRYPT_H

#include "Aes.h"

#define KEY_SIZE 16
#define BLOCK_SIZE 16
#ifndef DWORD
	typedef unsigned long DWORD;
#endif

/**
 * WonderKing crypt library
 *
 * @author		Intline9 <Intline9@gmail.com>
 * @version		2010.0721
 */
class WonderCrypt
{
	public:
		//Destructor
		~WonderCrypt();

		//Instance
		static WonderCrypt *instance();

		//Crypt functions
		void paddingXor(unsigned char *bytes, unsigned char* buffer, int offset, int leftNo);
		void cryptAuth(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize);
		void decryptGame(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize);
		void encryptGame(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize, bool encrypt = true);

		void decrypt(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize);
		void encrypt(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize);

		//Property
		void setAuth(bool flag);

	private:
		//Make everything static so people have to access this through the instance
		WonderCrypt();
		WonderCrypt(WonderCrypt const&);
		WonderCrypt& operator=(WonderCrypt const&);

		//Helpers
		void flipBlock(unsigned char *buffer, int blockSize);

		//Instance
		static WonderCrypt *instancePointer;

		//Vars
		Aes *aes;
		bool _isAuth;

		//Data
		static unsigned char aesKey[KEY_SIZE];
		static unsigned char paddingTable[BLOCK_SIZE];

	protected:

};

#endif