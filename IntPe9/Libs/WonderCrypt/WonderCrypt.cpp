#include "WonderCrypt.h"

//Instance pointer
WonderCrypt* WonderCrypt::instancePointer = 0;

//Static WonderKing data
//unsigned char WonderCrypt::aesKey[] =		{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
unsigned char WonderCrypt::aesKey[] =		{0x04, 0x03, 0x02, 0x01, 0x08, 0x07, 0x06, 0x05, 0x03, 0x02, 0x01, 0x09, 0x07, 0x06, 0x05, 0x04};
unsigned char WonderCrypt::paddingTable[] = {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 0x0F, 0x1E, 0x2D, 0x3C, 0x4B, 0x5A, 0x69, 0x78};

/**
 * Access this singleton
 *
 * @return Database	The single instance for the database
 */
WonderCrypt* WonderCrypt::instance()
{
	//If there is no instancePointer yet, create it.
	if(!instancePointer)
		instancePointer = new WonderCrypt;

	//Return a solo instance of this class
	return instancePointer;
}

/**
 * Constructor
 *
 * Creates a new WonderCrypt instance and sets up Aes for WonderKing encryption/decryption
 */
WonderCrypt::WonderCrypt()
{
	//Create a Aes instance and set the correct key and buffer size
	aes = new Aes();
	aes->SetParameters(KEY_SIZE*8, BLOCK_SIZE*8);

	_isAuth = true;
}

/**
 * Destructor
 *
 * Destroys the Aes instance and sets the instance pointer to zero
 */
WonderCrypt::~WonderCrypt()
{
	delete aes;
	instancePointer = 0;
}

/**
 * Crypts the remaining bytes
 *
 * Crypts the remaining bytes that does not fit in the BLOCK_SIZE
 *
 * @param const unsigned char*	Bytes the bytes to crypt
 * @param unsigned char*		Buffer to put the crypted bytes in
 * @param int					Offset from where to start crypting the bytes
 * @param int					Ammount of bytes left to crypt
 */
void WonderCrypt::paddingXor(unsigned char *bytes, unsigned char* buffer, int offset, int leftNo)
{
	//xor every byte with the corresponding position in the paddingTable
	for(int i = 0; i < leftNo; i++)
		buffer[offset + i] = bytes[offset + i] ^ paddingTable[i];
}

/**
 * Decrypts the packets
 *
 * Decrypts the packet based on internal flag for auth or game
 *
 * @param const unsigned char*	Bytes the bytes to crypt
 * @param unsigned char*		Buffer to put the crypted bytes in
 * @param unsigned int			The size of the buffer
 */
void WonderCrypt::decrypt(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize)
{
	if(_isAuth)
		cryptAuth(bytes, buffer, bufferSize);
	else
		decryptGame(bytes, buffer, bufferSize);
}

/**
 * Encrypts the packets
 *
 * Encrypts the packet based on internal flag for auth or game
 *
 * @param const unsigned char*	Bytes the bytes to crypt
 * @param unsigned char*		Buffer to put the crypted bytes in
 * @param unsigned int			The size of the buffer
 */
void WonderCrypt::encrypt(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize)
{
	if(_isAuth)
		cryptAuth(bytes, buffer, bufferSize);
	else
		encryptGame(bytes, buffer, bufferSize);
}

/**
 * Crypts the authentication packets
 *
 * Crypts the authentication packets with simple xor
 *
 * @param const unsigned char*	Bytes the bytes to crypt
 * @param unsigned char*		Buffer to put the crypted bytes in
 * @param unsigned int			The size of the buffer
 */
void WonderCrypt::cryptAuth(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize)
{
	//Setup
	unsigned short byteSize = *(short*)&bytes[0];

	//First 8 bytes never are encrypted
	if(byteSize <= 8) return;

	//Just copy first 8 bytes
	*(int*)&buffer[0] = *(int*)&bytes[0];
	*(int*)&buffer[4] = *(int*)&bytes[4];

	//Copy rest and crypt it also
	for(int i = 0; i < byteSize-8; i++)
		buffer[i + 8] = bytes[i + 8] ^ (i ^ (3*(-2-i)));
}

/**
 * Decrypts the game packets
 *
 * Decrypts the game packets with AES and custom padding
 *
 * @param const unsigned char*	Bytes the bytes to crypt
 * @param unsigned char*		Buffer to put the crypted bytes in
 * @param unsigned int			The size of the buffer
 */
void WonderCrypt::decryptGame(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize)
{
	//Call encryptGame with false flag for decryption settings
	encryptGame(bytes, buffer, bufferSize, false);
}

/**
 * Flips bytes in little endian
 *
 * @param unsigned char*		Buffer the bytes to flip
 * @param int					BlockSize the lenght of the buffer block
 */
void WonderCrypt::flipBlock(unsigned char *buffer, int blockSize)
{
	unsigned char b0, b1;
	for(int i=0; i<blockSize; i+=4)
	{
		b0 = buffer[i+0];
		b1 = buffer[i+1];
		buffer[i+0] = buffer[i+3];
		buffer[i+1] = buffer[i+2];
		buffer[i+2] = b1;
		buffer[i+3] = b0;
	}
}

/**
 * Encrypts the game packets
 *
 * Encrypts the game packets with AES and custom padding
 *
 * @param const unsigned char*	Bytes the bytes to crypt
 * @param unsigned char*		Buffer to put the crypted bytes in
 * @param unsigned int			The size of the buffer
 * @param bool = true			If this is an encrypt or a decrypt call (default encrypt)
 */
void WonderCrypt::encryptGame(unsigned char *bytes, unsigned char *buffer, unsigned int bufferSize, bool encrypt)
{
	//Setup
	unsigned int padding = 0;
	unsigned int byteNo = 8;
	unsigned short byteSize = *(short*)&bytes[0];

	//Wrong buffer, to small buffer or an packet that does not contain encryption!
	if(buffer == 0 || byteSize > bufferSize || byteSize <= 8) return;

	//Set up the encryption or decryption
	(encrypt) ? aes->StartEncryption(aesKey) : aes->StartDecryption(aesKey);

	//Just copy first 8 bytes
	*(int*)&buffer[0] = *(int*)&bytes[0];
	*(int*)&buffer[4] = *(int*)&bytes[4];

	//Encrypt/Decrypt blocks from 16 bytes and put them in the buffer at the same position as where we got them from
	while(byteNo+BLOCK_SIZE < byteSize)
	{
		//Flip block
		flipBlock(&bytes[byteNo], BLOCK_SIZE);

		//Crypt block
		(encrypt) ? aes->EncryptBlock(&bytes[byteNo], &buffer[byteNo]) : aes->DecryptBlock(&bytes[byteNo], &buffer[byteNo]);

		//Flip block back (and fix up the bytes buffer)
		flipBlock(&buffer[byteNo], BLOCK_SIZE);
		flipBlock(&bytes[byteNo], BLOCK_SIZE);

		//Do next block
		byteNo += BLOCK_SIZE;
	}

	//Calculate padding
	padding = byteSize - byteNo;
	if(padding > 0)
	{
		//We got padding
		paddingXor(bytes, buffer, byteNo, padding);
	}
}

/**
 * Sets the state of wonderCrypt
 *
 * true for authentication server,
 * false for game server
 */
void WonderCrypt::setAuth(bool flag)
{
	_isAuth = flag;
}