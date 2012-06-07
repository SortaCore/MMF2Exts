// CkCrypt2.h: interface for the CkCrypt2 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKCRYPT2_H
#define _CKCRYPT2_H



#ifdef WIN32
#ifndef WINCE
class CkCSP;
#endif
#endif

class CkCert;

#include "CkString.h"
class CkByteData;
#include "CkObject.h"
class CkPrivateKey;
class CkCrypt2Progress;

#ifndef __sun__
#pragma pack (push, 8)
#endif
 

// CLASS: CkCrypt2
class CkCrypt2  : public CkObject
{
    private:
	CkCrypt2Progress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCrypt2(const CkCrypt2 &);
	CkCrypt2 &operator=(const CkCrypt2 &);
	CkCrypt2(void *impl);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkCrypt2();
	virtual ~CkCrypt2();

	// May be called when finished with the object to free/dispose of any
	// internal resources held by the object. 
	void dispose(void);

	void put_EventCallbackObject(CkCrypt2Progress *progress);

	void SetSecretKey(const char *keyData, int numBytes);
	void SetIV(const char *ivData, int numBytes);

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Used for symmetric encryption algorithms.
	bool get_FirstChunk(void) const;
	void put_FirstChunk(bool b);
	bool get_LastChunk(void) const;
	void put_LastChunk(bool b);

	// Get the block size of the selected encryption algorithm.
	int get_BlockSize(void) const;

	bool CkEncryptFile(const char *inFile, const char *outFile);
	bool CkDecryptFile(const char *inFile, const char *outFile);

	bool SetSigningCert2(const CkCert &cert, CkPrivateKey &key);
	bool SetDecryptCert2(const CkCert &cert, CkPrivateKey &key);

#if defined(WIN32) && !defined(WINCE) && !defined(__MINGW32__)
	bool SetCSP(const CkCSP *csp);
	bool SetCSP(const CkCSP &csp) { return this->SetCSP(&csp); }
#endif
	CkCert *GetLastCert(void);
	void SetEncryptCert(const CkCert *cert);
	bool SetSigningCert(const CkCert *cert);
	void SetVerifyCert(const CkCert *cert);
	void SetEncryptCert(const CkCert &cert) { this->SetEncryptCert(&cert); }
	bool SetSigningCert(const CkCert &cert) { return this->SetSigningCert(&cert); }
	void SetVerifyCert(const CkCert &cert) { this->SetVerifyCert(&cert); }

	bool CreateDetachedSignature(const char *inFile, const char *sigFile);
	bool VerifyDetachedSignature(const char *inFile, const char *sigFile);

	bool VerifyStringENC(const char *str, const char *encodedSig);
	bool VerifyString(const char *str, const CkByteData &sigData);
	bool VerifyBytesENC(const CkByteData &bData, const char *encodedSig);
	bool VerifyBytes(const CkByteData &bData1, const CkByteData &sigData);

	bool SignStringENC(const char *str, CkString &outStr);
	bool SignBytesENC(const CkByteData &bData, CkString &outStr);
	bool SignString(const char *str, CkByteData &outData);
	bool SignBytes(const CkByteData &bData, CkByteData &outData);

	bool OpaqueVerifyStringENC(const char *p7s, CkString &outOriginal);
	bool OpaqueVerifyString(const CkByteData &p7s, CkString &outOriginal);
	bool OpaqueVerifyBytesENC(const char *p7s, CkByteData &outOriginal);
	bool OpaqueVerifyBytes(const CkByteData &p7s, CkByteData &outOriginal);

	bool OpaqueSignStringENC(const char *str, CkString &outStr);
	bool OpaqueSignBytesENC(const CkByteData &bData, CkString &outStr);
	bool OpaqueSignString(const char *str, CkByteData &outData);
	bool OpaqueSignBytes(const CkByteData &bData, CkByteData &outData);

	bool CreateP7S(const char *inFilename, const char *p7sFilename);
	bool VerifyP7S(const char *inFilename, const char *p7sFilename);

	bool CreateP7M(const char *inFilename, const char *p7mFilename);
	bool VerifyP7M(const char *p7mFilename, const char *outFilename);

	const char *opaqueSignStringENC(const char *str);
	const char *opaqueSignBytesENC(const CkByteData &bData);

	const char *opaqueVerifyStringENC(const char *p7s);
	const char *opaqueVerifyString(const CkByteData &p7s);

	const char *signStringENC(const char *str);
	const char *signBytesENC(const CkByteData &bData);

	bool InflateStringENC(const char *str, CkString &outStr);
	bool InflateString(const CkByteData &bData, CkString &outStr);
	bool InflateBytesENC(const char *str, CkByteData &outData);
	bool InflateBytes(const CkByteData &bData, CkByteData &outData);

	bool CompressStringENC(const char *str, CkString &outStr);
	bool CompressBytesENC(const CkByteData &bData, CkString &outStr);
	bool CompressString(const char *str, CkByteData &outData);
	bool CompressBytes(const CkByteData &bData, CkByteData &outData);

	void get_CompressionAlgorithm(CkString &outStr);
	void put_CompressionAlgorithm(const char *alg);

	bool DecryptStringENC(const char *str, CkString &outStr);
	bool DecryptString(const CkByteData &bData, CkString &outStr);
	bool DecryptBytesENC(const char *str, CkByteData &outData);
	bool DecryptBytes(const CkByteData &bData, CkByteData &outData);

	bool EncryptStringENC(const char *str, CkString &outStr);
	bool EncryptBytesENC(const CkByteData &bData, CkString &outStr);
	bool EncryptString(const char *str, CkByteData &outData);
	bool EncryptBytes(const CkByteData &bData, CkByteData &outData);

	bool HashBytesENC(const CkByteData &bData, CkString &outStr);
	bool HashStringENC(const char *str, CkString &outStr);
	bool HashBytes(const CkByteData &bData, CkByteData &outData);
	bool HashString(const char *str, CkByteData &outData);

	bool HashFileENC(const char *filename, CkString &outStr);
	bool HashFile(const char *filename, CkByteData &outData);
	const char *hashFileENC(const char *filename);

	void ByteSwap4321(CkByteData &inOut);

	void get_EncodingMode(CkString &outStr);
	void put_EncodingMode(const char *str);
	void get_CryptAlgorithm(CkString &outStr);
	void put_CryptAlgorithm(const char *str);
	void get_HashAlgorithm(CkString &outStr);
	void put_HashAlgorithm(const char *str);
	void get_Charset(CkString &outStr);
	void put_Charset(const char *str);

	void GenerateSecretKey(const char *password, CkByteData &outData);
	void SetSecretKeyViaPassword(const char *password);

	const char *genEncodedSecretKey(const char *password, const char *encoding);

	void get_CipherMode(CkString &outStr);
	void put_CipherMode(const char *newVal);
	// 0 = Each padding byte is the pad count (16 extra added if size is already a multiple of 16)
	// 1 = Random bytes except the last is the pad count (16 extra added if size is already multiple of 16)
	// 2 = Pad with random data. (If already a multiple of 16, no padding is added).
	// 3 = Pad with NULLs. (If already a multiple of 16, no padding is added).
	// 4 = Pad with SPACE chars(0x20). (If already a multiple of 16, no padding is added).
	long get_PaddingScheme(void);
	void put_PaddingScheme(long newVal);
	long get_KeyLength(void);
	void put_KeyLength(long newVal);
	void get_IV(CkByteData &bData);
	void put_IV(const CkByteData &bData);
	void get_SecretKey(CkByteData &bData);
	void put_SecretKey(const CkByteData &bData);
	void get_Version(CkString &outStr);

	// Should be between 8 and 1024 bits, inclusive.
	long get_Rc2EffectiveKeyLength(void);
	void put_Rc2EffectiveKeyLength(long newVal);

	// must be 3, 4, or 5.
	long get_HavalRounds(void);
	void put_HavalRounds(long newVal);

	bool IsUnlocked(void);
	bool UnlockComponent(const char *unlockCode);

	// Encoding can be base64, hex, URL
	void SetEncodedKey(const char *keyStr, const char *encoding);
	void SetEncodedIV(const char *ivStr, const char *encoding);
	bool GetEncodedKey(const char *encoding, CkString &outKey);
	bool GetEncodedIV(const char *encoding, CkString &outIV);

	void SetSecretKey(const unsigned char *keyData, int numBytes);
	void SetIV(const unsigned char *ivData, int numBytes);

	bool Encode(const CkByteData &bData, const char *encoding, CkString &outStr);
	void Decode(const char *str, const char *encoding, CkByteData &outData);

	void RandomizeIV(void);

	void SetHmacKeyBytes(const CkByteData &keyBytes);
	void SetHmacKeyString(const char *key);
	void SetHmacKeyEncoded(const char *key, const char *encoding);

	// HMAC using the currently set hash alg and HMAC key.
	void HmacBytes(const CkByteData &inBytes, CkByteData &outHmac);
	void HmacString(const char *inText, CkByteData &outHmac);
	bool HmacBytesENC(const CkByteData &inBytes, CkString &outEncodedHmac);
	bool HmacStringENC(const char *inText, CkString &outEncodedHmac);
	const char *hmacStringENC(const char *inText);
	const char *hmacBytesENC(const CkByteData &inBytes);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	const char *encodeBytes2(const unsigned char *byteData, int numBytes, const char *encoding);
	const char *encodeBytes(const char *byteData, int numBytes, const char *encoding);

	const char *inflateStringENC(const char *str);
	const char *inflateString(const CkByteData &bData);

	const char *compressStringENC(const char *str);
	const char *compressBytesENC(const CkByteData &bData);

	const char *encryptStringENC(const char *str);
	const char *encryptBytesENC(const CkByteData &bData);

	const char *decryptString(const CkByteData &bData);
	const char *decryptStringENC(const char *str);

	const char *hashStringENC(const char *str);
	const char *hashBytesENC(const CkByteData &bData);

	const char *getEncodedKey(const char *encoding);
	const char *getEncodedIV(const char *encoding);

	const char *encode(const CkByteData &bData, const char *encoding); //

	const char *encodingMode(void);
	const char *compressionAlgorithm(void); //
	const char *cryptAlgorithm(void);
	const char *hashAlgorithm(void);
	const char *charset(void);
	const char *cipherMode(void);
	const char *version(void); //

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	// INCLUDECERTCHAIN_BEGIN
	bool get_IncludeCertChain(void);
	void put_IncludeCertChain(bool newVal);
	// INCLUDECERTCHAIN_END
	// GENENCODEDSECRETKEY_BEGIN
	bool GenEncodedSecretKey(const char *password, const char *encoding, CkString &outStr);
	// GENENCODEDSECRETKEY_END
	// READFILE_BEGIN
	bool ReadFile(const char *filename, CkByteData &outBytes);
	// READFILE_END
	// STRINGTOBYTES_BEGIN
	bool StringToBytes(const char *inStr, const char *charset, CkByteData &outBytes);
	// STRINGTOBYTES_END
	// BYTESTOSTRING_BEGIN
	bool BytesToString(CkByteData &inData, const char *charset, CkString &outStr);
	const char *bytesToString(CkByteData &inData, const char *charset);
	// BYTESTOSTRING_END
	// TRIMENDINGWITH_BEGIN
	bool TrimEndingWith(const char *inStr, const char *ending, CkString &outStr);
	const char *trimEndingWith(const char *inStr, const char *ending);
	// TRIMENDINGWITH_END
	// WRITEFILE_BEGIN
	bool WriteFile(const char *filename, CkByteData &fileData);
	// WRITEFILE_END
	// SETDECRYPTCERT_BEGIN
	bool SetDecryptCert(CkCert &cert);
	// SETDECRYPTCERT_END
	// ENCRYPTENCODED_BEGIN
	bool EncryptEncoded(const char *str, CkString &outStr);
	const char *encryptEncoded(const char *str);
	// ENCRYPTENCODED_END
	// DECRYPTENCODED_BEGIN
	bool DecryptEncoded(const char *str, CkString &outStr);
	const char *decryptEncoded(const char *str);
	// DECRYPTENCODED_END
	// REENCODE_BEGIN
	bool ReEncode(const char *data, const char *fromEncoding, const char *toEncoding, CkString &outStr);
	const char *reEncode(const char *data, const char *fromEncoding, const char *toEncoding);
	// REENCODE_END
	// RANDOMIZEKEY_BEGIN
	void RandomizeKey(void);
	// RANDOMIZEKEY_END
	// ADDENCRYPTCERT_BEGIN
	void AddEncryptCert(CkCert &cert);
	// ADDENCRYPTCERT_END
	// CLEARENCRYPTCERTS_BEGIN
	void ClearEncryptCerts(void);
	// CLEARENCRYPTCERTS_END
	// GENRANDOMBYTESENC_BEGIN
	bool GenRandomBytesENC(int numBytes, CkString &outStr);
	const char *genRandomBytesENC(int numBytes);
	// GENRANDOMBYTESENC_END
	// PBKDF1_BEGIN
	bool Pbkdf1(const char *password, const char *charset, const char *hashAlg, const char *salt, int iterationCount, int outputKeyBitLen, const char *encoding, CkString &outStr);
	const char *pbkdf1(const char *password, const char *charset, const char *hashAlg, const char *salt, int iterationCount, int outputKeyBitLen, const char *encoding);
	// PBKDF1_END
	// PBKDF2_BEGIN
	bool Pbkdf2(const char *password, const char *charset, const char *hashAlg, const char *salt, int iterationCount, int outputKeyBitLen, const char *encoding, CkString &outStr);
	const char *pbkdf2(const char *password, const char *charset, const char *hashAlg, const char *salt, int iterationCount, int outputKeyBitLen, const char *encoding);
	// PBKDF2_END
	// PBESPASSWORD_BEGIN
	void get_PbesPassword(CkString &str);
	const char *pbesPassword(void);
	void put_PbesPassword(const char *newVal);
	// PBESPASSWORD_END
	// SALT_BEGIN
	void get_Salt(CkByteData &data);
	void put_Salt(CkByteData &data);
	// SALT_END
	// ITERATIONCOUNT_BEGIN
	int get_IterationCount(void);
	void put_IterationCount(int newVal);
	// ITERATIONCOUNT_END
	// SETENCODEDSALT_BEGIN
	void SetEncodedSalt(const char *saltStr, const char *encoding);
	// SETENCODEDSALT_END
	// GETENCODEDSALT_BEGIN
	bool GetEncodedSalt(const char *encoding, CkString &outStr);
	const char *getEncodedSalt(const char *encoding);
	// GETENCODEDSALT_END
	// PBESALGORITHM_BEGIN
	void get_PbesAlgorithm(CkString &str);
	const char *pbesAlgorithm(void);
	void put_PbesAlgorithm(const char *newVal);
	// PBESALGORITHM_END
	// HASHBEGINBYTES_BEGIN
	bool HashBeginBytes(CkByteData &data);
	// HASHBEGINBYTES_END
	// HASHBEGINSTRING_BEGIN
	bool HashBeginString(const char *strData);
	// HASHBEGINSTRING_END
	// HASHMORESTRING_BEGIN
	bool HashMoreString(const char *strData);
	// HASHMORESTRING_END
	// HASHMOREBYTES_BEGIN
	bool HashMoreBytes(CkByteData &data);
	// HASHMOREBYTES_END
	// HASHFINAL_BEGIN
	bool HashFinal(CkByteData &outBytes);
	// HASHFINAL_END
	// HASHFINALENC_BEGIN
	bool HashFinalENC(CkString &outStr);
	const char *hashFinalENC(void);
	// HASHFINALENC_END
	// MYSQLAESENCRYPT_BEGIN
	bool MySqlAesEncrypt(const char *strData, const char *strKey, CkString &outStr);
	const char *mySqlAesEncrypt(const char *strData, const char *strKey);
	// MYSQLAESENCRYPT_END
	// MYSQLAESDECRYPT_BEGIN
	bool MySqlAesDecrypt(const char *strEncrypted, const char *strKey, CkString &outStr);
	const char *mySqlAesDecrypt(const char *strEncrypted, const char *strKey);
	// MYSQLAESDECRYPT_END
	// ENCODESTRING_BEGIN
	bool EncodeString(const char *inStr, const char *charset, const char *encoding, CkString &outStr);
	const char *encodeString(const char *inStr, const char *charset, const char *encoding);
	// ENCODESTRING_END
	// DECODESTRING_BEGIN
	bool DecodeString(const char *inStr, const char *charset, const char *encoding, CkString &outStr);
	const char *decodeString(const char *inStr, const char *charset, const char *encoding);
	// DECODESTRING_END
	// UUMODE_BEGIN
	void get_UuMode(CkString &str);
	const char *uuMode(void);
	void put_UuMode(const char *newVal);
	// UUMODE_END
	// UUFILENAME_BEGIN
	void get_UuFilename(CkString &str);
	const char *uuFilename(void);
	void put_UuFilename(const char *newVal);
	// UUFILENAME_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END
	// ADDPFXSOURCEDATA_BEGIN
	bool AddPfxSourceData(CkByteData &pfxData, const char *password);
	// ADDPFXSOURCEDATA_END
	// ADDPFXSOURCEFILE_BEGIN
	bool AddPfxSourceFile(const char *pfxFilePath, const char *password);
	// ADDPFXSOURCEFILE_END
	// PKCS7CRYPTALG_BEGIN
	void get_Pkcs7CryptAlg(CkString &str);
	const char *pkcs7CryptAlg(void);
	void put_Pkcs7CryptAlg(const char *newVal);
	// PKCS7CRYPTALG_END
	// NUMSIGNERCERTS_BEGIN
	int get_NumSignerCerts(void);
	// NUMSIGNERCERTS_END
	// GETSIGNERCERT_BEGIN
	CkCert *GetSignerCert(int index);
	// GETSIGNERCERT_END
	// HASSIGNATURESIGNINGTIME_BEGIN
	bool HasSignatureSigningTime(int index);
	// HASSIGNATURESIGNINGTIME_END
	// GETSIGNATURESIGNINGTIME_BEGIN
	bool GetSignatureSigningTime(int index, SYSTEMTIME &outSysTime);
	// GETSIGNATURESIGNINGTIME_END
	// CADESENABLED_BEGIN
	bool get_CadesEnabled(void);
	void put_CadesEnabled(bool newVal);
	// CADESENABLED_END
	// CADESSIGPOLICYID_BEGIN
	void get_CadesSigPolicyId(CkString &str);
	const char *cadesSigPolicyId(void);
	void put_CadesSigPolicyId(const char *newVal);
	// CADESSIGPOLICYID_END
	// CADESSIGPOLICYURI_BEGIN
	void get_CadesSigPolicyUri(CkString &str);
	const char *cadesSigPolicyUri(void);
	void put_CadesSigPolicyUri(const char *newVal);
	// CADESSIGPOLICYURI_END
	// CADESSIGPOLICYHASH_BEGIN
	void get_CadesSigPolicyHash(CkString &str);
	const char *cadesSigPolicyHash(void);
	void put_CadesSigPolicyHash(const char *newVal);
	// CADESSIGPOLICYHASH_END


	// CRYPT2_INSERT_POINT

	// END PUBLIC INTERFACE

	// The following method(s) should not be called by an application.
	// They for internal use only.
	void setLastErrorProgrammingLanguage(int v);


};

#ifndef __sun__
#pragma pack (pop)
#endif



#endif


